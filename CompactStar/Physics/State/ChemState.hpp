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
 * @file ChemState.hpp
 * @brief ODE state block for chemical imbalances used by evolution drivers.
 *
 * Represents the dynamical degrees of freedom associated with departures from
 * beta-equilibrium, typically written as a vector of chemical imbalances
 * \f$\eta_i\f$ (e.g., \f$\eta_{npe}, \eta_{np\mu}\f$).
 *
 * Design notes:
 *  - Stores only the *dynamic* variables (the \f$\eta_i\f vector).
 *  - No time, no spin, no tags — those live in StepMeta or other states.
 *  - Layout is a flat contiguous array compatible with the global ODE vector.
 *  - Size is configured once via Resize(N) prior to integration.
 *
 * Typical usage:
 *  - Rotochemical and BNV chemical drivers read/write components through
 *    the packed y-vector managed by the evolution system.
 *
 * @ingroup PhysicsState
 */

#ifndef CompactStar_Physics_State_ChemState_H
#define CompactStar_Physics_State_ChemState_H

#include <cstddef>
#include <vector>

#include "CompactStar/Physics/State/State.hpp"

namespace CompactStar::Physics::State
{

/**
 * @class ChemState
 * @brief Contiguous state block for chemical imbalances \f$\eta_i\f$.
 *
 * The components are stored as a simple 1D vector; their physical meaning
 * (which index corresponds to which reaction channel such as \f$\eta_{npe}\f,
 * \f$\eta_{np\mu}\f, etc.) is defined by the drivers/microphysics layer,
 * not by this class.
 *
 * Units of \f$\eta_i\f$ are typically energy (e.g. erg), but this is not
 * enforced here.
 *
 * @note Storage (the `values_` vector) deliberately lives in this derived
 *       class rather than in the State base class.  This design allows
 *       future state types to use non-contiguous, multi-zone, or
 *       externally owned layouts while still exposing the uniform
 *       PackTo/UnpackFrom interface required by the evolution framework.
 *       See @ref CompactStar::Physics::State::State for details.
 */
class ChemState : public State
{
  public:
	// ------------------------------------------------------------------
	// Required State interface
	// ------------------------------------------------------------------

	/// Short human-readable identifier for logging.
	const char *Name() const override { return "ChemState"; }

	/// Number of scalar degrees of freedom (number of \f$\eta_i\f components).
	std::size_t Size() const override { return eta_.size(); }

	/// Mutable pointer to the first element (contiguous storage).
	double *Data() override
	{
		return eta_.empty() ? nullptr : eta_.data();
	}

	/// Const pointer to the first element (contiguous storage).
	const double *Data() const override
	{
		return eta_.empty() ? nullptr : eta_.data();
	}

	/**
	 * @brief Configure the number of chemical-imbalance components.
	 *
	 * Resizes the internal storage to @p N components and clears them to zero.
	 * Must be called before the state is handed to the evolution system.
	 */
	void Resize(std::size_t N) override
	{
		eta_.assign(N, 0.0);
	}

	/// For this simple layout, "grid size" coincides with the number of components.
	std::size_t GridSize() const override { return eta_.size(); }

	/// Reset all components to zero (does not release capacity).
	void Clear() override
	{
		for (double &x : eta_)
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
	// Convenience accessors
	// ------------------------------------------------------------------

	/// Return the number of chemical-imbalance components.
	std::size_t NumComponents() const { return eta_.size(); }

	/// Mutable access to the i-th chemical imbalance \f$\eta_i\f$.
	double &Eta(std::size_t i) { return eta_.at(i); }

	/// Const access to the i-th chemical imbalance \f$\eta_i\f$.
	const double &Eta(std::size_t i) const { return eta_.at(i); }

  private:
	/// Packed vector of chemical imbalances \f$\eta_i\f\ (energy units).
	std::vector<double> eta_;
};

} // namespace CompactStar::Physics::State

#endif /* CompactStar_Physics_State_ChemState_H */