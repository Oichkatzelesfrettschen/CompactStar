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
 * @file HeatingFromChem.hpp
 * @brief Driver that converts chemical free energy (η⋅Γ) into thermal heating.
 *
 * Contributes to dT̃/dt via Q_heat = Σ_i Γ_i(η, T̃, …) * η_i, distributed over
 * the stellar volume and normalized by the effective heat capacity. Chemical
 * *evolution* (dη/dt) is handled by separate drivers (e.g., WeakRestoration).
 *
 * @ingroup PhysicsDriver
 * @author
 *   Mohammadreza Zakeri (M.Zakeri@eku.edu)
 */

#ifndef CompactStar_Physics_Driver_Thermal_HeatingFromChem_H
#define CompactStar_Physics_Driver_Thermal_HeatingFromChem_H

#include "CompactStar/Physics/Driver/IDriver.hpp"
#include <array>
#include <span>
#include <string>

namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class HeatingFromChem
 * @brief Accumulates the thermal RHS from chemical heating (η⋅Γ terms).
 *
 * **Depends on:** Chem, Thermal
 * **Updates:**    Thermal
 *
 * The microphysics for Γ_i should be accessed via the StarContext and/or
 * Microphysics modules (rates/emissivities). This driver only performs the
 * bookkeeping to add its contribution to the thermal component of dY/dt.
 */
class HeatingFromChem final : public IDriver
{
  public:
	/// Optional knobs for channel selection / scaling.
	struct Options
	{
		bool use_electron_channel = true;
		bool use_muon_channel = true;
		double global_scale = 1.0; ///< multiplicative safety factor
	};

	explicit HeatingFromChem(Options opts = {}) : opts_(opts) {}

	// --- IDriver interface -----------------------------------------------------
	std::string Name() const override { return "HeatingFromChem"; }

	const std::vector<State::StateTag> &DependsOn() const override
	{
		static const std::vector<State::StateTag> deps{State::StateTag::Chem, State::StateTag::Thermal};
		return deps;
	}

	const std::vector<State::StateTag> &Updates() const override
	{
		static const std::vector<State::StateTag> ups{State::StateTag::Thermal};
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

#endif /* CompactStar_Physics_Driver_Thermal_HeatingFromChem_H */