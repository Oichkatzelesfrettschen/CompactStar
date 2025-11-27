// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file IntegratorGSL.hpp
 * @brief RAII front-end for a GSL ODE driver (interfaces only in Phase 0).
 *
 * Wraps stepper choice, tolerances, and integrates while invoking an Observer.
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_IntegratorGSL_H
#define CompactStar_ChemicalHeating_IntegratorGSL_H

#include <cstddef>

namespace CompactStar
{
namespace ChemicalHeating
{
class EvolutionSystem;
struct EvolutionState;
struct Config;
class Observer;
} // namespace ChemicalHeating
} // namespace CompactStar

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                      IntegratorGSL Class
//==============================================================
/**
 * @class IntegratorGSL
 * @brief Thin RAII wrapper around gsl_odeiv2 (impl added in Phase 1).
 */
class IntegratorGSL
{
  public:
	/** @brief Construct from RHS and configuration (uses Config tolerances/stepper). */
	IntegratorGSL(const EvolutionSystem &sys, const Config &cfg);

	/**
	 * @brief Integrate from t0 to t1, saving every @c cfg.dt_save via the Observer.
	 * @param t0    Start time (s).
	 * @param t1    End time (s).
	 * @param state In/out state (read initial y, write final y).
	 * @param obs   Observer to receive saved samples.
	 * @return True if completed to @p t1; false if aborted (error or guard).
	 */
	bool integrate(double t0, double t1, EvolutionState &state, Observer &obs) const;

  private:
	const EvolutionSystem *m_sys = 0;
	const Config *m_cfg = 0;
};

} // namespace ChemicalHeating
} // namespace CompactStar

#endif /* CompactStar_ChemicalHeating_IntegratorGSL_H */