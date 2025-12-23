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
 * @file Thermal.hpp
 * @brief Thermal physics algorithms operating on StarProfile and ThermalState.
 *
 * Header-only declarations; put implementations in Thermal.cpp.
 * @ingroup Physics
 *
 * @author Mohammadreza Zakeri
 * Contact: M.Zakeri@eku.edu
 */
#ifndef CompactStar_Physics_Thermal_H
#define CompactStar_Physics_Thermal_H

#include "CompactStar/Core/StarProfile.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

/**
 * @namespace CompactStar::Physics::Thermal
 * @brief Thermal utilities: cooling/heating helpers.
 *
 * Contains static thermal functions (e.g., neutrino luminosities)
 *  and interfaces used by thermal drivers.
 */
namespace CompactStar::Physics::Thermal
{

/**
 * @brief Redshifted photon luminosity \(L_\infty\) from the surface.
 * @ingroup Physics
 *
 * Uses Stefan–Boltzmann law with gravitational redshift. Assumes thin blanket
 * (R_blanket ≈ R) so that \(T_\infty = z_\text{surf}\, T_\text{surf}\).
 *
 * @param view   Non-owning structural profile view (needs R and z_surf).
 * @param state  Thermal state (uses T_surf).
 * @return Redshifted luminosity \(L_\infty\) in [erg s\(^{-1}\)].
 *
 * @note If we prefer to use a \(T_\infty\)-based fit (e.g., \(T_\infty^{2.42}\)),
 *       we should implement a sibling function instead of overloading this one.
 */
double SurfacePhotonLuminosity(Core::StarProfileView view, const State::ThermalState &state);

/**
 * @brief Direct Urca neutrino emissivity \(Q_\nu^{\text{DUrca}}(r)\).
 * @ingroup Physics
 *
 * Returns the *prefactor-only* emissivity in \([ \mathrm{erg}\ \mathrm{cm}^{-3}\ \mathrm{s}^{-1} ]\)
 * so that callers can scale with \( (T_\infty/10^9\,\mathrm{K})^6 \) when desired.
 *
 * @param view    Structural profile (uses composition/threshold masks if available).
 * @param state   Thermal state (may use T_core if working in local frame).
 * @param r_km    Radius at which to evaluate [km].
 * @return Emissivity prefactor at r (exclude the \(T^6\) factor).
 */
double DUrcaEmissivityPrefactor(Core::StarProfileView view,
								const State::ThermalState &state,
								double r_km);

/**
 * @brief Modified Urca neutrino emissivity \(Q_\nu^{\text{MUrca}}(r)\), prefactor form.
 * @ingroup Physics
 * @param view    Structural profile view.
 * @param state   Thermal state.
 * @param r_km    Radius [km].
 * @return Emissivity prefactor at r (exclude the \(T^8\) factor).
 */
double MUrcaEmissivityPrefactor(Core::StarProfileView view,
								const State::ThermalState &state,
								double r_km);

/**
 * @brief Total redshifted neutrino luminosity \(L_{\nu,\infty}\) (DUrca+MUrca).
 * @ingroup Physics
 *
 * Integrates emissivities over proper volume with metric factors. Callers decide whether
 * to pass local T or a redshifted temperature ansatz.
 *
 * @param view   Structural profile view.
 * @param state  Thermal state.
 * @return Redshifted neutrino luminosity [erg s\(^{-1}\)].
 */
double TotalNeutrinoLuminosity(Core::StarProfileView view, const State::ThermalState &state);

} // namespace CompactStar::Physics::Thermal

#endif /* CompactStar_Physics_Thermal_H */