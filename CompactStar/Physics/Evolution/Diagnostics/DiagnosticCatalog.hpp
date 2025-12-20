#pragma once
#include <map>
#include <string>
#include <vector>

#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticPacket.hpp" // for Cadence
#include "CompactStar/Physics/Evolution/Diagnostics/Schema.hpp"

namespace CompactStar::Physics::Evolution::Diagnostics
{
//--------------------------------------------------------------
/**
 * @brief Static descriptor for one scalar key that may appear in packets.
 *
 * This is schema-level metadata: it should change rarely and be tooling-friendly.
 */
struct ScalarDescriptor
{
	std::string key;		 // e.g. "L_gamma_inf_erg_s"
	std::string unit;		 // e.g. "erg/s"
	std::string description; // human-readable
	std::string source_hint; // e.g. "computed", "state", "cache"
	Cadence default_cadence = Cadence::Always;

	// Optional extras that help tooling
	bool required = false; // if true, packet validator can enforce presence
	bool is_dimensionless = false;
};

//--------------------------------------------------------------
/**
 * @brief Catalog section for a single producer (usually one driver diagnostics name).
 */
struct ProducerCatalog
{
	std::string producer; // matches pkt.Producer()/DiagnosticsName()
	std::vector<ScalarDescriptor> scalars;
	std::vector<std::string> contract_lines; // optional: stable unit contract lines

	struct Profile
	{
		std::string name;
		std::vector<std::string> keys;
	};
	std::vector<Profile> profiles;
};

//--------------------------------------------------------------
/**
 * @brief Full catalog across producers.
 *
 * Deterministic ordering: map by producer name.
 */
class DiagnosticCatalog
{
  public:
	void AddProducer(ProducerCatalog pc);

	[[nodiscard]] const std::map<std::string, ProducerCatalog> &Producers() const { return producers_; }

	// Convenience: merge-in scalars for an existing producer
	void AddScalar(const std::string &producer, ScalarDescriptor sd);

  private:
	std::map<std::string, ProducerCatalog> producers_;
};
//--------------------------------------------------------------
} // namespace CompactStar::Physics::Evolution::Diagnostics