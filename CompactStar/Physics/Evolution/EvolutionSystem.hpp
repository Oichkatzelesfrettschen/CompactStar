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
 * @defgroup PhysicsEvolution Physics evolution module
 * @brief Time evolution of neutron-star states (thermal, spin, chem, BNV).
 */
/**
 * @file EvolutionSystem.hpp
 * @brief Right-hand side (RHS) functor for the ODE system dY/dt.
 *
 * EvolutionSystem wires together:
 *  - static model context (StarContext, GeometryCache, envelope, Config),
 *  - the dynamic StateVector (Spin/Thermal/Chem/BNV blocks),
 *  - an RHSAccumulator for dY/dt contributions, and
 *  - a set of physics drivers implementing IDriver.
 *
 * It does **not** implement any physics directly. The workflow is:
 *
 *  1. Unpack the flat y[] into State blocks (StatePacking::UnpackStateVector).
 *  2. Clear RHSAccumulator.
 *  3. For each driver:
 *        driver->AccumulateRHS(t, state, rhs, *ctx.star);
 *  4. Scatter RHSAccumulator back into dydt[]
 *     (StatePacking::ScatterRHSFromAccumulator).
 *
 * Designed to be wrapped by a GSL driver.
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_EvolutionSystem_H
#define CompactStar_Physics_Evolution_EvolutionSystem_H

#include <memory>
#include <vector>

namespace CompactStar
{
namespace Physics
{
// Forward declarations for static context pieces
namespace Thermal
{
class IEnvelope;
}

namespace Evolution
{
class StarContext;
class StateVector;
class GeometryCache;
class RHSAccumulator;
class StateLayout;
struct Config;
} // namespace Evolution

// Driver interface lives at the Physics level (see IDriver.hpp)
class IDriver;

} // namespace Physics
} // namespace CompactStar

namespace CompactStar
{
namespace Physics
{
namespace Evolution
{

// Shared-pointer alias for owning driver instances.
using DriverPtr = std::shared_ptr<Physics::IDriver>;

//==============================================================
//                      EvolutionSystem Class
//==============================================================
/**
 * @class EvolutionSystem
 * @brief GSL-compatible RHS functor for dY/dt.
 *
 * EvolutionSystem owns no physics; it only coordinates:
 *  - unpacking y[] into State blocks,
 *  - invoking each IDriver to accumulate into RHSAccumulator,
 *  - scattering RHS back into dydt[].
 */
class EvolutionSystem
{
  public:
	/**
	 * @brief Bundles read-only references to static model components.
	 */
	struct Context
	{
		const StarContext *star = nullptr;			  ///< geometry, EOS, profiles
		const GeometryCache *geo = nullptr;			  ///< precomputed weights
		const Thermal::IEnvelope *envelope = nullptr; ///< surface T_b → T_s
		const Config *cfg = nullptr;				  ///< evolution configuration
	};

	/**
	 * @brief Construct the RHS functor.
	 *
	 * @param ctx     Static model context (stored by value, pointers non-owning).
	 * @param state   Non-owning reference to logical state blocks.
	 * @param rhs     Non-owning reference to RHS accumulator.
	 * @param layout  Layout describing how tags map into the flat y[]/dydt[].
	 * @param drivers List of physics drivers (owned via DriverPtr).
	 *
	 * Lifetime rules:
	 *  - The StarContext, GeometryCache, IEnvelope, Config pointed to by @p ctx
	 *    must outlive this EvolutionSystem.
	 *  - @p state, @p rhs, and @p layout must outlive this EvolutionSystem.
	 *  - Drivers are owned by EvolutionSystem via the DriverPtr vector.
	 */
	EvolutionSystem(const Context &ctx,
					StateVector &state,
					RHSAccumulator &rhs,
					const StateLayout &layout,
					std::vector<DriverPtr> drivers);

	/**
	 * @brief Evaluate RHS \f$\dot{y} = f(t,y)\f$.
	 *
	 * @param t     Time (s).
	 * @param y     State array (size = layout.TotalSize()).
	 * @param dydt  Output derivatives (same size as @p y).
	 *
	 * @return 0 on success; nonzero on failure (GSL convention).
	 */
	int operator()(double t, const double y[], double dydt[]) const;

  private:
	/// Validate that the static Context is non-null / self-consistent.
	void ValidateContext() const;

	Context m_ctx;					  ///< static model context (non-owning pointers)
	StateVector &m_state;			  ///< logical state blocks (non-owning)
	RHSAccumulator &m_rhs;			  ///< RHS scratch storage (non-owning)
	const StateLayout &m_layout;	  ///< y[] / dydt[] layout (non-owning)
	std::vector<DriverPtr> m_drivers; ///< owned physics drivers
};

} // namespace Evolution
} // namespace Physics
} // namespace CompactStar

#endif /* CompactStar_Physics_Evolution_EvolutionSystem_H */