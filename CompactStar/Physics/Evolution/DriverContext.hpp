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
 * @file DriverContext.hpp
 * @brief Lightweight read-only context passed to evolution drivers.
 *
 * @ingroup PhysicsEvolution
 *
 * This file defines a **non-owning aggregation of static model data**
 * that evolution drivers may need in order to compute RHS contributions.
 *
 * The DriverContext is intentionally:
 *  - **small** (just pointers),
 *  - **read-only** (drivers must not mutate it),
 *  - **stable across time steps** (unlike StateVector),
 *  - **decoupled from EvolutionSystem internals**.
 *
 * This avoids circular dependencies between IDriver and EvolutionSystem,
 * and makes the driver interface explicit and robust.
 */

#ifndef CompactStar_Physics_Evolution_DriverContext_H
#define CompactStar_Physics_Evolution_DriverContext_H

namespace CompactStar
{
namespace Physics
{

namespace Thermal
{
class IEnvelope;
}

namespace Evolution
{

// Forward declarations only — no heavy includes here
class StarContext;
class GeometryCache;
struct Config;
class StateVector;	  ///< Composite view over sub-states (Spin/Thermal/Chem/…)
class RHSAccumulator; ///< Write-only accumulator for dY/dt components
class StateLayout;	  ///< Layout of the StateVector

/**
 * @struct DriverContext
 * @brief Read-only static context for evolution drivers.
 *
 * DriverContext bundles together *all static, slowly-varying objects*
 * that physics drivers may consult when computing dY/dt.
 *
 * It is passed **by const reference** into IDriver::AccumulateRHS().
 *
 * ### Design principles
 * - **Non-owning**: all pointers refer to objects owned elsewhere
 *   (typically by the calling code / EvolutionSystem).
 * - **Read-only**: drivers must treat everything here as immutable.
 * - **Optional components**: pointers may be null if a given feature
 *   is not enabled (e.g. envelope models).
 *
 * ### Typical usage inside a driver
 * @code
 * void MyDriver::AccumulateRHS(..., const DriverContext& ctx) const
 * {
 *     if (ctx.geo)
 *     {
 *         const auto& w = ctx.geo->WC();
 *         ...
 *     }
 *
 *     if (ctx.star)
 *     {
 *         const auto& prof = ctx.star->Profile();
 *         ...
 *     }
 * }
 * @endcode
 */
struct DriverContext
{
	/**
	 * @brief Pointer to the stellar structure context.
	 *
	 * Provides access to:
	 *  - radial grids,
	 *  - metric functions,
	 *  - EOS hooks,
	 *  - composition profiles,
	 *  - any cached background quantities.
	 *
	 * Typically constructed from a solved NStar / TOV profile.
	 *
	 * May be null if a driver does not require structural data.
	 */
	const StarContext *star = nullptr;

	/**
	 * @brief Pointer to precomputed geometric integration weights.
	 *
	 * GeometryCache contains commonly-used metric combinations such as:
	 *  - proper-volume factors,
	 *  - redshifted shell weights,
	 *  - exp(ν), exp(2ν), exp(Λ), etc.
	 *
	 * Drivers performing spatial integrals (cooling, heating, luminosities)
	 * should prefer using GeometryCache over recomputing these factors.
	 *
	 * May be null for purely local / toy drivers.
	 */
	const GeometryCache *geo = nullptr;

	/**
	 * @brief Pointer to thermal envelope / atmosphere model.
	 *
	 * Provides mappings such as:
	 *   T_core → T_surface
	 *   T_surface → photon luminosity
	 *
	 * Optional; may be null if no envelope model is configured.
	 */
	const Thermal::IEnvelope *envelope = nullptr;

	/**
	 * @brief Pointer to global evolution configuration.
	 *
	 * Provides access to:
	 *  - numerical tolerances,
	 *  - enabled physics switches,
	 *  - model-wide flags.
	 *
	 * Drivers should treat this as read-only policy information.
	 */
	const Config *cfg = nullptr;
};

} // namespace Evolution
} // namespace Physics
} // namespace CompactStar

#endif /* CompactStar_Physics_Evolution_DriverContext_H */