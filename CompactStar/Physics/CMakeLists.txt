add_subdirectory(Driver)
add_subdirectory(State)
add_subdirectory(Evolution)

set(CompactStar_Physics_sources

    ${CompactStar_Physics_Driver_sources}
    ${CompactStar_Physics_State_sources}
    ${CompactStar_Physics_Evolution_sources}

    PARENT_SCOPE)