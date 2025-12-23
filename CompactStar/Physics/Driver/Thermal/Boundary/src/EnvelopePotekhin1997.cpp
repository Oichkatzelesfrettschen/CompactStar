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
 * @file EnvelopePotekhin1997.cpp
 * @brief Implementation of Potekhin-style Tb→Ts envelope fits declared in EnvelopePotekhin1997.hpp.
 *
 * @ingroup PhysicsDriverThermalBoundary
 *
 * ## Purpose
 * Implements concrete Boundary::IEnvelope models that provide the thermal boundary condition
 * at the base of the blanketing envelope:
 *
 * \f[
 *   T_s = T_s(T_b, g_{14}, \xi)
 * \f]
 *
 * with the conventions:
 * - Tb  : local temperature at the base of the envelope [K]
 * - Ts  : local effective surface temperature [K]
 * - g14 : surface gravity in units of \f$10^{14}\,\mathrm{cm\,s^{-2}}\f$
 * - xi  : light-element/accreted-envelope parameter (model dependent)
 *
 * ## Validation policy
 * - If Tb <= 0 or g14 <= 0, the functions return 0 (non-physical input).
 * - NaN/Inf inputs are treated as invalid and return 0.
 * - The iron fit ignores xi.
 *
 * ## Notes on physics content
 * - The iron fit here matches the compact form you used previously:
 *
 *   \f[
 *     T_s^4 = 10^{24}\, g_{14}\, \left( 1.81\, \frac{T_b}{10^8\,\mathrm{K}} \right)^{2.42}
 *   \f]
 *
 * - The "accreted" model below is currently a conservative placeholder using xi as a monotonic
 *   interpolation knob. Replace it with the exact Potekhin/Yakovlev light-element column fits
 *   when you are ready; the interface remains stable.
 */

#include "CompactStar/Physics/Driver/Thermal/Boundary/EnvelopePotekhin1997.hpp"

#include <cmath>
#include <limits>

namespace CompactStar::Physics::Driver::Thermal::Boundary
{
namespace
{
inline bool FinitePos(double x)
{
	return (x > 0.0) && std::isfinite(x);
}

inline double Clamp01(double x)
{
	if (!std::isfinite(x))
		return 0.0;
	if (x < 0.0)
		return 0.0;
	if (x > 1.0)
		return 1.0;
	return x;
}

inline double ClampNonNeg(double x)
{
	if (!std::isfinite(x))
		return 0.0;
	return (x > 0.0) ? x : 0.0;
}
} // namespace

// -----------------------------------------------------------------------------
// EnvelopePotekhin1997_Iron
// -----------------------------------------------------------------------------
double EnvelopePotekhin1997_Iron::Ts_from_Tb(double Tb, double g14, double /*xi*/) const
{
	if (!FinitePos(Tb) || !FinitePos(g14))
		return 0.0;

	// Tb8 ≡ Tb / 1e8 K
	const double Tb8 = Tb / 1.0e8;

	// Ts^4 = 1e24 * g14 * (1.81 * Tb8)^{2.42}
	const double Ts4 = 1.0e24 * g14 * std::pow(1.81 * Tb8, 2.42);

	// Ts = (Ts^4)^{1/4}
	return std::pow(ClampNonNeg(Ts4), 0.25);
}

// -----------------------------------------------------------------------------
// EnvelopePotekhin1997_Accreted
// -----------------------------------------------------------------------------
double EnvelopePotekhin1997_Accreted::Ts_from_Tb(double Tb, double g14, double xi) const
{
	if (!FinitePos(Tb) || !FinitePos(g14))
		return 0.0;

	// Placeholder: treat xi as a bounded interpolation knob in [0,1].
	// Replace with the true Potekhin/Yakovlev light-element column fits when ready.
	xi = Clamp01(xi);

	// Baseline iron Ts:
	const double Ts_fe = EnvelopePotekhin1997_Iron{}.Ts_from_Tb(Tb, g14, 0.0);

	// Conservative monotonic boost for accreted envelopes (placeholder):
	// xi=0 -> iron; xi=1 -> modest enhancement.
	const double boost = 1.0 + 0.6 * xi;

	return Ts_fe * boost;
}

} // namespace CompactStar::Physics::Driver::Thermal::Boundary