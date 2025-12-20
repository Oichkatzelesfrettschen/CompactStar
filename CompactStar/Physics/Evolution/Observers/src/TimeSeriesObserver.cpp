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
 * @file TimeSeriesObserver.cpp
 * @brief Implementation of TimeSeriesObserver.
 *
 * This observer writes a compact table (CSV/TSV) suitable for plotting,
 * with one row per recorded sample and a stable set of columns.
 *
 * Key behavior:
 *  - Scheduling: record_every_n_samples (sample-index cadence) and/or record_every_dt (time cadence).
 *  - record_at_start: optionally emits an initial row at t0 before integration samples.
 *  - Header: one line with column keys (optional).
 *  - Sidecar metadata: optional column documentation (units/descriptions) in a tiny JSON file.
 *
 * Notes:
 *  - This module is deliberately low-dependency; it does not rely on external JSON libs.
 *  - DriverScalar columns query IDriverDiagnostics each row; this is acceptable for a small
 *    number of columns and moderate cadence. If performance becomes an issue, add a
 *    “bulk snapshot” interface or cache precomputed derived values upstream.
 */

#include "CompactStar/Physics/Evolution/Observers/TimeSeriesObserver.hpp"

#include <cmath>
#include <iomanip>
#include <limits>
#include <stdexcept>
#include <unordered_set>

#include <Zaki/Util/Logger.hpp>

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticsCatalogJson.hpp"

#include "CompactStar/Physics/Evolution/StateVector.hpp"
#include "CompactStar/Physics/State/SpinState.hpp"
#include "CompactStar/Physics/State/ThermalState.hpp"

