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
 * @file StateLayout.hpp
 * @brief Mapping between flat ODE vector indices and StateTag blocks.
 *
 * The evolution system uses a single flat ODE vector y[] (and dydt[]) that
 * concatenates all active state blocks (SpinState, ThermalState, ChemState,
 * BNVState, ...). StateLayout records:
 *
 *   - which StateTag blocks are active,
 *   - where each block starts in y[] (offset),
 *   - how many components each block has (size),
 *   - the total dimension of the system.
 *
 * Typical usage:
 *   1) Build and register State blocks in a StateVector.
 *   2) Call StateLayout::Configure(state, {StateTag::Thermal, StateTag::Chem, ...}).
 *   3) Use Offset(tag) and BlockSize(tag) when packing/unpacking y[].
 *
 * This keeps the "wiring" between tagged State blocks and their positions in
 * the global ODE vector explicit and easy to audit.
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_StateLayout_H
#define CompactStar_Physics_Evolution_StateLayout_H

#include <array>
#include <cstddef>
#include <initializer_list>

#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Evolution
{

/**
 * @class StateLayout
 * @brief Records offsets and sizes of each StateTag block in the flat ODE vector.
 *
 * The layout is configured once (typically before the first call to the
 * integrator) and is then treated as immutable during a given evolution run.
 *
 * It does not store any data itself; it only knows how the y[] / dydt[]
 * arrays are partitioned into tagged blocks.
 */
class StateLayout
{
  public:
	// ---------------------------------------------------------------------
	//  Public types
	// ---------------------------------------------------------------------

	/**
	 * @struct Block
	 * @brief Layout information for a single StateTag block.
	 *
	 * - offset : starting index in y[] for this block
	 * - size   : number of scalar components in this block
	 * - active : whether this block participates in the ODE system
	 */
	struct Block
	{
		std::size_t offset = 0;
		std::size_t size = 0;
		bool active = false;
	};

	// ---------------------------------------------------------------------
	//  Ctors
	// ---------------------------------------------------------------------

	/// Default constructor: all blocks inactive, total size = 0.
	StateLayout() = default;

	// ---------------------------------------------------------------------
	//  Configuration
	// ---------------------------------------------------------------------

	/**
	 * @brief Configure layout from a StateVector and an ordered list of tags.
	 *
	 * The tags in @p tags define the concatenation order in the flat ODE
	 * vector y[]. For each tag:
	 *
	 *   - StateVector::Get(tag).Size() is queried to determine block size;
	 *   - a contiguous block [offset, offset+size) is assigned in y[];
	 *   - the internal Block entry is marked active.
	 *
	 * Blocks for tags not listed in @p tags remain inactive.
	 *
	 * @param state  Composite view over registered State blocks.
	 * @param tags   Ordered list of StateTag values to include in y[].
	 *
	 * @throws std::runtime_error if any requested tag is not registered in
	 *         the provided StateVector.
	 */
	void Configure(const StateVector &state,
				   std::initializer_list<Physics::State::StateTag> tags);

	// ---------------------------------------------------------------------
	//  Queries
	// ---------------------------------------------------------------------

	/// Total dimension of the flat ODE vector y[] after configuration.
	[[nodiscard]] std::size_t TotalSize() const noexcept { return m_totalSize; }

	/// Check whether a given tag is active in this layout.
	[[nodiscard]] bool IsActive(Physics::State::StateTag tag) const noexcept
	{
		return m_blocks[Index(tag)].active;
	}

	/**
	 * @brief Starting offset in y[] for the block with the given tag.
	 *
	 * @throws std::runtime_error if the tag is not active.
	 */
	[[nodiscard]] std::size_t Offset(Physics::State::StateTag tag) const;

	/**
	 * @brief Number of scalar components in the block with the given tag.
	 *
	 * @throws std::runtime_error if the tag is not active.
	 */
	[[nodiscard]] std::size_t BlockSize(Physics::State::StateTag tag) const;

	/**
	 * @brief Get the full Block descriptor for @p tag.
	 *
	 * @throws std::runtime_error if the tag is not active.
	 */
	[[nodiscard]] Block GetBlock(Physics::State::StateTag tag) const;

  private:
	// ---------------------------------------------------------------------
	//  Internal helpers / data
	// ---------------------------------------------------------------------

	/// Number of StateTag enumeration values.
	static constexpr std::size_t NumTags =
		static_cast<std::size_t>(Physics::State::StateTag::Custom) + 1;

	/// Convert a StateTag to an array index.
	static constexpr std::size_t Index(Physics::State::StateTag tag)
	{
		return static_cast<std::size_t>(tag);
	}

	/// Per-tag layout information (inactive by default).
	std::array<Block, NumTags> m_blocks{};

	/// Total dimension of the flat ODE system.
	std::size_t m_totalSize = 0;
};

} // namespace CompactStar::Physics::Evolution

#endif /* CompactStar_Physics_Evolution_StateLayout_H */