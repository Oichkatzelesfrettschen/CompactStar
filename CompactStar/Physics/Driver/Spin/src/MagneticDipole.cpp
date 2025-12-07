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

// -----------------------------------------------------------------------------
//  MagneticDipole.cpp
// -----------------------------------------------------------------------------
//  Implementation of the magnetic dipole spin-down driver.
//
//  This driver implements a simple torque law of the form
//
//      dΩ/dt = -K_eff * Ω^n
//
//  where K_eff and n are provided via MagneticDipole::Options.
//  In this first implementation, K_eff is taken directly from the options;
//  later versions may construct K_eff from B, R, I, etc. using StarContext.
// -----------------------------------------------------------------------------

#include "CompactStar/Physics/Driver/Spin/MagneticDipole.hpp"
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"

#include <cmath> // std::pow, std::abs

#include <Zaki/Util/Instrumentor.hpp> // Z_LOG_INFO/WARNING/ERROR, PROFILE_FUNCTION

// -----------------------------------------------------------------------------
//  Namespace
// -----------------------------------------------------------------------------

namespace CompactStar::Physics::Driver::Spin
{

// -----------------------------------------------------------------------------
//  MagneticDipole::AccumulateRHS
// -----------------------------------------------------------------------------
/**
 * @brief Add magnetic-dipole spin-down contribution to dY/dt.
 *
 * The current implementation assumes:
 *   - The evolved spin degree of freedom is the angular frequency Ω,
 *     stored in component 0 of the Spin state block (`SpinState::Omega()`).
 *   - The torque law is:
 *         dΩ/dt = -K_eff * |Ω|^n * sign(Ω)
 *     where:
 *         K_eff = opts_.K_prefactor
 *         n     = opts_.braking_index
 *
 * Notes:
 *   - If `opts_.K_prefactor == 0`, the driver does nothing.
 *   - If the Spin block has zero components (`NumComponents()==0`), the
 *     driver logs a warning and returns.
 *   - `StarContext` is currently unused; in a more detailed model we may
 *     use it to construct K_eff from I(M,R), B, etc.
 *
 * @param t     Current time (model units; typically seconds).
 * @param Y     Read-only composite state vector (must provide access to SpinState).
 * @param dYdt  Write-only accumulator for RHS components; we add to the Spin block.
 * @param ctx   Read-only star/context data (currently unused in this minimal model).
 */
void MagneticDipole::AccumulateRHS(double t,
								   const Evolution::StateVector &Y,
								   Evolution::RHSAccumulator &dYdt,
								   const Evolution::StarContext &ctx) const
{
	PROFILE_FUNCTION();

	(void)t;   // unused in this simple model (could support explicit time-dependence later)
	(void)ctx; // reserved for future use (e.g., I(M,R), B-field geometry)

	// -------------------------------------------------------------------------
	//  1. Early exit if the prefactor is zero
	// -------------------------------------------------------------------------
	if (opts_.K_prefactor == 0.0)
	{
		// No torque configured; nothing to do.
		return;
	}

	// -------------------------------------------------------------------------
	//  2. Extract the SpinState from the composite state vector
	// -------------------------------------------------------------------------
	//
	// We assume Evolution::StateVector provides an accessor returning a
	// const reference to the SpinState. The exact method name is part of
	// the Evolution API contract and should be implemented accordingly.
	//
	// Example (to be defined in Evolution::StateVector):
	//   const Physics::State::SpinState &GetSpin() const;
	//
	const Physics::State::SpinState &spin = Y.GetSpin();

	if (spin.NumComponents() == 0)
	{
		Z_LOG_WARNING("MagneticDipole::AccumulateRHS: SpinState has zero components; "
					  "driver is effectively disabled.");
		return;
	}

	// -------------------------------------------------------------------------
	//  3. Read the evolved spin DOF (Ω) and compute dΩ/dt
	// -------------------------------------------------------------------------
	const double Omega = spin.Omega(); // component 0 by convention

	const double n = opts_.braking_index;
	double K_eff = opts_.K_prefactor;

	// NOTE: If we later decide to use moment of inertia from the context,
	// this is where we would modify K_eff. For now, we simply honor the
	// value configured in Options.
	if (opts_.use_moment_of_inertia)
	{
		// Placeholder for future use of ctx (I, R, B, geometry).
		// Example sketch (not implemented here):
		//   double I = ctx.MomentOfInertia();
		//   if (I > 0.0) K_eff /= I;
		Z_LOG_INFO("MagneticDipole::AccumulateRHS: use_moment_of_inertia=true, "
				   "but no context-based scaling is implemented yet.");
	}

	// If Ω = 0, the torque vanishes (no update needed).
	const double absOmega = std::abs(Omega);
	if (absOmega == 0.0)
	{
		return;
	}

	// Protect against non-integer braking indices and negative Ω:
	//   |Ω|^n * sign(Ω) → behaves like Ω^n for odd integer n,
	//   but remains real-valued for any real n.
	const double sign = (Omega >= 0.0) ? 1.0 : -1.0;
	const double dOmega_dt = -K_eff * sign * std::pow(absOmega, n);

	// -------------------------------------------------------------------------
	//  4. Accumulate into the Spin block of dY/dt
	// -------------------------------------------------------------------------
	//
	// We assume RHSAccumulator exposes a generic AddTo method of the form:
	//
	//   void AddTo(State::StateTag tag, std::size_t component, double value);
	//
	// By convention, component 0 of the Spin block is Ω.
	//
	dYdt.AddTo(State::StateTag::Spin, 0, dOmega_dt);
}

// -----------------------------------------------------------------------------
//  End namespace
// -----------------------------------------------------------------------------

} // namespace CompactStar::Physics::Driver::Spin