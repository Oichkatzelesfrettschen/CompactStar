add_subdirectory(Chem)
add_subdirectory(Spin)
add_subdirectory(Thermal)

set(CompactStar_Physics_Driver_headers
    IDriver.hpp
    Coupling.hpp
)

install(FILES ${CompactStar_Physics_Driver_headers}
    DESTINATION include/CompactStar/Physics/Driver)

set(CompactStar_Physics_Driver_sources

    ${CompactStar_Physics_Driver_Chem_sources}
    ${CompactStar_Physics_Driver_Spin_sources}
    ${CompactStar_Physics_Driver_Thermal_sources}

    PARENT_SCOPE)