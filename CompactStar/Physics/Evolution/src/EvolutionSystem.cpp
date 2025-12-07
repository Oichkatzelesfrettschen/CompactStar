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
 * @file EvolutionSystem.cpp
 * @brief Implementation of EvolutionSystem (RHS functor for dY/dt).
 */

#include "CompactStar/Physics/Evolution/EvolutionSystem.hpp"

#include <stdexcept>

#include "CompactStar/Physics/Driver/IDriver.hpp"
#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp"
#include "CompactStar/Physics/Evolution/GeometryCache.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateLayout.hpp"
#include "CompactStar/Physics/Evolution/StatePacking.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"

namespace CompactStar::Physics::Evolution
{

//--------------------------------------------------------------
// EvolutionSystem::EvolutionSystem
//--------------------------------------------------------------
EvolutionSystem::EvolutionSystem(const Context &ctx,
								 StateVector &state,
								 RHSAccumulator &rhs,
								 const StateLayout &layout,
								 std::vector<DriverPtr> drivers)
	: m_ctx(ctx),
	  m_state(state),
	  m_rhs(rhs),
	  m_layout(layout),
	  m_drivers(std::move(drivers))
{
	// Sanity-check the static context up front so misconfigurations
	// fail early and loudly.
	ValidateContext();

	// Ensure we have at least one physics driver.
	if (m_drivers.empty())
	{
		throw std::runtime_error(
			"EvolutionSystem: constructed with no physics drivers. "
			"At least one IDriver must be provided.");
	}
}

//--------------------------------------------------------------
// EvolutionSystem::ValidateContext
//--------------------------------------------------------------
void EvolutionSystem::ValidateContext() const
{
	// Check that all required context pointers are non-null.
	// if (!m_ctx.star)
	// {
	// 	throw std::runtime_error("EvolutionSystem: Context.star is null.");
	// }
	// if (!m_ctx.geo)
	// {
	// 	throw std::runtime_error("EvolutionSystem: Context.geo is null.");
	// }
	// if (!m_ctx.envelope)
	// {
	// 	throw std::runtime_error("EvolutionSystem: Context.envelope is null.");
	// }
	// if (!m_ctx.cfg)
	// {
	// 	throw std::runtime_error("EvolutionSystem: Context.cfg is null.");
	// }

	// Additional invariants could be checked here, e.g.:
	//  - that StateLayout::TotalSize() matches what the integrator expects,
	//  - consistency between Config and State/StateLayout sizes.
}

//--------------------------------------------------------------
// EvolutionSystem::operator()
//--------------------------------------------------------------
int EvolutionSystem::operator()(double t, const double y[], double dydt[]) const
{
	// 1) Unpack flat y[] into logical State blocks.
	//
	// This uses the free helper defined in StatePacking.hpp:
	//   UnpackStateVector(StateVector&, const StateLayout&, const double*)
	UnpackStateVector(m_state, m_layout, y);

	// 2) Clear RHS accumulator before driver contributions.
	m_rhs.Clear();

	// 3) Let each physics driver accumulate its contribution to dY/dt.
	//
	// Drivers are responsible for:
	//   - reading whatever parts of m_state they depend on,
	//   - using the StarContext (geometry, EOS, etc.) as needed,
	//   - adding their contributions into m_rhs via AddTo(...).
	const StarContext &starCtx = *m_ctx.star;

	for (const auto &drv : m_drivers)
	{
		if (!drv)
		{
			throw std::runtime_error(
				"EvolutionSystem::operator(): encountered null driver pointer.");
		}

		drv->AccumulateRHS(t, m_state, m_rhs, starCtx);
	}

	// 4) Scatter accumulated RHS blocks back into the flat dydt[] array.
	//
	// This uses the helper from StatePacking.hpp:
	//   ScatterRHSFromAccumulator(const RHSAccumulator&, const StateLayout&, double*)
	ScatterRHSFromAccumulator(m_rhs, m_layout, dydt);

	// GSL convention: return 0 for success.
	return 0;
}

} // namespace CompactStar::Physics::Evolution