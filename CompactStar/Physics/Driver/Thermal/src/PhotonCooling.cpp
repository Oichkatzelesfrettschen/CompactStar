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

// -----------------------------------------------------------------------------
//  PhotonCooling.cpp
// -----------------------------------------------------------------------------
//  Implementation of the photon cooling evolution driver.
//
//  Minimal model:
//
//      L_γ  = A_eff * σ_SB * T_surf^4 * global_scale
//      dT∞/dt = - L_γ / C_eff
//
//  where T_surf is either taken directly from ThermalState::T_surf or
//  approximated from T∞ depending on Options::surface_model.
//
//  This is intentionally simple and “toy-like” so we can verify:
//    - multi-block evolution (Thermal + Spin),
//    - driver wiring / RHSAccumulator behavior,
//    - stability of coupled integration.
//
//  Later we can:
//    - pull T_surf from an envelope model via StarContext,
//    - compute A_eff and C_eff from structure + microphysics,
//    - add neutrino cooling and rotochemical heating drivers.
// -----------------------------------------------------------------------------

#include "CompactStar/Physics/Driver/Thermal/PhotonCooling.hpp"

#include <cmath> // std::pow
#include <limits>

#include "CompactStar/Physics/Evolution/RHSAccumulator.hpp"
#include "CompactStar/Physics/Evolution/StarContext.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

#include <Zaki/Util/Instrumentor.hpp> // Z_LOG_INFO/WARNING/ERROR, PROFILE_FUNCTION

namespace CompactStar::Physics::Driver::Thermal
{

// -----------------------------------------------------------------------------
//  Internal helper: Stefan–Boltzmann constant (cgs)
// -----------------------------------------------------------------------------
// You can adjust this or absorb it into area_eff/global_scale if your unit
// policy differs. For now we keep the physical value for clarity.
namespace
{
constexpr double SigmaSB_cgs = 5.670374419e-5; // erg cm^-2 s^-1 K^-4
} // namespace

// -----------------------------------------------------------------------------
//  PhotonCooling::AccumulateRHS
// -----------------------------------------------------------------------------
/**
 * @brief Add photon cooling contribution to the thermal RHS dT∞/dt.
 *
 * This implementation assumes:
 *   - ThermalState has at least one ODE DOF; component 0 is T∞.
 *   - Auxiliary ThermalState::T_surf is optional; if zero and SurfaceModel
 *     is DirectTSurf, we fall back to T∞.
 *   - C_eff > 0; otherwise the driver logs an error and returns.
 *
 * @param t     Current time (model units; typically seconds).
 * @param Y     Read-only composite state vector (must provide access to ThermalState).
 * @param dYdt  Write-only accumulator for RHS components; we add to the Thermal block.
 * @param ctx   Read-only star/context data (currently unused in this minimal model).
 */
void PhotonCooling::AccumulateRHS(double t,
								  const Evolution::StateVector &Y,
								  Evolution::RHSAccumulator &dYdt,
								  const Evolution::StarContext &ctx) const
{
	PROFILE_FUNCTION();

	(void)t;   // Not used in this simple cooling law (no explicit time dependence).
	(void)ctx; // Reserved for future use (envelope mapping, geometry, redshift).

	// -------------------------------------------------------------------------
	//  1. Extract the ThermalState from the composite state vector
	// -------------------------------------------------------------------------
	const Physics::State::ThermalState &thermal = Y.GetThermal();

	if (thermal.NumComponents() == 0)
	{
		Z_LOG_WARNING("PhotonCooling::AccumulateRHS: ThermalState has zero components; "
					  "driver is effectively disabled.");
		return;
	}

	// T∞ is the primary evolved thermal DOF (component 0 by convention).
	const double Tinf = thermal.Tinf();

	// If T∞ is non-positive, photon cooling is ill-defined physically.
	if (!(Tinf > 0.0))
	{
		Z_LOG_WARNING("PhotonCooling::AccumulateRHS: Tinf <= 0; skipping photon cooling term.");
		return;
	}

	// -------------------------------------------------------------------------
	//  2. Determine surface temperature T_surf
	// -------------------------------------------------------------------------
	double Tsurf = 0.0;

	switch (opts_.surface_model)
	{
	case Options::SurfaceModel::DirectTSurf:
	{
		// Use auxiliary T_surf if available; fall back to T∞ if not set.
		if (thermal.T_surf > 0.0)
		{
			Tsurf = thermal.T_surf;
		}
		else
		{
			Z_LOG_INFO("PhotonCooling::AccumulateRHS: T_surf <= 0, "
					   "falling back to Tinf for DirectTSurf model.");
			Tsurf = Tinf;
		}
		break;
	}

	case Options::SurfaceModel::ApproxFromTinf:
	default:
	{
		// Minimal approximation: T_surf ≈ T∞.
		// Later we can replace this by an envelope mapping, e.g. T_surf(T∞, g_s).
		Tsurf = Tinf;
		break;
	}
	}

	if (!(Tsurf > 0.0))
	{
		Z_LOG_WARNING("PhotonCooling::AccumulateRHS: Tsurf <= 0 after mapping; skipping term.");
		return;
	}

	// -------------------------------------------------------------------------
	//  3. Compute effective photon luminosity L_γ
	// -------------------------------------------------------------------------
	if (!(opts_.C_eff > 0.0))
	{
		Z_LOG_ERROR("PhotonCooling::AccumulateRHS: C_eff <= 0; cannot compute dT/dt. "
					"Check PhotonCooling::Options.");
		return;
	}

	const double area_eff = (opts_.area_eff > 0.0) ? opts_.area_eff : 0.0;
	if (area_eff <= 0.0)
	{
		Z_LOG_WARNING("PhotonCooling::AccumulateRHS: area_eff <= 0; no photon cooling applied.");
		return;
	}

	// L_γ,∞ ~ area_eff * σ_SB * T_surf^4 * global_scale
	const double Tsurf4 = std::pow(Tsurf, 4.0);
	double L_gamma = area_eff * SigmaSB_cgs * Tsurf4 * opts_.global_scale;

	// -------------------------------------------------------------------------
	//  4. Convert L_γ into dT∞/dt and accumulate into RHS
	// -------------------------------------------------------------------------
	const double dTinf_dt = -L_gamma / opts_.C_eff;

	// By convention, component 0 of the Thermal block is T∞.
	dYdt.AddTo(State::StateTag::Thermal, 0, dTinf_dt);
}

} // namespace CompactStar::Physics::Driver::Thermal