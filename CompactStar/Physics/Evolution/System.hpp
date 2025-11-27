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
 * @file EvolutionSystem.hpp
 * @brief Right-hand side (RHS) functor for the ODE system dY/dt.
 *
 * The RHS assembles volume integrals using GeometryCache and RateSet, and computes
 * time derivatives for \f$T^\infty\f$ and \f$\boldsymbol{\eta}\f$ (and optionally \f$\Omega\f$).
 * Designed to be wrapped by a GSL driver in @c IntegratorGSL.
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_EvolutionSystem_H
#define CompactStar_ChemicalHeating_EvolutionSystem_H

#include <vector>

namespace CompactStar
{
namespace ChemicalHeating
{
class StarContext;
class GeometryCache;
class Microphysics;
class IEnvelope;
class RateSet;
struct EvolutionState;
struct Config;
} // namespace ChemicalHeating
} // namespace CompactStar

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                      EvolutionSystem Class
//==============================================================
/**
 * @class EvolutionSystem
 * @brief GSL-compatible RHS functor (interfaces only in Phase 0).
 */
class EvolutionSystem
{
  public:
	/**
	 * @brief Bundles read-only references to the model components.
	 */
	struct Context
	{
		const StarContext *star = 0;
		const GeometryCache *geo = 0;
		const Microphysics *micro = 0;
		const IEnvelope *envelope = 0;
		const RateSet *rates = 0;
		const Config *cfg = 0;
	};

	/** @brief Construct with a fully-populated Context. */
	explicit EvolutionSystem(const Context &ctx);

	/**
	 * @brief Evaluate RHS \f$\dot{y} = f(t,y)\f$.
	 *
	 * Signature mirrors GSL callbacks (no includes here). Implementation will:
	 *  1) convert \f$T^\infty \to T(r)\f$ via redshift,
	 *  2) assemble integrals for \f{C, L_\nu^\infty, H^\infty, L_\gamma^\infty\f},
	 *  3) compute \f$\dot{T}^\infty\f$ and \f$\dot{\boldsymbol{\eta}}\f$ (and \f$\dot\Omega\f$ if coupled).
	 *
	 * @param t     Time (s).
	 * @param y     State array (size = 1 + n_eta + [\Omega]).
	 * @param dydt  Output derivatives (same size as @p y).
	 * @return 0 if success; nonzero on failure (consistent with GSL codes).
	 */
	int operator()(double t, const double y[], double dydt[]) const;

  private:
	Context m_ctx;
};
//==============================================================
} // namespace ChemicalHeating
//==============================================================
} // namespace CompactStar
//==============================================================
#endif /* CompactStar_ChemicalHeating_EvolutionSystem_H */