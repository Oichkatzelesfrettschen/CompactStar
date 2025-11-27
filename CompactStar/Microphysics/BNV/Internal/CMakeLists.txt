set(CompactStar_Microphysics_BNV_Internal_headers
    BNV_Chi.hpp
)

install(FILES ${CompactStar_Microphysics_BNV_Internal_headers} DESTINATION include/CompactStar/BNV/Internal)

set(CompactStar_Microphysics_BNV_Internal_sources

    CompactStar/Microphysics/BNV/Internal/src/BNV_Chi.cpp

    PARENT_SCOPE
)