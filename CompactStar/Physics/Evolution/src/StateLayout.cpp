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
 * @file StateLayout.cpp
 * @brief Implementation of StateLayout (flat y[] layout helper).
 */

#include "CompactStar/Physics/Evolution/StateLayout.hpp"

#include <stdexcept>
#include <string>

#include "CompactStar/Physics/State/State.hpp"

namespace CompactStar::Physics::Evolution
{

//--------------------------------------------------------------
// StateLayout::Configure (initializer_list)
//--------------------------------------------------------------
void StateLayout::Configure(const StateVector &state,
							const std::initializer_list<Physics::State::StateTag> &tags)
{
	// Reset all blocks to inactive and zero size.
	for (auto &b : m_blocks)
	{
		b.offset = 0;
		b.size = 0;
		b.active = false;
	}

	m_totalSize = 0;

	// Walk the provided tag list in order, giving each block a contiguous
	// segment in y[] based on the block's Size().
	for (auto tag : tags)
	{
		const auto idx = Index(tag);

		// Query the registered State block for this tag. If the tag is not
		// registered, StateVector::Get(tag) will throw; we deliberately
		// propagate that error so misconfigured runs fail loudly.
		const auto &blockState = state.Get(tag);

		const std::size_t n = blockState.Size();

		Block &b = m_blocks[idx];
		b.offset = m_totalSize;
		b.size = n;
		b.active = true;

		m_totalSize += n;
	}
}

//--------------------------------------------------------------
// StateLayout::Configure (vector)
//--------------------------------------------------------------
void StateLayout::Configure(const StateVector &state,
							const std::vector<Physics::State::StateTag> &tags)
{
	// Delegate to the existing implementation without duplicating logic.
	// We cannot construct an initializer_list from a vector, so we just
	// run the same logic by iterating the vector through the same code path.
	// Easiest is to inline the loop here while sharing the reset section.

	// Reset all blocks to inactive and zero size.
	for (auto &b : m_blocks)
	{
		b.offset = 0;
		b.size = 0;
		b.active = false;
	}

	m_totalSize = 0;

	for (auto tag : tags)
	{
		const auto idx = Index(tag);
		const auto &blockState = state.Get(tag);

		const std::size_t n = blockState.Size();

		Block &b = m_blocks[idx];
		b.offset = m_totalSize;
		b.size = n;
		b.active = true;

		m_totalSize += n;
	}
}

//--------------------------------------------------------------
// StateLayout::Offset
//--------------------------------------------------------------
std::size_t StateLayout::Offset(Physics::State::StateTag tag) const
{
	const auto idx = Index(tag);
	const auto &b = m_blocks[idx];

	if (!b.active)
	{
		throw std::runtime_error("StateLayout::Offset: tag '" +
								 std::string(Physics::State::ToString(tag)) +
								 "' is not active in this layout.");
	}

	return b.offset;
}

//--------------------------------------------------------------
// StateLayout::BlockSize
//--------------------------------------------------------------
std::size_t StateLayout::BlockSize(Physics::State::StateTag tag) const
{
	const auto idx = Index(tag);
	const auto &b = m_blocks[idx];

	if (!b.active)
	{
		throw std::runtime_error("StateLayout::BlockSize: tag '" +
								 std::string(Physics::State::ToString(tag)) +
								 "' is not active in this layout.");
	}

	return b.size;
}

//--------------------------------------------------------------
// StateLayout::GetBlock
//--------------------------------------------------------------
StateLayout::Block StateLayout::GetBlock(Physics::State::StateTag tag) const
{
	const auto idx = Index(tag);
	const auto &b = m_blocks[idx];

	if (!b.active)
	{
		throw std::runtime_error("StateLayout::GetBlock: tag '" +
								 std::string(Physics::State::ToString(tag)) +
								 "' is not active in this layout.");
	}

	return b;
}

} // namespace CompactStar::Physics::Evolution