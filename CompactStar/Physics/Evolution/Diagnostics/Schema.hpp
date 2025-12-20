#pragma once
#include <cstddef>

namespace CompactStar::Physics::Evolution::Diagnostics::Schema
{
inline constexpr const char *PacketId = "compactstar.diagnostics.packet";
inline constexpr std::size_t PacketVer = 1;

inline constexpr const char *CatalogId = "compactstar.diagnostics.catalog";
inline constexpr std::size_t CatalogVer = 1;
} // namespace CompactStar::Physics::Evolution::Diagnostics::Schema