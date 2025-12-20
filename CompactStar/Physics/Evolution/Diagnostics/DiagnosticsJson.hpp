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
 * @file DiagnosticsJson.hpp
 * @brief Minimal JSON writer for diagnostics packets.
 *
 * We intentionally do not use a full JSON library:
 *  - avoids bringing in dependencies,
 *  - keeps output deterministic and easy to diff,
 *  - sufficient for tooling/regression tests.
 *
 * Output styles supported:
 *  - JSON object (single packet)
 *  - JSONL (one JSON object per line) for append-only logs.
 *
 * Determinism:
 *  - Scalars iterate in key order due to std::map.
 *  - Text arrays preserve insertion order.
 */

#include <ostream>
#include <string>

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/UnitContract.hpp"

namespace CompactStar::Physics::Evolution::Diagnostics
{

/**
 * @brief Configuration options for JSON rendering.
 */
struct JsonOptions
{
	/// If true, pretty-print with indentation and newlines.
	bool pretty = true;

	/// Indentation size for pretty-print.
	int indent_spaces = 2;

	/// If true, include contract lines in JSON output.
	bool include_contract = true;

	/// If true, include warnings and errors arrays.
	bool include_messages = true;

	/// If true, include descriptions and sources for scalars.
	bool include_scalar_metadata = true;
};

/**
 * @brief JSON rendering helpers.
 */
class DiagnosticsJson
{
  public:
	/**
	 * @brief Write a single packet as a JSON object.
	 *
	 * @param os Output stream.
	 * @param pkt DiagnosticPacket to render.
	 * @param opts Rendering options.
	 * @param vocab Optional unit vocabulary for linting (may be null).
	 *
	 * If vocab is provided and a scalar unit is not allowed, the writer does not throw;
	 * instead it emits a `"unit_ok": false` metadata field for that scalar (when metadata enabled).
	 */
	static void WritePacket(std::ostream &os,
							const DiagnosticPacket &pkt,
							const JsonOptions &opts = {},
							const UnitVocabulary *vocab = nullptr);

	/**
	 * @brief Write a single packet in JSONL form (one object + '\n').
	 *
	 * Useful for append-only logs where each line is independently parseable.
	 * Forces `pretty=false` (JSONL should be one line).
	 */
	static void WritePacketJsonl(std::ostream &os,
								 const DiagnosticPacket &pkt,
								 const UnitVocabulary *vocab = nullptr);

  private:
	static void WriteEscapedString(std::ostream &os, const std::string &s);

	static void Indent(std::ostream &os, int n);

	static void Newline(std::ostream &os, bool pretty);

	static void Sep(std::ostream &os, bool pretty, int indent);

	static void Key(std::ostream &os, const std::string &k);

	static void Bool(std::ostream &os, bool v);

	// Convenience: convert Cadence enum to string
	static const char *CadenceToString(Cadence c);

	// ---- new structural helpers ----
	static void EmitObjectBegin(std::ostream &os, bool pretty, int indent_level,
								bool &first_field, const std::string &key);

	static void EmitObjectEnd(std::ostream &os, bool pretty, int indent_level);

	static void EmitArrayBegin(std::ostream &os, bool pretty, int indent_level,
							   bool &first_field, const std::string &key);

	static void EmitArrayEnd(std::ostream &os, bool pretty, int indent_level);

	// Convenience: begin a field by emitting comma/newline if needed + "key":
	static void EmitFieldKey(std::ostream &os, bool pretty, int indent_level,
							 bool &first_field, const std::string &key);
};

} // namespace CompactStar::Physics::Evolution::Diagnostics