namespace CompactStar::Physics::Evolution::Observers
{
//------------------------------------------------------------------------------
//  Internal helpers (local to this translation unit)
//------------------------------------------------------------------------------
namespace
{
/**
 * @brief Minimal JSON string escape for sidecar metadata.
 *
 * This is sufficient for short, developer-authored strings such as keys, units,
 * and descriptions. It escapes backslash, quote, and common whitespace.
 */
void WriteJsonEscaped(std::ostream &os, const std::string &s)
{
	os << "\"";
	for (char c : s)
	{
		switch (c)
		{
		case '\\':
			os << "\\\\";
			break;
		case '"':
			os << "\\\"";
			break;
		case '\n':
			os << "\\n";
			break;
		case '\r':
			os << "\\r";
			break;
		case '\t':
			os << "\\t";
			break;
		default:
			os << c;
			break;
		}
	}
	os << "\"";
}

//------------------------------------------------------------------------------
/**
 * @brief Return true if a file-like output path is empty.
 */
bool IsEmptyPath(const Zaki::String::Directory &p)
{
	return p.Str().empty();
}
} // namespace

//------------------------------------------------------------------------------
//  Construction / destruction
//------------------------------------------------------------------------------
TimeSeriesObserver::TimeSeriesObserver(const Options &opts)
	: opts_(opts)
{
	// Do not open output here: observers should be cheap to construct.
	// Open output in OnStart().
}

//------------------------------------------------------------------------------
TimeSeriesObserver::TimeSeriesObserver(Options &&opts)
	: opts_(std::move(opts))
{
}

//------------------------------------------------------------------------------
TimeSeriesObserver::TimeSeriesObserver(
	Options opts,
	std::vector<const Driver::Diagnostics::IDriverDiagnostics *> drivers)
	: opts_(std::move(opts)),
	  drivers_(std::move(drivers))
{
}

//------------------------------------------------------------------------------
TimeSeriesObserver::TimeSeriesObserver(
	Options opts,
	std::vector<const Driver::Diagnostics::IDriverDiagnostics *> drivers,
	std::shared_ptr<const Diagnostics::DiagnosticCatalog> catalog)
	: opts_(std::move(opts)),
	  drivers_(std::move(drivers)),
	  catalog_(std::move(catalog))
{
}

//------------------------------------------------------------------------------
TimeSeriesObserver::~TimeSeriesObserver()
{
	if (out_.is_open())
		out_.close();
}

//------------------------------------------------------------------------------
//  Scheduling
//------------------------------------------------------------------------------
bool TimeSeriesObserver::ShouldRecord(double t, std::uint64_t sample_index) const
{
	bool step_trigger = false;
	if (opts_.record_every_n_samples > 0)
	{
		step_trigger = ((sample_index % opts_.record_every_n_samples) == 0);
	}

	bool time_trigger = false;
	if (opts_.record_every_dt > 0.0)
	{
		time_trigger = (t >= next_time_trigger_);
	}

	// If both are disabled, do not record (except possibly record_at_start handled elsewhere).
	return step_trigger || time_trigger;
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::AdvanceTimeTrigger(double t)
{
	if (opts_.record_every_dt <= 0.0)
		return;

	// Ensure monotonic progression of the trigger time even if multiple samples
	// arrive with the same t (or integrator jitter).
	while (t >= next_time_trigger_)
		next_time_trigger_ += opts_.record_every_dt;
}

//------------------------------------------------------------------------------
//  Output utilities
//------------------------------------------------------------------------------
const char *TimeSeriesObserver::Delim() const
{
	switch (opts_.format)
	{
	case OutputFormat::TSV:
		return "\t";
	case OutputFormat::CSV:
	default:
		return ",";
	}
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::OpenOutput()
{
	if (IsEmptyPath(opts_.output_path))
	{
		throw std::runtime_error("TimeSeriesObserver: Options.output_path is empty.");
	}

	const std::ios_base::openmode mode =
		std::ios::out | (opts_.append ? std::ios::app : std::ios::trunc);

	out_.open(opts_.output_path.Str(), mode);
	if (!out_)
	{
		throw std::runtime_error(
			"TimeSeriesObserver: failed to open output file: " + opts_.output_path.Str());
	}

	// Prefer deterministic formatting across platforms.
	out_ << std::setprecision(opts_.float_precision);
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::WriteHeader()
{
	if (!out_)
		return;

	// If user did not specify columns, default to something sensible:
	// time, sample_index, Tinf_K, Omega_rad_s (if present).
	// However, because you explicitly asked for “full implementation”, we keep
	// behavior strict: if columns are empty, we still write a minimal header.
	if (opts_.columns.empty())
	{
		out_ << "t" << Delim() << "sample_index" << "\n";
		header_written_ = true;
		return;
	}

	for (std::size_t i = 0; i < opts_.columns.size(); ++i)
	{
		out_ << opts_.columns[i].key;
		if (i + 1 < opts_.columns.size())
			out_ << Delim();
	}
	out_ << "\n";
	header_written_ = true;
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::WriteSidecarMetadata(const RunInfo &run) const
{
	if (!opts_.write_sidecar_metadata)
		return;

	// Sidecar filename: "<output_path>.meta.json"
	const std::string meta_path = opts_.output_path.Str() + ".meta.json";

	std::ofstream meta(meta_path, std::ios::out | std::ios::trunc);
	if (!meta)
	{
		Z_LOG_WARNING("TimeSeriesObserver: failed to open sidecar metadata file: " + meta_path);
		return;
	}

	meta << "{\n";
	meta << "  \"observer\": ";
	WriteJsonEscaped(meta, "TimeSeriesObserver");
	meta << ",\n";

	meta << "  \"run\": {\n";
	meta << "    \"tag\": ";
	WriteJsonEscaped(meta, run.tag);
	meta << ",\n";
	meta << "    \"output_dir\": ";
	WriteJsonEscaped(meta, run.output_dir);
	meta << ",\n";
	meta << "    \"t0\": " << std::setprecision(17) << run.t0 << ",\n";
	meta << "    \"tf\": " << std::setprecision(17) << run.tf << "\n";
	meta << "  },\n";

	meta << "  \"table\": {\n";
	meta << "    \"path\": ";
	WriteJsonEscaped(meta, opts_.output_path.Str());
	meta << ",\n";
	meta << "    \"format\": ";
	WriteJsonEscaped(meta, (opts_.format == OutputFormat::TSV ? "TSV" : "CSV"));
	meta << ",\n";
	meta << "    \"delimiter\": ";
	WriteJsonEscaped(meta, Delim());
	meta << "\n";
	meta << "  },\n";

	meta << "  \"columns\": [\n";
	for (std::size_t i = 0; i < opts_.columns.size(); ++i)
	{
		const auto &c = opts_.columns[i];
		meta << "    {\n";
		meta << "      \"key\": ";
		WriteJsonEscaped(meta, c.key);
		meta << ",\n";

		meta << "      \"source\": ";
		WriteJsonEscaped(meta, (c.source == ColumnSource::DriverScalar ? "DriverScalar" : "BuiltinState"));
		meta << ",\n";

		meta << "      \"unit\": ";
		WriteJsonEscaped(meta, c.unit);
		meta << ",\n";

		meta << "      \"description\": ";
		WriteJsonEscaped(meta, c.description);

		// DriverScalar details
		if (c.source == ColumnSource::DriverScalar)
		{
			meta << ",\n      \"producer\": ";
			WriteJsonEscaped(meta, c.catalog_ref.producer);
			meta << ",\n      \"scalar_key\": ";
			WriteJsonEscaped(meta, c.catalog_ref.key);
		}

		// BuiltinState details
		if (c.source == ColumnSource::BuiltinState)
		{
			meta << ",\n      \"builtin\": ";
			// Persist enum as a stable string.
			switch (c.builtin)
			{
			case Column::Builtin::Time:
				WriteJsonEscaped(meta, "Time");
				break;
			case Column::Builtin::SampleIndex:
				WriteJsonEscaped(meta, "SampleIndex");
				break;
			case Column::Builtin::StepIndex:
				WriteJsonEscaped(meta, "StepIndex");
				break;
			case Column::Builtin::Tinf_K:
				WriteJsonEscaped(meta, "Tinf_K");
				break;
			case Column::Builtin::Omega_rad_s:
				WriteJsonEscaped(meta, "Omega_rad_s");
				break;
			default:
				WriteJsonEscaped(meta, "Unknown");
				break;
			}
		}

		meta << "\n    }";
		if (i + 1 < opts_.columns.size())
			meta << ",";
		meta << "\n";
	}
	meta << "  ]\n";
	meta << "}\n";
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::WriteRow(const SampleInfo &s,
								  const Evolution::StateVector &Y,
								  const Evolution::DriverContext &ctx)
{
	if (!out_)
		return;

	// If user did not configure columns, emit minimal “t, sample_index”.
	if (opts_.columns.empty())
	{
		out_ << std::setprecision(opts_.float_precision)
			 << s.t << Delim() << s.sample_index << "\n";
		out_.flush();
		AdvanceTimeTrigger(s.t);
		return;
	}

	for (std::size_t i = 0; i < opts_.columns.size(); ++i)
	{
		const auto &col = opts_.columns[i];

		double v = std::numeric_limits<double>::quiet_NaN();
		switch (col.source)
		{
		case ColumnSource::BuiltinState:
			v = ExtractBuiltin(col, s, Y, ctx);
			break;
		case ColumnSource::DriverScalar:
			v = ExtractDriverScalar(col, s, Y, ctx);
			break;
		default:
			v = std::numeric_limits<double>::quiet_NaN();
			break;
		}

		// Print value
		if (std::isfinite(v))
		{
			out_ << std::setprecision(opts_.float_precision) << v;
		}
		else
		{
			// Keep parseable: "nan" is accepted by many toolchains.
			out_ << "nan";
		}

		if (i + 1 < opts_.columns.size())
			out_ << Delim();
	}
	out_ << "\n";

	out_.flush();
	AdvanceTimeTrigger(s.t);
}

//------------------------------------------------------------------------------
//  Driver lookup / extraction
//------------------------------------------------------------------------------
const Driver::Diagnostics::IDriverDiagnostics *
TimeSeriesObserver::FindDriverByProducer(const std::string &producer) const
{
	auto it = driver_cache_.find(producer);
	if (it != driver_cache_.end())
		return it->second;

	for (const auto *drv : drivers_)
	{
		if (!drv)
			continue;
		if (drv->DiagnosticsName() == producer)
		{
			driver_cache_[producer] = drv;
			return drv;
		}
	}

	driver_cache_[producer] = nullptr;
	return nullptr;
}

//------------------------------------------------------------------------------
double TimeSeriesObserver::ExtractDriverScalar(const Column &col,
											   const SampleInfo &s,
											   const Evolution::StateVector &Y,
											   const Evolution::DriverContext &ctx) const
{
	const auto &producer = col.catalog_ref.producer;
	const auto &key = col.catalog_ref.key;

	if (producer.empty() || key.empty())
		return std::numeric_limits<double>::quiet_NaN();

	const auto *drv = FindDriverByProducer(producer);
	if (!drv)
		return std::numeric_limits<double>::quiet_NaN();

	Physics::Evolution::Diagnostics::DiagnosticPacket pkt(drv->DiagnosticsName());
	pkt.SetTime(s.t);
	pkt.SetStepIndex(static_cast<std::size_t>(s.sample_index));

	drv->DiagnoseSnapshot(s.t, Y, ctx, pkt);

	if (!pkt.HasScalar(key))
		return std::numeric_limits<double>::quiet_NaN();

	return pkt.GetScalar(key).value;
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::LoadCatalogIfNeeded()
{
	if (catalog_)
		return;

	if (!opts_.use_catalog)
		return;

	if (opts_.catalog_path.Str().empty())
	{
		Z_LOG_WARNING("TimeSeriesObserver: use_catalog=true but no catalog_ and Options.catalog_path is empty.");
		return;
	}

	std::ifstream in(opts_.catalog_path.Str());
	if (!in)
	{
		Z_LOG_WARNING("TimeSeriesObserver: failed to open catalog_path: " + opts_.catalog_path.Str());
		return;
	}

	auto cat = std::make_shared<Diagnostics::DiagnosticCatalog>();
	// You need to implement ReadCatalog(...) (or similar) in DiagnosticsCatalogJson.
	// If your DiagnosticsCatalogJson currently only writes, then for now:
	//   - either skip parsing and require passing catalog_ in,
	//   - or implement a minimal reader.
	Diagnostics::DiagnosticsCatalogJson::ReadCatalog(in, *cat);

	catalog_ = std::move(cat);
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::BuildColumnsFromCatalog()
{
	if (!opts_.use_catalog)
		return;

	LoadCatalogIfNeeded();
	if (!catalog_)
	{
		Z_LOG_WARNING("TimeSeriesObserver: use_catalog=true but no catalog available; leaving columns unchanged.");
		return;
	}

	producer_catalog_cache_.clear();
	for (const auto &kv : catalog_->Producers())
		producer_catalog_cache_[kv.first] = &kv.second;

	std::vector<Column> built;

	// Builtins first (optional)
	if (opts_.include_builtin_time)
	{
		Column c;
		c.key = "t_s";
		c.source = ColumnSource::BuiltinState;
		c.unit = "s";
		c.description = "Simulation time";
		c.builtin = Column::Builtin::Time;
		built.push_back(std::move(c));
	}

	if (opts_.include_builtin_sample_index)
	{
		Column c;
		c.key = "sample_index";
		c.source = ColumnSource::BuiltinState;
		c.unit = "";
		c.description = "Monotonic sample counter";
		c.builtin = Column::Builtin::SampleIndex;
		built.push_back(std::move(c));
	}

	// Helper: append scalars for a producer, optionally filtered by profile keys.
	auto append_keys_for_producer = [&](const std::string &producer,
										const std::vector<std::string> &keys)
	{
		auto it = producer_catalog_cache_.find(producer);
		if (it == producer_catalog_cache_.end() || !it->second)
			return;

		const auto *pc = it->second;

		// Build a set for membership test
		std::unordered_map<std::string, const Diagnostics::ScalarDescriptor *> sd_map;
		sd_map.reserve(pc->scalars.size());
		for (const auto &sd : pc->scalars)
			sd_map[sd.key] = &sd;

		for (const auto &k : keys)
		{
			auto jt = sd_map.find(k);
			if (jt == sd_map.end() || !jt->second)
				continue;

			const auto &sd = *jt->second;

			Column c;
			c.key = sd.key; // header label default = scalar key
			c.source = ColumnSource::DriverScalar;
			c.unit = sd.unit;
			c.description = sd.description;
			c.catalog_ref.producer = producer;
			c.catalog_ref.key = sd.key;
			built.push_back(std::move(c));
		}
	};

	// Strategy:
	// - If catalog_profiles provided: for each producer, if it has those profiles, append keys.
	// - Otherwise: if producer has profile "timeseries_default", use that.
	// - Otherwise: append nothing (conservative).
	const auto &producers = catalog_->Producers();
	for (const auto &kv : producers)
	{
		const std::string &producer = kv.first;
		const auto &pc = kv.second;

		auto use_profile = [&](const std::string &pname) -> bool
		{
			for (const auto &p : pc.profiles)
			{
				if (p.name == pname)
				{
					append_keys_for_producer(producer, p.keys);
					return true;
				}
			}
			return false;
		};

		bool used_any = false;

		if (!opts_.catalog_profiles.empty())
		{
			for (const auto &pname : opts_.catalog_profiles)
				used_any = use_profile(pname) || used_any;
		}
		else
		{
			used_any = use_profile("timeseries_default");
		}

		(void)used_any;
	}

	// Merge policy:
	// If user already provided explicit columns, append built driver scalars to them,
	// but keep explicit columns first (user intent).
	if (!opts_.columns.empty())
	{
		// Avoid duplicates by header key (simple). You can refine later.
		std::unordered_set<std::string> seen;
		for (const auto &c : opts_.columns)
			seen.insert(c.key);

		for (auto &c : built)
		{
			if (seen.insert(c.key).second)
				opts_.columns.push_back(std::move(c));
		}
	}
	else
	{
		opts_.columns = std::move(built);
	}
}

//------------------------------------------------------------------------------
double TimeSeriesObserver::ExtractBuiltin(const Column &col,
										  const SampleInfo &s,
										  const Evolution::StateVector &Y,
										  const Evolution::DriverContext & /*ctx*/) const
{
	switch (col.builtin)
	{
	case Column::Builtin::Time:
		return s.t;

	case Column::Builtin::SampleIndex:
		return static_cast<double>(s.sample_index);

	case Column::Builtin::StepIndex:
		return static_cast<double>(s.step_index);

	case Column::Builtin::Tinf_K:
	{
		// This assumes your StateVector exposes a thermal component compatible
		// with the ThermalState you showed (Tinf() returns Kelvin).
		const auto &thermal = Y.GetThermal();
		if (thermal.NumComponents() == 0)
			return std::numeric_limits<double>::quiet_NaN();
		return thermal.Tinf();
	}

	case Column::Builtin::Omega_rad_s:
	{
		// This assumes your StateVector has a spin component with an Omega() accessor.
		// If your API differs (e.g., spin.Omega_rad_s()), adjust here.
		const auto &spin = Y.GetSpin();
		if (spin.NumComponents() == 0)
			return std::numeric_limits<double>::quiet_NaN();
		return spin.Omega();
	}

	default:
		return std::numeric_limits<double>::quiet_NaN();
	}
}

//------------------------------------------------------------------------------
//  Observer lifecycle
//------------------------------------------------------------------------------
void TimeSeriesObserver::OnStart(const RunInfo &run,
								 const Evolution::StateVector &Y0,
								 const Evolution::DriverContext &ctx)
{
	(void)ctx; // reserved for future use (e.g., derived columns from geo)

	started_ = true;
	header_written_ = false;
	driver_cache_.clear();

	producer_catalog_cache_.clear();
	BuildColumnsFromCatalog();

	OpenOutput();

	// Initialize time-trigger schedule
	if (opts_.record_every_dt > 0.0)
	{
		next_time_trigger_ = run.t0;

		// If we do NOT record at start, first eligible time is t0 + dt.
		if (!opts_.record_at_start)
			next_time_trigger_ = run.t0 + opts_.record_every_dt;
	}

	// Write sidecar metadata once per run.
	if (opts_.write_sidecar_metadata)
		WriteSidecarMetadata(run);

	// Write table header if enabled.
	// If append==true, we assume the file already contains a header.
	if (opts_.write_header && !opts_.append)
		WriteHeader();
	else
		header_written_ = true; // logically, we consider header “handled”.

	Z_LOG_INFO("TimeSeriesObserver::OnStart(t0=" + std::to_string(run.t0) + ")");

	// Optional record at start (t=t0) using a synthetic SampleInfo.
	// To avoid double-recording, OnSample() will skip sample_index==0 if record_at_start==true.
	if (opts_.record_at_start)
	{
		SampleInfo s0;
		s0.t = run.t0;
		s0.sample_index = 0;
		s0.step_index = 0;
		WriteRow(s0, Y0, ctx);
	}
}
//------------------------------------------------------------------------------

void TimeSeriesObserver::OnSample(const SampleInfo &s,
								  const Evolution::StateVector &Y,
								  const Evolution::DriverContext &ctx)
{
	if (!started_)
	{
		// Defensive behavior if caller forgot OnStart.
		RunInfo dummy;
		dummy.t0 = s.t;
		dummy.tf = s.t;
		OnStart(dummy, Y, ctx);
	}

	// Common integrator behavior: first sample is (t0, sample_index=0).
	// If we already wrote record_at_start in OnStart, skip this one to prevent duplication.
	if (opts_.record_at_start && s.sample_index == 0)
		return;

	if (!ShouldRecord(s.t, s.sample_index))
		return;

	WriteRow(s, Y, ctx);
}

//------------------------------------------------------------------------------
void TimeSeriesObserver::OnFinish(const FinishInfo &fin,
								  const Evolution::StateVector & /*Yf*/,
								  const Evolution::DriverContext & /*ctx*/)
{
	// Keep this intentionally lightweight: just close the stream.
	if (out_.is_open())
	{
		out_.flush();
		out_.close();
	}

	if (fin.ok)
	{
		Z_LOG_INFO("TimeSeriesObserver::OnFinish(ok=true, t_final=" + std::to_string(fin.t_final) + ")");
	}
	else
	{
		Z_LOG_WARNING("TimeSeriesObserver::OnFinish(ok=false, t_final=" + std::to_string(fin.t_final) +
					  ", message='" + fin.message + "')");
	}
}
//------------------------------------------------------------------------------
} // namespace CompactStar::Physics::Evolution::Observers