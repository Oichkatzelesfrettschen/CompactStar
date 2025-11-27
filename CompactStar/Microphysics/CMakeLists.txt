add_subdirectory(BNV)

# add_subdirectory(Rates)
set(CompactStar_Microphysics_sources

    ${CompactStar_Microphysics_BNV_sources}

    PARENT_SCOPE)