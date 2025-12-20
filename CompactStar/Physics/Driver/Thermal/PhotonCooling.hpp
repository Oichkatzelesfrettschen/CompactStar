// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025
 * Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */

/**
 * @file PhotonCooling.hpp
 * @brief Driver that contributes surface photon cooling to the thermal evolution RHS.
 *
 * @ingroup PhysicsDriver
 *
 * ## Physical intent
 * This driver adds a **photon luminosity loss** term to the redshifted thermal
 * degree of freedom (typically \(T_\infty\)), producing a contribution
 *
 * \f[
 *   \frac{d\ln (T_\infty / T_{\rm ref})}{dt} = - \frac{L_{\gamma,\infty}}{C_{\rm eff}\,T_\infty}
 * \f]
 *
 * where \(T_\infty\) is the redshifted isothermal-core temperature in Kelvin.
 *
 * @note **ODE variable convention:** CompactStar evolves the dimensionless logarithmic variable
 *       \(\ln (T_\infty / T_{\rm ref})\). If a physical model naturally computes \(dT_\infty/dt\),
 *       it must be converted via:
 * \f[
 *   \frac{d\ln (T_\infty / T_{\rm ref})}{dt} = \frac{1}{T_\infty}\,\frac{dT_\infty}{dt}.
 * \f]
 *
 * with a simple blackbody-like luminosity at infinity:
 *
 * \f[
 *   L_{\gamma,\infty} = \mathcal{F}_{\rm rad}\; A_{\infty}\; \sigma_{\rm SB}\; T_{\rm surf}^4 .
 * \f]
 *
 * Where:
 * - \f$T_{\rm surf}\f$ is a local-frame surface temperature [K].
 * - \f$\sigma_{\rm SB}\f$ is the Stefan–Boltzmann constant.
 * - \f$A_{\infty}\f$ is the **redshifted emitting area** appropriate for
 *   luminosity measured at infinity.
 * - \f$\mathcal{F}_{\rm rad}\f$ is a **dimensionless** prefactor capturing
 *   partial surface emission (hot spots, beaming, opacity fudge, etc.).
 *
 * ## Units policy (IMPORTANT)
 * This header documents the unit expectations; the implementation must be
 * consistent with whatever unit conventions CompactStar uses.
 *
 * ### Temperature
 * - \(T_{\rm surf}\), \(T_\infty\): **Kelvin [K]**.
 *
 * ### Luminosity
 * If \(\sigma_{\rm SB}\) is taken in **cgs**,
 * \(\sigma_{\rm SB} \approx 5.670374419\times10^{-5}\;{\rm erg\,cm^{-2}\,s^{-1}\,K^{-4}}\),
 * then:
 * - \(A_\infty\) must be in **cm²**
 * - \(L_{\gamma,\infty}\) is in **erg/s**
 *
 * ### Heat capacity
 * - C_eff must be [erg/K] so that \(dT_\infty/dt\) is [K/s].
 *   When evolving \(\ln T_\infty\), the driver ultimately contributes
 *   \(d(\ln T_\infty)/dt\) with units [1/s].
 *
 * ### Geometry input
 * CompactStar structure profiles often store radius in **km**.
 * If the GeometryCache provides \(R\) in km, the implementation must convert:
 * - \(R_{\rm cm} = 10^5 \, R_{\rm km}\)
 * before constructing \(A_\infty\).
 *
 * ### Redshift convention
 * For a static metric with \(\nu(r)\) defined via \(g_{tt}=-e^{2\nu(r)}\),
 * the luminosity observed at infinity from a local surface blackbody scales as:
 *
 * \f[
 *   L_{\gamma,\infty} = 4\pi R^2 \, e^{2\nu(R)} \, \sigma_{\rm SB}\, T_{\rm surf}^4 ,
 * \f]
 *
 * i.e. the area factor used here is:
 * \f[
 *   A_\infty \equiv 4\pi R^2 e^{2\nu(R)} .
 * \f]
 *
 * The driver will obtain the needed metric/redshift factors from the
 * GeometryCache (via DriverContext::geo) when available.
 *
 * ## DriverContext usage
 * - `ctx.geo` (GeometryCache) is the preferred source for \(R\) and \(e^{2\nu}\).
 * - `ctx.envelope` may later supply a physically motivated mapping
 *   \(T_\infty \mapsto T_{\rm surf}\) (or \(T_{\rm core}\mapsto T_{\rm surf}\)).
 *
 * ## Scope / limitations
 * This is intentionally minimal:
 * - No neutrino cooling, no internal heating, no envelope microphysics by default.
 * - `C_eff` is treated as an externally provided effective heat capacity.
 *   A future model should compute \(C_{\rm eff}(T)\) from microphysics + structure.
 */

