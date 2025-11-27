// -*- lsst-c++ -*-
/*
 * CompactStar — ChemicalHeating
 *
 * Phase 1: Minimal RHS wiring (no real microphysics yet).
 * Returns zeros for channels/heating/emissivity if components are null.
 */

#include <Zaki/Vector/DataSet.hpp>

#include "CompactStar/ChemicalHeating/Config.hpp"
#include "CompactStar/ChemicalHeating/Envelope.hpp"
#include "CompactStar/ChemicalHeating/EvolutionState.hpp"
#include "CompactStar/ChemicalHeating/EvolutionSystem.hpp"
#include "CompactStar/ChemicalHeating/GeometryCache.hpp"
#include "CompactStar/ChemicalHeating/Microphysics.hpp"
#include "CompactStar/ChemicalHeating/RateChannels.hpp"
#include "CompactStar/ChemicalHeating/StarContext.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                   EvolutionSystem Class
//==============================================================
EvolutionSystem::EvolutionSystem(const Context &ctx)
	: m_ctx(ctx)
{
	// [ZAKI-QUESTION]: ok for Phase-1 to allow null pointers (micro, rates, envelope)?
	// Current behavior: treat null components as zeros/no-ops.
}
//--------------------------------------------------------------
/**
 * Internal helper (Phase-1): safe read of cfg->n_eta with fallback.
 */
static inline std::size_t n_eta_or_1(const Config *cfg)
{
	return (cfg && cfg->n_eta > 0) ? cfg->n_eta : std::size_t(1);
}
//--------------------------------------------------------------
/**
 * Internal helper (Phase-1):
 * Map redshifted T_inf to local T(r) using e^{-nu}.
 * Requires Nu() column; if missing, fallback to T_loc = T_inf.
 */
static inline double local_temperature(double Tinf, double nu)
{
	return Tinf * std::exp(-nu);
}
//--------------------------------------------------------------
int EvolutionSystem::operator()(double t, const double y[], double dydt[]) const
{
	(void)t; // RHS is autonomous in Phase-1 stub

	// ---- Unpack state --------------------------------------------------------
	const std::size_t Neta = n_eta_or_1(m_ctx.cfg);
	const bool include_Omega = (m_ctx.cfg && m_ctx.cfg->couple_spin);

	const double Tinf = y[0];
	const double *y_eta = y + 1;
	const double Omega = include_Omega ? y[1 + Neta] : 0.0;

	// ---- Guards (basic) ------------------------------------------------------
	const double Tinf_safe = (Tinf > 0.0 ? Tinf : 0.0); // avoid negative T in stubs

	// ---- Assemble volume integrals (Phase-1 zeros if components missing) ----
	double C_tot = 0.0;	  // total heat capacity [erg/K]
	double Lnu_inf = 0.0; // neutrino luminosity at infinity [erg/s]
	double H_inf = 0.0;	  // internal heating at infinity [erg/s]
	double Lg_inf = 0.0;  // photon luminosity at infinity [erg/s] (surface)

	const auto *geo = m_ctx.geo;
	const auto *ctx = m_ctx.star;

	const bool have_geo = (geo && ctx && ctx->Nu() && ctx->Radius());
	const bool have_micro = (m_ctx.micro != nullptr);
	const bool have_rates = (m_ctx.rates != nullptr);
	const bool have_env = (m_ctx.envelope != nullptr);

	const std::size_t N = have_geo ? geo->Size() : 0;

	// [UNIT-NOTE]: Mass in your Core is in "km" (G=c=1), radii in km.
	// Our weights WC, WNu are dimensionless placeholders in Phase-1.
	// Real unit conversions arrive when microphysics is connected.

	if (have_geo)
	{
		// We’ll sum local contributions shell-by-shell.
		for (std::size_t i = 0; i < N; ++i)
		{
			const double nu_i = (*(ctx->Nu()))[i];
			const double Tloc = local_temperature(Tinf_safe, nu_i);

			// c_V
			double cV_i = 0.0;
			if (have_micro)
			{
				cV_i = m_ctx.micro->cV(i, Tloc);
			}
			C_tot += cV_i * geo->WC()[i];

			// Emissivity/heating from channels
			if (have_rates)
			{
				// Borrow view of raw channel pointers if available; otherwise iterate unique_ptrs.
				for (const auto &up : m_ctx.rates->channels())
				{
					const ReactionChannel *ch = up.get();
					const double Qnu_i = ch->emissivity(i, Tloc, /*eta*/ std::vector<double>(y_eta, y_eta + Neta));
					const double H_i = ch->heating(i, Tloc, /*eta*/ std::vector<double>(y_eta, y_eta + Neta));
					Lnu_inf += Qnu_i * geo->WNu()[i];
					H_inf += H_i * geo->WNu()[i];
					// [ZAKI-QUESTION]: we’re creating a temporary std::vector for eta each call.
					// In Phase-2 we should pass a span or reuse a preallocated buffer to avoid allocs.
				}
			}
		}
	}

	// Surface photon luminosity (placeholder zero in Phase-1)
	// [ZAKI-QUESTION]: Do you prefer L_gamma(Ts) to be computed here
	// using envelope + R, or upstream in Integrator via a "Derived" hook?
	(void)have_env;
	(void)Omega;
	Lg_inf = 0.0;

	// Heat capacity floor to avoid division by zero in the walking skeleton
	if (C_tot <= 0.0)
		C_tot = 1.0; // [ZAKI-QUESTION]: ok as a Phase-1 guard?

	// ---- Temperature derivative ---------------------------------------------
	// dTinf/dt = -(Lnu + Lgamma - H) / C
	dydt[0] = -(Lnu_inf + Lg_inf - H_inf) / C_tot;

	// ---- Chemical imbalances -------------------------------------------------
	// Phase-1: set to zero unless rates exist (then let channels add to accumulator).
	// Accumulator buffer:
	std::vector<double> deta_dt(Neta, 0.0);

	if (have_geo && have_rates)
	{
		// [ZAKI-QUESTION]: We currently need eta as a contiguous vector.
		const std::vector<double> eta(y_eta, y_eta + Neta);
		for (std::size_t i = 0; i < N; ++i)
		{
			const double nu_i = (*(ctx->Nu()))[i];
			const double Tloc = local_temperature(Tinf_safe, nu_i);
			for (const auto &up : m_ctx.rates->channels())
			{
				up->accumulate_deta_dt(i, Tloc, eta, deta_dt);
			}
		}
	}

	for (std::size_t k = 0; k < Neta; ++k)
	{
		dydt[1 + k] = deta_dt[k];
	}

	// ---- Optional spin channel (decoupled in Phase-1) -----------------------
	if (include_Omega)
	{
		// [ZAKI-QUESTION]: Provide torque law here later? For now keep dΩ/dt = 0.
		dydt[1 + Neta] = 0.0;
	}

	return 0; // success
}
//--------------------------------------------------------------

//==============================================================
} // namespace ChemicalHeating
//==============================================================
} // namespace CompactStar
//==============================================================