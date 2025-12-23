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
 * @file PhotonCooling.cpp
 * @brief Implementation of PhotonCooling driver (surface photon cooling).
 *
 * This file implements:
 *   CompactStar::Physics::Driver::Thermal::PhotonCooling::AccumulateRHS
 *
 * The driver contributes a cooling term to the redshifted thermal degree of
 * freedom (typically T_inf), using a blackbody-like photon luminosity measured
 * at infinity:
 *
 *   L_{γ,∞} = F_rad * A_∞ * σ_SB * T_surf^4 * global_scale
 *
 * and:
 *
 *   dT_inf/dt += - L_{γ,∞} / C_eff .
 *
 * ---------------------------------------------------------------------------
 * UNITS (must remain consistent):
 * ---------------------------------------------------------------------------
 * - T_surf, T_inf are in Kelvin [K]
 * - σ_SB is in cgs: [erg cm^-2 s^-1 K^-4]
 * - A_∞ must be in [cm^2] if σ_SB is cgs
 * - L_{γ,∞} is then [erg/s]
 * - C_eff must be [erg/K] so that dT/dt is [K/s]
 *
 * Geometry:
 * - If GeometryCache provides R in km (typical), convert to cm using:
 *     1 km = 1e5 cm
 *
 * Redshift:
 * - We assume g_tt = -exp(2ν(r)); GeometryCache provides exp(2ν(r)) or similar.
 * - For luminosity at infinity from the surface:
 *     A_∞ = 4π R^2 * exp(2ν(R))
 *
 * NOTE:
 * - The DriverContext (Evolution::DriverContext) is expected to provide access
 *   to GeometryCache via ctx.geo (preferred), plus optional envelope via ctx.envelope.
 */

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticCatalog.hpp" // ProducerCatalog, ScalarDescriptor
#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"  // Cadence

#include "CompactStar/Physics/Driver/Thermal/PhotonCooling.hpp"
#include "CompactStar/Physics/Driver/Thermal/PhotonCooling_Details.hpp"

#include <cmath> // std::pow
#include <limits>
#include <stdexcept> // (not used here, but often used in drivers)

#include "CompactStar/Physics/Evolution/DriverContext.hpp" // defines Evolution::DriverContext
#include "CompactStar/Physics/Evolution/GeometryCache.hpp" // GeometryCache API
#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

#include <Zaki/Util/Instrumentor.hpp> // PROFILE_FUNCTION
#include <Zaki/Util/Logger.hpp>		  // Z_LOG_INFO/WARNING/ERROR

