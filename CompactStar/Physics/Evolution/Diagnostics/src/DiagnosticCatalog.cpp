#include "CompactStar/Physics/Evolution/Diagnostics/DiagnosticCatalog.hpp"

namespace CompactStar::Physics::Evolution::Diagnostics
{
void DiagnosticCatalog::AddProducer(ProducerCatalog pc)
{
	producers_[pc.producer] = std::move(pc);
}

void DiagnosticCatalog::AddScalar(const std::string &producer, ScalarDescriptor sd)
{
	auto &pc = producers_[producer];
	pc.producer = producer;
	pc.scalars.push_back(std::move(sd));
}
} // namespace CompactStar::Physics::Evolution::Diagnostics