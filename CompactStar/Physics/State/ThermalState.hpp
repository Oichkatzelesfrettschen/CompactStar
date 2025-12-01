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
 * @file ThermalState.hpp
 * @brief Thermal evolution state block for a compact star.
 *
 * This class provides the **dynamical thermal degrees of freedom** used by the
 * evolution system (typically the redshifted internal temperature T∞, or
 * multiple temperature components if needed), as well as optional auxiliary
 * temperatures used by microphysics or cooling models.
 *
 * Directory-based namespace: CompactStar::Physics::State
 *
 * @ingroup PhysicsState
 *
 * ### Design
 * - The evolution DOFs live in a contiguous vector (`values_`).
 * - A typical minimal configuration has just one DOF = T∞.
 * - Subclasses or callers may configure multiple components by calling Resize().
 *
 * ### Auxiliary variables
 * The local-frame temperatures (T_core, T_blanket, T_surf) are **not**
 * part of the ODE vector. They are cached values used by cooling models,
 * envelopes, emissivity computations, and diagnostics.
 */

#ifndef CompactStar_Physics_State_ThermalState_H
#define CompactStar_Physics_State_ThermalState_H

#include <cstddef>
#include <vector>

#include "CompactStar/Physics/State/State.hpp"

namespace CompactStar::Physics::State
{

/**
 * @class ThermalState
 * @brief Dynamical thermal DOFs + cached physical temperatures.
 *
 * The ODE part (values_) is what evolution drivers operate on. By default
 * callers usually use a single DOF representing the **redshifted internal
 * temperature** T∞. If more thermal components (e.g. multi-zone models)
 * are needed, callers simply call Resize(N).
 *
 * The auxiliary fields store physically meaningful temperatures in the
 * **local fluid frame**, which microphysics and cooling models often require.
 *
 * @note Storage (the `values_` vector) deliberately lives in this derived
 *       class rather than in the State base class.  This design allows
 *       future state types to use non-contiguous, multi-zone, or
 *       externally owned layouts while still exposing the uniform
 *       PackTo/UnpackFrom interface required by the evolution framework.
 *       See @ref CompactStar::Physics::State::State for details.
 */
class ThermalState : public State
{
  public:
	// ------------------------------------------------------------------
	// Required interface from State
	// ------------------------------------------------------------------

	const char *Name() const override { return "ThermalState"; }

	/// Number of ODE thermal components.
	std::size_t Size() const override { return values_.size(); }

	/// Mutable pointer to contiguous ODE data.
	double *Data() override
	{
		return values_.empty() ? nullptr : values_.data();
	}

	/// Const pointer to contiguous ODE data.
	const double *Data() const override
	{
		return values_.empty() ? nullptr : values_.data();
	}

	/**
	 * @brief Resize the ODE thermal vector to N components.
	 *
	 * Typical choice:
	 *   N = 1 → single DOF = T∞
	 *
	 * More components can be added for multi-zone thermal evolution models.
	 */
	void Resize(std::size_t N) override
	{
		values_.assign(N, 0.0);
	}

	/// Grid size == number of thermal DOFs in this simple layout.
	std::size_t GridSize() const override { return values_.size(); }

	/// Zero the thermal DOFs (capacity preserved).
	void Clear() override
	{
		for (double &x : values_)
			x = 0.0;
	}

	// ------------------------------------------------------------------
	// Packing/unpacking for global ODE vector
	// ------------------------------------------------------------------

	/// Copy the internal ODE components into a flat buffer.
	void PackTo(double *dest) const override;

	/// Populate internal ODE components from a flat buffer.
	void UnpackFrom(const double *src) override;
	// ------------------------------------------------------------------
	// Convenience accessors for ODE DOFs
	// ------------------------------------------------------------------

	/// Number of thermal DOFs.
	std::size_t NumComponents() const { return values_.size(); }

	/// Mutable access to i-th component.
	double &Value(std::size_t i) { return values_.at(i); }

	/// Const access to i-th component.
	const double &Value(std::size_t i) const { return values_.at(i); }

	/**
	 * @brief Convenience accessor for the primary thermal DOF (usually T∞).
	 *
	 * By convention, component 0 is the **redshifted isothermal-core temperature**.
	 */
	double &Tinf() { return values_.at(0); }

	/// Const accessor for T∞.
	const double &Tinf() const { return values_.at(0); }

	// ------------------------------------------------------------------
	// Auxiliary temperatures (NOT part of the ODE vector)
	// ------------------------------------------------------------------

	/// Local-frame core temperature [K].
	double T_core = 0.0;

	/// Local-frame blanket temperature [K] (ρ ~ 10^10 g/cm³).
	double T_blanket = 0.0;

	/// Local-frame surface temperature [K].
	double T_surf = 0.0;

  private:
	/// Contiguous ODE thermal components (e.g. T∞, or multi-zone T1,T2,...).
	std::vector<double> values_;
};

} // namespace CompactStar::Physics::State

#endif /* CompactStar_Physics_State_ThermalState_H */