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
 * @file MagneticDipole.hpp
 * @brief Spin-down driver for vacuum-like magnetic dipole radiation.
 *
 * This driver implements a simple torque law of the form
 *   \dot{\Omega} = -K \Omega^n
 * with configurable braking index n and prefactor K. In more realistic
 * setups K can be built from B, R, I, and an obliquity angle using the
 * StarContext and SpinState.
 *
 * Directory-based namespace: CompactStar::Physics::Driver::Spin
 *
 * @ingroup Driver
 */

#ifndef CompactStar_Physics_Driver_Spin_MagneticDipole_H
#define CompactStar_Physics_Driver_Spin_MagneticDipole_H

#include <array>
#include <span>
#include <string>

#include "CompactStar/Physics/Driver/IDriver.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/Tags.hpp"

namespace CompactStar::Physics::Driver::Spin
{

/**
 * @class MagneticDipole
 * @brief Evolution driver for spin-down via magnetic dipole torque.
 *
 * **Depends on:** Spin
 * **Updates:**    Spin
 *
 * The current implementation assumes that the primary evolved spin DOF is
 * stored in `SpinState::Omega()` (component 0 of the internal vector).
 *
 * A minimal model is:
 *   \dot{\Omega} = -K \Omega^n
 * where `K` and `n` are provided via Options. More detailed models can
 * read B, I, R, etc. from `StarContext` and/or `SpinState`.
 */
class MagneticDipole final : public IDriver
{
  public:
	struct Options
	{
		/// Braking index n in \dot{\Omega} = -K \Omega^n (default 3 for pure dipole).
		double braking_index = 3.0;

		/// Prefactor K in \dot{\Omega} = -K \Omega^n (user units; to be calibrated).
		double K_prefactor = 0.0;

		/// If true, allow K to be modified using I(M,R,...) from the context.
		bool use_moment_of_inertia = false;
	};

	explicit MagneticDipole(Options opts = {}) : opts_(opts) {}

	// ------------------------------------------------------------------
	// IDriver interface
	// ------------------------------------------------------------------

	std::string Name() const override { return "MagneticDipole"; }

	std::span<const State::StateTag> DependsOn() const override
	{
		static constexpr std::array<State::StateTag, 1> deps{
			State::StateTag::Spin};
		return deps;
	}

	std::span<const State::StateTag> Updates() const override
	{
		static constexpr std::array<State::StateTag, 1> ups{
			State::StateTag::Spin};
		return ups;
	}

	/**
	 * @brief Add magnetic-dipole spin-down contribution to dY/dt.
	 *
	 * Conceptual model:
	 *   - Extract SpinState from the composite `Y` (via StateTag::Spin).
	 *   - Read the evolved spin DOF Ω = spin.Omega().
	 *   - Compute \dot{\Omega} = -K \Omega^n with K,n from `opts_`
	 *     and/or from `ctx` (e.g. moment of inertia I(M,R)).
	 *   - Add this contribution into the Spin block of `dYdt`.
	 *
	 * Actual extraction of SpinState and indexing into the Spin block
	 * is delegated to `Evolution::StateVector` / `RHSAccumulator`.
	 */
	void AccumulateRHS(double t,
					   const Evolution::StateVector &Y,
					   Evolution::RHSAccumulator &dYdt,
					   const Evolution::StarContext &ctx) const override;

	// ------------------------------------------------------------------
	// Options access
	// ------------------------------------------------------------------

	const Options &GetOptions() const { return opts_; }
	void SetOptions(const Options &o) { opts_ = o; }

  private:
	Options opts_;
};

} // namespace CompactStar::Physics::Driver::Spin

#endif /* CompactStar_Physics_Driver_Spin_MagneticDipole_H */