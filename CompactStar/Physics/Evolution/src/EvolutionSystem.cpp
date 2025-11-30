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

// Pull in the full definitions for the types we only forward-declared
// in the header. This keeps the header light but allows us to actually
// use the classes here.
#include "CompactStar/Physics/Evolution/EvolutionConfig.hpp"
#include "CompactStar/Physics/Evolution/GeometryCache.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/Thermal.hpp"

#include <cstddef>
#include <stdexcept>

namespace CompactStar::Physics::Evolution
{

//--------------------------------------------------------------
// EvolutionSystem::EvolutionSystem
//--------------------------------------------------------------
/**
 * @brief Construct the RHS functor with a static model context and
 *        references to the dynamic state / RHS helpers.
 *
 * @param ctx   Static model context (star structure, geometry, envelope, config).
 * @param state Non-owning reference to the current StateVector
 *              (how we interpret the flat y[] array).
 * @param rhs   Non-owning reference to an RHSAccumulator used as
 *              scratch storage for dY/dt contributions.
 *
 * Ownership / lifetime:
 *  - The EvolutionSystem stores the @p ctx by value (pointers inside it
 *    must remain valid for the lifetime of the EvolutionSystem).
 *  - @p state and @p rhs are stored as references; the caller is responsible
 *    for ensuring they outlive the EvolutionSystem.
 *
 * At this stage (Phase 0 / scaffolding), we only perform basic validation
 * of the context pointers and store the references. The actual physics
 * will be implemented once the driver / rate infrastructure is in place.
 */
EvolutionSystem::EvolutionSystem(const Context &ctx,
								 StateVector &state,
								 RHSAccumulator &rhs)
	: m_ctx(ctx),
	  m_state(state),
	  m_rhs(rhs)
{
	// Basic safety checks: it's better to fail early with a clear message
	// than to segfault later during an RHS evaluation.

	if (!m_ctx.star)
	{
		throw std::runtime_error("EvolutionSystem: Context.star must not be null.");
	}
	if (!m_ctx.geo)
	{
		throw std::runtime_error("EvolutionSystem: Context.geo must not be null.");
	}
	if (!m_ctx.envelope)
	{
		throw std::runtime_error("EvolutionSystem: Context.envelope must not be null.");
	}
	if (!m_ctx.cfg)
	{
		throw std::runtime_error("EvolutionSystem: Context.cfg must not be null.");
	}

	// NOTE:
	// - We do NOT touch m_state or m_rhs here. They are assumed to have been
	//   configured by the calling code (System/Driver) before the first call
	//   to operator().
	// - The mapping between the flat y[] array and the StateVector blocks is
	//   also the responsibility of the higher-level driver for now.
}

//--------------------------------------------------------------
// EvolutionSystem::operator()
//--------------------------------------------------------------
/**
 * @brief Evaluate RHS \f$\dot{y} = f(t,y)\f$.
 *
 * This method is designed to be wrapped by a GSL ODE driver. Its signature
 * mirrors the usual callback:
 *
 *   - @p t     is the current time (s),
 *   - @p y     is the current state vector (flat array),
 *   - @p dydt  is where the derivatives are written.
 *
 * In the *final* implementation, the steps will roughly be:
 *
 *  1. Decode @p y into the sub-states via StateVector (Spin/Thermal/Chem/BNV,…).
 *  2. Use GeometryCache and StarContext to build any per-call radial
 *     quantities (e.g., redshifted temperatures).
 *  3. Clear RHSAccumulator and let each driver accumulate its contribution
 *     to the appropriate tagged block (thermal, chem, spin…).
 *  4. Project the accumulated RHS back into the flat @p dydt array
 *     in the same layout used by the integrator.
 *
 * For the current Phase 0, we provide a safe, minimal implementation that:
 *  - computes the expected size of the state vector from Config,
 *  - zeroes out @p dydt (no evolution),
 *  - leaves detailed physics as a TODO.
 *
 * @param t     Time (s).
 * @param y     State array (size = 1 + n_eta + [Omega] depending on Config).
 * @param dydt  Output derivatives (same size as @p y).
 *
 * @return 0 if success; nonzero on failure (consistent with GSL conventions).
 */
int EvolutionSystem::operator()(double t, const double y[], double dydt[]) const
{
	(void)t; // unused in the stub implementation, but kept to match interface.
	(void)y; // likewise; actual decoding comes later.

	// Sanity check: config must still be valid.
	if (!m_ctx.cfg)
	{
		// This should have already been enforced in the constructor, but we
		// check again defensively in case of misuse.
		return -1;
	}

	const Config &cfg = *m_ctx.cfg;

	// ---------------------------------------------------------------------
	//  Determine the expected size of the state vector
	// ---------------------------------------------------------------------
	//
	// Layout (by convention for now):
	//   index 0                : T^∞ (redshifted internal temperature)
	//   indices 1 .. n_eta     : η_i components
	//   optional last component: Ω (spin) if cfg.couple_spin == true
	//
	// We do NOT have the length n passed in explicitly (like GSL does),
	// so we infer it from the configuration parameters.
	std::size_t n_eta = cfg.n_eta;
	std::size_t n_state = 1u + n_eta + (cfg.couple_spin ? 1u : 0u);

	// ---------------------------------------------------------------------
	//  Zero-out RHS as a safe default
	// ---------------------------------------------------------------------
	//
	// NOTE: In the full implementation, we will:
	//  - Map y[] -> StateVector blocks,
	//  - Use GeometryCache, StarContext, and envelope to compute the RHS,
	//  - Fill m_rhs via tagged contributions,
	//  - Then flatten m_rhs back into dydt[].
	//
	// For now, we just set all derivatives to zero. This is harmless for
	// unit tests that only exercise infrastructure, and it clearly signals
	// that the physics has not yet been wired in.
	for (std::size_t i = 0; i < n_state; ++i)
	{
		dydt[i] = 0.0;
	}

	// ---------------------------------------------------------------------
	//  Placeholder for future physics
	// ---------------------------------------------------------------------
	//
	// Future steps may look like:
	//
	//  // 1) Decode the flat y[] into the StateVector blocks:
	//  //    e.g., m_state.GetThermal().FromFlatSegment(y + offset_Tinf);
	//  //          m_state.GetChem().FromFlatSegment(y + offset_eta);
	//  //          (etc., once State API is defined).
	//
	//  // 2) Clear RHS accumulator before adding new contributions.
	//  m_rhs.Clear();
	//
	//  // 3) Call each driver to accumulate its tagged contributions:
	//  //    thermalDriver.AccumulateRHS(t, m_ctx, m_state, m_rhs);
	//  //    chemDriver.AccumulateRHS(t, m_ctx, m_state, m_rhs);
	//  //    spinDriver.AccumulateRHS(t, m_ctx, m_state, m_rhs);
	//  //    ...
	//
	//  // 4) Flatten the RHSAccumulator blocks back into the flat dydt[]:
	//  //    - thermal block -> dT^∞/dt (with appropriate projection)
	//  //    - chem block    -> dη_i/dt
	//  //    - spin block    -> dΩ/dt (if coupled)
	//
	// Until those pieces are ready, we simply return 0 to indicate
	// successful evaluation with zero RHS.
	//

	return 0;
}

} // namespace CompactStar::Physics::Evolution