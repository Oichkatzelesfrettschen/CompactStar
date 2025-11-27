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
 * @file ThermalState.hpp
 * @brief Thermal state variables for a compact star.
 * @ingroup Physics
 *
 * @author Mohammadreza Zakeri
 * Contact: M.Zakeri@eku.edu
 */

#ifndef CompactStar_ThermalState_H
#define CompactStar_ThermalState_H

namespace CompactStar
{
/**
 * @struct ThermalState
 * @brief Local (non-redshifted) temperatures at key regions.
 * @ingroup Physics
 *
 * Temperatures are in kelvin as measured in the local fluid frame.
 */
struct ThermalState
{
	double T_core = 0.0;	///< Core temperature [K].
	double T_blanket = 0.0; ///< Heat-blanket temperature [K] (ρ~1e10 g/cm^3).
	double T_surf = 0.0;	///< Surface temperature [K].
};

} // namespace CompactStar

#endif /* CompactStar_ThermalState_H */