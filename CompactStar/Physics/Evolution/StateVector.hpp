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
 * @file StateVector.hpp
 * @brief Composite view over sub-states (Spin/Thermal/Chem/BNV/…).
 *
 * This class provides a light-weight registry that maps StateTag → State*.
 * Drivers and the evolution system can use it to access the underlying
 * State-derived objects without hard-coding indices.
 *
 * Design goals:
 *  - Tag-based access (StateTag → State&).
 *  - Non-owning: StateVector does not manage lifetimes.
 *  - Optional typed convenience accessors (GetSpin(), GetThermal(), …).
 *
 * In a typical evolution setup:
 *  - System builds a StateVector once, registering each State block.
 *  - Drivers receive a const StateVector& in AccumulateRHS() and use it
 *    to read whatever blocks they depend on.
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_StateVector_H
#define CompactStar_Physics_Evolution_StateVector_H

#include <array>

#include "CompactStar/Physics/State/State.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::State
{
class SpinState;
class ThermalState;
class ChemState;
class BNVState;
} // namespace CompactStar::Physics::State

namespace CompactStar::Physics::Evolution
{

// -----------------------------------------------------------------------------
//  Helpers
// -----------------------------------------------------------------------------

/**
 * @brief Number of StateTag enumeration values.
 *
 * Assumes that StateTag::Custom is the last enumerator in the enum.
 */
inline constexpr std::size_t NumStateTags()
{
	using Physics::State::StateTag;
	return static_cast<std::size_t>(StateTag::Custom) + 1;
}

// -----------------------------------------------------------------------------
//  StateVector
// -----------------------------------------------------------------------------

/**
 * @class StateVector
 * @brief Composite view over the current ODE state (tag → State*).
 *
 * This class does not store any actual data; it only keeps pointers to
 * State-derived objects, indexed by StateTag. It is intended to be
 * constructed and owned by the evolution System, then passed by const
 * reference into driver calls.
 *
 * Ownership:
 *  - StateVector is **non-owning**; the caller must ensure the underlying
 *    State objects outlive the StateVector.
 */
class StateVector
{
  public:
	// ---------------------------------------------------------------------
	//  Ctors / Dtor
	// ---------------------------------------------------------------------

	/// Construct an empty registry (no tags registered).
	StateVector();

	/// Default destructor.
	~StateVector() = default;

	// ---------------------------------------------------------------------
	//  Registration
	// ---------------------------------------------------------------------

	/**
	 * @brief Register a State block for a given tag.
	 *
	 * @param tag    Which StateTag this block corresponds to.
	 * @param state  Reference to a State-derived object.
	 *
	 * This simply stores the pointer; it does not take ownership.
	 * Re-registering the same tag overwrites the previous pointer.
	 */
	void Register(Physics::State::StateTag tag, Physics::State::State &state);

	// ---------------------------------------------------------------------
	//  Generic accessors
	// ---------------------------------------------------------------------

	/**
	 * @brief Get a const reference to the State associated with @p tag.
	 *
	 * @throws std::runtime_error if the tag has not been registered.
	 */
	const Physics::State::State &Get(Physics::State::StateTag tag) const;

	/**
	 * @brief Get a mutable reference to the State associated with @p tag.
	 *
	 * @throws std::runtime_error if the tag has not been registered.
	 */
	Physics::State::State &Get(Physics::State::StateTag tag);

	// ---------------------------------------------------------------------
	//  Typed convenience accessors
	// ---------------------------------------------------------------------
	// These assume that the corresponding tag has been registered with an
	// object of the correct dynamic type (SpinState, ThermalState, ...).
	// If the dynamic type does not match, std::bad_cast is thrown.

	/// Const SpinState accessor (tag = StateTag::Spin).
	const Physics::State::SpinState &GetSpin() const;

	/// Mutable SpinState accessor (tag = StateTag::Spin).
	Physics::State::SpinState &GetSpin();

	/// Const ThermalState accessor (tag = StateTag::Thermal).
	const Physics::State::ThermalState &GetThermal() const;

	/// Mutable ThermalState accessor (tag = StateTag::Thermal).
	Physics::State::ThermalState &GetThermal();

	/// Const ChemState accessor (tag = StateTag::Chem).
	const Physics::State::ChemState &GetChem() const;

	/// Mutable ChemState accessor (tag = StateTag::Chem).
	Physics::State::ChemState &GetChem();

	/// Const BNVState accessor (tag = StateTag::BNV).
	const Physics::State::BNVState &GetBNV() const;

	/// Mutable BNVState accessor (tag = StateTag::BNV).
	Physics::State::BNVState &GetBNV();

  private:
	// ---------------------------------------------------------------------
	//  Internal helpers
	// ---------------------------------------------------------------------

	/// Convert a StateTag to an array index.
	static constexpr std::size_t Index(Physics::State::StateTag tag)
	{
		return static_cast<std::size_t>(tag);
	}

	/// Pointers to State blocks for each tag (nullptr if not registered).
	std::array<Physics::State::State *, NumStateTags()> blocks_;
};

} // namespace CompactStar::Physics::Evolution

#endif /* CompactStar_Physics_Evolution_StateVector_H */