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
 * @file SpinState.hpp
 * @brief Spin/kinematic observables for a compact star.
 * @ingroup Physics
 *
 * @author Mohammadreza Zakeri
 * Contact: M.Zakeri@eku.edu
 */
#ifndef CompactStar_SpinState_H
#define CompactStar_SpinState_H

#include <Zaki/Math/Math_Core.hpp>

namespace CompactStar
{
/**
 * @struct SpinState
 * @brief Observed spin and kinematic parameters.
 * @ingroup Physics
 *
 * Keep this orthogonal to structure; it can be attached to any StarProfile.
 */
struct SpinState
{
	Zaki::Math::Quantity P;	   ///< Spin period P [s].
	Zaki::Math::Quantity Pdot; ///< Period derivative dP/dt [s/s].
	Zaki::Math::Quantity mu;   ///< Proper motion μ [mas/yr].
	Zaki::Math::Quantity d;	   ///< Distance from SSB [kpc].
};

} // namespace CompactStar

#endif /* CompactStar_SpinState_H */