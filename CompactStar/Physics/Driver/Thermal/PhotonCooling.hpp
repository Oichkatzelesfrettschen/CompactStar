#ifndef COMPACTSTAR_PHYSICS_DRIVER_THERMAL_PHOTONCOOLING_HPP
#define COMPACTSTAR_PHYSICS_DRIVER_THERMAL_PHOTONCOOLING_HPP
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
 * @brief Photon (surface) cooling driver for the thermal evolution subsystem.
 *
 * @ingroup PhysicsDriver
 *
 * This driver contributes **surface photon emission** as an energy-loss term
 * in the thermal evolution equation. It is intentionally minimal and is meant
 * to be composed with other thermal drivers (neutrino cooling, heating, etc.).
 *
 * ## Thermal variable convention
 * CompactStar evolves a dimensionless logarithmic thermal variable:
 *
 * \f[
 *   y_T \equiv \ln\!\left(\frac{T_\infty}{T_{\rm ref}}\right),
 * \f]
 *
 * where \(T_\infty\) is a redshifted (isothermal-core) temperature in Kelvin.
 * If a physical model yields \(dT_\infty/dt\), it must be converted as:
 *
 * \f[
 *   \frac{dy_T}{dt} = \frac{1}{T_\infty}\frac{dT_\infty}{dt}.
 * \f]
 *
 * ## Photon luminosity at infinity
 * For a static metric with \(g_{tt}=-e^{2\nu(r)}\), a local blackbody surface
 * emission corresponds to:
 *
 * \f[
 *   L_{\gamma,\infty}
 *   = \mathcal{S}\;\mathcal{F}_{\rm rad}\;A_\infty\;\sigma_{\rm SB}\;T_{\rm surf}^4,
 * \quad
 *   A_\infty \equiv 4\pi R^2 e^{2\nu(R)}.
 * \f]
 *
 * - \(T_{\rm surf}\): local effective surface temperature [K]
 * - \(\sigma_{\rm SB}\): Stefan–Boltzmann constant
 * - \(A_\infty\): redshifted emitting area for luminosity measured at infinity
 * - \(\mathcal{F}_{\rm rad}\): dimensionless “radiating fraction” (hot spots, emissivity, etc.)
 * - \(\mathcal{S}\): additional dimensionless global scale (debug / knob)
 *
 * The RHS contribution (in terms of the evolved variable) is:
 *
 * \f[
 *   \frac{dy_T}{dt}
 *   \mathrel{+}= - \frac{L_{\gamma,\infty}}{C_{\rm eff}\,T_\infty}.
 * \f]
 *
 * where \(C_{\rm eff}\) is an effective heat capacity [erg/K] (in cgs conventions).
 *
 * ## Envelope / blanket modeling
 * Determining \(T_{\rm surf}\) from an interior temperature requires an envelope
 * (“blanket”) prescription. In the new architecture, **the envelope choice and
 * parameters live in PhotonCooling::Options**, not in the global Evolution config
 * and not in DriverContext.
 *
 * The driver may obtain \(T_{\rm surf}\) by:
 *  - direct use of a stored \(T_{\rm surf}\) in ThermalState (if provided),
 *  - computing \(T_{\rm surf}\) from a Tb→Ts envelope mapping (preferred path),
 *  - using a simple approximation for debugging (e.g., \(T_{\rm surf}\approx T_\infty\)).
 *
 * ## Units policy (implementation must be consistent)
 * This header documents intended units; the implementation must honor them.
 *
 * - Temperatures: Kelvin [K]
 * - Luminosity: erg/s
 * - Heat capacity \(C_{\rm eff}\): erg/K
 * - If \(\sigma_{\rm SB}\) is cgs:
 *     \(\sigma_{\rm SB}\approx 5.670374419\times 10^{-5}\,{\rm erg\,cm^{-2}\,s^{-1}\,K^{-4}}\),
 *   then \(A_\infty\) must be in cm².
 *
 * CompactStar profiles commonly store radius in km; the implementation must
 * convert \(R_{\rm km}\to R_{\rm cm}\) before forming areas when using cgs.
 *
 * ## Context usage
 * - `ctx.geo` is the preferred source for \(R\) and \(e^{2\nu(R)}\) (via cached geometry).
 * - `ctx.star` may be used to compute surface gravity \(g_{14}\) for envelope fits.
 * - This driver does **not** require `ctx.envelope`; envelope policy is internal to Options.
 */

