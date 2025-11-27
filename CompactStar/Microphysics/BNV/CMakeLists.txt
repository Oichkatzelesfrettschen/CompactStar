add_subdirectory(Analysis)
add_subdirectory(Channels)
add_subdirectory(Internal)

set(CompactStar_Microphysics_BNV_sources

    ${CompactStar_Microphysics_BNV_Analysis_sources}
    ${CompactStar_Microphysics_BNV_Channels_sources}
    ${CompactStar_Microphysics_BNV_Internal_sources}

    PARENT_SCOPE)