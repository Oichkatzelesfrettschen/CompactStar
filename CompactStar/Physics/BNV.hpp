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
 * @file BNV.hpp
 * @brief Baryon-number-violation utilities built on profiles + spin state.
 * @ingroup Physics
 *
 * @author Mohammadreza Zakeri
 * Contact: M.Zakeri@eku.edu
 */
#ifndef CompactStar_BNV_H
#define CompactStar_BNV_H

#include "CompactStar/Core/StarProfile.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"

namespace CompactStar
{
namespace BNV
{
/**
 * @brief Spin-down energy budget bound on the total BNV rate.
 * @ingroup Physics
 *
 * Computes an upper limit on \f$\Gamma_{\mathrm{BNV}}\f$ by requiring the BNV
 * heating power not to exceed the observed spin-down power (model assumptions apply).
 *
 * @param view Structural profile view (may use I(M,R) if available).
 * @param s    Spin state (uses P, Pdot).
 * @return Upper bound on \f$\Gamma_{\mathrm{BNV}}\f$ [1/yr].
 */
double SpinDownLimit(StarProfileView view, const SpinState &s);

} // namespace BNV
} // namespace CompactStar

#endif /* CompactStar_BNV_H */