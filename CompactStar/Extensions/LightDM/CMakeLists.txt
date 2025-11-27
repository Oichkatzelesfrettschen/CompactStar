set(CompactStar_Extensions_LightDM_headers
    LightDM_Scalar_Density.hpp
)

install(FILES ${CompactStar_Extensions_LightDM_headers} DESTINATION include/CompactStar/Extensions/LightDM)

set(CompactStar_Extensions_LightDM_sources

    CompactStar/Extensions/LightDM/src/LightDM_Scalar_Density.cpp

    PARENT_SCOPE
)