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
 * @file GSLIntegrator.hpp
 * @brief RAII front-end for a GSL ODE driver.
 *
 * Wraps stepper choice, tolerances, and integrates the flat ODE vector y[]
 * while delegating the RHS evaluation to `EvolutionSystem`.
 *
 * Design:
 *  - Dimension N of the system is provided at construction.
 *  - The integrator does *not* know about StateVector/StateLayout; callers
 *    are responsible for packing/unpacking via StatePacking helpers.
 *  - The RHS callback simply forwards to `EvolutionSystem::operator()`.
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_GSLIntegrator_H
#define CompactStar_Physics_Evolution_GSLIntegrator_H

#include <cstddef>

namespace CompactStar::Physics::Evolution
{

class EvolutionSystem;
struct Config;

/**
 * @class GSLIntegrator
 * @brief Thin RAII wrapper around `gsl_odeiv2_driver`.
 *
 * Usage sketch:
 *   - Construct your StateVector / StateLayout / RHSAccumulator / EvolutionSystem.
 *   - Let N = layout.TotalSize().
 *   - Allocate `std::vector<double> y(N)` and pack the State blocks into it.
 *   - Construct `GSLIntegrator integrator(sys, cfg, N);`
 *   - Call `integrator.Integrate(t0, t1, y.data());`
 *   - Unpack `y` back into State blocks.
 */
class GSLIntegrator
{
  public:
	/**
	 * @brief Construct from RHS functor, configuration, and dimension.
	 *
	 * @param sys   Reference to the evolution RHS functor.
	 * @param cfg   Evolution configuration (tolerances, stepper, max_steps, dt_save).
	 * @param dim   Dimension of the flat ODE vector y[] (must match StateLayout::TotalSize()).
	 */
	GSLIntegrator(const EvolutionSystem &sys,
				  const Config &cfg,
				  std::size_t dim);

	/**
	 * @brief Integrate from t0 to t1 in-place on y[].
	 *
	 * @param t0   Start time (s).
	 * @param t1   End time (s).
	 * @param y    In/out state vector of length `dim` (provided at construction).
	 *
	 * @return true if the integration reached t1 successfully;
	 *         false if GSL reported an error or max_steps was exceeded.
	 */
	bool Integrate(double t0, double t1, double *y) const;

  private:
	const EvolutionSystem *m_sys = nullptr;
	const Config *m_cfg = nullptr;
	std::size_t m_dim = 0;
};

} // namespace CompactStar::Physics::Evolution

#endif /* CompactStar_Physics_Evolution_GSLIntegrator_H */