namespace CompactStar::Physics::Driver::Thermal
{

// -----------------------------------------------------------------------------
//  Internal constants
// -----------------------------------------------------------------------------
// namespace
// {
// /// Stefan–Boltzmann constant in cgs units: erg cm^-2 s^-1 K^-4.
// constexpr double SigmaSB_cgs = 5.670374419e-5;

// /// km -> cm conversion for radius if GeometryCache stores radius in km.
// constexpr double KM_TO_CM = 1.0e5;
// } // namespace

// -----------------------------------------------------------------------------
//  PhotonCooling::AccumulateRHS
// -----------------------------------------------------------------------------
void PhotonCooling::AccumulateRHS(double t,
								  const Evolution::StateVector &Y,
								  Evolution::RHSAccumulator &dYdt,
								  const Evolution::DriverContext &ctx) const
{
	PROFILE_FUNCTION();

	// This cooling law has no explicit time dependence at present.
	(void)t;

	// -------------------------------------------------------------------------
	//  Centralized computation (shared with diagnostics)
	// -------------------------------------------------------------------------
	//
	// We compute everything (Tsurf mapping, geometry/redshift area, luminosity,
	// and dTinf/dt) through the shared helper to prevent “diagnostics drift”.
	//
	const auto d = Detail::ComputeDerived(*this, Y, ctx);

	// If the helper cannot compute a well-defined contribution, do nothing.
	// It returns a human-readable message describing why.
	if (!d.ok)
	{
		// Keep this as WARNING (not ERROR) because common causes include
		// infrastructure-first runs (missing geo), disabled cooling (Frad<=0),
		// or non-physical initial conditions used for wiring tests.
		// Z_LOG_WARNING("PhotonCooling skipped: " + d.message);
		// For an adaptive integrator, "skip" can happen at trial states.
		// Default: do nothing (silent).
		return;
	}

	// Optional: surface-model notes are informational (e.g., DirectTSurf fallback).
	if (!d.message.empty())
	{
		Z_LOG_INFO("PhotonCooling note: " + d.message);
	}

	// -------------------------------------------------------------------------
	//  Accumulate RHS contribution
	// -------------------------------------------------------------------------
	// We evolve x = ln(Tinf/Tref), so we must add dx/dt, not dT/dt:
	//
	//   dx/dt = (1/Tinf) * dTinf/dt   [1/s]
	//
	if (Y.GetThermal().Size() == 0)
		return; // No thermal DOF to update.

	// const double Tinf_K = d.Tinf_K; // physical temperature in Kelvin
	if (!(d.Tinf_K > 0.0)) // physical temperature in Kelvin
		return;			   // defensively skip (ComputeDerived should already enforce this)

	dYdt.AddTo(State::StateTag::Thermal, 0, d.dLnTinf_dt_1_s);
}

// -----------------------------------------------------------------------------
//  IDriverDiagnostics interface
// -----------------------------------------------------------------------------
std::string PhotonCooling::DiagnosticsName() const
{
	return "PhotonCooling";
}

// -----------------------------------------------------------------------------
//  UnitContract interface
// -----------------------------------------------------------------------------
Evolution::Diagnostics::UnitContract PhotonCooling::UnitContract() const
{
	Evolution::Diagnostics::UnitContract c;
	// Fill using *your* UnitContract API (whatever fields you defined).
	// e.g., c.title="PhotonCooling"; c.lines.push_back("Tinf, Tsurf in [K] ...");
	return c;
}

// -----------------------------------------------------------------------------
//  DiagnosticsCatalog interface
// -----------------------------------------------------------------------------
Evolution::Diagnostics::ProducerCatalog PhotonCooling::DiagnosticsCatalog() const
{
	using namespace CompactStar::Physics::Evolution::Diagnostics;

	ProducerCatalog pc;
	pc.producer = DiagnosticsName(); // must match packet Producer used by observer

	// Keep this list stable (schema-level). Add scalars you actually emit in DiagnoseSnapshot().
	{
		ScalarDescriptor sd;
		sd.key = "Tinf_K";
		sd.unit = "K";
		sd.description = "Redshifted internal temperature";
		sd.source_hint = "state";
		sd.default_cadence = Cadence::Always;
		sd.required = true; // if you always emit it
		pc.scalars.push_back(sd);
	}

	{
		ScalarDescriptor sd;
		sd.key = "L_gamma_inf_erg_s";
		sd.unit = "erg/s";
		sd.description = "Photon luminosity at infinity";
		sd.source_hint = "computed";
		sd.default_cadence = Cadence::OnChange; // optional
		sd.required = false;					// set true only if always present
		pc.scalars.push_back(sd);
	}

	// Add profile(s)
	ProducerCatalog::Profile p;
	p.name = "timeseries_default";
	p.keys = {"L_gamma_inf_erg_s"};
	pc.profiles.push_back(std::move(p));

	// Add more descriptors matching your actual emitted keys.

	return pc;
}

// -----------------------------------------------------------------------------
//  DiagnoseSnapshot interface
// -----------------------------------------------------------------------------
void PhotonCooling::DiagnoseSnapshot(double t,
									 const Evolution::StateVector &Y,
									 const Evolution::DriverContext &ctx,
									 Evolution::Diagnostics::DiagnosticPacket &out) const
{
	// Overwrite/fill `out` deterministically.
	Detail::Diagnose(*this, t, Y, ctx, out);
}
// -----------------------------------------------------------------------------
} // namespace CompactStar::Physics::Driver::Thermal