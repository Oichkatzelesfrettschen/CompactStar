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
 * @brief Driver for surface photon cooling contribution to dT∞/dt.
 *
 * Converts an effective photon luminosity
 *   L_{γ,∞} ~ A_eff σ T_surf^4
 * (or an envelope model provided via StarContext) into the redshifted thermal
 * RHS term −L_γ / C_eff. Envelope physics (T_core ↔ T_surf relations) should
 * eventually be provided by the context/microphysics layer.
 *
 * Directory-based namespace: CompactStar::Physics::Driver::Thermal
 *
 * @ingroup PhysicsDriver
 */

#ifndef CompactStar_Physics_Driver_Thermal_PhotonCooling_H
#define CompactStar_Physics_Driver_Thermal_PhotonCooling_H

#include <string>
#include <vector>

#include "CompactStar/Physics/Driver/IDriver.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class PhotonCooling
 * @brief Adds −L_γ / C_eff to the thermal RHS (redshifted frame).
 *
 * **Depends on:** Thermal
 * **Updates:**    Thermal
 *
 * This is a deliberately simple toy model:
 *   - picks a surface temperature T_surf from the ThermalState, either
 *     directly or via an approximate mapping from T∞,
 *   - computes L_γ ∝ A_eff σ T_surf^4,
 *   - maps this to dT∞/dt = −L_γ / C_eff.
 *
 * All geometric factors (4πR², redshift, etc.) are absorbed into A_eff and
 * C_eff for now.
 */
class PhotonCooling final : public IDriver
{
  public:
	struct Options
	{
		/// How to obtain T_surf for the blackbody luminosity.
		enum class SurfaceModel
		{
			DirectTSurf,	 ///< Use ThermalState::T_surf directly.
			EnvelopeMapping, ///< (reserved) Use a true envelope model via StarContext.
			ApproxFromTinf	 ///< Simple T_surf ≈ T∞ fallback.
		} surface_model = SurfaceModel::ApproxFromTinf;

		/// Effective emitting area (absorbs 4πR² and redshift factors).
		double area_eff = 1.0;

		/// Effective heat capacity of the redshifted thermal DOF.
		double C_eff = 1.0e40;

		/// Global multiplicative scale (for experiments / fudge factors).
		double global_scale = 1.0;
	};

	/// Default-construct with Options() defaults.
	PhotonCooling() = default;

	/// Construct with explicit options.
	explicit PhotonCooling(const Options &opts) : opts_(opts) {}

	// ------------------------------------------------------------------
	//  IDriver interface
	// ------------------------------------------------------------------

	std::string Name() const override { return "PhotonCooling"; }

	const std::vector<State::StateTag> &DependsOn() const override
	{
		static const std::vector<State::StateTag> deps{
			State::StateTag::Thermal};
		return deps;
	}

	const std::vector<State::StateTag> &Updates() const override
	{
		static const std::vector<State::StateTag> ups{
			State::StateTag::Thermal};
		return ups;
	}

	/**
	 * @brief Add photon-cooling contribution to the thermal RHS.
	 *
	 * Conceptual model:
	 *   - Read ThermalState from the composite Y.
	 *   - Choose T_surf using opts_.surface_model.
	 *   - Compute L_γ = global_scale * area_eff * σ T_surf^4
	 *     (σ = Stefan–Boltzmann in cgs units).
	 *   - Add dT∞/dt = -L_γ / C_eff to the Thermal block of dY/dt.
	 */
	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::StarContext &ctx) const override;

	// ------------------------------------------------------------------
	//  Options access
	// ------------------------------------------------------------------

	const Options &GetOptions() const { return opts_; }
	void SetOptions(const Options &o) { opts_ = o; }

  private:
	Options opts_{};
};

} // namespace CompactStar::Physics::Driver::Thermal

#endif /* CompactStar_Physics_Driver_Thermal_PhotonCooling_H */