// -*- lsst-c++ -*-
/*
 * CompactStar
 * See License file at the top of the source tree.
 *
 * Copyright (c) 2025 Mohammadreza Zakeri
 *
 * MIT License — see LICENSE at repo root.
 */

/**
 * @file Tags.hpp
 * @brief Enumeration of physical state components used by evolution drivers.
 *
 * Provides canonical identifiers (`StateTag`) for each sub-state (spin, thermal,
 * chemical, BNV, etc.) so that drivers and the evolution graph can declare
 * dependencies and update targets in a type-safe manner.
 *
 * @ingroup PhysicsState
 *
 * Example:
 * @code
 * static constexpr std::array<StateTag,2> deps{
 *     StateTag::Spin,
 *     StateTag::Thermal
 * };
 * @endcode
 *
 * @see CompactStar::Physics::IDriver
 * @see CompactStar::Physics::Evolution::System
 *
 * @author
 *   Mohammadreza Zakeri (M.Zakeri@eku.edu)
 */

#ifndef CompactStar_Physics_State_Tags_H
#define CompactStar_Physics_State_Tags_H

#include <string>
#include <string_view>

namespace CompactStar::Physics::State
{

/**
 * @enum StateTag
 * @brief Enumerates high-level dynamic subsystems ("state blocks") of the model.
 *
 * Each distinct `StateTag` corresponds to one substate in the global
 * `StateVector` managed by the evolution system.
 */
enum class StateTag : unsigned int
{
	Spin = 0,  ///< Rotational degree of freedom (Ω, Ω̇, I, etc.)
	Thermal,   ///< Temperature / energy content (T̃, C_v, L_ν, L_γ)
	Chem,	   ///< Chemical imbalance (η_e, η_μ, etc.)
	BNV,	   ///< Baryon-number-violating or exotic particle population
	Structure, ///< Optional: structural/geometric parameters (R, M, etc.)
	Magnetic,  ///< Optional: magnetic-field state (if evolved explicitly)
	Custom	   ///< Placeholder for user-extended or experimental subsystems
};

/**
 * @brief Convert a `StateTag` to a short human-readable name.
 *
 * @param tag  Enumeration value.
 * @return Constant string_view naming the state.
 */
inline constexpr std::string_view ToString(StateTag tag)
{
	using enum StateTag;
	switch (tag)
	{
	case Spin:
		return "Spin";
	case Thermal:
		return "Thermal";
	case Chem:
		return "Chem";
	case BNV:
		return "BNV";
	case Structure:
		return "Structure";
	case Magnetic:
		return "Magnetic";
	case Custom:
		return "Custom";
	}
	return "Unknown";
}

/**
 * @brief Return a printable `std::string` from a `StateTag`.
 *
 * @param tag Enumeration value.
 * @return Printable string.
 */
inline std::string ToStringCopy(StateTag tag)
{
	return std::string(ToString(tag));
}

} // namespace CompactStar::Physics::State

#endif /* CompactStar_Physics_State_Tags_H */