#include <string>
#include <vector>

#include "CompactStar/Physics/Driver/Diagnostics/DriverDiagnostics.hpp"
#include "CompactStar/Physics/Driver/IDriver.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Driver::Thermal::Detail
{
struct PhotonCooling_Details;
} // namespace CompactStar::Physics::Driver::Thermal::Detail

namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class PhotonCooling
 * @brief Surface photon cooling driver (redshifted frame).
 *
 * **Depends on:** Thermal
 * **Updates:**    Thermal
 *
 * This driver:
 *  1) obtains \(T_\infty\) from ThermalState,
 *  2) determines a local \(T_{\rm surf}\) using the selected surface/envelope model,
 *  3) computes \(L_{\gamma,\infty}\) using geometry and redshift factors,
 *  4) accumulates \(-L_{\gamma,\infty}/(C_{\rm eff}T_\infty)\) into the thermal RHS.
 *
 * It does not compute \(C_{\rm eff}\) from EOS/microphysics (yet), and it does not
 * evolve a multi-zone temperature profile (isothermal-core assumption).
 */
class PhotonCooling final : public IDriver,
							public Driver::Diagnostics::IDriverDiagnostics
{
  public:
	//==============================================================
	/**
	 * @enum EnvelopeModel
	 * @brief Canonical envelope/blanket prescriptions (Tb → Ts).
	 *
	 * These values encode which analytic fit / table is used internally.
	 * The meaning of `xi` depends on the chosen model; see Options::envelope_xi.
	 */
	enum class EnvelopeModel
	{
		Iron,	  ///< Heavy-element (iron-like) envelope.
		Accreted, ///< Light-element (accreted) envelope; typically hotter Ts for same Tb.
		Custom	  ///< Reserved: user-supplied mapping (Phase 2+).
	};
	//==============================================================

	/**
	 * @struct Options
	 * @brief Configuration parameters for PhotonCooling.
	 *
	 * Design goals:
	 *  - Parameters should be physically interpretable.
	 *  - All scale factors are dimensionless.
	 *  - Temperatures are in Kelvin.
	 *  - Heat capacity is in erg/K (if using cgs σ_SB).
	 *
	 * Validation policy (recommended in implementation):
	 *  - If radiating_fraction <= 0 or global_scale <= 0: treat as intentionally disabled
	 *    and produce zero RHS contribution (no exception).
	 *  - If C_eff <= 0: this is physically invalid; throw or log+skip (choose one policy).
	 */
	struct Options
	{
		/**
		 * @enum SurfaceModel
		 * @brief How the driver obtains the local surface temperature \(T_{\rm surf}\).
		 */
		enum class SurfaceModel
		{
			/**
			 * Use ThermalState::T_surf directly.
			 *
			 * Assumes some external code populated ThermalState with a local
			 * surface temperature (Kelvin).
			 */
			DirectTSurf,

			/**
			 * Use an envelope (blanket) model Tb → Ts.
			 *
			 * The driver computes/assumes a base-of-envelope temperature Tb
			 * (local frame) from the evolved interior temperature variable,
			 * then applies the selected EnvelopeModel.
			 *
			 * This is the physically preferred mode once Tb is defined consistently.
			 */
			EnvelopeTbTs,

			/**
			 * Simple fallback approximation for infrastructure testing.
			 *
			 * For debugging only: \(T_{\rm surf} \approx T_\infty\).
			 * Not physically accurate.
			 */
			ApproxFromTinf

		} surface_model = SurfaceModel::EnvelopeTbTs;

		/**
		 * @brief Dimensionless radiating fraction \f$\mathcal{F}_{\rm rad}\f$.
		 *
		 * Multiplies the redshifted emitting area to represent:
		 *  - partial surface emission (hot spots),
		 *  - effective emissivity < 1,
		 *  - crude atmosphere suppression factors,
		 *  - a convenient knob for controlled tests.
		 *
		 * Units: dimensionless.
		 *
		 * Convention:
		 *  - 1.0 : full-surface emission
		 *  - 0.0 : disabled (no photon cooling)
		 */
		double radiating_fraction = 1.0;

		/**
		 * @brief Effective heat capacity \(C_{\rm eff}\) of the evolved thermal DOF.
		 *
		 * Enters as:
		 * \f[
		 *   \frac{dT_\infty}{dt} = -\frac{L_{\gamma,\infty}}{C_{\rm eff}}.
		 * \f]
		 *
		 * Units:
		 *  - erg/K (if \(L\) is erg/s and \(T\) is K).
		 *
		 * Notes:
		 *  - In realistic models, \(C_{\rm eff}\) depends on T and composition.
		 *  - Here it is treated as a user-provided effective constant.
		 */
		double C_eff = 1.0e40;

		/**
		 * @brief Global dimensionless multiplicative scale \f$\mathcal{S}\f$.
		 *
		 * Applied as a final factor multiplying the luminosity.
		 * Useful for:
		 *  - debugging sensitivity,
		 *  - controlled unit tests,
		 *  - temporary “turn up/down” without changing semantic parameters.
		 *
		 * Units: dimensionless.
		 */
		double global_scale = 1.0;

		// ---- Envelope / blanket policy -----------------------------------

		/**
		 * @brief Select which analytic envelope prescription is used (Tb → Ts).
		 */
		EnvelopeModel envelope = EnvelopeModel::Iron;

		/**
		 * @brief Envelope composition/column parameter \f$\xi\f$ (model-dependent).
		 *
		 * Interpretation depends on the chosen `envelope` prescription.
		 * For some common fits, this may represent an effective light-element
		 * column depth or a proxy parameter controlling the transition between
		 * iron-like and fully accreted envelopes.
		 *
		 * Units: model-defined (often treated as dimensionless in fit formulas).
		 */
		double envelope_xi = 0.0;

		/**
		 * @brief Base-of-envelope density threshold (for documentation / future use).
		 *
		 * Many envelope fits define Tb at a fiducial density \(\rho_b\sim 10^{10}\,{\rm g\,cm^{-3}}\).
		 * This option is included to make that boundary explicit; it may become active
		 * once Tb is computed from a resolved crust model or a stored profile marker.
		 *
		 * Units: g/cm^3.
		 */
		double rho_b = 1.0e10;
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
	 * @brief Accumulate photon-cooling contribution into the thermal RHS.
	 *
	 * This method must **accumulate** into @p dYdt (never overwrite).
	 *
	 * Outline:
	 *  1) Read \(T_\infty\) from Thermal state.
	 *  2) Compute \(T_{\rm surf}\) per Options::surface_model.
	 *  3) Compute \(A_\infty = 4\pi R^2 e^{2\nu(R)}\) (prefer ctx.geo).
	 *  4) Compute \(L_{\gamma,\infty}\).
	 *  5) Add \(-L_{\gamma,\infty}/(C_{\rm eff}T_\infty)\) to the thermal RHS element.
	 *
	 * @param t     Current time [s] (by convention).
	 * @param Y     Composite state vector (read-only).
	 * @param dYdt  RHS accumulator (write-only; driver adds its contribution).
	 * @param ctx   Static driver context (geometry/star/config).
	 */
	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::DriverContext &ctx) const override;

	// --------------------------------------------------------------
	//  Options access
	// --------------------------------------------------------------

	/// Current options (read-only).
	const Options &GetOptions() const { return opts_; }

	/// Replace options.
	void SetOptions(const Options &o) { opts_ = o; }

	// --------------------------------------------------------------
	//  Diagnostics interface (IDriverDiagnostics)
	// --------------------------------------------------------------
	[[nodiscard]] std::string DiagnosticsName() const override;
	[[nodiscard]] Evolution::Diagnostics::UnitContract UnitContract() const override;
	[[nodiscard]] Evolution::Diagnostics::ProducerCatalog DiagnosticsCatalog() const override;

	void DiagnoseSnapshot(double t,
						  const Evolution::StateVector &Y,
						  const Evolution::DriverContext &ctx,
						  Evolution::Diagnostics::DiagnosticPacket &out) const override;

  private:
	Options opts_{};

	// Allow details layer to access internals (constants/helpers) without bloating the header.
	friend struct CompactStar::Physics::Driver::Thermal::Detail::PhotonCooling_Details;
};

} // namespace CompactStar::Physics::Driver::Thermal

#endif /* COMPACTSTAR_PHYSICS_DRIVER_THERMAL_PHOTONCOOLING_HPP */