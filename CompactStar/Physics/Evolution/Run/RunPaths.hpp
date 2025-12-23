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
 * @file RunPaths.hpp
 * @brief Standardized run output paths for Evolution-based runs.
 *
 * This header defines a small, deterministic convention for:
 *  - log file path
 *  - diagnostics JSONL path
 *  - diagnostics catalog JSON path
 *  - time-series CSV path
 *  - time-series sidecar metadata path
 *
 * Motivation:
 * - Debug mains and examples tend to duplicate output path wiring.
 * - Observers expect consistent conventions (catalog → timeseries, etc.).
 * - Keeping this in Evolution (not in Observers) centralizes run I/O policy.
 *
 * Design principles:
 * - Pure data container + deterministic builder.
 * - No side effects (does not create directories).
 * - No dependency on core star-building machinery.
 *
 * @ingroup Evolution
 */

#include <string>

#include "Zaki/String/Directory.hpp"

namespace CompactStar::Physics::Evolution::Run
{

/**
 * @brief Canonical file layout for an Evolution run.
 *
 * All members are absolute or run-root-relative paths depending on how you pass
 * the base directory. No filesystem mutations are performed by this struct.
 *
 * Typical layout:
 *   <base_results_dir>/<out_dir>/
 *      diagnostics.jsonl
 *      diagnostics.catalog.json
 *      timeseries.csv
 *      timeseries.csv.meta.json
 *      <optional logs>
 */
struct RunPaths
{
	/// Base results directory (e.g. ".../results").
	Zaki::String::Directory base_results_dir = "";

	/// Run output directory name (e.g. "spin_therm_evol_2").
	Zaki::String::Directory out_dir = "";

	/// Full run directory path: base_results_dir + "/" + out_dir.
	Zaki::String::Directory run_dir = "";

	/// Log file path (optional; can be empty).
	Zaki::String::Directory log_file = "";

	/// DiagnosticsObserver JSONL output.
	Zaki::String::Directory diagnostics_jsonl = "";

	/// DiagnosticsObserver catalog output (schema file).
	Zaki::String::Directory diagnostics_catalog_json = "";

	/// TimeSeriesObserver CSV/TSV output.
	Zaki::String::Directory timeseries_table = "";

	/// Sidecar metadata for TimeSeriesObserver (usually "<timeseries_table>.meta.json").
	Zaki::String::Directory timeseries_meta_json = "";
};

/**
 * @brief Build canonical Evolution run output paths.
 *
 * @param base_results_dir Directory that contains all runs (e.g. ".../results").
 * @param out_dir          Run folder name (e.g. "spin_therm_evol_2").
 * @param log_file_name    Optional log filename; if empty, log_file is left empty.
 *
 * @return Fully populated RunPaths with deterministic filenames.
 *
 * @note This function does not create directories. It only computes paths.
 */
RunPaths MakeRunPaths(const Zaki::String::Directory &base_results_dir,
					  const Zaki::String::Directory &out_dir,
					  const std::string &log_file_name = "");

/**
 * @brief Convenience helper: make a path under the run directory.
 *
 * @param p RunPaths.
 * @param filename File name relative to run_dir (no leading slash).
 * @return p.run_dir + "/" + filename
 */
Zaki::String::Directory UnderRunDir(const RunPaths &p, const std::string &filename);

} // namespace CompactStar::Physics::Evolution::Run