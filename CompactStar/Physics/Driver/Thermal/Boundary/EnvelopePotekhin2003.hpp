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
 * @file EnvelopePotekhin2003.hpp
 * @brief Potekhin et al. (2003-era) Tb→Ts envelope interface (currently delegating to 1997 fits).
 *
 * @ingroup PhysicsDriverThermalBoundary
 *
 * ## Status
 * This header provides the *2003 model identifiers* and stable class names so that:
 * - Evolution::Config can select a "Potekhin2003" family without refactoring,
 * - drivers remain stable,
 * - you can later replace the implementation with the proper 2003 piecewise/log-polynomial fits.
 *
 * ## Current behavior
 * The .cpp implementation currently delegates to the Potekhin1997 models:
 * - `EnvelopePotekhin2003_Iron`     → `EnvelopePotekhin1997_Iron`
 * - `EnvelopePotekhin2003_Accreted` → `EnvelopePotekhin1997_Accreted`
 *
 * The interface remains identical:
 *   Ts = Ts_from_Tb(Tb, g14, xi)
 *
 * Conventions:
 * - Tb  : base-of-envelope local temperature [K]
 * - Ts  : local effective surface temperature [K]
 * - g14 : surface gravity in units of 1e14 cm s^-2
 * - xi  : light-element/accreted parameter (model dependent)
 */

#ifndef CompactStar_Physics_Driver_Thermal_Boundary_EnvelopePotekhin2003_H
#define CompactStar_Physics_Driver_Thermal_Boundary_EnvelopePotekhin2003_H

#include "CompactStar/Physics/Driver/Thermal/Boundary/IEnvelope.hpp"

namespace CompactStar::Physics::Driver::Thermal::Boundary
{

/**
 * @brief Potekhin2003 iron envelope model (currently delegates to Potekhin1997 iron fit).
 */
class EnvelopePotekhin2003_Iron final : public IEnvelope
{
  public:
	~EnvelopePotekhin2003_Iron() override = default;

	/// Map Tb→Ts using the 2003-family interface (delegates for now).
	double Ts_from_Tb(double Tb, double g14, double xi) const override;

	const char *ModelName() { return "Potekhin2003_Iron"; }
};

/**
 * @brief Potekhin2003 accreted envelope model (currently delegates to Potekhin1997 accreted fit).
 */
class EnvelopePotekhin2003_Accreted final : public IEnvelope
{
  public:
	~EnvelopePotekhin2003_Accreted() override = default;

	/// Map Tb→Ts using the 2003-family interface (delegates for now).
	double Ts_from_Tb(double Tb, double g14, double xi) const override;

	const char *ModelName() override { return "Potekhin2003_Accreted"; }
};

} // namespace CompactStar::Physics::Driver::Thermal::Boundary

#endif /* CompactStar_Physics_Driver_Thermal_Boundary_EnvelopePotekhin2003_H */