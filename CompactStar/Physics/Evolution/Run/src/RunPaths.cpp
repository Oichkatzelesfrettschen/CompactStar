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
 * @file RunPaths.cpp
 * @brief Implementation of Evolution run path helpers.
 *
 * @ingroup Evolution
 */

#include "CompactStar/Physics/Evolution/Run/RunPaths.hpp"

#include <iostream>

namespace CompactStar::Physics::Evolution::Run
{
//--------------------------------------------------------------
Zaki::String::Directory UnderRunDir(const RunPaths &p, const std::string &filename)
{
	return p.run_dir + "/" + filename;
}

//--------------------------------------------------------------
RunPaths MakeRunPaths(const Zaki::String::Directory &base_results_dir,
					  const Zaki::String::Directory &out_dir,
					  const std::string &log_file_name)
{
	RunPaths p;
	p.base_results_dir = base_results_dir;
	p.out_dir = out_dir;
	p.run_dir = base_results_dir + "/" + out_dir;

	// Canonical filenames
	p.diagnostics_jsonl = UnderRunDir(p, "diagnostics.jsonl");
	p.diagnostics_catalog_json = UnderRunDir(p, "diagnostics.catalog.json");

	// Default to CSV filename, but caller may choose TSV by overriding observer options.
	p.timeseries_table = UnderRunDir(p, "timeseries.csv");
	p.timeseries_meta_json = Zaki::String::Directory(p.timeseries_table.Str() + ".meta.json");

	if (!log_file_name.empty())
	{
		// Log file lives under run_dir by default.
		p.log_file = UnderRunDir(p, log_file_name);
	}
	else
	{
		p.log_file = "";
	}

	return p;
}
//--------------------------------------------------------------
} // namespace CompactStar::Physics::Evolution::Run