#ifndef CompactStar_Physics_Driver_Thermal_PhotonCooling_H
#define CompactStar_Physics_Driver_Thermal_PhotonCooling_H

#include <string>
#include <vector>

#include "CompactStar/Physics/Driver/IDriver.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Driver::Thermal::Detail
{
struct PhotonCooling_Details;
}
namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class PhotonCooling
 * @brief Adds a photon-cooling contribution to the thermal RHS (redshifted frame).
 *
 * **Depends on:** Thermal
 * **Updates:**    Thermal
 *
 * ### What this driver does
 * 1. Determines a surface temperature \(T_{\rm surf}\) (K) from the Thermal state:
 *    - directly from the auxiliary ThermalState::T_surf, or
 *    - via an envelope mapping (future), or
 *    - via a simple approximation from \(T_\infty\).
 * 2. Computes \(L_{\gamma,\infty}\) using geometry + Stefan–Boltzmann.
 * 3. Adds \(-L_{\gamma,\infty}/C_{\rm eff}\) to the RHS for the evolved thermal DOF.
 *
 * ### What this driver does *not* do
 * - It does not modify the static star structure.
 * - It does not compute \(C_{\rm eff}\) from EOS/microphysics (yet).
 * - It does not implement a realistic envelope unless provided externally.
 */
