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
 * @file NeutrinoCooling.hpp
 * @brief Core neutrino-cooling driver (DUrca, MUrca, PBF hooks) for thermal evolution.
 *
 * This driver contributes a *cooling* term to the thermal evolution equation in the
 * redshifted/logarithmic variable used by CompactStar Evolution.
 *
 * ## What this driver does (current scope)
 * - Reads the current thermal degree of freedom from the Evolution::StateVector.
 * - Computes a neutrino-cooling contribution expressed as:
 *   \f[
 *     \frac{d}{dt}\ln\!\left(\frac{T_\infty}{T_\mathrm{ref}}\right)
 *     \equiv \dot{x}
 *   \f]
 * - Accumulates \f$\dot{x}\f$ into the RHS accumulator at the Thermal state tag.
 *
 * ## Full physics target (future scope)
 * Ultimately, this module should compute:
 * - Proper-volume integrals of local emissivities (DUrca, MUrca, PBF, bremsstrahlung, …)
 *   with appropriate metric/redshift factors.
 * - Divide by an effective heat capacity \f$C_\mathrm{eff}\f$ to obtain \f$dT_\infty/dt\f$,
 *   then convert to \f$d\ln(T_\infty/T_\mathrm{ref})/dt\f$.
 *
 * In this first milestone we intentionally keep the implementation low-dependency
 * so the driver can be wired into the evolution infrastructure before microphysics
 * dependencies are finalized.
 *
 * @ingroup PhysicsDriver
 */

#include <string>
#include <vector>

#include "CompactStar/Physics/Driver/IDriver.hpp"

namespace CompactStar::Physics::Driver::Thermal
{

/**
 * @class NeutrinoCooling
 * @brief Driver that adds a neutrino-cooling contribution to the thermal RHS.
 *
 * The Evolution module typically evolves a thermal variable in logarithmic form:
 * \f[
 *   x \equiv \ln(T_\infty/T_\mathrm{ref})
 * \f]
 * therefore the driver accumulates \f$dx/dt\f$ directly into the RHS.
 *
 * ### State dependencies
 * - **DependsOn:** Thermal
 * - **Updates:**   Thermal
 *
 * ### Determinism contract
 * - This driver must be pure w.r.t. the evolution state: it does not mutate Y or ctx.
 * - AccumulateRHS must be deterministic for a fixed (t, Y, ctx).
 */
class NeutrinoCooling final : public IDriver
{
  public:
	/**
	 * @struct Options
	 * @brief Configuration knobs controlling which neutrino channels are enabled.
	 *
	 * Notes:
	 * - These toggles are intended to map onto future microphysics channel selection.
	 * - `global_scale` is a convenient safety/testing knob for early integration.
	 */
	struct Options
	{
		/// Enable a (placeholder) direct Urca-like cooling contribution.
		bool include_direct_urca = true;

		/// Enable a (placeholder) modified Urca-like cooling contribution.
		bool include_modified_urca = true;

		/// Enable a (placeholder) pair-breaking/formation (PBF) cooling contribution.
		bool include_pair_breaking = false;

		/// Multiplicative scale applied to the net cooling term (dimensionless).
		double global_scale = 1.0;
	};

	/**
	 * @brief Default constructor.
	 *
	 * Uses default Options values (channels enabled/disabled as per Options defaults).
	 *
	 * This overload avoids the C++17/C++14 pitfall of `Options opts = {}` when the
	 * nested struct contains default member initializers under some toolchains.
	 */
	NeutrinoCooling() = default;

	/**
	 * @brief Construct with explicit Options.
	 *
	 * @param opts Driver configuration.
	 */
	explicit NeutrinoCooling(const Options &opts);

	/**
	 * @brief Construct with explicit Options (move).
	 *
	 * @param opts Driver configuration.
	 */
	explicit NeutrinoCooling(Options &&opts);

	// -------------------------------------------------------------------------
	// IDriver interface
	// -------------------------------------------------------------------------

	/**
	 * @brief Human-readable driver name (stable identifier).
	 *
	 * @return "NeutrinoCooling"
	 */
	[[nodiscard]] std::string Name() const override { return "NeutrinoCooling"; }

	/**
	 * @brief Return the state tags this driver reads from.
	 *
	 * @return Vector containing State::StateTag::Thermal.
	 */
	[[nodiscard]] const std::vector<State::StateTag> &DependsOn() const override
	{
		static const std::vector<State::StateTag> deps{State::StateTag::Thermal};
		return deps;
	}

	/**
	 * @brief Return the state tags this driver writes to (via RHSAccumulator).
	 *
	 * @return Vector containing State::StateTag::Thermal.
	 */
	[[nodiscard]] const std::vector<State::StateTag> &Updates() const override
	{
		static const std::vector<State::StateTag> ups{State::StateTag::Thermal};
		return ups;
	}

	/**
	 * @brief Accumulate neutrino-cooling contribution into the thermal RHS.
	 *
	 * This function must:
	 * - treat Y and ctx as read-only inputs,
	 * - compute a contribution \f$\dot{x}\f$ to the evolved thermal variable
	 *   \f$x = \ln(T_\infty/T_\mathrm{ref})\f$,
	 * - write it into the RHS accumulator at (Thermal, component 0).
	 *
	 * @param t    Current simulation time.
	 * @param Y    Current evolution state vector (read-only).
	 * @param dYdt RHS accumulator to which the driver adds its contribution.
	 * @param ctx  Driver context (geometry, caches, EOS handles, etc.).
	 */
	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::DriverContext &ctx) const override;

	// -------------------------------------------------------------------------
	// Options accessors
	// -------------------------------------------------------------------------

	/**
	 * @brief Get current Options.
	 */
	[[nodiscard]] const Options &GetOptions() const { return opts_; }

	/**
	 * @brief Replace Options.
	 */
	void SetOptions(const Options &o) { opts_ = o; }

  private:
	/// Driver configuration.
	Options opts_{};
};

} // namespace CompactStar::Physics::Driver::Thermal