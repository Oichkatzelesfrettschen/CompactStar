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

#include "CompactStar/Physics/Evolution/Observers/DiagnosticsObserver.hpp"

#include <stdexcept>

#include <Zaki/Util/Logger.hpp> // Z_LOG_INFO/WARNING/ERROR (assuming available)

namespace CompactStar::Physics::Evolution::Observers
{
// -----------------------------------------------------------------------------
//  Helpers
// -----------------------------------------------------------------------------
void DiagnosticsObserver::OpenOutput()
{
	if (opts_.output_path.Str().empty())
	{
		throw std::runtime_error("DiagnosticsObserver: Options.output_path is empty.");
	}

	const std::ios_base::openmode mode =
		std::ios::out | (opts_.append ? std::ios::app : std::ios::trunc);

	out_.open(opts_.output_path.Str(), mode);
	if (!out_)
	{
		throw std::runtime_error(
			"DiagnosticsObserver: failed to open output file: " + opts_.output_path.Str());
	}
}

// -----------------------------------------------------------------------------
//  Constructor / Destructor
// -----------------------------------------------------------------------------
DiagnosticsObserver::DiagnosticsObserver(const Options &opts)
	: opts_(opts)
{
	OpenOutput();
}
// -----------------------------------------------------------------------------
DiagnosticsObserver::DiagnosticsObserver(Options &&opts)
	: opts_(std::move(opts))
{
	OpenOutput();
}
// -----------------------------------------------------------------------------
/**
 * @brief Construct with options and driver diagnostics providers.
 * @param opts Observer options.
 * @param drivers List of drivers that implement IDriverDiagnostics.
 */
DiagnosticsObserver::DiagnosticsObserver(
	Options opts,
	std::vector<const Driver::Diagnostics::IDriverDiagnostics *> drivers)
	: opts_(std::move(opts)),
	  drivers_(std::move(drivers))
{
	const std::ios_base::openmode mode =
		std::ios::out | (opts_.append ? std::ios::app : std::ios::trunc);

	out_.open(opts_.output_path.Str(), mode);
	if (!out_)
	{
		throw std::runtime_error("DiagnosticsObserver: failed to open output file: " + opts_.output_path.Str());
	}

	// Initialize the time trigger.
	if (opts_.record_every_dt > 0.0)
	{
		next_time_trigger_ = 0.0; // first eligible record at t>=0 unless record_at_start==false
	}
}
// -----------------------------------------------------------------------------
/**
 * @brief Destroy the Diagnostics Observer:: Diagnostics Observer object.
 * 	Closes the output file if open.
 */
DiagnosticsObserver::~DiagnosticsObserver()
{
	if (out_.is_open())
		out_.close();
}

// -----------------------------------------------------------------------------
//  ShouldRecord
// -----------------------------------------------------------------------------
/**
 * @brief Decide whether to record diagnostics at time t.
 * @param t Current simulation time.
 * @return true if recording should occur.
 */
bool DiagnosticsObserver::ShouldRecord(double t) const
{
	// Step trigger
	if (opts_.record_every_n_steps > 0)
	{
		if ((step_counter_ % opts_.record_every_n_steps) == 0)
			return true;
	}

	// Time trigger
	if (opts_.record_every_dt > 0.0)
	{
		if (t >= next_time_trigger_)
			return true;
	}

	return false;
}

// -----------------------------------------------------------------------------
//  Record
// -----------------------------------------------------------------------------
/**
 * @brief Record diagnostics snapshots from all drivers at time t.
 *
 * @param t   Current simulation time.
 * @param Y   Current logical state vector.
 * @param ctx Driver context (geometry, star context, config).
 */
void DiagnosticsObserver::Record(double t,
								 const StateVector &Y,
								 const DriverContext &ctx)
{
	if (!out_)
	{
		Z_LOG_ERROR("DiagnosticsObserver: output stream is not writable; skipping Record().");
		return;
	}

	if (drivers_.empty())
	{
		Z_LOG_WARNING("DiagnosticsObserver: drivers_ is empty; nothing to record.");
		return;
	}

	const Diagnostics::UnitVocabulary *vocab_ptr =
		(opts_.unit_vocab.Allowed().empty() ? nullptr : &opts_.unit_vocab);

	for (const auto *drv : drivers_)
	{
		if (!drv)
			continue;

		Diagnostics::DiagnosticPacket pkt(drv->DiagnosticsName());
		pkt.SetTime(t);
		pkt.SetStepIndex(step_counter_);

		// Contract lines (stable assumptions)
		const auto contract = drv->UnitContract();
		for (const auto &line : contract.Lines())
			pkt.AddContractLine(line);

		// Driver snapshot scalars
		drv->DiagnoseSnapshot(t, Y, ctx, pkt);

		// Apply cadence filtering (observer-level policy)
		ApplyCadenceFilter(pkt);

		// Basic sanity checks
		pkt.ValidateBasic();

		// JSONL (one line per driver)
		Diagnostics::DiagnosticsJson::WritePacketJsonl(out_, pkt, vocab_ptr);
	}

	out_.flush();

	// Update time-trigger schedule to avoid repeated triggers at same t
	if (opts_.record_every_dt > 0.0)
	{
		while (t >= next_time_trigger_)
			next_time_trigger_ += opts_.record_every_dt;
	}
}

// -----------------------------------------------------------------------------
//  OnStart
// -----------------------------------------------------------------------------
/**
 * @brief Called once at the beginning of the evolution (t = t0).
 *
 * This is the preferred place for:
 *   - Initial-condition diagnostics
 *   - Unit contracts
 *   - Geometry sanity checks
 *
 * We intentionally do NOT increment step counters here — this is a logical
 * initialization phase, not an evolution step.
 */
void DiagnosticsObserver::OnStart(const RunInfo &run,
								  const StateVector &Y0,
								  const DriverContext &ctx)
{
	started_ = true;

	// Clear per-producer emission state
	last_value_.clear();

	//	Clear once-per-run state
	once_emitted_.clear();

	// Reset step counter
	step_counter_ = 0;

	// Initialize time trigger schedule
	if (opts_.record_every_dt > 0.0)
	{
		// next eligible record at t0, unless you conceptually want (t0 + dt)
		next_time_trigger_ = run.t0;
	}

	Z_LOG_INFO("OnStart(t0=" + std::to_string(run.t0) + ")");

	if (opts_.record_at_start)
	{
		Record(run.t0, Y0, ctx);
	}
}

// -----------------------------------------------------------------------------
//  OnSample
// -----------------------------------------------------------------------------
/**
 * @brief Called whenever the evolution loop emits a sample.
 *
 * This is typically triggered every dt_save by the integrator, but the observer
 * remains agnostic of the integrator details.
 *
 * @param s   Sample metadata (time, index, save count, etc.)
 * @param Y   Current logical state vector (already unpacked)
 * @param ctx Driver context (geometry, star context, config)
 */
void DiagnosticsObserver::OnSample(const SampleInfo &s,
								   const StateVector &Y,
								   const DriverContext &ctx)
{
	if (!started_)
	{
		// Defensive: if someone forgets to call OnStart.
		// Keep behavior sane and still allow recording.
		started_ = true;
		if (opts_.record_every_dt > 0.0)
			next_time_trigger_ = s.t;
	}

	++step_counter_;

	if (!ShouldRecord(s.t))
		return;

	Record(s.t, Y, ctx);
}
// -----------------------------------------------------------------------------
bool DiagnosticsObserver::ApproximatelyEqual(double a, double b, double atol, double rtol)
{
	// Handle exact equality fast (also handles infinities, though those should be caught elsewhere)
	if (a == b)
		return true;

	const double diff = std::abs(a - b);
	const double scale = std::max(std::abs(a), std::abs(b));
	return diff <= (atol + rtol * scale);
}
// -----------------------------------------------------------------------------
void DiagnosticsObserver::ApplyCadenceFilter(Diagnostics::DiagnosticPacket &pkt)
{
	const std::string &prod = pkt.Producer();

	auto &last_for_prod = last_value_[prod];
	auto &once_for_prod = once_emitted_[prod];

	// Build a filtered map (keep deterministic ordering by reusing std::map)
	std::map<std::string, Diagnostics::ScalarEntry> kept;

	for (const auto &kv : pkt.Scalars())
	{
		const std::string &key = kv.first;
		const auto &e = kv.second;

		using Cad = Diagnostics::Cadence;

		switch (e.cadence)
		{
		case Cad::Always:
			kept.emplace(key, e);
			last_for_prod[key] = e.value; // optional, but useful for later switches
			break;

		case Cad::OncePerRun:
			if (once_for_prod.find(key) == once_for_prod.end())
			{
				kept.emplace(key, e);
				once_for_prod.insert(key);
				last_for_prod[key] = e.value;
			}
			break;

		case Cad::OnChange:
		default:
		{
			auto it = last_for_prod.find(key);
			if (it == last_for_prod.end())
			{
				kept.emplace(key, e);		  // first time: emit
				last_for_prod[key] = e.value; // record
			}
			else
			{
				if (!ApproximatelyEqual(e.value, it->second,
										opts_.on_change_atol, opts_.on_change_rtol))
				{
					kept.emplace(key, e);
					it->second = e.value;
				}
			}
			break;
		}
		}
	}

	pkt.ClearScalars();
	for (const auto &kv : kept)
	{
		const auto &key = kv.first;
		const auto &e = kv.second;
		pkt.AddScalar(key, e.value, e.unit, e.description, e.source, e.cadence);
	}
}
// -----------------------------------------------------------------------------
} // namespace CompactStar::Physics::Evolution::Observers