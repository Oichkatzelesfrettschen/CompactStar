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
 * @file DiagnosticsObserver.hpp
 * @brief Observer that periodically records driver diagnostics to JSONL.
 *
 * This observer is intended to be:
 *  - separate from physics (no effect on integration),
 *  - deterministic output for regression testing,
 *  - flexible about what drivers expose diagnostics (opt-in interface).
 *
 * Operation:
 *  - At Observe(t, Y, ctx) it checks whether it's time to record.
 *  - It queries each registered driver diagnostics interface and writes
 *    one JSON object per driver (JSONL) into an output file.
 *
 * Output format:
 *  - JSONL (one object per line), which scales and is tooling-friendly.
 *  - Each line includes producer/time/step/scalars/contract/messages.
 */

#include <cstddef>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticsJson.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/UnitContract.hpp"
#include "CompactStar/Physics/Evolution/Observers/IObserver.hpp"

#include "CompactStar/Physics/Driver/Diagnostics/DriverDiagnostics.hpp"

#include "Zaki/String/Directory.hpp"

namespace CompactStar::Physics::Evolution::Observers
{

/**
 * @brief Periodic diagnostics writer for drivers (JSONL).
 */
class DiagnosticsObserver final : public IObserver
{
  public:
	/**
	 * @brief Configuration for DiagnosticsObserver.
	 */
	struct Options
	{
		/// Output path for JSONL file.
		Zaki::String::Directory output_path = "diagnostics.jsonl";

		/// Record every N observer calls (step-based). If 0, disables step-based triggering.
		std::size_t record_every_n_steps = 0;

		/// Record every dt in simulation-time. If <=0, disables time-based triggering.
		double record_every_dt = 0.0;

		/// If true, record at the first Observe() call (typically t=0).
		bool record_at_start = true;

		/// If true, append to file instead of truncating.
		bool append = false;

		/// JSON output options.
		Diagnostics::JsonOptions json_opts;

		/// Enforce a unit vocabulary (if empty => no enforcement).
		Diagnostics::UnitVocabulary unit_vocab;
	};

	/// Construct from options (copy).
	explicit DiagnosticsObserver(const Options &opts);

	/// Construct from options (move).
	explicit DiagnosticsObserver(Options &&opts);

	/**
	 * @brief Construct with options and driver diagnostics providers.
	 *
	 * @param opts Observer options.
	 * @param drivers List of drivers that implement IDriverDiagnostics.
	 */
	DiagnosticsObserver(Options opts,
						std::vector<const Driver::Diagnostics::IDriverDiagnostics *> drivers);

	~DiagnosticsObserver() override;

	/**
	 * @brief Called whenever the evolution loop records a sample (typically every dt_save).
	 */
	void OnSample(const SampleInfo &s,
				  const StateVector &Y,
				  const DriverContext &ctx) override;

	/**
	 * @brief Called once before integration begins.
	 *
	 * @param run Run-level metadata.
	 * @param Y0  Initial state snapshot (read-only).
	 * @param ctx Driver context (read-only).
	 */
	void OnStart(const RunInfo &run,
				 const StateVector &Y0,
				 const DriverContext &ctx) override;

  private:
	/// Decide whether we should record at current (t, step_counter_).
	bool ShouldRecord(double t) const;

	/// Record diagnostics snapshots from all drivers at time t.
	void Record(double t, const StateVector &Y, const DriverContext &ctx);

	/// Open the output stream according to Options.
	void OpenOutput();

  private:
	Options opts_;

	std::vector<const Driver::Diagnostics::IDriverDiagnostics *> drivers_;

	std::ofstream out_;

	// Scheduling state
	// bool first_call_ = true;
	std::size_t step_counter_ = 0;	 ///< counts OnSample calls
	double next_time_trigger_ = 0.0; ///< next eligible record time for time-trigger mode
	bool started_ = false;			 ///< true after OnStart has run
};

} // namespace CompactStar::Physics::Evolution::Observers