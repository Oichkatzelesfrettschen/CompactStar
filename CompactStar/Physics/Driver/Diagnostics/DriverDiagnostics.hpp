#pragma once
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
 * @file DriverDiagnostics.hpp
 * @brief Optional diagnostics interface for Physics drivers.
 *
 * This header defines a small, non-intrusive interface that any driver can
 * implement to provide a structured diagnostics snapshot at runtime.
 *
 * Motivation:
 *  - Avoid ad-hoc debug prints scattered across drivers.
 *  - Provide deterministic JSON/JSONL records for tooling and regression tests.
 *  - Centralize "unit contract" lines per driver (conventions & assumptions).
 *
 * Design constraints:
 *  - Numeric payload is still plain doubles.
 *  - Diagnostics are intended to be called rarely (e.g., t=0 or every N steps),
 *    so they should not influence performance.
 *
 * Usage:
 *  - A driver may implement IDriverDiagnostics.
 *  - An observer (DiagnosticsObserver) queries drivers that implement this
 *    interface and writes results to JSONL.
 */

#include <string>

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/UnitContract.hpp"

// Forward declarations to avoid heavy includes here.
namespace CompactStar::Physics::Evolution
{
class StateVector;
class DriverContext;
} // namespace CompactStar::Physics::Evolution

namespace CompactStar::Physics::Driver::Diagnostics
{

/**
 * @brief Interface that a driver can implement to expose diagnostics.
 *
 * This is intentionally orthogonal to IDriver's runtime API:
 *  - If a driver doesn't implement it, nothing changes.
 *  - If it does, an observer can call DiagnoseSnapshot() and UnitContract().
 */
class IDriverDiagnostics
{
  public:
	virtual ~IDriverDiagnostics() = default;

	/**
	 * @brief Human-readable stable name for this driver instance.
	 *
	 * Example: "PhotonCooling"
	 * If you have multiple instances (different options), you can include an
	 * instance suffix (e.g., "PhotonCooling#0") but keep it stable for tooling.
	 */
	[[nodiscard]] virtual std::string DiagnosticsName() const = 0;

	/**
	 * @brief Return unit conventions and assumptions for this driver.
	 *
	 * This is intended to:
	 *  - be stable and rarely change,
	 *  - NOT duplicate computations,
	 *  - document inputs/outputs and required unit systems.
	 */
	[[nodiscard]] virtual Evolution::Diagnostics::UnitContract UnitContract() const = 0;

	/**
	 * @brief Fill a packet with driver-specific diagnostics at (t, Y, ctx).
	 *
	 * This method should:
	 *  - only *read* state and cache (no mutation),
	 *  - be safe even if some caches are missing (log as notes/warnings),
	 *  - prefer calling the same internal helper functions as the physics path
	 *    to avoid drift between the code and the diagnostics.
	 *
	 * @param t Simulation time [same unit as integrator uses].
	 * @param Y Composite state vector (read-only).
	 * @param ctx Driver context (geometry/cache handles, star pointer, etc).
	 * @param out Packet to be filled/overwritten by this method.
	 */
	virtual void DiagnoseSnapshot(double t,
								  const Evolution::StateVector &Y,
								  const Evolution::DriverContext &ctx,
								  Evolution::Diagnostics::DiagnosticPacket &out) const = 0;
};

} // namespace CompactStar::Physics::Driver::Diagnostics