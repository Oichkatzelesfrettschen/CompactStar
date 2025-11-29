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
 * @file StateVector.hpp
 * @brief Lightweight composite view over sub-states (Spin/Thermal/Chem/…).
 *
 * For the first implementation and tests, this class only exposes a SpinState
 * block. Later, it can be generalized to hold any number of StateTag → State*
 * mappings and provide generic accessors.
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_StateVector_H
#define CompactStar_Physics_Evolution_StateVector_H

#include "CompactStar/Physics/State/SpinState.hpp"

namespace CompactStar::Physics::Evolution
{

/**
 * @class StateVector
 * @brief Composite view over the current ODE state.
 *
 * In the full design, StateVector will:
 *   - own or reference several State-derived blocks (Spin, Thermal, Chem, BNV),
 *   - provide both tag-based and typed accessors,
 *   - expose a flat, concatenated vector to the integrator.
 *
 * For now, to get spin evolution working, it only stores a reference to
 * a SpinState and exposes `GetSpin()`.
 */
class StateVector
{
  public:
	/**
	 * @brief Construct a StateVector wrapping a SpinState.
	 *
	 * @param spin  Reference to the current spin state.
	 *
	 * The caller is responsible for ensuring that @p spin remains alive
	 * while this StateVector is in use.
	 */
	explicit StateVector(const Physics::State::SpinState &spin)
		: spin_(spin)
	{
	}

	/**
	 * @brief Access the SpinState block (const).
	 *
	 * This is what drivers like MagneticDipole use to read Ω (and in
	 * principle other spin-related DOFs).
	 */
	const Physics::State::SpinState &GetSpin() const { return spin_; }

  private:
	const Physics::State::SpinState &spin_;
};

} // namespace CompactStar::Physics::Evolution

#endif /* CompactStar_Physics_Evolution_StateVector_H */