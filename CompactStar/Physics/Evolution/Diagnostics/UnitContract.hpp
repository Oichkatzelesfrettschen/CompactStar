#pragma once
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
 * @file UnitContract.hpp
 * @brief Runtime “unit contract” utilities for diagnostics.
 *
 * This module supports two ideas you discussed:
 *
 * (1) Centralize unit assumptions in one place per driver (runtime contract):
 *     - A driver provides a UnitContract with stable text lines.
 *     - Diagnostics can print the contract verbatim (no duplication of formulas).
 *
 * (2) Enforce a global allowed unit vocabulary (runtime linting):
 *     - You can validate that unit strings used by diagnostics are from a
 *       predefined set (e.g., "K", "erg/s", "cm^2", "rad/s").
 *     - This prevents drift like "erg s^-1" vs "erg/s" across drivers.
 *
 * Important:
 *  - This is NOT a unit type system; it is metadata and linting.
 *  - Validation should be cheap and used only in debug/diagnostic paths.
 */

#include <set>
#include <string>
#include <vector>

namespace CompactStar::Physics::Evolution::Diagnostics
{

/**
 * @brief A simple runtime contract describing unit conventions and assumptions.
 *
 * The contract is intended to be:
 *  - stable (changes only when conventions/assumptions change),
 *  - human-readable (printed in logs and exported),
 *  - tool-friendly (line-based text).
 */
class UnitContract
{
  public:
	UnitContract() = default;

	/**
	 * @brief Add one contract line.
	 *
	 * Example:
	 *   "T_inf, T_surf are Kelvin [K]"
	 */
	void AddLine(std::string line);

	/**
	 * @brief Return the lines.
	 */
	[[nodiscard]] const std::vector<std::string> &Lines() const { return lines_; }

	/**
	 * @brief Return true if no lines exist.
	 */
	[[nodiscard]] bool Empty() const { return lines_.empty(); }

  private:
	std::vector<std::string> lines_;
};

/**
 * @brief Global allowed-vocabulary for unit strings.
 *
 * You can keep this strict or permissive. Strict is better for regression tests.
 * If you want to allow arbitrary units, just skip validation or keep vocabulary empty.
 */
class UnitVocabulary
{
  public:
	UnitVocabulary() = default;

	/**
	 * @brief Initialize with a predefined allowed set.
	 */
	explicit UnitVocabulary(std::set<std::string> allowed_units);

	/**
	 * @brief Return true if the unit is allowed.
	 *
	 * If the vocabulary is empty, this returns true (treat as “no enforcement”).
	 */
	[[nodiscard]] bool IsAllowed(const std::string &unit) const;

	/**
	 * @brief Add one allowed unit string.
	 */
	void AddAllowed(std::string unit);

	/**
	 * @brief Return the current allowed set.
	 */
	[[nodiscard]] const std::set<std::string> &Allowed() const { return allowed_; }

  private:
	std::set<std::string> allowed_;
};

} // namespace CompactStar::Physics::Evolution::Diagnostics