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
 * @file RHSAccumulator.hpp
 * @brief Helper for accumulating contributions to dY/dt per state block.
 *
 * This class stores a tagged collection of RHS segments, one per StateTag.
 * Drivers add contributions using (tag, component) indices, and the evolution
 * system can later apply these to the corresponding State objects.
 *
 * Design goals:
 *  - Tag-aware accumulation: AddTo(StateTag, component, value).
 *  - Configurable block sizes per tag.
 *  - Zero-cost clear between RHS evaluations.
 *
 * In a typical evolution setup:
 *  - System calls Configure(tag, size) for each active state block.
 *  - Before assembling the RHS, System calls Clear().
 *  - Each driver adds its contributions via AddTo(...).
 *  - Once all drivers have run, System uses Block(tag) to access the
 *    assembled RHS for that state and update the associated State object.
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_RHSAccumulator_H
#define CompactStar_Physics_Evolution_RHSAccumulator_H

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Evolution
{

// -----------------------------------------------------------------------------
//  RHSAccumulator
// -----------------------------------------------------------------------------

/**
 * @class RHSAccumulator
 * @brief Write-only accumulator for dY/dt components, keyed by StateTag.
 *
 * Internally, this class maintains a small array of "blocks", one per
 * StateTag. Each block is a vector<double> of length equal to the number
 * of DOFs for that state. Drivers contribute via AddTo(tag, i, value),
 * and the evolution system reads the final RHS via Block(tag).
 */
class RHSAccumulator
{
  public:
	// ---------------------------------------------------------------------
	//  Ctors / Dtor
	// ---------------------------------------------------------------------

	/// Construct an empty accumulator (no blocks configured).
	RHSAccumulator()
	{
		for (auto &b : blocks_)
		{
			b.configured = false;
		}
	}

	/// Default destructor.
	~RHSAccumulator() = default;

	// ---------------------------------------------------------------------
	//  Configuration
	// ---------------------------------------------------------------------

	/**
	 * @brief Configure the RHS block size for a given state tag.
	 *
	 * @param tag   StateTag to configure.
	 * @param size  Number of scalar components in this state's RHS block.
	 *
	 * Calling Configure() overwrites any previous configuration for @p tag.
	 * The data are initialized to zero.
	 */
	void Configure(Physics::State::StateTag tag, std::size_t size)
	{
		auto &block = blocks_[Index(tag)];
		block.data.assign(size, 0.0);
		block.configured = true;
	}

	/**
	 * @brief Check whether a block for @p tag has been configured.
	 */
	[[nodiscard]] bool IsConfigured(Physics::State::StateTag tag) const
	{
		return blocks_[Index(tag)].configured;
	}

	// ---------------------------------------------------------------------
	//  Accumulation
	// ---------------------------------------------------------------------

	/**
	 * @brief Add a contribution to the RHS of a given state component.
	 *
	 * @param tag       Which state block this contribution belongs to.
	 * @param component Component index within that block.
	 * @param value     Value to add.
	 *
	 * @throws std::runtime_error if the tag has not been configured or
	 *         the component index is out of range.
	 */
	void AddTo(Physics::State::StateTag tag, std::size_t component, double value)
	{
		auto &block = blocks_[Index(tag)];
		if (!block.configured)
		{
			throw std::runtime_error("RHSAccumulator::AddTo: tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "' not configured.");
		}
		if (component >= block.data.size())
		{
			throw std::runtime_error("RHSAccumulator::AddTo: component index out of range.");
		}
		block.data[component] += value;
	}

	/**
	 * @brief Reset all configured blocks to zero.
	 *
	 * This does not change the sizes or configuration flags; it just
	 * zeroes the stored RHS values.
	 */
	void Clear()
	{
		for (auto &block : blocks_)
		{
			if (block.configured)
			{
				for (auto &v : block.data)
				{
					v = 0.0;
				}
			}
		}
	}

	// ---------------------------------------------------------------------
	//  Accessors
	// ---------------------------------------------------------------------

	/**
	 * @brief Read-only access to the RHS block for a given tag.
	 *
	 * @param tag  StateTag whose RHS block is requested.
	 * @return const reference to the internal vector<double> for this tag.
	 *
	 * @throws std::runtime_error if the tag has not been configured.
	 */
	[[nodiscard]] const std::vector<double> &Block(Physics::State::StateTag tag) const
	{
		const auto &block = blocks_[Index(tag)];
		if (!block.configured)
		{
			throw std::runtime_error("RHSAccumulator::Block(const): tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "' not configured.");
		}
		return block.data;
	}

	/**
	 * @brief Mutable access to the RHS block for a given tag.
	 *
	 * @param tag  StateTag whose RHS block is requested.
	 * @return reference to the internal vector<double> for this tag.
	 *
	 * @throws std::runtime_error if the tag has not been configured.
	 */
	std::vector<double> &Block(Physics::State::StateTag tag)
	{
		auto &block = blocks_[Index(tag)];
		if (!block.configured)
		{
			throw std::runtime_error("RHSAccumulator::Block: tag '" +
									 std::string(Physics::State::ToString(tag)) +
									 "' not configured.");
		}
		return block.data;
	}

  private:
	// ---------------------------------------------------------------------
	//  Internal structures / helpers
	// ---------------------------------------------------------------------

	// Renamed to avoid collision with Block(...) member functions.
	struct BlockStorage
	{
		std::vector<double> data;
		bool configured = false;
	};

	/// Number of StateTag enumerators (Spin, Thermal, Chem, …, Custom).
	static constexpr std::size_t NumTags =
		static_cast<std::size_t>(Physics::State::StateTag::Custom) + 1;

	/// Convert a StateTag to an array index.
	static constexpr std::size_t Index(Physics::State::StateTag tag)
	{
		return static_cast<std::size_t>(tag);
	}

	/// One block per StateTag.
	std::array<BlockStorage, NumTags> blocks_;
};

} // namespace CompactStar::Physics::Evolution

#endif /* CompactStar_Physics_Evolution_RHSAccumulator_H */