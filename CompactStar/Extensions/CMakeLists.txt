add_subdirectory(MixedStar)
add_subdirectory(LightDM)

set(CompactStar_Extensions_sources

    ${CompactStar_Extensions_MixedStar_sources}
    ${CompactStar_Extensions_LightDM_sources}

    PARENT_SCOPE)