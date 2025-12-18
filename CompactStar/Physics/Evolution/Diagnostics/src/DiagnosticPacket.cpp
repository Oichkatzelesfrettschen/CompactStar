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

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"

#include <cmath>
#include <stdexcept>

namespace CompactStar::Physics::Evolution::Diagnostics
{
//--------------------------------------------------------------
DiagnosticPacket::DiagnosticPacket(std::string producer)
	: producer_(std::move(producer))
{
}
//--------------------------------------------------------------
void DiagnosticPacket::SetTime(double t)
{
	time_ = t;
}
//--------------------------------------------------------------
void DiagnosticPacket::SetStepIndex(std::size_t step)
{
	step_index_ = step;
}
//--------------------------------------------------------------

void DiagnosticPacket::SetRunId(std::string run_id)
{
	run_id_ = std::move(run_id);
}

void DiagnosticPacket::SetProducer(std::string producer)
{
	producer_ = std::move(producer);
}
//--------------------------------------------------------------

void DiagnosticPacket::AddScalar(const std::string &key,
								 double value,
								 std::string unit,
								 std::string description,
								 std::string source)
{
	ScalarEntry e;
	e.value = value;
	e.unit = std::move(unit);
	e.description = std::move(description);
	e.source = std::move(source);
	e.is_finite = std::isfinite(value);

	scalars_[key] = std::move(e);
}
//--------------------------------------------------------------
void DiagnosticPacket::AddScalar(const std::string &key,
								 double value,
								 std::string unit,
								 std::string description,
								 std::string source,
								 Cadence cadence)
{
	ScalarEntry e;
	e.value = value;
	e.unit = std::move(unit);
	e.description = std::move(description);
	e.source = std::move(source);
	e.is_finite = std::isfinite(value);
	e.cadence = cadence;

	scalars_[key] = std::move(e);
}

//--------------------------------------------------------------

bool DiagnosticPacket::HasScalar(const std::string &key) const
{
	return scalars_.find(key) != scalars_.end();
}
//--------------------------------------------------------------

const ScalarEntry &DiagnosticPacket::GetScalar(const std::string &key) const
{
	return scalars_.at(key);
}
//--------------------------------------------------------------

void DiagnosticPacket::ClearScalars()
{
	scalars_.clear();
}
//--------------------------------------------------------------

void DiagnosticPacket::AddContractLine(std::string line)
{
	contract_lines_.push_back(std::move(line));
}
//--------------------------------------------------------------

void DiagnosticPacket::AddWarning(std::string line)
{
	warnings_.push_back(std::move(line));
}
//--------------------------------------------------------------

void DiagnosticPacket::AddError(std::string line)
{
	errors_.push_back(std::move(line));
}
//--------------------------------------------------------------

void DiagnosticPacket::AddNote(std::string line)
{
	notes_.push_back(std::move(line));
}
//--------------------------------------------------------------

void DiagnosticPacket::ClearTextBlocks()
{
	contract_lines_.clear();
	warnings_.clear();
	errors_.clear();
	notes_.clear();
}
//--------------------------------------------------------------

void DiagnosticPacket::ValidateBasic()
{
	// Validate finiteness of all scalars.
	for (const auto &kv : scalars_)
	{
		const auto &key = kv.first;
		const auto &e = kv.second;

		if (!e.is_finite)
		{
			AddError("Non-finite scalar: '" + key + "'");
		}
	}

	// Producer is helpful for tooling.
	if (producer_.empty())
	{
		AddWarning("DiagnosticPacket producer is empty.");
	}
}
//--------------------------------------------------------------

} // namespace CompactStar::Physics::Evolution::Diagnostics