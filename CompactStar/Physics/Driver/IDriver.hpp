// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */

/**
 * @file IDriver.hpp
 * @brief Interface for evolution drivers that contribute to the RHS dY/dt.
 *
 * A driver:
 *  - declares which state blocks it reads (DependsOn)
 *  - declares which state blocks it updates (Updates)
 *  - accumulates its contribution to the global RHS via AccumulateRHS(...)
 *
 * Examples:
 *   - Driver::Spin::MagneticDipole      (Spin → Spin)
 *   - Driver::Chem::Rotochemical        (Spin → Chem)
 *   - Driver::Thermal::NeutrinoCooling  (Thermal → Thermal)
 *   - Driver::Thermal::HeatingFromChem  (Chem, Thermal → Thermal)
 *
 * Implementations should be pure functions of (t, Y, ctx, options)
 * and must accumulate (add) into dY/dt, never overwrite it.
 *
 * @ingroup PhysicsDriver
 */

#ifndef CompactStar_Physics_Driver_IDriver_H
#define CompactStar_Physics_Driver_IDriver_H

#include <array>
#include <span>
#include <string>

#include "CompactStar/Physics/State/Tags.hpp" // defines enum class StateTag

namespace CompactStar::Physics
{

// Forward declarations to avoid heavy includes here
namespace Evolution
{
class StateVector;	  ///< Composite view over sub-states (Spin/Thermal/Chem/…)
class RHSAccumulator; ///< Write-only accumulator for dY/dt components
class StarContext;	  ///< Geometry, EOS hooks, microphysics caches, etc.
} // namespace Evolution

/**
 * @class IDriver
 * @brief Abstract interface for all evolution drivers.
 *
 * Drivers contribute terms to the RHS dY/dt for one or more state components.
 * They may *read* other components (dependencies) to compute their update.
 */
class IDriver
{
  public:
	virtual ~IDriver() = default;

	/**
	 * @brief Human-readable name for diagnostics/logging.
	 */
	virtual std::string Name() const = 0;

	/**
	 * @brief Which state blocks this driver reads.
	 *
	 * Return a span to a static array (e.g., `static constexpr std::array<StateTag,2> {...}`).
	 */
	virtual std::span<const State::StateTag> DependsOn() const = 0;

	/**
	 * @brief Which state blocks this driver updates (adds contributions to).
	 *
	 * Return a span to a static array (e.g., `static constexpr std::array<StateTag,1> {...}`).
	 */
	virtual std::span<const State::StateTag> Updates() const = 0;

	/**
	 * @brief Add this driver’s contribution to the global RHS dY/dt.
	 *
	 * Implementations must *accumulate* into @p dYdt (i.e., add), and must
	 * not assume any specific call order beyond what the Evolution graph resolves.
	 *
	 * @param t     Current time (model units; typically seconds).
	 * @param Y     Read-only composite state vector (Spin/Thermal/Chem/…).
	 * @param dYdt  Write-only accumulator for RHS components (add to it).
	 * @param ctx   Read-only star/context data (geometry, EOS, microphysics).
	 */
	virtual void AccumulateRHS(double t,
							   const Evolution::StateVector &Y,
							   Evolution::RHSAccumulator &dYdt,
							   const Evolution::StarContext &ctx) const = 0;
};

} // namespace CompactStar::Physics

#endif /* CompactStar_Physics_Driver_IDriver_H */