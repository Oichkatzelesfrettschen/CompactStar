#ifndef COMPACTSTAR_PHYSICS_DRIVER_THERMAL_BOUNDARY_IENVELOPE_HPP
#define COMPACTSTAR_PHYSICS_DRIVER_THERMAL_BOUNDARY_IENVELOPE_HPP
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
 * @file IEnvelope.hpp
 * @brief Thermal envelope (blanket) interface: maps base temperature Tb to surface temperature Ts.
 *
 * This header defines the abstract interface for neutron-star envelope/blanket models.
 * Envelope models provide the thermal boundary condition connecting the nearly-isothermal
 * interior to the radiating surface:
 *
 *   - The interior is typically evolved in terms of a single temperature variable
 *     (e.g., redshifted temperature T∞ or a local core temperature).
 *   - The outer envelope (ρ ≲ 10^10 g cm^-3, order-of-magnitude) is not evolved explicitly
 *     in the ODE system; instead its effect is encoded in a phenomenological/fit mapping
 *     Tb → Ts that depends on surface gravity and composition (light-element content).
 *
 * Typical usage:
 *   - PhotonCooling evaluates Ts(Tb, g14, xi) and then computes Lγ,∞.
 *   - The implementation choice (iron vs accreted vs custom) is injected via options/config.
 *
 * Conventions:
 *   - Tb and Ts are **local-frame** temperatures [K] (not redshifted).
 *   - g14 is surface gravity in units of 10^14 cm s^-2.
 *   - xi is a model-dependent “light-element” parameter. Its meaning depends on the
 *     chosen envelope prescription (e.g., column depth, composition proxy, etc.).
 *
 * This interface is intentionally small and pure; it carries no ownership of star structure,
 * geometry caches, or EOS. Those belong in higher-level objects (StarContext/GeometryCache)
 * and are used by drivers to compute inputs like g14 or to decide what Tb represents.
 *
 * @ingroup Thermal
 */

namespace CompactStar::Physics::Driver::Thermal::Boundary
{

/**
 * @class IEnvelope
 * @brief Abstract strategy for the thermal boundary condition Tb → Ts.
 *
 * An envelope model supplies a mapping from the temperature at the base of the envelope Tb
 * (often defined near ρ_b ~ 10^10 g cm^-3) to the local effective surface temperature Ts,
 * optionally depending on:
 *  - surface gravity (via g14),
 *  - composition / light-element content (via xi).
 *
 * This boundary condition is one of the dominant systematic inputs in photon-cooling
 * calculations because it controls the relation between the evolved interior temperature
 * variable and the observable photon luminosity.
 *
 * Notes:
 *  - Implementations should be deterministic and side-effect free.
 *  - Implementations should handle physically invalid inputs defensively (documented
 *    behavior: either clamp/return NaN/throw; choose one policy and keep consistent).
 *  - This interface does not mandate any particular functional form; fits (e.g. Potekhin
 *    et al.) and tabulated mappings are both supported.
 */
class IEnvelope
{
  public:
	/// Virtual destructor for safe polymorphic deletion.
	virtual ~IEnvelope() = default;

	/**
	 * @brief Get the unique model name/identifier.
	 *
	 * Used for diagnostics, logging, and configuration.
	 *
	 * @return Null-terminated string with the model name.
	 */
	virtual const char *ModelName() = 0;

	/**
	 * @brief Compute local surface temperature Ts from base-of-envelope temperature Tb.
	 *
	 * @param Tb  Base-of-envelope temperature [K], local frame.
	 *            This is typically the temperature at the inner boundary of the
	 *            envelope/blanket region, where the star is assumed isothermal.
	 *
	 * @param g14 Surface gravity in units of 10^14 cm s^-2.
	 *            (i.e., g14 = g / (1e14 cm s^-2)).
	 *
	 * @param xi  Composition/light-element parameter (dimensionless or model-defined).
	 *            Interpretation is model dependent (e.g., accreted layer parameterization).
	 *
	 * @return Local effective surface temperature Ts [K], local frame.
	 *
	 * @warning This method returns a *local* Ts. If you need the redshifted effective
	 *          temperature T∞, apply the appropriate surface redshift separately.
	 */
	virtual double Ts_from_Tb(double Tb, double g14, double xi) const = 0;

	/**
	 * @brief Optional inverse mapping Ts → Tb (not required).
	 *
	 * Many cooling workflows only need Tb → Ts. If you need Ts → Tb (e.g. when setting
	 * initial conditions from an observed surface temperature), implement it in a derived
	 * class and expose it there; this base interface intentionally does not require it.
	 *
	 * Keeping the base interface minimal avoids forcing numerical inversion on all models.
	 */
	// virtual double Tb_from_Ts(double Ts, double g14, double xi) const;
};

} // namespace CompactStar::Physics::Driver::Thermal::Boundary

#endif /* COMPACTSTAR_PHYSICS_DRIVER_THERMAL_BOUNDARY_IENVELOPE_HPP */