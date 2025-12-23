#ifndef CompactStar_Physics_Driver_Thermal_Boundary_TbDefinition_H
#define CompactStar_Physics_Driver_Thermal_Boundary_TbDefinition_H

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
 * @file TbDefinition.hpp
 * @brief Definition and helpers for the base-of-envelope temperature Tb.
 *
 * Envelope fits are typically expressed as Ts(Tb, g14, xi, ...), where Tb is the
 * *local* temperature at the base of the heat blanketing envelope, often chosen
 * at a fiducial density:
 *
 *   ρ_b ≈ 1e10 g/cm^3
 *
 * The interior is frequently assumed isothermal in the redshifted sense:
 *
 *   T(r) = T_inf * exp(-ν(r))
 *
 * where T_inf is constant through the isothermal region.
 *
 * This module encodes:
 *  - A canonical "base density" choice.
 *  - A strategy for picking the radius/index of the base-of-envelope.
 *  - A helper that computes Tb from T_inf and the metric at the base.
 *
 * Important: this module is *policy*, not EOS microphysics.
 * If we later implement crust/envelope conduction explicitly, this can be bypassed.
 */

#include <cstddef>
#include <stdexcept>

namespace CompactStar::Physics::Evolution
{
class StarContext;
class GeometryCache;
} // namespace CompactStar::Physics::Evolution

namespace CompactStar::Physics::Driver::Thermal::Boundary
{
// -------------------------------------------------------
/**
 * @brief Policy for defining the base-of-envelope location.
 */
struct TbDefinition
{
	/**
	 * @brief Base density ρ_b in g/cm^3 (classic choice: 1e10).
	 */
	double rho_b = 1.0e10;

	/**
	 * @brief If true, Tb is computed from isothermal redshifted T_inf:
	 *
	 *   Tb = T_inf * exp(-ν(r_b))
	 *
	 * Otherwise, caller may supply Tb directly or compute via another model.
	 */
	bool assume_isothermal_redshifted = true;

	/**
	 * @brief If true, prefer GeometryCache for metric factors at r_b.
	 */
	bool prefer_geometry_cache = true;
};
// -------------------------------------------------------
/**
 * @brief Find the index of the base-of-envelope radius (r_b) in the star grid.
 *
 * Strategy:
 * - Use the StarContext profile to find the first radius where ρ(r) <= ρ_b
 *   (i.e., moving outward from core to crust/envelope).
 *
 * Requirements:
 * - StarContext::Profile() exposes density column ρ(r) in g/cm^3 and radius grid R (km).
 *
 * @throws std::runtime_error if density profile is missing or the threshold cannot be bracketed.
 */
std::size_t FindTbIndex(const Evolution::StarContext &star, double rho_b);

/**
 * @brief Compute Tb (local) given T_inf and a TbDefinition policy.
 *
 * @param star   StarContext (structure + profile).
 * @param geo    Optional GeometryCache for exp(-ν) or exp(2ν) at r_b.
 * @param T_inf  Redshifted isothermal temperature [K].
 * @param def    Policy describing rho_b and how to compute Tb.
 *
 * @return Tb in Kelvin (local frame at base of envelope).
 */
double ComputeTb(const Evolution::StarContext &star,
				 const Evolution::GeometryCache *geo,
				 double T_inf,
				 const TbDefinition &def);

} // namespace CompactStar::Physics::Driver::Thermal::Boundary
// -------------------------------------------------------
#endif /* CompactStar_Physics_Driver_Thermal_Boundary_TbDefinition_H */