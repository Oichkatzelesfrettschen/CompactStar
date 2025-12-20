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
 * @file DiagnosticsCatalogJson.hpp
 * @brief Minimal JSON writer for diagnostics catalog.
 *
 * Writes a stable “catalog schema” describing all producers and scalar descriptors.
 *
 * We intentionally do not use a full JSON library:
 *  - avoids bringing in dependencies,
 *  - keeps output deterministic and easy to diff,
 *  - sufficient for tooling/regression tests.
 *
 * Determinism:
 *  - Producers iterate in key order due to std::map in DiagnosticCatalog.
 *  - Scalars preserve insertion order from each ProducerCatalog.
 *  - Contract lines preserve insertion order.
 */

#include <ostream>
#include <string>

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticCatalog.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/Schema.hpp"

namespace CompactStar::Physics::Evolution::Diagnostics
{

/**
 * @brief Configuration options for catalog JSON rendering.
 */
struct CatalogJsonOptions
{
	/// If true, pretty-print with indentation and newlines.
	bool pretty = true;

	/// Indentation size for pretty-print.
	int indent_spaces = 2;

	/// If true, include contract lines for each producer (if present).
	bool include_contract = true;
};

/**
 * @brief JSON writer for DiagnosticCatalog.
 */
class DiagnosticsCatalogJson
{
  public:
	/**
	 * @brief Write a diagnostics catalog as a JSON object.
	 *
	 * Output format:
	 * {
	 *   "schema": "<Schema::CatalogId>",
	 *   "schema_version": <Schema::CatalogVer>,
	 *   "producers": {
	 *     "<producer>": {
	 *       "contract": [...],
	 *       "scalars": [
	 *         { "key": "...", "unit": "...", "description": "...", "source": "...",
	 *           "cadence": "...", "required": true/false, "dimensionless": true/false }
	 *       ]
	 *     }
	 *   }
	 * }
	 */
	static void WriteCatalog(std::ostream &os,
							 const DiagnosticCatalog &catalog,
							 const CatalogJsonOptions &opts = {});

	// Minimal reader for schema-driven TimeSeriesObserver
	static void ReadCatalog(std::istream &is, DiagnosticCatalog &out);

  private:
	// ---- low-level helpers (kept private; mirrors DiagnosticsJson style) ----
	static void WriteEscapedString(std::ostream &os, const std::string &s);

	static void Indent(std::ostream &os, int n);

	static void Newline(std::ostream &os, bool pretty);

	static void Key(std::ostream &os, const std::string &k);

	static void Bool(std::ostream &os, bool v);

	static void EmitFieldKey(std::ostream &os, bool pretty, int indent_level,
							 bool &first_field, const std::string &key);

	static void EmitObjectBegin(std::ostream &os, bool pretty, int indent_level,
								bool &first_field, const std::string &key);

	static void EmitObjectEnd(std::ostream &os, bool pretty, int indent_level);

	static void EmitArrayBegin(std::ostream &os, bool pretty, int indent_level,
							   bool &first_field, const std::string &key);

	static void EmitArrayEnd(std::ostream &os, bool pretty, int indent_level);

	static const char *CadenceToString(Cadence c);
};

} // namespace CompactStar::Physics::Evolution::Diagnostics