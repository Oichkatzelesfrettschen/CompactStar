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
 * @file NeutrinoCooling.hpp
 * @brief Driver for core neutrino cooling (DUrca, MUrca, …) contribution to dT̃/dt.
 *
 * Integrates emissivities over proper volume (with metric factors available via
 * StarContext / EOS hooks) and converts total L_ν,∞ into a redshifted thermal
 * cooling term. Superfluid suppression, thresholds, and composition masks are
 * delegated to Microphysics and the context.
 *
 * @ingroup PhysicsDriver
 */

#ifndef CompactStar_Physics_Driver_Thermal_NeutrinoCooling_H
#define CompactStar_Physics_Driver_Thermal_NeutrinoCooling_H

#include "CompactStar/Physics/Driver/IDriver.hpp"
// #include "CompactStar/Physics/State/Tags.hpp"

#include <string>
#include <vector>

namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class NeutrinoCooling
 * @brief Adds −L_ν/C_eff to the thermal RHS (redshifted frame).
 *
 * **Depends on:** Thermal (and possibly Chem via ctx/microphysics)
 * **Updates:**    Thermal
 */
class NeutrinoCooling final : public IDriver
{
  public:
	struct Options
	{
		bool include_direct_urca = true;
		bool include_modified_urca = true;
		bool include_pair_breaking = false; ///< hook for future superfluid PBF
		double global_scale = 1.0;			///< multiplicative safety factor
	};

	explicit NeutrinoCooling(Options opts = {}) : opts_(opts) {}

	// --- IDriver interface -----------------------------------------------------
	std::string Name() const override { return "NeutrinoCooling"; }

	const std::vector<State::StateTag> &DependsOn() const override
	{
		// Thermal is required; Chem may be read internally via ctx if rates need η.
		// static constexpr std::array<State::StateTag, 1> deps{State::StateTag::Thermal};
		static const std::vector<State::StateTag> deps{
			State::StateTag::Thermal};
		return deps;
	}

	const std::vector<State::StateTag> &Updates() const override
	{
		// static constexpr std::array<State::StateTag, 1> ups{State::StateTag::Thermal};
		static const std::vector<State::StateTag> ups{
			State::StateTag::Thermal};
		return ups;
	}

	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::DriverContext &ctx) const override;

	// --------------------------------------------------------------------------
	const Options &GetOptions() const { return opts_; }
	void SetOptions(const Options &o) { opts_ = o; }

  private:
	Options opts_;
};

} // namespace CompactStar::Physics::Driver::Thermal

#endif /* CompactStar_Physics_Driver_Thermal_NeutrinoCooling_H */