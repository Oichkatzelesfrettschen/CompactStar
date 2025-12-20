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
 * @file NeutrinoCooling.cpp
 * @brief Implementation of NeutrinoCooling driver (core neutrino cooling).
 *
 * This file implements:
 *   CompactStar::Physics::Driver::Thermal::NeutrinoCooling::AccumulateRHS
 *
 * ---------------------------------------------------------------------------
 * CURRENT SCOPE (minimal working driver):
 * ---------------------------------------------------------------------------
 * - The goal is to wire neutrino cooling into the evolution loop *correctly*
 *   without requiring the full microphysics/integration machinery yet.
 *
 * - Therefore, this implementation computes a phenomenological cooling term in
 *   the evolved variable:
 *
 *     x := ln(T_inf / T_ref)
 *
 *   and accumulates:
 *
 *     dx/dt += dLnTinf_dt_1_s
 *
 * - This matches the pattern used by PhotonCooling, which also evolves the log
 *   temperature variable and adds to Thermal tag index 0.
 *
 * ---------------------------------------------------------------------------
 * PHYSICS NOTES (placeholder model):
 * ---------------------------------------------------------------------------
 * In a full implementation, one would compute:
 *   L_{nu,∞} = ∫ dV_proper  Q_nu(r) * redshift_factors
 * and then:
 *   dT_inf/dt = - L_{nu,∞} / C_eff(T, composition, SF suppression, ...)
 *   d/dt ln(T_inf/T_ref) = (1/T_inf) dT_inf/dt
 *
 * Here we do a minimal version that does NOT require C_eff or EOS access:
 *
 *   d/dt ln(T_inf/T_ref) = - [ A_DU (T/1e9 K)^{p_DU} + A_MU (T/1e9 K)^{p_MU} + ... ]
 *
 * where:
 *  - A_* have units [1/s]
 *  - p_* are chosen to roughly mimic expected steep temperature dependence
 *    (DUrca faster than MUrca).
 *
 * IMPORTANT:
 * - The constants below are placeholders intended for infrastructure testing.
 * - Replace the model with volume-integrated emissivities + C_eff once your
 *   Microphysics hooks are ready.
 *
 * ---------------------------------------------------------------------------
 * UNITS:
 * ---------------------------------------------------------------------------
 * - T_inf is in Kelvin [K]
 * - dLnTinf_dt is in [1/s]
 *
 * @ingroup PhysicsDriver
 */

#include "CompactStar/Physics/Driver/Thermal/NeutrinoCooling.hpp"

#include <cmath>  // std::pow, std::isfinite
#include <limits> // quiet_NaN

#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

#include <Zaki/Util/Instrumentor.hpp> // PROFILE_FUNCTION
#include <Zaki/Util/Logger.hpp>		  // Z_LOG_INFO/WARNING/ERROR

namespace CompactStar::Physics::Driver::Thermal
{
namespace
{
/**
 * @brief Safe power for nonnegative base values.
 *
 * @param x Base (should be >= 0).
 * @param p Exponent.
 * @return x^p for x>0; 0 for x==0; NaN for x<0 or non-finite.
 */
double PowNonneg(double x, double p)
{
	if (!std::isfinite(x) || x < 0.0)
		return std::numeric_limits<double>::quiet_NaN();
	if (x == 0.0)
		return 0.0;
	return std::pow(x, p);
}

/**
 * @brief Convert temperature in K to the dimensionless ratio (T / 1e9 K).
 */
double Theta9(double T_K)
{
	return (T_K / 1.0e9);
}

/**
 * @brief Placeholder coefficients for minimal neutrino cooling law.
 *
 * These are **not** meant to be physical defaults.
 * They are chosen to be numerically tame so the driver can be enabled for
 * wiring/tests without instantly dominating the evolution.
 *
 * Replace with real emissivity integrals + C_eff later.
 */
struct PlaceholderLaw
{
	// DUrca-like (fast) and MUrca-like (slower) contributions in d(ln T)/dt.
	// Units: [1/s]
	double A_DU_1_s = 0.0;	  ///< default off unless you want it on by default
	double A_MU_1_s = 1.0e-3; ///< small but nonzero for wiring tests

