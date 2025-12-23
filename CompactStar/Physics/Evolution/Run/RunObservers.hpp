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
 * @file RunObservers.hpp
 * @brief Standard observer option presets and factories for Evolution runs.
 *
 * This module centralizes *run-level* observer policy so debug mains stay small:
 *  - DiagnosticsObserver (JSONL packets + optional catalog)
 *  - TimeSeriesObserver (CSV/TSV table; optionally schema-driven via catalog)
 *
 * This is intentionally separate from Observers/ implementation:
 * Observers are generic components; this file encodes “how we like to run them”.
 *
 * @ingroup Evolution
 */

#include <memory>
#include <vector>

#include "CompactStar/Physics/Evolution/Observers/DiagnosticsObserver.hpp"
#include "CompactStar/Physics/Evolution/Observers/TimeSeriesObserver.hpp"

#include "CompactStar/Physics/Evolution/Run/RunPaths.hpp"

// diagnostics provider interface
#include "CompactStar/Physics/Driver/Diagnostics/DriverDiagnostics.hpp"

namespace CompactStar::Physics::Evolution::Run
{

/**
 * @brief Build default DiagnosticsObserver::Options using canonical RunPaths.
 *
 * @param p RunPaths for this run.
 *
 * @return Options with:
 *  - output_path           = p.diagnostics_jsonl
 *  - record_every_n_steps  = 1000
 *  - record_at_start       = true
 *  - write_catalog         = true
 *  - catalog_output_path   = p.diagnostics_catalog_json
 */
Observers::DiagnosticsObserver::Options
MakeDefaultDiagnosticsOptions(const RunPaths &p);

/**
 * @brief Build default TimeSeriesObserver::Options using canonical RunPaths.
 *
 * This uses schema-driven columns by default:
 *  - use_catalog = true
 *  - catalog_path = p.diagnostics_catalog_json
 *  - catalog_profiles = {"timeseries_default"}
 *  - includes builtin time + sample_index
 *
 * @param p RunPaths for this run.
 *
 * @return Options for a compact CSV time series.
 */
Observers::TimeSeriesObserver::Options
MakeDefaultTimeSeriesOptions(const RunPaths &p);

/**
 * @brief Factory: construct a DiagnosticsObserver for a run.
 *
 * @param p             RunPaths for output/cat paths.
 * @param diag_drivers  Non-owning pointers to diagnostics-capable drivers.
 * @param overrides     Optional overrides (applied after defaults).
 *
 * @return Shared pointer to DiagnosticsObserver.
 */
std::shared_ptr<Observers::DiagnosticsObserver>
MakeDiagnosticsObserver(const RunPaths &p,
						const std::vector<const Driver::Diagnostics::IDriverDiagnostics *> &diag_drivers,
						const Observers::DiagnosticsObserver::Options *overrides = nullptr);

/**
 * @brief Factory: construct a TimeSeriesObserver for a run.
 *
 * @param p            RunPaths for timeseries path and catalog path.
 * @param diag_drivers Non-owning pointers to diagnostics-capable drivers.
 * @param overrides    Optional overrides (applied after defaults).
 *
 * @return Shared pointer to TimeSeriesObserver.
 *
 * @note TimeSeriesObserver reads the catalog JSON at runtime if use_catalog==true.
 *       Therefore, ensure DiagnosticsObserver writes the catalog early enough.
 *       Your current flow (record_at_start + write_catalog) is compatible.
 */
std::shared_ptr<Observers::TimeSeriesObserver>
MakeTimeSeriesObserver(const RunPaths &p,
					   const std::vector<const Driver::Diagnostics::IDriverDiagnostics *> &diag_drivers,
					   const Observers::TimeSeriesObserver::Options *overrides = nullptr);

} // namespace CompactStar::Physics::Evolution::Run