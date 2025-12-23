#pragma once
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
 * @file NeutrinoCooling_Details.hpp
 * @brief Shared computations for NeutrinoCooling (physics + diagnostics).
 *
 * This module centralizes all computations used by both:
 *  - NeutrinoCooling::AccumulateRHS (evolution physics), and
 *  - NeutrinoCooling::DiagnoseSnapshot (diagnostic output).
 *
 * The key design goal is to prevent "diagnostics drift": if the neutrino cooling
 * model changes, diagnostics remain consistent automatically because they call
 * the same ComputeDerived() helper.
 *
 * @ingroup PhysicsDriver
 */

#include <string>
#include <vector>

#include "CompactStar/Physics/Evolution/DriverContext.hpp"
#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

namespace CompactStar::Physics::Evolution
{
class StateVector;
class DriverContext;
} // namespace CompactStar::Physics::Evolution

namespace CompactStar::Physics::Evolution::Diagnostics
{
class DiagnosticPacket;
}

namespace CompactStar::Physics::Driver::Thermal
{
class NeutrinoCooling;
} // namespace CompactStar::Physics::Driver::Thermal

namespace CompactStar::Physics::Driver::Thermal::Detail
{

/**
 * @brief Bundle of derived neutrino-cooling quantities for physics/diagnostics.
 *
 * All fields are *computed outputs* based on the driver options, state vector,
 * and driver context. If a required piece of infrastructure is missing
 * (e.g. geometry/structure tables for volume integration), `ok=false` is set
 * and `message` provides a human-readable reason.
 *
 * ### Units (policy)
 * - Temperatures: [K]
 * - Luminosities at infinity: [erg/s]
 * - Heat capacity: [erg/K]
 * - dT/dt: [K/s]
 * - d/dt ln(Tinf/Tref): [1/s]
 */
struct NeutrinoCooling_Details
{
	/// Whether derived quantities are physically/computationally well-defined.
	bool ok = true;

	/// Informational message (notes) or error description if ok==false.
	std::string message;

	// ---------------------------------------------------------------------
	// Resolved inputs
	// ---------------------------------------------------------------------
	/// Redshifted isothermal interior temperature (evolved DOF), Kelvin.
	double Tinf_K = 0.0;

	/// Effective heat capacity used by this driver, [erg/K].
	double C_eff_erg_K = 0.0;

	// ---------------------------------------------------------------------
	// Neutrino luminosity bookkeeping (at infinity)
	// ---------------------------------------------------------------------
	/// Total neutrino luminosity at infinity, [erg/s].
	double L_nu_inf_erg_s = 0.0;

	/// Optional partition: direct Urca contribution, [erg/s].
	double L_nu_DU_inf_erg_s = 0.0;

	/// Optional partition: modified Urca contribution, [erg/s].
	double L_nu_MU_inf_erg_s = 0.0;

	/// Optional partition: pair breaking/formation (future), [erg/s].
	double L_nu_PBF_inf_erg_s = 0.0;

	// ---------------------------------------------------------------------
	// Evolution RHS contribution
	// ---------------------------------------------------------------------
	/// Physical cooling rate contribution to dTinf/dt, [K/s].
	double dTinf_dt_K_s = 0.0;

	/// ODE RHS contribution to d/dt ln(Tinf/Tref), [1/s].
	double dLnTinf_dt_1_s = 0.0;

	// ---------------------------------------------------------------------
	// Integration diagnostics (optional but very useful)
	// ---------------------------------------------------------------------
	/// Whether structure/geometry information required for integration was present.
	bool has_structure = false;

	/// Number of radial zones used in the integration (if applicable).
	std::size_t n_zones = 0;
};

/**
 * @brief Compute derived quantities for NeutrinoCooling.
 *
 * This function should:
 * 1) extract Tinf from ThermalState,
 * 2) validate driver options (C_eff, global_scale, enabled channels),
 * 3) compute L_nu_inf by integrating emissivities over proper volume (future),
 * 4) convert to dTinf/dt = -L_nu_inf / C_eff,
 * 5) convert to d/dt ln(Tinf/Tref) = (1/Tinf) dTinf/dt.
 *
 * @param drv NeutrinoCooling driver instance (options live in drv).
 * @param Y   Composite state vector (ThermalState read here).
 * @param ctx Driver context (structure, EOS, microphysics hooks live here).
 */
NeutrinoCooling_Details ComputeDerived(const NeutrinoCooling &drv,
									   const Evolution::StateVector &Y,
									   const Evolution::DriverContext &ctx);

/**
 * @brief Emit a diagnostics packet for NeutrinoCooling at a given snapshot.
 *
 * Uses ComputeDerived() so diagnostics remain consistent with physics.
 *
 * @param self Driver instance (used for name/options).
 * @param t    Current simulation time.
 * @param Y    Composite state vector.
 * @param ctx  Driver context.
 * @param out  Output packet (will be overwritten/deterministically filled).
 */
void Diagnose(const NeutrinoCooling &self,
			  double t,
			  const Evolution::StateVector &Y,
			  const Evolution::DriverContext &ctx,
			  Evolution::Diagnostics::DiagnosticPacket &out);

} // namespace CompactStar::Physics::Driver::Thermal::Detail