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
 * @file SurfaceGravity.cpp
 * @brief Implementation of relativistic surface gravity helper for thermal boundary models.
 *
 * @ingroup PhysicsDriverThermalBoundary
 *
 * ## Purpose
 * This file implements a utility function for computing the **relativistic surface
 * gravity** of a compact star, expressed in the conventional dimensionless unit
 *
 * \f[
 *   g_{14} \equiv \frac{g}{10^{14}\;\mathrm{cm\,s^{-2}}}.
 * \f]
 *
 * This quantity is required by most neutron-star envelope (heat-blanket)
 * prescriptions, including the Potekhin–Chabrier models, which parameterize
 * the \( T_b \rightarrow T_s \) relation in terms of \( g_{14} \).
 *
 * The implementation is deliberately placed in the **Thermal/Boundary** layer,
 * as surface gravity is:
 * - independent of thermal microphysics,
 * - derived purely from background structure,
 * - shared across envelope models and photon-cooling drivers.
 *
 * ## Physical conventions
 *
 * ### Metric and units
 * We assume a static, spherically symmetric metric of the form:
 *
 * \f[
 *   ds^2 = -e^{2\nu(r)} dt^2 + e^{2\lambda(r)} dr^2 + r^2 d\Omega^2 .
 * \f]
 *
 * The surface gravity for a relativistic star is:
 *
 * \f[
 *   g = \frac{GM}{R^2} \frac{1}{\sqrt{1 - 2GM/(Rc^2)}} .
 * \f]
 *
 * Using the metric convention above, this can be written compactly as:
 *
 * \f[
 *   g = \frac{GM}{R^2 \, e^{\nu(R)}} .
 * \f]
 *
 * ### Geometric mass convention (IMPORTANT)
 * Throughout CompactStar:
 * - Mass is stored in **geometric length units**:
 *
 * \f[
 *   M_{\rm geom} \equiv \frac{GM}{c^2},
 * \f]
 *
 * expressed in **kilometers (km)**.
 *
 * Therefore:
 * - No factor of \(G\) appears explicitly,
 * - Conversion to cgs acceleration requires **only multiplying by \(c^2\)**.
 *
 * ### Final formula used
 * With:
 * - \(R\) in km,
 * - \(M\) in km (geometric),
 * - \(e^{\nu(R)}\) dimensionless,
 *
 * the surface gravity in cgs units is computed as:
 *
 * \f[
 *   g_{\rm cgs}
 *   = \frac{c^2 \, M_{\rm cm}}{R_{\rm cm}^2 \, e^{\nu(R)}},
 * \f]
 *
 * where:
 * - \(M_{\rm cm} = M_{\rm km} \times 10^5\),
 * - \(R_{\rm cm} = R_{\rm km} \times 10^5\).
 *
 * The returned value is:
 *
 * \f[
 *   g_{14} = g_{\rm cgs} / 10^{14}.
 * \f]
 *
 * ## Data sources
 * The function prefers to read quantities from `GeometryCache` when available:
 * - surface radius \(R\),
 * - geometric mass \(M\),
 * - redshift factor \(e^{\nu(R)}\).
 *
 * If `GeometryCache` is not provided, it falls back to `StarContext`.
 *
 * ## Validation and safety checks
 * The implementation performs:
 * - strict positivity checks on \(R\), \(M\), and \(e^{\nu}\),
 * - a compactness check \(R > 2M\) to avoid horizon-level configurations.
 *
 * Violations result in a `std::runtime_error`, as such cases indicate
 * a broken background model rather than a recoverable runtime condition.
 *
 * ## Scope
 * This file:
 * - does **not** depend on EOS or thermal microphysics,
 * - does **not** cache results,
 * - is suitable for repeated calls during evolution.
 *
 * Any future extensions (e.g. rotation corrections) should live in a
 * separate helper to preserve clarity and correctness.
 */
#include <cmath>
#include <stdexcept>

#include "CompactStar/Physics/Driver/Thermal/Boundary/SurfaceGravity.hpp"

#include "CompactStar/Physics/Evolution/GeometryCache.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"

namespace CompactStar::Physics::Driver::Thermal::Boundary
{
// -------------------------------------------------------
namespace detail
{
// Unit conversions / constants (cgs)
constexpr double km_to_cm = 1.0e5;		// 1 km = 1e5 cm
constexpr double c_cgs = 2.99792458e10; // cm s^-1
constexpr double g14_unit = 1.0e14;		// cm s^-2
// -------------------------------------------------------
/**
 * @brief Convert geometric length in km to cm.
 */
inline double KmToCm(double x_km) { return x_km * km_to_cm; }
// -------------------------------------------------------
/**
 * @brief Safe positivity check (strict).
 */
inline bool Pos(double x) { return x > 0.0 && std::isfinite(x); }

} // namespace detail
// -------------------------------------------------------
/**
 * @brief Surface gravity in g14 units: g14 ≡ g / (1e14 cm s^-2).
 *
 * Assumptions / conventions:
 * - R_km is the circumferential radius at the surface [km].
 * - M_km is the gravitational mass in geometric length units [km], i.e. M_km = GM/c^2 expressed in km.
 * - expnu is exp(ν(R)), dimensionless. For an exterior Schwarzschild metric expnu = sqrt(1 - 2 M_km / R_km).
 *
 * Then:
 *   g_cgs = c^2 * M_cm / (R_cm^2 * expnu)
 *
 * which matches:
 *   g = GM/R^2 / sqrt(1 - 2GM/(Rc^2)).
 */
double SurfaceGravity_g14(const Evolution::StarContext &star,
						  const Evolution::GeometryCache *geo)
{
	double R_km = 0.0;
	double M_km = 0.0;
	double expnu = 0.0;

	// 1) Prefer GeometryCache (fast path, avoids profile lookups)
	if (geo)
	{
		if (geo->R().Size() > 0)
			R_km = geo->R()[-1];

		if (geo->Mass().Size() > 0)
			M_km = geo->Mass()[-1];

		if (geo->ExpNu().Size() > 0)
			expnu = geo->ExpNu()[-1];
	}

	// 2) Fallback to StarContext
	if (!detail::Pos(R_km))
		R_km = star.RadiusSurface();

	if (!detail::Pos(M_km))
		M_km = star.MassSurface(); // IMPORTANT: here MassSurface() must return M in km (GM/c^2)

	if (!detail::Pos(expnu))
		expnu = star.ExpNuSurface();

	// Validate inputs
	if (!detail::Pos(R_km) || !detail::Pos(M_km) || !detail::Pos(expnu))
		throw std::runtime_error("SurfaceGravity_g14: could not obtain (R_km, M_km, expnu) at surface.");

	// Additional physical sanity: require R > 2M (no horizon).
	if (R_km <= 2.0 * M_km)
		throw std::runtime_error("SurfaceGravity_g14: invalid compactness (R_km <= 2 M_km).");

	// Compute in cgs:
	const double R_cm = detail::KmToCm(R_km);
	const double M_cm = detail::KmToCm(M_km);

	const double g_cgs = (detail::c_cgs * detail::c_cgs) * M_cm / (R_cm * R_cm * expnu); // cm s^-2
	return g_cgs / detail::g14_unit;
}
// -------------------------------------------------------
} // namespace CompactStar::Physics::Driver::Thermal::Boundary