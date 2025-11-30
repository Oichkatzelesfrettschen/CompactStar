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
 * @file StateVector.cpp
 * @brief Implementation of the StateVector registry.
 */

#include "CompactStar/Physics/Evolution/StateVector.hpp"

#include <stdexcept>
#include <string>

#include "CompactStar/Physics/State/BNVState.hpp"
#include "CompactStar/Physics/State/ChemState.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/Tags.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

namespace CompactStar::Physics::Evolution
{

//--------------------------------------------------------------
//  Constructor
//--------------------------------------------------------------

StateVector::StateVector()
{
	// Initialize all tag slots to nullptr (unregistered).
	blocks_.fill(nullptr);
}

//--------------------------------------------------------------
//  Registration
//--------------------------------------------------------------

void StateVector::Register(Physics::State::StateTag tag,
						   Physics::State::State &state)
{
	// Store a non-owning pointer to the provided State-derived object.
	// Re-registering the same tag simply overwrites the pointer.
	blocks_[Index(tag)] = &state;
}

//--------------------------------------------------------------
//  Generic accessors (const)
//--------------------------------------------------------------

const Physics::State::State &StateVector::Get(Physics::State::StateTag tag) const
{
	const auto *ptr = blocks_[Index(tag)];
	if (!ptr)
	{
		// Build a helpful error message that includes the tag name.
		throw std::runtime_error("StateVector::Get(const): requested tag '" +
								 std::string(Physics::State::ToString(tag)) +
								 "' is not registered.");
	}
	return *ptr;
}

//--------------------------------------------------------------
//  Generic accessors (mutable)
//--------------------------------------------------------------

Physics::State::State &StateVector::Get(Physics::State::StateTag tag)
{
	auto *ptr = blocks_[Index(tag)];
	if (!ptr)
	{
		// Build a helpful error message that includes the tag name.
		throw std::runtime_error("StateVector::Get: requested tag '" +
								 std::string(Physics::State::ToString(tag)) +
								 "' is not registered.");
	}
	return *ptr;
}

//--------------------------------------------------------------
//  Typed accessors: SpinState
//--------------------------------------------------------------

const Physics::State::SpinState &StateVector::GetSpin() const
{
	// Retrieve the base State reference and cast to SpinState.
	const auto &base = Get(Physics::State::StateTag::Spin);
	return dynamic_cast<const Physics::State::SpinState &>(base);
}

//--------------------------------------------------------------

Physics::State::SpinState &StateVector::GetSpin()
{
	auto &base = Get(Physics::State::StateTag::Spin);
	return dynamic_cast<Physics::State::SpinState &>(base);
}

//--------------------------------------------------------------
//  Typed accessors: ThermalState
//--------------------------------------------------------------

const Physics::State::ThermalState &StateVector::GetThermal() const
{
	const auto &base = Get(Physics::State::StateTag::Thermal);
	return dynamic_cast<const Physics::State::ThermalState &>(base);
}

//--------------------------------------------------------------

Physics::State::ThermalState &StateVector::GetThermal()
{
	auto &base = Get(Physics::State::StateTag::Thermal);
	return dynamic_cast<Physics::State::ThermalState &>(base);
}

//--------------------------------------------------------------
//  Typed accessors: ChemState
//--------------------------------------------------------------

const Physics::State::ChemState &StateVector::GetChem() const
{
	const auto &base = Get(Physics::State::StateTag::Chem);
	return dynamic_cast<const Physics::State::ChemState &>(base);
}

//--------------------------------------------------------------

Physics::State::ChemState &StateVector::GetChem()
{
	auto &base = Get(Physics::State::StateTag::Chem);
	return dynamic_cast<Physics::State::ChemState &>(base);
}

//--------------------------------------------------------------
//  Typed accessors: BNVState
//--------------------------------------------------------------

const Physics::State::BNVState &StateVector::GetBNV() const
{
	const auto &base = Get(Physics::State::StateTag::BNV);
	return dynamic_cast<const Physics::State::BNVState &>(base);
}

//--------------------------------------------------------------

Physics::State::BNVState &StateVector::GetBNV()
{
	auto &base = Get(Physics::State::StateTag::BNV);
	return dynamic_cast<Physics::State::BNVState &>(base);
}

//--------------------------------------------------------------

} // namespace CompactStar::Physics::Evolution