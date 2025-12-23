#ifndef CompactStar_Physics_Driver_Thermal_Boundary_SurfaceGravity_H
#define CompactStar_Physics_Driver_Thermal_Boundary_SurfaceGravity_H

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
 * @file SurfaceGravity.hpp
 * @brief Helpers to compute neutron-star surface gravity in the "g14" convention.
 *
 * The envelope literature typically parameterizes the surface gravity as:
 *
 *   g14 ≡ g / (1e14 cm s^-2)
 *
 * and then provides Ts(Tb, g14, ...).
 *
 * In GR, a commonly used effective surface gravity is:
 *
 *   g_s = (G M / R^2) * (1 - 2GM/(Rc^2))^{-1/2}
 *
 * (equivalently g_s = GM e^{-ν(R)} / R^2 with e^{ν(R)} = sqrt(1-2GM/Rc^2)
 * for a Schwarzschild exterior).
 *
 * CompactStar stores structure in geometric units internally; this helper avoids
 * forcing any particular unit system into the envelope code. It computes g14
 * from StarContext/GeometryCache using the metric and/or compactness.
 *
 * Requirements:
 * - StarContext provides surface radius R (km) and gravitational mass M (Msun or km),
 *   OR provides exterior metric factor at the surface (e^{ν(R)} or e^{2ν(R)}).
 * - If GeometryCache is available, it may provide R and exp(2ν) at the surface.
 *
 * If your StarContext API differs, adapt the small “adapter” functions in the .hpp.
 */

#include <cmath>
#include <stdexcept>

namespace CompactStar::Physics::Evolution
{
class StarContext;
class GeometryCache;
} // namespace CompactStar::Physics::Evolution

namespace CompactStar::Physics::Driver::Thermal::Boundary
{

/**
 * @brief Return g14 = g / (1e14 cm s^-2).
 *
 * This tries (in order):
 *  1) Use GeometryCache if it exposes R and exp(2ν) at the surface.
 *  2) Use StarContext profile view if it exposes R and ν(R) / exp(ν(R)).
 *  3) Fallback: if only compactness C = 2GM/(Rc^2) is available, use
 *     g_s = GM/R^2 / sqrt(1 - C).
 *
 * @param star StarContext providing mass/radius/metric info.
 * @param geo  Optional GeometryCache for cached geometry at the surface.
 *
 * @return Surface gravity g14 in units of 1e14 cm s^-2.
 *
 * @throws std::runtime_error if required quantities cannot be obtained.
 *
 * @note Units:
 * - Input R is assumed in km.
 * - Mass is assumed in km if provided as GM/c^2.
 *   This helper uses explicit conversion constants below.
 */
double SurfaceGravity_g14(const Evolution::StarContext &star,
						  const Evolution::GeometryCache *geo = nullptr);

/**
 * @brief Convenience: compute g (cgs) from g14.
 * @param g14 Surface gravity in units of 1e14 cm s^-2.
 * @return Surface gravity g in cm s^-2.
 */
inline double g_cgs_from_g14(double g14)
{
	return g14 * 1.0e14; // cm s^-2
}

} // namespace CompactStar::Physics::Driver::Thermal::Boundary

#endif /* CompactStar_Physics_Driver_Thermal_Boundary_SurfaceGravity_H */