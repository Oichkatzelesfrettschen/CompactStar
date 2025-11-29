set(CompactStar_Physics_State_headers
    State.hpp
    SpinState.hpp
    ThermalState.hpp
    BNVState.hpp
    ChemState.hpp
    Tags.hpp
)

install(FILES ${CompactStar_Physics_State_headers} DESTINATION include/CompactStar/Physics/State)

set(CompactStar_Physics_State_sources

    PARENT_SCOPE
)