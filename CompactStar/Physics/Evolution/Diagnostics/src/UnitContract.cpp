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

#include "CompactStar/Physics/Evolution/Diagnostics/UnitContract.hpp"

namespace CompactStar::Physics::Evolution::Diagnostics
{
//--------------------------------------------------------------

void UnitContract::AddLine(std::string line)
{
	lines_.push_back(std::move(line));
}
//--------------------------------------------------------------

UnitVocabulary::UnitVocabulary(std::set<std::string> allowed_units)
	: allowed_(std::move(allowed_units))
{
}
//--------------------------------------------------------------

bool UnitVocabulary::IsAllowed(const std::string &unit) const
{
	if (allowed_.empty())
		return true;

	// empty unit string: allow it (some fields can be dimensionless or omitted)
	if (unit.empty())
		return true;

	return allowed_.find(unit) != allowed_.end();
}
//--------------------------------------------------------------

void UnitVocabulary::AddAllowed(std::string unit)
{
	allowed_.insert(std::move(unit));
}
//--------------------------------------------------------------

} // namespace CompactStar::Physics::Evolution::Diagnostics