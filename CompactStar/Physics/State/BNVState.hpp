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
 * @file BNVState.hpp
 * @brief Dynamic or cached quantities associated with baryon-number violation.
 *
 * This state block provides storage for BNV-related diagnostics or dynamic
 * variables that may appear in BNV evolution drivers (e.g. ψ-population
 * evolution, effective η-like variables, or observationally relevant
 * meta-parameters).
 *
 * Depending on your BNV model, this may store:
 *  - a small vector of ODE-evolved quantities, or
 *  - a cached set of derived diagnostic values (non-ODE),
 * or both.
 *
 * Present version implements a simple ODE-compatible state with two components:
 *   1. η_I              — dimensionless imbalance parameter (model-specific)
 *   2. spin_down_limit  — Γ_BNV limit inferred from pulsar spin-down
 *
 * Additional DOFs can be added without touching the evolution core.
 *
 * @ingroup PhysicsState
 */

#ifndef CompactStar_Physics_State_BNVState_H
#define CompactStar_Physics_State_BNVState_H

#include <cstddef>
#include <vector>

#include "CompactStar/Physics/State/State.hpp"

namespace CompactStar::Physics::State
{

/**
 * @class BNVState
 * @brief State block for BNV-related dynamical or cached variables.
 *
 * This class behaves exactly like ChemState/ThermalState in terms of
 * interface: a resizable contiguous vector of doubles exposed to the
 * evolution system and its drivers.
 *
 * Physical meaning of each component is determined by your BNV model.
 *
 * @note Storage (the `values_` vector) deliberately lives in this derived
 *       class rather than in the State base class.  This design allows
 *       future state types to use non-contiguous, multi-zone, or
 *       externally owned layouts while still exposing the uniform
 *       PackTo/UnpackFrom interface required by the evolution framework.
 *       See @ref CompactStar::Physics::State::State for details.
 */
class BNVState : public State
{
  public:
	// -------------------------------------------------------------
	// Required interface from State
	// -------------------------------------------------------------

	const char *Name() const override { return "BNVState"; }

	std::size_t Size() const override { return values_.size(); }

	double *Data() override
	{
		return values_.empty() ? nullptr : values_.data();
	}

	const double *Data() const override
	{
		return values_.empty() ? nullptr : values_.data();
	}

	/**
	 * @brief Resize underlying DOF vector to @p N entries.
	 *
	 * Default layout:
	 *   N = 2
	 *     index 0 → η_I
	 *     index 1 → spin_down_limit
	 *
	 * You may expand this vector when implementing more elaborate BNV models.
	 */
	void Resize(std::size_t N) override
	{
		values_.assign(N, 0.0);
	}

	std::size_t GridSize() const override { return values_.size(); }

	/// Zero out all values (capacity preserved).
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
	// -------------------------------------------------------------
	// Convenience accessors
	// -------------------------------------------------------------

	/// Return number of BNV state components.
	std::size_t NumComponents() const { return values_.size(); }

	/// Access component i (mutable).
	double &Value(std::size_t i) { return values_.at(i); }

	/// Access component i (const).
	const double &Value(std::size_t i) const { return values_.at(i); }

	// Named accessors for the two canonical components
	double &EtaI() { return values_.at(0); }
	double &SpinDownLimit() { return values_.at(1); }

	const double &EtaI() const { return values_.at(0); }
	const double &SpinDownLimit() const { return values_.at(1); }

  private:
	/// Contiguous DOF vector, semantics determined by BNV model.
	std::vector<double> values_;
};

} // namespace CompactStar::Physics::State

#endif /* CompactStar_Physics_State_BNVState_H */