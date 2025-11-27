set(CompactStar_Extensions_MixedStar_headers
    DarkCore_Analysis.hpp
)

install(FILES ${CompactStar_Extensions_MixedStar_headers} DESTINATION include/CompactStar/Extensions/MixedStar)

set(CompactStar_Extensions_MixedStar_sources

    CompactStar/Extensions/MixedStar/src/DarkCore_Analysis.cpp

    PARENT_SCOPE
)