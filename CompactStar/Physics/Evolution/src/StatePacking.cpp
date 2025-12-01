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
 * @file StatePacking.cpp
 * @brief Implementation of helpers for packing/unpacking State blocks.
 */

#include "CompactStar/Physics/Evolution/StatePacking.hpp"

#include <stdexcept>
#include <string>

#include "CompactStar/Physics/State/State.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Evolution
{

//--------------------------------------------------------------
// PackStateVector
//--------------------------------------------------------------
void PackStateVector(const StateVector &state,
					 const StateLayout &layout,
					 double *y)
{
	if (!y)
	{
		throw std::runtime_error("PackStateVector: null y pointer.");
	}

	// Walk over all possible tags; only active ones contribute.
	for (std::size_t i = 0; i < NumStateTags(); ++i)
	{
		const auto tag = static_cast<Physics::State::StateTag>(i);

		if (!layout.IsActive(tag))
		{
			continue;
		}

		const auto offset = layout.Offset(tag);
		const auto n = layout.BlockSize(tag);

		// Retrieve the corresponding State block from the registry.
		const auto &block = state.Get(tag);

		// Optional safety check: block.Size() should match layout.BlockSize().
		if (block.Size() != n)
		{
			throw std::runtime_error("PackStateVector: size mismatch for tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "': State::Size() != layout.BlockSize().");
		}

		// Delegate the actual packing to the State implementation.
		block.PackTo(y + offset);
	}
}

//--------------------------------------------------------------
// UnpackStateVector
//--------------------------------------------------------------
void UnpackStateVector(StateVector &state,
					   const StateLayout &layout,
					   const double *y)
{
	if (!y)
	{
		throw std::runtime_error("UnpackStateVector: null y pointer.");
	}

	// Walk over all possible tags; only active ones contribute.
	for (std::size_t i = 0; i < NumStateTags(); ++i)
	{
		const auto tag = static_cast<Physics::State::StateTag>(i);

		if (!layout.IsActive(tag))
		{
			continue;
		}

		const auto offset = layout.Offset(tag);
		const auto n = layout.BlockSize(tag);

		// Retrieve the corresponding State block from the registry.
		auto &block = state.Get(tag);

		// Optional safety check: block.Size() should match layout.BlockSize().
		if (block.Size() != n)
		{
			throw std::runtime_error("UnpackStateVector: size mismatch for tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "': State::Size() != layout.BlockSize().");
		}

		// Delegate the actual unpacking to the State implementation.
		block.UnpackFrom(y + offset);
	}
}

//--------------------------------------------------------------
// ScatterRHSFromAccumulator
//--------------------------------------------------------------
void ScatterRHSFromAccumulator(const RHSAccumulator &rhs,
							   const StateLayout &layout,
							   double *dydt)
{
	if (!dydt)
	{
		throw std::runtime_error("ScatterRHSFromAccumulator: null dydt pointer.");
	}

	// For each active tag, if the RHSAccumulator has a configured block,
	// copy its contents into the corresponding slice of dydt[].
	for (std::size_t i = 0; i < NumStateTags(); ++i)
	{
		const auto tag = static_cast<Physics::State::StateTag>(i);

		if (!layout.IsActive(tag))
		{
			continue;
		}

		const auto offset = layout.Offset(tag);
		const auto n = layout.BlockSize(tag);

		if (!rhs.IsConfigured(tag))
		{
			// For now, treat "active but not configured" as an error, because
			// it almost always indicates a wiring mismatch.
			throw std::runtime_error("ScatterRHSFromAccumulator: RHS block for tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "' is not configured.");
		}

		const auto &block = rhs.Block(tag);

		if (block.size() != n)
		{
			throw std::runtime_error("ScatterRHSFromAccumulator: size mismatch for tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "': RHS block size != layout.BlockSize().");
		}

		for (std::size_t j = 0; j < n; ++j)
		{
			dydt[offset + j] = block[j];
		}
	}
}

} // namespace CompactStar::Physics::Evolution