class PhotonCooling final : public IDriver,
							public Driver::Diagnostics::IDriverDiagnostics
{
  public:
	/**
	 * @struct Options
	 * @brief Configuration parameters for PhotonCooling.
	 *
	 * The goal is to keep these **physically interpretable** and
	 * **unit-consistent**:
	 * - Temperatures are in Kelvin.
	 * - Heat capacity is in erg/K if we use cgs \(\sigma_{\rm SB}\).
	 * - All scale factors are dimensionless.
	 */
	struct Options
	{
		/**
		 * @enum SurfaceModel
		 * @brief Specifies how the driver obtains \(T_{\rm surf}\).
		 */
		enum class SurfaceModel
		{
			/**
			 * Use ThermalState::T_surf directly.
			 *
			 * This assumes the caller (or another driver) has already
			 * populated `ThermalState::T_surf` in Kelvin.
			 */
			DirectTSurf,

			/**
			 * Use a physically motivated envelope model.
			 *
			 * Reserved for: `ctx.envelope != nullptr`.
			 * The envelope typically maps from \(T_\infty\) or \(T_{\rm core}\)
			 * (and possibly surface gravity) to \(T_{\rm surf}\).
			 */
			EnvelopeMapping,

			/**
			 * Simple fallback model.
			 *
			 * For infrastructure testing: \(T_{\rm surf} \approx T_\infty\).
			 * This is *not* physically accurate but is useful for wiring tests.
			 */
			ApproxFromTinf

		} surface_model = SurfaceModel::ApproxFromTinf;

		/**
		 * @brief Dimensionless radiating fraction \f$\mathcal{F}_{\rm rad}\f$.
		 *
		 * Multiplies the redshifted geometric area \(A_\infty\) to represent:
		 * - partial surface emission (hot spots),
		 * - effective emissivity < 1,
		 * - crude beaming / atmosphere suppression factors,
		 * - quick “knob” for unit tests.
		 *
		 * ### Units
		 * - Dimensionless.
		 *
		 * ### Recommended behavior
		 * - If set to 1: full surface emission.
		 * - If set to 0 (or ≤ 0): disables photon cooling from this driver.
		 *
		 * @note **Disabled-but-valid policy:** if `radiating_fraction <= 0`, this driver
		 *       is treated as intentionally disabled and produces a **zero** luminosity
		 *       and **zero** RHS contribution (not an error condition).
		 */
		double radiating_fraction = 1.0;

		/**
		 * @brief Effective heat capacity of the evolved thermal DOF.
		 *
		 * This is the denominator in:
		 * \f[
		 *   \frac{dT_\infty}{dt} = -\frac{L_{\gamma,\infty}}{C_{\rm eff}} .
		 * \f]
		 *
		 * ### Units
		 * If \(L_{\gamma,\infty}\) is in erg/s and \(T\) is in K:
		 * - \(C_{\rm eff}\) must be in **erg/K**.
		 *
		 * ### Notes
		 * - In realistic models, \(C_{\rm eff}\) is computed from structure +
		 *   microphysics (composition, superfluidity, etc.) and can depend on T.
		 * - Here it is a user-provided effective constant for simplicity.
		 */
		double C_eff = 1.0e40;

		/**
		 * @brief Global dimensionless multiplicative scale.
		 *
		 * A final multiplicative factor applied to the luminosity.
		 * Useful for:
		 * - controlled unit tests,
		 * - turning effects up/down without changing Options semantics,
		 * - debugging sensitivity.
		 *
		 * ### Units
		 * - Dimensionless.
		 *
		 *
		 * @note **Disabled-but-valid policy:** if `global_scale <= 0`, the driver
		 *       produces a **zero** luminosity and **zero** RHS contribution
		 *       (not an error condition).
		 */
		double global_scale = 1.0;
	};

	/// Default-construct with Options() defaults.
	PhotonCooling() = default;

	/// Construct with explicit options.
	explicit PhotonCooling(const Options &opts) : opts_(opts) {}

	// --------------------------------------------------------------
	//  IDriver interface
	// --------------------------------------------------------------

	/// Human-readable name for diagnostics/logging.
	std::string Name() const override { return "PhotonCooling"; }

	/// State blocks this driver reads.
	const std::vector<State::StateTag> &DependsOn() const override
	{
		static const std::vector<State::StateTag> deps{State::StateTag::Thermal};
		return deps;
	}

	/// State blocks this driver updates (accumulates into RHS).
	const std::vector<State::StateTag> &Updates() const override
	{
		static const std::vector<State::StateTag> ups{State::StateTag::Thermal};
		return ups;
	}

	/**
	 * @brief Add photon cooling contribution to the thermal RHS.
	 *
	 * This method must **accumulate** into @p dYdt (never overwrite).
	 *
	 * ### Algorithm outline
	 * 1. Read Thermal state from @p Y and obtain \(T_\infty\).
	 * 2. Determine \(T_{\rm surf}\) according to Options::surface_model:
	 *    - DirectTSurf: use ThermalState::T_surf if set (>0), else fallback.
	 *    - EnvelopeMapping: if ctx.envelope exists, use it (future).
	 *    - ApproxFromTinf: set \(T_{\rm surf} \leftarrow T_\infty\).
	 * 3. Determine \(A_\infty\) from geometry (preferred):
	 *    \f[
	 *       A_\infty = 4\pi R^2 e^{2\nu(R)} .
	 *    \f]
	 *    taking care to convert \(R\) into cm if \(\sigma_{\rm SB}\) is cgs.
	 * 4. Form the luminosity and the physical cooling rate:
	 *    \f[
	 *      L_{\gamma,\infty} = \mathrm{global\_scale}\;
	 *                          \mathrm{radiating\_fraction}\;
	 *                          A_\infty\;
	 *                          \sigma_{\rm SB}\;
	 *                          T_{\rm surf}^4 ,
	 *    \f]
	 *    \f[
	 *      \frac{dT_\infty}{dt} = -\frac{L_{\gamma,\infty}}{C_{\rm eff}} .
	 *    \f]
	 *
	 * 5. Convert to the evolved ODE variable and accumulate into the RHS:
	 *    \f[
	 *      \frac{d\ln (T_\infty / T_{\rm ref})}{dt} \mathrel{+}= \frac{1}{T_\infty}\,\frac{dT_\infty}{dt}
	 *      = -\frac{L_{\gamma,\infty}}{C_{\rm eff}\,T_\infty}.
	 *    \f]
	 *
	 * @note CompactStar evolves \(\ln (T_\infty / T_{\rm ref})\) (not \(T_\infty\)) in the thermal
	 *       state vector. Drivers must therefore contribute \(d(\ln (T_\infty / T_{\rm ref}))/dt\).
	 *
	 * @param t     Current time (seconds, by convention for evolution drivers).
	 * @param Y     Read-only composite state vector.
	 * @param dYdt  Write-only RHS accumulator (driver adds to it).
	 * @param ctx   Static driver context (geometry, star, envelope, config).
	 */
	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::DriverContext &ctx) const override;

	// --------------------------------------------------------------
	//  Options access
	// --------------------------------------------------------------

	/// Get current options (read-only).
	const Options &GetOptions() const { return opts_; }

	/// Replace options (by value).
	void SetOptions(const Options &o) { opts_ = o; }

	// --- IDriverDiagnostics ---
	[[nodiscard]] std::string DiagnosticsName() const override;
	[[nodiscard]] Evolution::Diagnostics::UnitContract UnitContract() const override;
	[[nodiscard]] Evolution::Diagnostics::ProducerCatalog DiagnosticsCatalog() const override;

	void DiagnoseSnapshot(double t,
						  const Evolution::StateVector &Y,
						  const Evolution::DriverContext &ctx,
						  Evolution::Diagnostics::DiagnosticPacket &out) const override;

  private:
	/// Stored configuration options.
	Options opts_{};

	/// Allow PhotonCooling_Details to access private members.
	friend struct CompactStar::Physics::Driver::Thermal::Detail::PhotonCooling_Details;
};

} // namespace CompactStar::Physics::Driver::Thermal

#endif /* CompactStar_Physics_Driver_Thermal_PhotonCooling_H */