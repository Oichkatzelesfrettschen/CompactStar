// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */

/**
 * @file PhotonCooling.hpp
 * @brief Driver for surface photon cooling contribution to dT̃/dt.
 *
 * Converts \(L_{\gamma,\infty} = 4\pi R^2 \sigma T_{\text{surf}}^4 z_\text{surf}^2\)
 * (or an envelope model provided via StarContext) into the redshifted thermal
 * RHS term −L_γ/C_eff. Envelope physics (T_core↔T_surf relations) should be
 * provided by the context/microphysics layer.
 *
 * @ingroup PhysicsDriver
 */

#ifndef CompactStar_Physics_Driver_Thermal_PhotonCooling_H
#define CompactStar_Physics_Driver_Thermal_PhotonCooling_H

#include "CompactStar/Physics/Driver/IDriver.hpp"
#include <array>
#include <span>
#include <string>

namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class PhotonCooling
 * @brief Adds −L_γ/C_eff to the thermal RHS (redshifted frame).
 *
 * **Depends on:** Thermal
 * **Updates:**    Thermal
 */
class PhotonCooling final : public IDriver
{
  public:
	struct Options
	{
		/// Choose model for T_surface (direct state T_surf vs. envelope mapping).
		enum class SurfaceModel
		{
			DirectTSurf,
			EnvelopeMapping
		} surface_model = SurfaceModel::EnvelopeMapping;

		double global_scale = 1.0; ///< multiplicative safety factor
	};

	explicit PhotonCooling(Options opts = {}) : opts_(opts) {}

	// --- IDriver interface -----------------------------------------------------
	std::string Name() const override { return "PhotonCooling"; }

	std::span<const State::StateTag> DependsOn() const override
	{
		static constexpr std::array<State::StateTag, 1> deps{State::StateTag::Thermal};
		return deps;
	}

	std::span<const State::StateTag> Updates() const override
	{
		static constexpr std::array<State::StateTag, 1> ups{State::StateTag::Thermal};
		return ups;
	}

	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::StarContext &ctx) const override;

	// --------------------------------------------------------------------------
	const Options &GetOptions() const { return opts_; }
	void SetOptions(const Options &o) { opts_ = o; }

  private:
	Options opts_;
};

} // namespace CompactStar::Physics::Driver::Thermal

#endif /* CompactStar_Physics_Driver_Thermal_PhotonCooling_H */