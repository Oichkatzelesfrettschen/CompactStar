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
 * @file EnvelopePotekhin2003.cpp
 * @brief Implementation of Potekhin2003-family Tb→Ts envelopes (currently delegating to 1997 fits).
 *
 * @ingroup PhysicsDriverThermalBoundary
 *
 * ## Implementation note
 * The true Potekhin et al. 2003 fits are often provided as piecewise/log-polynomial
 * expressions for Ts(Tb, g14, composition/light-element column).
 *
 * For now, these classes forward to the already-available Potekhin1997 implementations
 * to keep the code path functional while preserving a stable "2003" API.
 *
 * Replace the forwarding with the real 2003 formulas when you are ready; no driver
 * code should need to change.
 */

#include "CompactStar/Physics/Driver/Thermal/Boundary/EnvelopePotekhin2003.hpp"

#include "CompactStar/Physics/Driver/Thermal/Boundary/EnvelopePotekhin1997.hpp"

namespace CompactStar::Physics::Driver::Thermal::Boundary
{

double EnvelopePotekhin2003_Iron::Ts_from_Tb(double Tb, double g14, double xi) const
{
	// Delegate: 2003 iron → 1997 iron (xi ignored by iron fit)
	return EnvelopePotekhin1997_Iron{}.Ts_from_Tb(Tb, g14, xi);
}

double EnvelopePotekhin2003_Accreted::Ts_from_Tb(double Tb, double g14, double xi) const
{
	// Delegate: 2003 accreted → 1997 accreted (xi used as parameter)
	return EnvelopePotekhin1997_Accreted{}.Ts_from_Tb(Tb, g14, xi);
}

} // namespace CompactStar::Physics::Driver::Thermal::Boundary