// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025
 * Mohammadreza Zakeri
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
 *
 * Provides helper routines to estimate bounds on the total BNV rate from
 * pulsar spin-down, and to populate the BNVState block with corresponding
 * diagnostics (e.g. Γ_BNV limits).
 *
 * Directory-based namespace: CompactStar::Physics::BNV
 *
 * @ingroup Physics
 *
 * @author
 *  Mohammadreza Zakeri (M.Zakeri@eku.edu)
 */

#ifndef CompactStar_Physics_BNV_H
#define CompactStar_Physics_BNV_H

#include "CompactStar/Core/StarProfile.hpp"
#include "CompactStar/Physics/State/BNVState.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"

/**
 * @namespace CompactStar::Physics::BNV
 * @brief High-level baryon-number-violation utilities.
 *
 * Contains spin-down limits, observational constraints, and wrappers that
 * connect pulsar quantities to BNV reaction models in Microphysics.
 */
namespace CompactStar::Physics::BNV
{

/**
 * @brief Spin-down energy budget bound on the total BNV rate.
 *
 * Computes an upper limit on \f$\Gamma_{\mathrm{BNV}}\f$ by requiring the
 * BNV heating power not to exceed the observed spin-down power
 * (model assumptions apply).
 *
 * This is a pure utility function and does not modify any state blocks.
 *
 * @param view Structural profile view (may use I(M,R) if available).
 * @param s    Spin state (uses P, Pdot).
 * @return Upper bound on \f$\Gamma_{\mathrm{BNV}}\f$ [1/yr].
 */
double SpinDownLimit(Core::StarProfileView view, const ::CompactStar::Physics::State::SpinState &s);

/**
 * @brief Compute the spin-down bound and store it in a BNVState block.
 *
 * Convenience wrapper around SpinDownLimit(...) that writes the resulting
 * bound into the BNV evolution/diagnostic state. This keeps BNV-related
 * quantities available to drivers and observers without recomputing them.
 *
 * By convention, the spin-down limit is stored in
 *   bnv.SpinDownLimit()
 * matching the accessor provided by Physics::State::BNVState.
 *
 * @param view Structural profile view (may use I(M,R) if available).
 * @param s    Spin state (uses P, Pdot).
 * @param bnv  BNV state block to update (SpinDownLimit() field is set).
 */
void UpdateSpinDownLimit(Core::StarProfileView view,
						 const ::CompactStar::Physics::State::SpinState &s,
						 Physics::State::BNVState &bnv);

} // namespace CompactStar::Physics::BNV

#endif /* CompactStar_Physics_BNV_H */