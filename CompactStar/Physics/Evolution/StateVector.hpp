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
 * @ingroup Physics
 */

#ifndef CompactStar_Physics_Evolution_StateVector_H
#define CompactStar_Physics_Evolution_StateVector_H

#include <array>
#include <stdexcept>
#include <string>

#include "CompactStar/Physics/State/BNVState.hpp"
#include "CompactStar/Physics/State/ChemState.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/State.hpp"
#include "CompactStar/Physics/State/Tags.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

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
	StateVector()
	{
		blocks_.fill(nullptr);
	}

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
	void Register(Physics::State::StateTag tag, Physics::State::State &state)
	{
		blocks_[Index(tag)] = &state;
	}

	// ---------------------------------------------------------------------
	//  Generic accessors
	// ---------------------------------------------------------------------

	/**
	 * @brief Get a const reference to the State associated with @p tag.
	 *
	 * @throws std::runtime_error if the tag has not been registered.
	 */
	const Physics::State::State &Get(Physics::State::StateTag tag) const
	{
		const auto *ptr = blocks_[Index(tag)];
		if (!ptr)
		{
			throw std::runtime_error("StateVector::Get: requested tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "' is not registered.");
		}
		return *ptr;
	}

	/**
	 * @brief Get a mutable reference to the State associated with @p tag.
	 *
	 * @throws std::runtime_error if the tag has not been registered.
	 */
	Physics::State::State &Get(Physics::State::StateTag tag)
	{
		auto *ptr = blocks_[Index(tag)];
		if (!ptr)
		{
			throw std::runtime_error("StateVector::Get: requested tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "' is not registered.");
		}
		return *ptr;
	}

	// ---------------------------------------------------------------------
	//  Typed convenience accessors
	// ---------------------------------------------------------------------
	// These assume that the corresponding tag has been registered with an
	// object of the correct dynamic type (SpinState, ThermalState, ...).
	// If the dynamic type does not match, std::bad_cast is thrown.

	/// Const SpinState accessor (tag = StateTag::Spin).
	const Physics::State::SpinState &GetSpin() const
	{
		const auto &base = Get(Physics::State::StateTag::Spin);
		return dynamic_cast<const Physics::State::SpinState &>(base);
	}

	/// Mutable SpinState accessor (tag = StateTag::Spin).
	Physics::State::SpinState &GetSpin()
	{
		auto &base = Get(Physics::State::StateTag::Spin);
		return dynamic_cast<Physics::State::SpinState &>(base);
	}

	/// Const ThermalState accessor (tag = StateTag::Thermal).
	const Physics::State::ThermalState &GetThermal() const
	{
		const auto &base = Get(Physics::State::StateTag::Thermal);
		return dynamic_cast<const Physics::State::ThermalState &>(base);
	}

	/// Mutable ThermalState accessor (tag = StateTag::Thermal).
	Physics::State::ThermalState &GetThermal()
	{
		auto &base = Get(Physics::State::StateTag::Thermal);
		return dynamic_cast<Physics::State::ThermalState &>(base);
	}

	/// Const ChemState accessor (tag = StateTag::Chem).
	const Physics::State::ChemState &GetChem() const
	{
		const auto &base = Get(Physics::State::StateTag::Chem);
		return dynamic_cast<const Physics::State::ChemState &>(base);
	}

	/// Mutable ChemState accessor (tag = StateTag::Chem).
	Physics::State::ChemState &GetChem()
	{
		auto &base = Get(Physics::State::StateTag::Chem);
		return dynamic_cast<Physics::State::ChemState &>(base);
	}

	/// Const BNVState accessor (tag = StateTag::BNV).
	const Physics::State::BNVState &GetBNV() const
	{
		const auto &base = Get(Physics::State::StateTag::BNV);
		return dynamic_cast<const Physics::State::BNVState &>(base);
	}

	/// Mutable BNVState accessor (tag = StateTag::BNV).
	Physics::State::BNVState &GetBNV()
	{
		auto &base = Get(Physics::State::StateTag::BNV);
		return dynamic_cast<Physics::State::BNVState &>(base);
	}

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