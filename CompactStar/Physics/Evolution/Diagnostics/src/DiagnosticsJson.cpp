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

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticsJson.hpp"
#include "CompactStar/Physics/Evolution/Diagnostics/Schema.hpp"

#include <iomanip> // std::setprecision
#include <sstream> // for safety if needed

namespace CompactStar::Physics::Evolution::Diagnostics
{
//--------------------------------------------------------------
void DiagnosticsJson::EmitFieldKey(std::ostream &os, bool pretty, int indent_level,
								   bool &first_field, const std::string &key)
{
	if (!first_field)
	{
		os << ",";
		Newline(os, pretty);
	}
	first_field = false;

	Indent(os, indent_level);
	Key(os, key);
}
//--------------------------------------------------------------
void DiagnosticsJson::EmitObjectBegin(std::ostream &os, bool pretty, int indent_level,
									  bool &first_field, const std::string &key)
{
	EmitFieldKey(os, pretty, indent_level, first_field, key);
	os << "{";
	Newline(os, pretty);
}
//--------------------------------------------------------------
void DiagnosticsJson::EmitObjectEnd(std::ostream &os, bool pretty, int indent_level)
{
	Newline(os, pretty);
	Indent(os, indent_level);
	os << "}";
}
//--------------------------------------------------------------
void DiagnosticsJson::EmitArrayBegin(std::ostream &os, bool pretty, int indent_level,
									 bool &first_field, const std::string &key)
{
	EmitFieldKey(os, pretty, indent_level, first_field, key);
	os << "[";
	Newline(os, pretty);
}
//--------------------------------------------------------------
void DiagnosticsJson::EmitArrayEnd(std::ostream &os, bool pretty, int indent_level)
{
	Newline(os, pretty);
	Indent(os, indent_level);
	os << "]";
}

//--------------------------------------------------------------
const char *DiagnosticsJson::CadenceToString(Cadence c)
{
	switch (c)
	{
	case Cadence::Always:
		return "Always";
	case Cadence::OnChange:
		return "OnChange";
	case Cadence::OncePerRun:
		return "OncePerRun";
	default:
		return "Always";
	}
}

//--------------------------------------------------------------
void DiagnosticsJson::WritePacket(std::ostream &os,
								  const DiagnosticPacket &pkt,
								  const JsonOptions &opts,
								  const UnitVocabulary *vocab)
{
	const bool pretty = opts.pretty;
	const int indent = opts.indent_spaces;

	os << "{";
	Newline(os, pretty);

	bool first_top = true;

	// --------------------
	// Schema (top-level, always present)
	// --------------------
	EmitFieldKey(os, pretty, indent, first_top, "schema");
	WriteEscapedString(os, Schema::PacketId);

	EmitFieldKey(os, pretty, indent, first_top, "schema_version");
	os << Schema::PacketVer;

	// --------------------
	// Top-level metadata
	// --------------------
	EmitFieldKey(os, pretty, indent, first_top, "producer");
	WriteEscapedString(os, pkt.Producer());

	EmitFieldKey(os, pretty, indent, first_top, "run_id");
	WriteEscapedString(os, pkt.RunId());

	EmitFieldKey(os, pretty, indent, first_top, "time");
	os << std::setprecision(17) << pkt.Time();

	EmitFieldKey(os, pretty, indent, first_top, "step");
	os << pkt.StepIndex();

	// --------------------
	// Scalars object
	// --------------------
	EmitObjectBegin(os, pretty, indent, first_top, "scalars");

	{
		bool first_scalar = true;
		const auto &S = pkt.Scalars();

		for (const auto &kv : S)
		{
			const auto &k = kv.first;
			const auto &e = kv.second;

			// scalar key inside "scalars" object
			if (!first_scalar)
			{
				os << ",";
				Newline(os, pretty);
			}
			first_scalar = false;

			Indent(os, indent * 2);
			Key(os, k);

			if (!opts.include_scalar_metadata)
			{
				os << std::setprecision(17) << e.value;
				continue;
			}

			// scalar metadata object
			os << "{";
			Newline(os, pretty);

			bool first_meta = true;

			EmitFieldKey(os, pretty, indent * 3, first_meta, "value");
			os << std::setprecision(17) << e.value;

			EmitFieldKey(os, pretty, indent * 3, first_meta, "unit");
			WriteEscapedString(os, e.unit);

			EmitFieldKey(os, pretty, indent * 3, first_meta, "description");
			WriteEscapedString(os, e.description);

			EmitFieldKey(os, pretty, indent * 3, first_meta, "source_hint");
			WriteEscapedString(os, e.source);

			EmitFieldKey(os, pretty, indent * 3, first_meta, "finite");
			Bool(os, e.is_finite);

			EmitFieldKey(os, pretty, indent * 3, first_meta, "default_cadence");
			WriteEscapedString(os, CadenceToString(e.cadence));

			if (vocab)
			{
				EmitFieldKey(os, pretty, indent * 3, first_meta, "unit_ok");
				Bool(os, vocab->IsAllowed(e.unit));
			}

			Newline(os, pretty);
			Indent(os, indent * 2);
			os << "}";
		}

		// close "scalars" object
		EmitObjectEnd(os, pretty, indent);
	}

	// --------------------
	// Contract lines (optional, only if non-empty)
	// --------------------
	if (opts.include_contract && !pkt.ContractLines().empty())
	{
		EmitArrayBegin(os, pretty, indent, first_top, "contract");

		const auto &C = pkt.ContractLines();
		for (std::size_t j = 0; j < C.size(); ++j)
		{
			Indent(os, indent * 2);
			WriteEscapedString(os, C[j]);
			if (j + 1 < C.size())
				os << ",";
			Newline(os, pretty);
		}

		EmitArrayEnd(os, pretty, indent);
	}

	// --------------------
	// Messages (optional)
	//   Emit a single object: "messages": { "warnings": [...], "errors": [...], "notes": [...] }
	//   Only include non-empty arrays, and only emit "messages" if at least one is non-empty.
	// --------------------
	if (opts.include_messages)
	{
		const auto &W = pkt.Warnings();
		const auto &E = pkt.Errors();
		const auto &N = pkt.Notes();

		if (!W.empty() || !E.empty() || !N.empty())
		{
			// Open "messages" object at top-level
			EmitObjectBegin(os, pretty, indent, first_top, "messages");

			bool first_msg = true;

			if (!W.empty())
			{
				EmitArrayBegin(os, pretty, indent * 2, first_msg, "warnings");
				for (std::size_t j = 0; j < W.size(); ++j)
				{
					Indent(os, indent * 3);
					WriteEscapedString(os, W[j]);
					if (j + 1 < W.size())
						os << ",";
					Newline(os, pretty);
				}
				EmitArrayEnd(os, pretty, indent * 2);
			}

			if (!E.empty())
			{
				EmitArrayBegin(os, pretty, indent * 2, first_msg, "errors");
				for (std::size_t j = 0; j < E.size(); ++j)
				{
					Indent(os, indent * 3);
					WriteEscapedString(os, E[j]);
					if (j + 1 < E.size())
						os << ",";
					Newline(os, pretty);
				}
				EmitArrayEnd(os, pretty, indent * 2);
			}

			if (!N.empty())
			{
				EmitArrayBegin(os, pretty, indent * 2, first_msg, "notes");
				for (std::size_t j = 0; j < N.size(); ++j)
				{
					Indent(os, indent * 3);
					WriteEscapedString(os, N[j]);
					if (j + 1 < N.size())
						os << ",";
					Newline(os, pretty);
				}
				EmitArrayEnd(os, pretty, indent * 2);
			}

			// Close "messages" object
			EmitObjectEnd(os, pretty, indent);
		}
	}

	Newline(os, pretty);
	os << "}";
}
//--------------------------------------------------------------
// void DiagnosticsJson::WritePacket(std::ostream &os,
// 								  const DiagnosticPacket &pkt,
// 								  const JsonOptions &opts,
// 								  const UnitVocabulary *vocab)
// {
// 	const bool pretty = opts.pretty;
// 	const int indent = opts.indent_spaces;

// 	os << "{";
// 	Newline(os, pretty);

// 	// Metadata
// 	Indent(os, indent);
// 	Key(os, "producer");
// 	WriteEscapedString(os, pkt.Producer());
// 	os << ",";
// 	Newline(os, pretty);

// 	Indent(os, indent);
// 	Key(os, "run_id");
// 	WriteEscapedString(os, pkt.RunId());
// 	os << ",";
// 	Newline(os, pretty);

// 	Indent(os, indent);
// 	Key(os, "time");
// 	os << std::setprecision(17) << pkt.Time();
// 	os << ",";
// 	Newline(os, pretty);

// 	Indent(os, indent);
// 	Key(os, "step");
// 	os << pkt.StepIndex();
// 	os << ",";
// 	Newline(os, pretty);

// 	// Scalars
// 	Indent(os, indent);
// 	Key(os, "scalars");
// 	os << "{";
// 	Newline(os, pretty);

// 	const auto &S = pkt.Scalars();
// 	std::size_t i = 0;
// 	for (const auto &kv : S)
// 	{
// 		const auto &k = kv.first;
// 		const auto &e = kv.second;

// 		Indent(os, indent * 2);
// 		Key(os, k);

// 		if (!opts.include_scalar_metadata)
// 		{
// 			os << std::setprecision(17) << e.value;
// 		}
// 		else
// 		{
// 			os << "{";
// 			Newline(os, pretty);

// 			Indent(os, indent * 3);
// 			Key(os, "value");
// 			os << std::setprecision(17) << e.value;
// 			os << ",";
// 			Newline(os, pretty);

// 			Indent(os, indent * 3);
// 			Key(os, "unit");
// 			WriteEscapedString(os, e.unit);
// 			os << ",";
// 			Newline(os, pretty);

// 			Indent(os, indent * 3);
// 			Key(os, "description");
// 			WriteEscapedString(os, e.description);
// 			os << ",";
// 			Newline(os, pretty);

// 			Indent(os, indent * 3);
// 			Key(os, "source_hint");
// 			WriteEscapedString(os, e.source);
// 			os << ",";
// 			Newline(os, pretty);

// 			Indent(os, indent * 3);
// 			Key(os, "finite");
// 			Bool(os, e.is_finite);

// 			if (vocab)
// 			{
// 				os << ",";
// 				Newline(os, pretty);
// 				Indent(os, indent * 3);
// 				Key(os, "unit_ok");
// 				Bool(os, vocab->IsAllowed(e.unit));
// 			}

// 			Newline(os, pretty);
// 			Indent(os, indent * 2);
// 			os << "}";
// 		}

// 		if (i + 1 < S.size())
// 			os << ",";

// 		Newline(os, pretty);
// 		++i;
// 	}

// 	Indent(os, indent);
// 	os << "}";

// 	// Contract lines
// 	if (opts.include_contract && !pkt.ContractLines().empty())
// 	{
// 		os << ",";
// 		Newline(os, pretty);

// 		Indent(os, indent);
// 		Key(os, "contract");
// 		os << "[";
// 		Newline(os, pretty);

// 		const auto &C = pkt.ContractLines();
// 		for (std::size_t j = 0; j < C.size(); ++j)
// 		{
// 			Indent(os, indent * 2);
// 			WriteEscapedString(os, C[j]);
// 			if (j + 1 < C.size())
// 				os << ",";
// 			Newline(os, pretty);
// 		}
// 		Indent(os, indent);
// 		os << "]";
// 	}

// 	// // Messages
// 	// if (opts.include_messages)
// 	// {
// 	// 	os << ",";
// 	// 	Newline(os, pretty);

// 	// 	Indent(os, indent);
// 	// 	Key(os, "warnings");
// 	// 	os << "[";
// 	// 	Newline(os, pretty);
// 	// 	const auto &W = pkt.Warnings();
// 	// 	for (std::size_t j = 0; j < W.size(); ++j)
// 	// 	{
// 	// 		Indent(os, indent * 2);
// 	// 		WriteEscapedString(os, W[j]);
// 	// 		if (j + 1 < W.size())
// 	// 			os << ",";
// 	// 		Newline(os, pretty);
// 	// 	}
// 	// 	Indent(os, indent);
// 	// 	os << "]";
// 	// 	os << ",";
// 	// 	Newline(os, pretty);

// 	// 	Indent(os, indent);
// 	// 	Key(os, "errors");
// 	// 	os << "[";
// 	// 	Newline(os, pretty);
// 	// 	const auto &E = pkt.Errors();
// 	// 	for (std::size_t j = 0; j < E.size(); ++j)
// 	// 	{
// 	// 		Indent(os, indent * 2);
// 	// 		WriteEscapedString(os, E[j]);
// 	// 		if (j + 1 < E.size())
// 	// 			os << ",";
// 	// 		Newline(os, pretty);
// 	// 	}
// 	// 	Indent(os, indent);
// 	// 	os << "]";

// 	// 	os << ",";
// 	// 	Newline(os, pretty);

// 	// 	Indent(os, indent);
// 	// 	Key(os, "notes");
// 	// 	os << "[";
// 	// 	Newline(os, pretty);
// 	// 	const auto &N = pkt.Notes();
// 	// 	for (std::size_t j = 0; j < N.size(); ++j)
// 	// 	{
// 	// 		Indent(os, indent * 2);
// 	// 		WriteEscapedString(os, N[j]);
// 	// 		if (j + 1 < N.size())
// 	// 			os << ",";
// 	// 		Newline(os, pretty);
// 	// 	}
// 	// 	Indent(os, indent);
// 	// 	os << "]";
// 	// }

// 	bool wrote_any_message_block = false;
// 	// Messages (only emit non-empty blocks)
// 	if (opts.include_messages)
// 	{
// 		const auto &W = pkt.Warnings();
// 		const auto &E = pkt.Errors();
// 		const auto &N = pkt.Notes();

// 		auto emit_comma_if_needed = [&]()
// 		{
// 			if (wrote_any_message_block)
// 			{
// 				os << ",";
// 				Newline(os, pretty);
// 			}
// 			wrote_any_message_block = true;
// 		};

// 		if (!W.empty())
// 		{
// 			emit_comma_if_needed();
// 			Indent(os, indent);
// 			Key(os, "warnings");
// 			os << "[";
// 			Newline(os, pretty);

// 			for (std::size_t j = 0; j < W.size(); ++j)
// 			{
// 				Indent(os, indent * 2);
// 				WriteEscapedString(os, W[j]);
// 				if (j + 1 < W.size())
// 					os << ",";
// 				Newline(os, pretty);
// 			}
// 			Indent(os, indent);
// 			os << "]";
// 		}

// 		if (!E.empty())
// 		{
// 			emit_comma_if_needed();
// 			Indent(os, indent);
// 			Key(os, "errors");
// 			os << "[";
// 			Newline(os, pretty);

// 			for (std::size_t j = 0; j < E.size(); ++j)
// 			{
// 				Indent(os, indent * 2);
// 				WriteEscapedString(os, E[j]);
// 				if (j + 1 < E.size())
// 					os << ",";
// 				Newline(os, pretty);
// 			}
// 			Indent(os, indent);
// 			os << "]";
// 		}

// 		if (!N.empty())
// 		{
// 			emit_comma_if_needed();
// 			Indent(os, indent);
// 			Key(os, "notes");
// 			os << "[";
// 			Newline(os, pretty);

// 			for (std::size_t j = 0; j < N.size(); ++j)
// 			{
// 				Indent(os, indent * 2);
// 				WriteEscapedString(os, N[j]);
// 				if (j + 1 < N.size())
// 					os << ",";
// 				Newline(os, pretty);
// 			}
// 			Indent(os, indent);
// 			os << "]";
// 		}
// 	}

// 	Newline(os, pretty);
// 	os << "}";
// }
//--------------------------------------------------------------

void DiagnosticsJson::WritePacketJsonl(std::ostream &os,
									   const DiagnosticPacket &pkt,
									   const UnitVocabulary *vocab)
{
	JsonOptions opts;
	opts.pretty = false;
	opts.indent_spaces = 0;
	WritePacket(os, pkt, opts, vocab);
	os << "\n";
}
//--------------------------------------------------------------

void DiagnosticsJson::WriteEscapedString(std::ostream &os, const std::string &s)
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
//--------------------------------------------------------------

void DiagnosticsJson::Indent(std::ostream &os, int n)
{
	for (int i = 0; i < n; ++i)
		os << ' ';
}
//--------------------------------------------------------------

void DiagnosticsJson::Newline(std::ostream &os, bool pretty)
{
	if (pretty)
		os << "\n";
}
//--------------------------------------------------------------

void DiagnosticsJson::Key(std::ostream &os, const std::string &k)
{
	WriteEscapedString(os, k);
	os << ": ";
}
//--------------------------------------------------------------

void DiagnosticsJson::Bool(std::ostream &os, bool v)
{
	os << (v ? "true" : "false");
}
//--------------------------------------------------------------

} // namespace CompactStar::Physics::Evolution::Diagnostics