	// Exponents in (T/1e9)^p for d(ln T)/dt.
	// Rough intuition: if L ~ T^n and C ~ T, then dlnT/dt ~ T^{n-1}.
	// DUrca: n~6 -> p~5 ; MUrca: n~8 -> p~7.
	double p_DU = 5.0;
	double p_MU = 7.0;

	// Optional placeholder for Pair-Breaking/Formation (PBF) cooling
	double A_PBF_1_s = 0.0;
	double p_PBF = 7.0;
};

} // namespace

NeutrinoCooling::NeutrinoCooling(const Options &opts)
	: opts_(opts)
{
}
// -----------------------------------------------------------------------------
NeutrinoCooling::NeutrinoCooling(Options &&opts)
	: opts_(std::move(opts))
{
}

// -----------------------------------------------------------------------------
//  NeutrinoCooling::AccumulateRHS
// -----------------------------------------------------------------------------
void NeutrinoCooling::AccumulateRHS(double t,
									const Evolution::StateVector &Y,
									Evolution::RHSAccumulator &dYdt,
									const Evolution::DriverContext &ctx) const
{
	PROFILE_FUNCTION();

	(void)t;
	(void)ctx;

	// -------------------------------------------------------------------------
	// Sanity: do we have a thermal DOF to update?
	// -------------------------------------------------------------------------
	if (Y.GetThermal().Size() == 0)
		return;

	const double Tinf_K = Y.GetThermal().Tinf();

	// Defensive: require a physical, finite temperature
	if (!std::isfinite(Tinf_K) || !(Tinf_K > 0.0))
	{
		// Keep warning muted by default; adaptive integrators can probe trial states.
		// Z_LOG_WARNING("NeutrinoCooling: nonphysical Tinf encountered; skipping.");
		return;
	}

	// -------------------------------------------------------------------------
	// Placeholder cooling law in d/dt ln(Tinf/Tref)
	// -------------------------------------------------------------------------
	const PlaceholderLaw law;

	const double th9 = Theta9(Tinf_K);
	if (!std::isfinite(th9) || th9 < 0.0)
		return;

	double dLnTinf_dt_1_s = 0.0;

	// DUrca-like contribution
	if (opts_.include_direct_urca)
	{
		const double term = PowNonneg(th9, law.p_DU);
		if (std::isfinite(term))
			dLnTinf_dt_1_s -= law.A_DU_1_s * term;
	}

	// MUrca-like contribution
	if (opts_.include_modified_urca)
	{
		const double term = PowNonneg(th9, law.p_MU);
		if (std::isfinite(term))
			dLnTinf_dt_1_s -= law.A_MU_1_s * term;
	}

	// PBF hook (future SF physics); placeholder only
	if (opts_.include_pair_breaking)
	{
		const double term = PowNonneg(th9, law.p_PBF);
		if (std::isfinite(term))
			dLnTinf_dt_1_s -= law.A_PBF_1_s * term;
	}

	// Apply a global multiplicative scaling factor (safety/testing knob).
	dLnTinf_dt_1_s *= opts_.global_scale;

	// If the computed value is non-finite, skip (do not poison RHS).
	if (!std::isfinite(dLnTinf_dt_1_s))
		return;

	// -------------------------------------------------------------------------
	// Accumulate RHS contribution
	// -------------------------------------------------------------------------
	// We evolve x = ln(Tinf/Tref), so add dx/dt directly:
	dYdt.AddTo(State::StateTag::Thermal, 0, dLnTinf_dt_1_s);
}

} // namespace CompactStar::Physics::Driver::Thermal