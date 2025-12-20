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

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticsCatalogJson.hpp"

#include <cstddef> // std::size_t
#include <istream>

namespace CompactStar::Physics::Evolution::Diagnostics
{
//--------------------------------------------------------------
void DiagnosticsCatalogJson::EmitFieldKey(std::ostream &os, bool pretty, int indent_level,
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
void DiagnosticsCatalogJson::EmitObjectBegin(std::ostream &os, bool pretty, int indent_level,
											 bool &first_field, const std::string &key)
{
	EmitFieldKey(os, pretty, indent_level, first_field, key);
	os << "{";
	Newline(os, pretty);
}
//--------------------------------------------------------------
void DiagnosticsCatalogJson::EmitObjectEnd(std::ostream &os, bool pretty, int indent_level)
{
	Newline(os, pretty);
	Indent(os, indent_level);
	os << "}";
}
//--------------------------------------------------------------
void DiagnosticsCatalogJson::EmitArrayBegin(std::ostream &os, bool pretty, int indent_level,
											bool &first_field, const std::string &key)
{
	EmitFieldKey(os, pretty, indent_level, first_field, key);
	os << "[";
	Newline(os, pretty);
}
//--------------------------------------------------------------
void DiagnosticsCatalogJson::EmitArrayEnd(std::ostream &os, bool pretty, int indent_level)
{
	Newline(os, pretty);
	Indent(os, indent_level);
	os << "]";
}
//--------------------------------------------------------------
const char *DiagnosticsCatalogJson::CadenceToString(Cadence c)
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
void DiagnosticsCatalogJson::WriteCatalog(std::ostream &os,
										  const DiagnosticCatalog &catalog,
										  const CatalogJsonOptions &opts)
{
	const bool pretty = opts.pretty;
	const int indent = opts.indent_spaces;

	os << "{";
	Newline(os, pretty);

	bool first_top = true;

	// --------------------
	// Catalog schema (top-level, always present)
	// --------------------
	EmitFieldKey(os, pretty, indent, first_top, "schema");
	WriteEscapedString(os, Schema::CatalogId);

	EmitFieldKey(os, pretty, indent, first_top, "schema_version");
	os << Schema::CatalogVer;

	// --------------------
	// Producers map (always present; may be empty)
	// --------------------
	EmitObjectBegin(os, pretty, indent, first_top, "producers");

	{
		const auto &P = catalog.Producers();
		bool first_prod = true;

		for (const auto &pkv : P)
		{
			const auto &producer_name = pkv.first;
			const auto &pc = pkv.second;

			if (!first_prod)
			{
				os << ",";
				Newline(os, pretty);
			}
			first_prod = false;

			Indent(os, indent * 2);
			Key(os, producer_name);
			os << "{";
			Newline(os, pretty);

			bool first_prod_field = true;

			// Contract lines (optional)
			if (opts.include_contract && !pc.contract_lines.empty())
			{
				EmitArrayBegin(os, pretty, indent * 3, first_prod_field, "contract_lines");
				for (std::size_t j = 0; j < pc.contract_lines.size(); ++j)
				{
					Indent(os, indent * 4);
					WriteEscapedString(os, pc.contract_lines[j]);
					if (j + 1 < pc.contract_lines.size())
						os << ",";
					Newline(os, pretty);
				}
				EmitArrayEnd(os, pretty, indent * 3);
			}

			// Scalars array (always present, even if empty)
			EmitArrayBegin(os, pretty, indent * 3, first_prod_field, "scalars");

			for (std::size_t j = 0; j < pc.scalars.size(); ++j)
			{
				const auto &sd = pc.scalars[j];

				Indent(os, indent * 4);
				os << "{";
				Newline(os, pretty);

				bool first_sd = true;

				EmitFieldKey(os, pretty, indent * 5, first_sd, "key");
				WriteEscapedString(os, sd.key);

				EmitFieldKey(os, pretty, indent * 5, first_sd, "unit");
				WriteEscapedString(os, sd.unit);

				EmitFieldKey(os, pretty, indent * 5, first_sd, "description");
				WriteEscapedString(os, sd.description);

				EmitFieldKey(os, pretty, indent * 5, first_sd, "source_hint");
				WriteEscapedString(os, sd.source_hint);

				EmitFieldKey(os, pretty, indent * 5, first_sd, "default_cadence");
				WriteEscapedString(os, CadenceToString(sd.default_cadence));

				EmitFieldKey(os, pretty, indent * 5, first_sd, "required");
				Bool(os, sd.required);

				EmitFieldKey(os, pretty, indent * 5, first_sd, "is_dimensionless");
				Bool(os, sd.is_dimensionless);

				Newline(os, pretty);
				Indent(os, indent * 4);
				os << "}";

				if (j + 1 < pc.scalars.size())
					os << ",";
				Newline(os, pretty);
			}

			EmitArrayEnd(os, pretty, indent * 3);
			// Profiles array (optional)
			if (!pc.profiles.empty())
			{
				EmitArrayBegin(os, pretty, indent * 3, first_prod_field, "profiles");

				for (std::size_t j = 0; j < pc.profiles.size(); ++j)
				{
					const auto &prof = pc.profiles[j];

					Indent(os, indent * 4);
					os << "{";
					Newline(os, pretty);

					bool first_pf = true;

					EmitFieldKey(os, pretty, indent * 5, first_pf, "name");
					WriteEscapedString(os, prof.name);

					EmitArrayBegin(os, pretty, indent * 5, first_pf, "keys");
					for (std::size_t k = 0; k < prof.keys.size(); ++k)
					{
						Indent(os, indent * 6);
						WriteEscapedString(os, prof.keys[k]);
						if (k + 1 < prof.keys.size())
							os << ",";
						Newline(os, pretty);
					}
					EmitArrayEnd(os, pretty, indent * 5);

					Newline(os, pretty);
					Indent(os, indent * 4);
					os << "}";

					if (j + 1 < pc.profiles.size())
						os << ",";
					Newline(os, pretty);
				}

				EmitArrayEnd(os, pretty, indent * 3);
			}

			// Close producer object
			Newline(os, pretty);
			Indent(os, indent * 2);
			os << "}";
		}

		// Close "producers" object
		EmitObjectEnd(os, pretty, indent);
	}

	Newline(os, pretty);
	os << "}";
}

//--------------------------------------------------------------
void DiagnosticsCatalogJson::WriteEscapedString(std::ostream &os, const std::string &s)
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
void DiagnosticsCatalogJson::Indent(std::ostream &os, int n)
{
	for (int i = 0; i < n; ++i)
		os << ' ';
}
//--------------------------------------------------------------
void DiagnosticsCatalogJson::Newline(std::ostream &os, bool pretty)
{
	if (pretty)
		os << "\n";
}
//--------------------------------------------------------------
void DiagnosticsCatalogJson::Key(std::ostream &os, const std::string &k)
{
	WriteEscapedString(os, k);
	os << ": ";
}
//--------------------------------------------------------------
void DiagnosticsCatalogJson::Bool(std::ostream &os, bool v)
{
	os << (v ? "true" : "false");
}
//--------------------------------------------------------------
namespace
{
// ============================================================
//  Minimal JSON tokenizer + parser (strings/bools/numbers/arrays/objects)
//  Intended for CompactStar-emitted catalog JSON (not general-purpose).
// ============================================================

struct JValue
{
	enum class Type
	{
		Null,
		Bool,
		Number,
		String,
		Array,
		Object
	};

	Type type = Type::Null;

	bool b = false;
	double num = 0.0;
	std::string s;
	std::vector<JValue> a;
	std::map<std::string, JValue> o;

	static JValue MakeNull() { return JValue{}; }
	static JValue MakeBool(bool v)
	{
		JValue x;
		x.type = Type::Bool;
		x.b = v;
		return x;
	}
	static JValue MakeNumber(double v)
	{
		JValue x;
		x.type = Type::Number;
		x.num = v;
		return x;
	}
	static JValue MakeString(std::string v)
	{
		JValue x;
		x.type = Type::String;
		x.s = std::move(v);
		return x;
	}
	static JValue MakeArray()
	{
		JValue x;
		x.type = Type::Array;
		return x;
	}
	static JValue MakeObject()
	{
		JValue x;
		x.type = Type::Object;
		return x;
	}
};

class JsonReader
{
  public:
	explicit JsonReader(std::istream &is)
		: is_(is)
	{
	}

	JValue ParseRoot()
	{
		SkipWS();
		JValue v = ParseValue();
		SkipWS();
		return v;
	}

  private:
	std::istream &is_;
	int c_ = -2; // -2 means uninitialized, -1 means EOF

	int Peek()
	{
		if (c_ == -2)
			c_ = is_.get();
		return c_;
	}

	int Get()
	{
		const int p = Peek();
		c_ = -2;
		return p;
	}

	void SkipWS()
	{
		for (;;)
		{
			int p = Peek();
			if (p == EOF || p == -1)
				return;
			if (!std::isspace(static_cast<unsigned char>(p)))
				return;
			Get();
		}
	}

	[[noreturn]] void Fail(const std::string &msg)
	{
		throw std::runtime_error("DiagnosticsCatalogJson::ReadCatalog JSON parse error: " + msg);
	}

	void Expect(char ch)
	{
		SkipWS();
		int p = Get();
		if (p != static_cast<unsigned char>(ch))
		{
			std::string m = "expected '";
			m.push_back(ch);
			m += "', got '";
			if (p == -1 || p == EOF)
				m += "EOF";
			else
				m.push_back(static_cast<char>(p));
			m += "'";
			Fail(m);
		}
	}

	static bool IsNumChar(int ch)
	{
		return std::isdigit(static_cast<unsigned char>(ch)) || ch == '-' || ch == '+' || ch == '.' || ch == 'e' || ch == 'E';
	}

	std::string ParseString()
	{
		SkipWS();
		if (Get() != '"')
			Fail("expected string opening quote");

		std::string out;
		for (;;)
		{
			int p = Get();
			if (p == -1 || p == EOF)
				Fail("unterminated string");
			const char ch = static_cast<char>(p);
			if (ch == '"')
				break;
			if (ch == '\\')
			{
				int e = Get();
				if (e == -1 || e == EOF)
					Fail("unterminated escape");
				switch (static_cast<char>(e))
				{
				case '"':
					out.push_back('"');
					break;
				case '\\':
					out.push_back('\\');
					break;
				case '/':
					out.push_back('/');
					break;
				case 'b':
					out.push_back('\b');
					break;
				case 'f':
					out.push_back('\f');
					break;
				case 'n':
					out.push_back('\n');
					break;
				case 'r':
					out.push_back('\r');
					break;
				case 't':
					out.push_back('\t');
					break;
				case 'u':
					// Minimal: accept \uXXXX but do not decode into UTF-8; store literally as '?'
					// This is sufficient for your internal keys/units/descriptions which should be ASCII.
					for (int i = 0; i < 4; ++i)
					{
						int h = Get();
						if (h == -1 || h == EOF)
							Fail("unterminated unicode escape");
						if (!std::isxdigit(static_cast<unsigned char>(h)))
							Fail("invalid unicode escape");
					}
					out.push_back('?');
					break;
				default:
					Fail("unknown escape sequence");
				}
			}
			else
			{
				out.push_back(ch);
			}
		}
		return out;
	}

	double ParseNumber()
	{
		SkipWS();
		std::string buf;
		int p = Peek();
		if (!IsNumChar(p))
			Fail("expected number");

		while (IsNumChar(Peek()))
		{
			buf.push_back(static_cast<char>(Get()));
		}

		// Use stod (fine for our simple use: versions, etc.)
		try
		{
			return std::stod(buf);
		}
		catch (...)
		{
			Fail("invalid number: " + buf);
		}
	}

	void ParseLiteral(const char *lit)
	{
		for (const char *q = lit; *q; ++q)
		{
			int p = Get();
			if (p != static_cast<unsigned char>(*q))
				Fail(std::string("expected literal: ") + lit);
		}
	}

	JValue ParseArray()
	{
		JValue arr = JValue::MakeArray();
		Expect('[');
		SkipWS();
		if (Peek() == ']')
		{
			Get();
			return arr;
		}

		for (;;)
		{
			arr.a.push_back(ParseValue());
			SkipWS();
			int p = Peek();
			if (p == ',')
			{
				Get();
				continue;
			}
			if (p == ']')
			{
				Get();
				break;
			}
			Fail("expected ',' or ']' in array");
		}
		return arr;
	}

	JValue ParseObject()
	{
		JValue obj = JValue::MakeObject();
		Expect('{');
		SkipWS();
		if (Peek() == '}')
		{
			Get();
			return obj;
		}

		for (;;)
		{
			SkipWS();
			if (Peek() != '"')
				Fail("expected string key in object");
			std::string key = ParseString();
			SkipWS();
			Expect(':');
			JValue val = ParseValue();
			obj.o.emplace(std::move(key), std::move(val));

			SkipWS();
			int p = Peek();
			if (p == ',')
			{
				Get();
				continue;
			}
			if (p == '}')
			{
				Get();
				break;
			}
			Fail("expected ',' or '}' in object");
		}
		return obj;
	}

	JValue ParseValue()
	{
		SkipWS();
		int p = Peek();
		if (p == -1 || p == EOF)
			Fail("unexpected EOF");

		switch (static_cast<char>(p))
		{
		case '{':
			return ParseObject();
		case '[':
			return ParseArray();
		case '"':
			return JValue::MakeString(ParseString());
		case 't':
			ParseLiteral("true");
			return JValue::MakeBool(true);
		case 'f':
			ParseLiteral("false");
			return JValue::MakeBool(false);
		case 'n':
			ParseLiteral("null");
			return JValue::MakeNull();
		default:
			if (IsNumChar(p))
				return JValue::MakeNumber(ParseNumber());
			Fail("unexpected token");
		}
	}
};

// -------- helpers to safely read fields from JValue --------

const JValue *GetField(const JValue &obj, const char *name)
{
	if (obj.type != JValue::Type::Object)
		return nullptr;
	auto it = obj.o.find(name);
	if (it == obj.o.end())
		return nullptr;
	return &it->second;
}

//--------------------------------------------------------------
std::string GetStringOrEmpty(const JValue &obj, const char *name)
{
	const JValue *v = GetField(obj, name);
	if (!v || v->type != JValue::Type::String)
		return "";
	return v->s;
}

//--------------------------------------------------------------
bool GetBoolOr(const JValue &obj, const char *name, bool def)
{
	const JValue *v = GetField(obj, name);
	if (!v)
		return def;
	if (v->type == JValue::Type::Bool)
		return v->b;
	return def;
}

//--------------------------------------------------------------
std::size_t GetSizeTOr(const JValue &obj, const char *name, std::size_t def)
{
	const JValue *v = GetField(obj, name);
	if (!v)
		return def;
	if (v->type == JValue::Type::Number)
		return static_cast<std::size_t>(v->num);
	return def;
}

//--------------------------------------------------------------
// Parse Cadence from string
Cadence ParseCadence(const std::string &s)
{
	if (s == "Always")
		return Cadence::Always;
	if (s == "OnChange")
		return Cadence::OnChange;
	if (s == "OncePerRun")
		return Cadence::OncePerRun;
	return Cadence::Always;
}

//--------------------------------------------------------------
// Parse a string array from a JValue
std::vector<std::string> ParseStringArray(const JValue &arr)
{
	std::vector<std::string> out;
	if (arr.type != JValue::Type::Array)
		return out;
	out.reserve(arr.a.size());
	for (const auto &v : arr.a)
	{
		if (v.type == JValue::Type::String)
			out.push_back(v.s);
	}
	return out;
}
//--------------------------------------------------------------
// Parse a ProducerCatalog from a JValue object
ProducerCatalog ParseProducerCatalogFromObject(const std::string &producer_name, const JValue &pcobj)
{
	ProducerCatalog pc;
	pc.producer = producer_name.empty() ? GetStringOrEmpty(pcobj, "producer") : producer_name;

	// contract_lines
	if (const JValue *cl = GetField(pcobj, "contract_lines"))
		pc.contract_lines = ParseStringArray(*cl);

	// scalars
	if (const JValue *sa = GetField(pcobj, "scalars"))
	{
		if (sa->type == JValue::Type::Array)
		{
			for (const auto &sdv : sa->a)
			{
				if (sdv.type != JValue::Type::Object)
					continue;

				ScalarDescriptor sd;
				sd.key = GetStringOrEmpty(sdv, "key");
				sd.unit = GetStringOrEmpty(sdv, "unit");
				sd.description = GetStringOrEmpty(sdv, "description");
				sd.source_hint = GetStringOrEmpty(sdv, "source_hint");

				const std::string cad = GetStringOrEmpty(sdv, "default_cadence");
				sd.default_cadence = ParseCadence(cad);

				sd.required = GetBoolOr(sdv, "required", false);
				sd.is_dimensionless = GetBoolOr(sdv, "is_dimensionless", false);

				if (!sd.key.empty())
					pc.scalars.push_back(std::move(sd));
			}
		}
	}

	// profiles (you said you added this inside ProducerCatalog)
	if (const JValue *pa = GetField(pcobj, "profiles"))
	{
		if (pa->type == JValue::Type::Array)
		{
			for (const auto &pv : pa->a)
			{
				if (pv.type != JValue::Type::Object)
					continue;

				ProducerCatalog::Profile prof;
				prof.name = GetStringOrEmpty(pv, "name");

				if (const JValue *keys = GetField(pv, "keys"))
					prof.keys = ParseStringArray(*keys);

				if (!prof.name.empty())
					pc.profiles.push_back(std::move(prof));
			}
		}
	}

	return pc;
}

} // namespace

// ============================================================
//  Public API
// ============================================================

void DiagnosticsCatalogJson::ReadCatalog(std::istream &is, DiagnosticCatalog &out)
{
	JsonReader r(is);
	const JValue root = r.ParseRoot();

	if (root.type != JValue::Type::Object)
		throw std::runtime_error("DiagnosticsCatalogJson::ReadCatalog: root must be a JSON object");

	// Validate schema id/version if present
	const std::string schema_id = GetStringOrEmpty(root, "schema");
	if (!schema_id.empty() && schema_id != Schema::CatalogId)
	{
		throw std::runtime_error(
			"DiagnosticsCatalogJson::ReadCatalog: schema mismatch (expected CatalogId)");
	}

	// Support either "schema_version" or "schema_version" (choose one convention later)
	std::size_t ver = 0;
	if (const JValue *sv = GetField(root, "schema_version"))
	{
		if (sv->type == JValue::Type::Number)
			ver = static_cast<std::size_t>(sv->num);
	}
	if (ver == 0)
	{
		if (const JValue *cv = GetField(root, "schema_version"))
		{
			if (cv->type == JValue::Type::Number)
				ver = static_cast<std::size_t>(cv->num);
		}
	}
	if (ver != 0 && ver != Schema::CatalogVer)
	{
		throw std::runtime_error("DiagnosticsCatalogJson::ReadCatalog: catalog version mismatch");
	}

	// Locate producers
	const JValue *prods = GetField(root, "producers");
	if (!prods)
	{
		// Some writers may emit "catalog": { "producers": ... }
		if (const JValue *cat = GetField(root, "catalog"))
			prods = GetField(*cat, "producers");
	}

	if (!prods)
		throw std::runtime_error("DiagnosticsCatalogJson::ReadCatalog: missing 'producers'");

	DiagnosticCatalog tmp;

	// Case A: producers is an array of objects
	if (prods->type == JValue::Type::Array)
	{
		for (const auto &p : prods->a)
		{
			if (p.type != JValue::Type::Object)
				continue;

			ProducerCatalog pc = ParseProducerCatalogFromObject("" /* producer_name */, p);
			if (pc.producer.empty())
				continue;

			tmp.AddProducer(std::move(pc));
		}
	}
	// Case B: producers is an object/map: { "PhotonCooling": {...}, "MagneticDipole": {...} }
	else if (prods->type == JValue::Type::Object)
	{
		for (const auto &kv : prods->o)
		{
			const std::string &producer_name = kv.first;
			const JValue &pcobj = kv.second;
			if (pcobj.type != JValue::Type::Object)
				continue;

			ProducerCatalog pc = ParseProducerCatalogFromObject(producer_name, pcobj);
			if (pc.producer.empty())
				pc.producer = producer_name;

			tmp.AddProducer(std::move(pc));
		}
	}
	else
	{
		throw std::runtime_error("DiagnosticsCatalogJson::ReadCatalog: 'producers' must be array or object");
	}

	out = std::move(tmp);
}
//--------------------------------------------------------------
} // namespace CompactStar::Physics::Evolution::Diagnostics