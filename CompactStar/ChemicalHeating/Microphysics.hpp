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
 * @file Microphysics.hpp
 * @brief Abstract interface for local thermal microphysics (c_V, reductions).
 *
 * Provides specific heat and optional superfluid reduction factors. Emissivities
 * and reaction rates live in @c ReactionChannel implementations.
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_Microphysics_H
#define CompactStar_ChemicalHeating_Microphysics_H

#include <cstddef>

namespace CompactStar
{
namespace ChemicalHeating
{
class StarContext;
}
} // namespace CompactStar

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                      Microphysics Class
//==============================================================
/**
 * @class Microphysics
 * @brief Strategy interface for thermal coefficients needed by the RHS.
 */
class Microphysics
{
  public:
	virtual ~Microphysics() {}

	/**
	 * @brief Local specific heat per unit volume \f$c_V(r,T)\f$.
	 * @param i Radial zone index.
	 * @param T Local temperature (K).
	 * @return \f$c_V\f$ in [erg cm^{-3} K^{-1}].
	 */
	virtual double cV(std::size_t i, double T) const = 0;

	/**
	 * @brief Optional multiplicative reduction (e.g., superfluid gaps) for rates.
	 * @param i Radial zone index.
	 * @param T Local temperature (K).
	 * @return Dimensionless factor \f$0 \le R \le 1\f$.
	 */
	virtual double reduction(std::size_t i, double T) const
	{
		(void)i;
		(void)T;
		return 1.0;
	}
};

} // namespace ChemicalHeating
//==============================================================
} // namespace CompactStar
//==============================================================
#endif /* CompactStar_ChemicalHeating_Microphysics_H */