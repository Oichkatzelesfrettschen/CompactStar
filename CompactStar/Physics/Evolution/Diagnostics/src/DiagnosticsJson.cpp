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

#include <iomanip> // std::setprecision
#include <sstream> // for safety if needed

namespace CompactStar::Physics::Evolution::Diagnostics
{
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

	// Metadata
	Indent(os, indent);
	Key(os, "producer");
	WriteEscapedString(os, pkt.Producer());
	os << ",";
	Newline(os, pretty);

	Indent(os, indent);
	Key(os, "run_id");
	WriteEscapedString(os, pkt.RunId());
	os << ",";
	Newline(os, pretty);

	Indent(os, indent);
	Key(os, "time");
	os << std::setprecision(17) << pkt.Time();
	os << ",";
	Newline(os, pretty);

	Indent(os, indent);
	Key(os, "step");
	os << pkt.StepIndex();
	os << ",";
	Newline(os, pretty);

	// Scalars
	Indent(os, indent);
	Key(os, "scalars");
	os << "{";
	Newline(os, pretty);

	const auto &S = pkt.Scalars();
	std::size_t i = 0;
	for (const auto &kv : S)
	{
		const auto &k = kv.first;
		const auto &e = kv.second;

		Indent(os, indent * 2);
		Key(os, k);

		if (!opts.include_scalar_metadata)
		{
			os << std::setprecision(17) << e.value;
		}
		else
		{
			os << "{";
			Newline(os, pretty);

			Indent(os, indent * 3);
			Key(os, "value");
			os << std::setprecision(17) << e.value;
			os << ",";
			Newline(os, pretty);

			Indent(os, indent * 3);
			Key(os, "unit");
			WriteEscapedString(os, e.unit);
			os << ",";
			Newline(os, pretty);

			Indent(os, indent * 3);
			Key(os, "description");
			WriteEscapedString(os, e.description);
			os << ",";
			Newline(os, pretty);

			Indent(os, indent * 3);
			Key(os, "source");
			WriteEscapedString(os, e.source);
			os << ",";
			Newline(os, pretty);

			Indent(os, indent * 3);
			Key(os, "finite");
			Bool(os, e.is_finite);

			if (vocab)
			{
				os << ",";
				Newline(os, pretty);
				Indent(os, indent * 3);
				Key(os, "unit_ok");
				Bool(os, vocab->IsAllowed(e.unit));
			}

			Newline(os, pretty);
			Indent(os, indent * 2);
			os << "}";
		}

		if (i + 1 < S.size())
			os << ",";

		Newline(os, pretty);
		++i;
	}

	Indent(os, indent);
	os << "}";

	// Contract lines
	if (opts.include_contract)
	{
		os << ",";
		Newline(os, pretty);

		Indent(os, indent);
		Key(os, "contract");
		os << "[";
		Newline(os, pretty);

		const auto &C = pkt.ContractLines();
		for (std::size_t j = 0; j < C.size(); ++j)
		{
			Indent(os, indent * 2);
			WriteEscapedString(os, C[j]);
			if (j + 1 < C.size())
				os << ",";
			Newline(os, pretty);
		}
		Indent(os, indent);
		os << "]";
	}

	// Messages
	if (opts.include_messages)
	{
		os << ",";
		Newline(os, pretty);

		Indent(os, indent);
		Key(os, "warnings");
		os << "[";
		Newline(os, pretty);
		const auto &W = pkt.Warnings();
		for (std::size_t j = 0; j < W.size(); ++j)
		{
			Indent(os, indent * 2);
			WriteEscapedString(os, W[j]);
			if (j + 1 < W.size())
				os << ",";
			Newline(os, pretty);
		}
		Indent(os, indent);
		os << "]";
		os << ",";
		Newline(os, pretty);

		Indent(os, indent);
		Key(os, "errors");
		os << "[";
		Newline(os, pretty);
		const auto &E = pkt.Errors();
		for (std::size_t j = 0; j < E.size(); ++j)
		{
			Indent(os, indent * 2);
			WriteEscapedString(os, E[j]);
			if (j + 1 < E.size())
				os << ",";
			Newline(os, pretty);
		}
		Indent(os, indent);
		os << "]";

		os << ",";
		Newline(os, pretty);

		Indent(os, indent);
		Key(os, "notes");
		os << "[";
		Newline(os, pretty);
		const auto &N = pkt.Notes();
		for (std::size_t j = 0; j < N.size(); ++j)
		{
			Indent(os, indent * 2);
			WriteEscapedString(os, N[j]);
			if (j + 1 < N.size())
				os << ",";
			Newline(os, pretty);
		}
		Indent(os, indent);
		os << "]";
	}

	Newline(os, pretty);
	os << "}";
}
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