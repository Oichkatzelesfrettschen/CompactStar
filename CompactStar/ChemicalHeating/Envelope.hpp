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
 * @file Envelope.hpp
 * @brief Interface for mapping base-of-envelope temperature to surface temperature.
 *
 * Typical usage: given a redshifted internal temperature (isothermal-core), compute
 * \f$T_b(r\approx R)\f$ and map to \f$T_s\f$ with a model (iron or accreted envelope).
 *
 * @ingroup ChemicalHeating
 */
#ifndef CompactStar_ChemicalHeating_Envelope_H
#define CompactStar_ChemicalHeating_Envelope_H

namespace CompactStar
{
namespace ChemicalHeating
{

//==============================================================
//                        IEnvelope Class
//==============================================================
/**
 * @class IEnvelope
 * @brief Strategy for \f$T_b \to T_s\f$ mapping.
 */
class IEnvelope
{
  public:
	virtual ~IEnvelope() {}

	/**
	 * @brief Map base temperature to surface temperature.
	 * @param Tb  Base-of-envelope temperature (K), local frame.
	 * @param g14 Surface gravity in units of \f$10^{14}\,\mathrm{cm\,s^{-2}}\f$.
	 * @param xi  Light-element parameter (column depth proxy).
	 * @return Surface temperature (K), local frame.
	 */
	virtual double Ts_from_Tb(double Tb, double g14, double xi) const = 0;
};

} // namespace ChemicalHeating
} // namespace CompactStar

#endif /* CompactStar_ChemicalHeating_Envelope_H */