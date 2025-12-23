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
 * @file NeutrinoCooling_Details.cpp
 * @brief Out-of-line implementation of NeutrinoCooling shared computations.
 *
 * Rationale:
 *  - Keep heavy computations and structure/microphysics dereferences out of headers.
 *  - Avoid incomplete-type problems when using ctx-provided objects.
 *  - Centralize neutrino luminosity computation so RHS and diagnostics agree.
 */

#include "CompactStar/Physics/Driver/Thermal/NeutrinoCooling_Details.hpp"
#include "CompactStar/Physics/Driver/Thermal/NeutrinoCooling.hpp"

#include <cmath>
#include <stdexcept>

// Diagnostics packet API
#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"

// If you have these in your project, include them here; otherwise remove.
// #include "CompactStar/Physics/Evolution/GeometryCache.hpp"
// #include "CompactStar/Physics/Evolution/StarContext.hpp"
// #include "CompactStar/Physics/Microphysics/NeutrinoEmissivity.hpp"

#include <Zaki/Util/Instrumentor.hpp> // PROFILE_FUNCTION
#include <Zaki/Util/Logger.hpp>		  // Z_LOG_INFO/WARNING/ERROR

namespace CompactStar::Physics::Driver::Thermal::Detail
{

namespace
{

/**
 * @brief Minimal placeholder emissivity model for wiring tests.
 *
 * This exists only so the base structure (catalog/packet/timeseries) can run
 * before you plug in real emissivities.
 *
 * Replace with: DUrca/MUrca/PBF emissivities integrated over proper volume.
 *
 * @param Tinf_K Redshifted interior temperature [K].
 * @return A tiny luminosity scale [erg/s] for non-zero RHS during smoke tests.
 */
double PlaceholderLuminosity(double Tinf_K)
{
	// Deliberately small-ish for stability; choose any smooth T dependence.
	// Typical neutrino luminosities can be huge; this is not physical.
	// L ~ 1e30 * (T/1e8)^6 erg/s as a harmless placeholder.
	const double T8 = Tinf_K / 1.0e8;
	return 1.0e30 * std::pow(T8, 6.0);
}

} // namespace

// -----------------------------------------------------------------------------
//  ComputeDerived
// -----------------------------------------------------------------------------
NeutrinoCooling_Details ComputeDerived(const NeutrinoCooling &drv,
									   const Evolution::StateVector &Y,
									   const Evolution::DriverContext &ctx)
{
	PROFILE_FUNCTION();

	NeutrinoCooling_Details d;

	// ---------------------------------------------------------------------
	// 1) Extract evolved DOF: T_inf
	// ---------------------------------------------------------------------
	const auto &thermal = Y.GetThermal();
	if (thermal.NumComponents() == 0)
	{
		d.ok = false;
		d.message = "ThermalState has zero components.";
		return d;
	}

	d.Tinf_K = thermal.Tinf();
	if (!(d.Tinf_K > 0.0))
	{
		d.ok = false;
		d.message = "Tinf <= 0; neutrino cooling ill-defined.";
		return d;
	}

	// ---------------------------------------------------------------------
	// 2) Resolve/validate options used in RHS
	// ---------------------------------------------------------------------
	// Your NeutrinoCooling::Options currently includes global_scale and channel toggles.
	// You also need an effective heat capacity; choose one policy:
	//   A) store C_eff inside NeutrinoCooling::Options (recommended for symmetry w/ PhotonCooling),
	//   B) fetch from ctx (if you already have a thermal capacity model),
	//   C) reuse the same C_eff as photon cooling in a shared thermal config object.
	//
	// For now, we assume NeutrinoCooling has a C_eff accessor or that you will add it.
	//
	// If you don't have it yet, set a temporary constant here and move it later.
	//
	// IMPORTANT: make this match your driver header/cpp choices.
	//
	const double C_eff = 1.0e40; // TODO: replace with drv.GetOptions().C_eff or ctx-thermal capacity.
	d.C_eff_erg_K = C_eff;

	if (!(d.C_eff_erg_K > 0.0))
	{
		d.ok = false;
		d.message = "C_eff <= 0.";
		return d;
	}

	if (!(drv.GetOptions().global_scale > 0.0))
	{
		// Disabled but valid: no cooling contribution.
		d.ok = true;
		d.message = "cooling disabled: global_scale <= 0.";
		d.L_nu_inf_erg_s = 0.0;
		d.dTinf_dt_K_s = 0.0;
		d.dLnTinf_dt_1_s = 0.0;
		return d;
	}

	// If all channels disabled, treat as disabled-but-valid.
	if (!drv.GetOptions().include_direct_urca &&
		!drv.GetOptions().include_modified_urca &&
		!drv.GetOptions().include_pair_breaking)
	{
		d.ok = true;
		d.message = "cooling disabled: all neutrino channels disabled by options.";
		d.L_nu_inf_erg_s = 0.0;
		d.dTinf_dt_K_s = 0.0;
		d.dLnTinf_dt_1_s = 0.0;
		return d;
	}

	// ---------------------------------------------------------------------
	// 3) Compute neutrino luminosity at infinity (placeholder vs real)
	// ---------------------------------------------------------------------
	//
	// This is where the real work will go:
	//   - loop over radial zones
	//   - compute local T(r) (isothermal redshifted => T_local(r) = Tinf * e^{-nu(r)})
	//   - compute emissivities Q_nu (erg cm^-3 s^-1) per process via microphysics
	//   - multiply by proper volume element dV_proper (cm^3)
	//   - apply redshift factors to get L_inf
	//
	// For now, implement a minimal placeholder so the infrastructure runs.
	//
	(void)ctx; // placeholder until structure/microphysics are wired in

	double L_DU = 0.0;
	double L_MU = 0.0;
	double L_PBF = 0.0;

	const double L0 = PlaceholderLuminosity(d.Tinf_K);

	if (drv.GetOptions().include_direct_urca)
		L_DU = 0.6 * L0; // arbitrary split for placeholder
	if (drv.GetOptions().include_modified_urca)
		L_MU = 0.4 * L0;
	if (drv.GetOptions().include_pair_breaking)
		L_PBF = 0.0; // keep zero until implemented

	d.L_nu_DU_inf_erg_s = drv.GetOptions().global_scale * L_DU;
	d.L_nu_MU_inf_erg_s = drv.GetOptions().global_scale * L_MU;
	d.L_nu_PBF_inf_erg_s = drv.GetOptions().global_scale * L_PBF;

	d.L_nu_inf_erg_s = d.L_nu_DU_inf_erg_s + d.L_nu_MU_inf_erg_s + d.L_nu_PBF_inf_erg_s;

	// ---------------------------------------------------------------------
	// 4) Convert to cooling rate and RHS term
	// ---------------------------------------------------------------------
	d.dTinf_dt_K_s = -d.L_nu_inf_erg_s / d.C_eff_erg_K;
	d.dLnTinf_dt_1_s = d.dTinf_dt_K_s / d.Tinf_K;

	return d;
}

// -----------------------------------------------------------------------------
//  Diagnostics interface
// -----------------------------------------------------------------------------
void Diagnose(const NeutrinoCooling &self,
			  double t,
			  const Evolution::StateVector &Y,
			  const Evolution::DriverContext &ctx,
			  Evolution::Diagnostics::DiagnosticPacket &out)
{
	PROFILE_FUNCTION();

	// Reset packet identity/time (packet may be reused by observer).
	out.SetProducer(self.DiagnosticsName());
	out.SetTime(t);

	// Compute the shared derived quantities (physics-consistent).
	const NeutrinoCooling_Details d = ComputeDerived(self, Y, ctx);

	// Status / messages
	if (!d.ok)
	{
		out.AddWarning("NeutrinoCooling details not OK: " + d.message);
	}
	else if (!d.message.empty())
	{
		out.AddNote(d.message);
	}

	// ---------------------------------------------------------------------
	// Scalars (schema metadata should match your DiagnosticsCatalog)
	// ---------------------------------------------------------------------
	out.AddScalar("Tinf_K", d.Tinf_K, "K",
				  "Redshifted internal temperature (evolved DOF)", "state");

	out.AddScalar("L_nu_inf_erg_s", d.L_nu_inf_erg_s, "erg/s",
				  "Total neutrino luminosity at infinity", "computed");

	out.AddScalar("L_nu_DU_inf_erg_s", d.L_nu_DU_inf_erg_s, "erg/s",
				  "Direct Urca neutrino luminosity at infinity", "computed",
				  Evolution::Diagnostics::Cadence::OnChange);

	out.AddScalar("L_nu_MU_inf_erg_s", d.L_nu_MU_inf_erg_s, "erg/s",
				  "Modified Urca neutrino luminosity at infinity", "computed",
				  Evolution::Diagnostics::Cadence::OnChange);

	out.AddScalar("L_nu_PBF_inf_erg_s", d.L_nu_PBF_inf_erg_s, "erg/s",
				  "Pair breaking/formation neutrino luminosity at infinity", "computed",
				  Evolution::Diagnostics::Cadence::OnChange);

	out.AddScalar("dTinf_dt_K_s", d.dTinf_dt_K_s, "K/s",
				  "NeutrinoCooling contribution to dTinf/dt", "computed");

	out.AddScalar("dLnTinf_dt_1_s", d.dLnTinf_dt_1_s, "1/s",
				  "NeutrinoCooling contribution to d/dt ln(Tinf/Tref)", "computed");

	out.ValidateBasic();
}

} // namespace CompactStar::Physics::Driver::Thermal::Detail