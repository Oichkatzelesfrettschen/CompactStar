add_subdirectory(Integrator)
add_subdirectory(Observers)
add_subdirectory(Diagnostics)
add_subdirectory(Run)

set(CompactStar_Physics_Evolution_headers
    EvolutionConfig.hpp
    EvolutionSystem.hpp
    GeometryCache.hpp
    RHSAccumulator.hpp
    StarContext.hpp
    StateVector.hpp
    StateLayout.hpp
    StatePacking.hpp
    DriverContext.hpp
)

install(FILES ${CompactStar_Physics_Evolution_headers}
    DESTINATION include/CompactStar/Physics/Evolution)

# Base evolution sources
set(CompactStar_Physics_Evolution_sources
    CompactStar/Physics/Evolution/src/StateVector.cpp
    CompactStar/Physics/Evolution/src/EvolutionSystem.cpp
    CompactStar/Physics/Evolution/src/StateLayout.cpp
    CompactStar/Physics/Evolution/src/StatePacking.cpp
    CompactStar/Physics/Evolution/src/StarContext.cpp
    CompactStar/Physics/Evolution/src/GeometryCache.cpp
)

# Append integrator sources that were bubbled up from Integrator/CMakeLists.txt
list(APPEND CompactStar_Physics_Evolution_sources
    ${CompactStar_Physics_Evolution_Integrator_sources}
    ${CompactStar_Physics_Evolution_Diagnostics_sources}
    ${CompactStar_Physics_Evolution_Observer_sources}
    ${CompactStar_Physics_Evolution_Run_sources}
)

# If/when you add Observer .cpps, you can do the same:
# list(APPEND CompactStar_Physics_Evolution_sources
# ${CompactStar_Physics_Evolution_Observer_sources}
# )

# Re-export combined list to parent
set(CompactStar_Physics_Evolution_sources
    "${CompactStar_Physics_Evolution_sources}"
    PARENT_SCOPE
)