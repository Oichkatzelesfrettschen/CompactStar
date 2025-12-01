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
    CompactStar/Physics/State/src/BNVState.cpp
    CompactStar/Physics/State/src/ChemState.cpp
    CompactStar/Physics/State/src/SpinState.cpp
    CompactStar/Physics/State/src/ThermalState.cpp

    PARENT_SCOPE
)