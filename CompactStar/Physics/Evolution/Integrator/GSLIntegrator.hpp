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
 * @brief RAII front-end for a GSL ODE driver (gsl_odeiv2).
 *
 * Wraps stepper choice, tolerances, and integrates a flat ODE system
 *
 *     dy/dt = f(t, y)
 *
 * where f(t, y) is provided by Physics::Evolution::EvolutionSystem.
 *
 * Packing/unpacking between State blocks and the flat y[] buffer is handled
 * elsewhere (StateLayout + StatePacking); this class only sees the flat y[].
 *
 * @ingroup PhysicsEvolution
 */

#ifndef CompactStar_Physics_Evolution_Integrator_GSLIntegrator_H
#define CompactStar_Physics_Evolution_Integrator_GSLIntegrator_H

#include <cstddef>

namespace CompactStar
{
namespace Physics
{
namespace Evolution
{

class EvolutionSystem;
struct Config;

/**
 * @class GSLIntegrator
 * @brief Thin RAII wrapper around gsl_odeiv2_driver.
 *
 * Public API is deliberately minimal:
 *   - construct with (EvolutionSystem, Config),
 *   - call Integrate(t0, t1, y, n).
 *
 * The caller is responsible for:
 *   - allocating y[0..n-1],
 *   - packing/unpacking state via StateLayout / StatePacking,
 *   - calling observers (if any) around Integrate().
 */
class GSLIntegrator
{
  public:
	/**
	 * @brief Construct from RHS and configuration.
	 *
	 * Stores **non-owning** pointers to @p sys and @p cfg; the caller
	 * must ensure they outlive this integrator.
	 */
	GSLIntegrator(const EvolutionSystem &sys, const Config &cfg);

	/**
	 * @brief Integrate from t0 to t1, updating y in place.
	 *
	 * Uses gsl_odeiv2_driver with:
	 *   - dimension n,
	 *   - stepper chosen from Config::stepper,
	 *   - tolerances taken from Config::rtol / Config::atol.
	 *
	 * @param t0  Start time.
	 * @param t1  End time.
	 * @param y   In/out state vector of length @p n.
	 * @param n   Dimension of the ODE system.
	 *
	 * @return true on success (GSL_SUCCESS); false otherwise.
	 */
	bool Integrate(double t0, double t1, double *y, std::size_t n) const;

  private:
	const EvolutionSystem *m_sys = nullptr;
	const Config *m_cfg = nullptr;
};

} // namespace Evolution
} // namespace Physics
} // namespace CompactStar

#endif /* CompactStar_Physics_Evolution_Integrator_GSLIntegrator_H */