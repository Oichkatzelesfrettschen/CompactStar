set(CompactStar_Physics_Driver_Thermal_headers

    # HeatingFromChem.hpp
    # NeutrinoCooling.hpp
    PhotonCooling.hpp
)

install(FILES ${CompactStar_Physics_Driver_Thermal_headers} DESTINATION include/CompactStar/Physics/Driver/Thermal)

set(CompactStar_Physics_Driver_Thermal_sources

    CompactStar/Physics/Driver/Thermal/src/PhotonCooling.cpp
    PARENT_SCOPE
)