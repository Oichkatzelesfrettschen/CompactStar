add_subdirectory(Integrator)
add_subdirectory(Observers)

set(CompactStar_Physics_Evolution_headers
    EvolutionConfig.hpp
    EvolutionSystem.hpp
    GeometryCache.hpp
    RHSAccumulator.hpp
    StarContext.hpp
    StateVector.hpp
    StateLayout.hpp
)

install(FILES ${CompactStar_Physics_Evolution_headers} DESTINATION include/CompactStar/Physics/Evolution)

set(CompactStar_Physics_Evolution_sources
    CompactStar/Physics/Evolution/src/StateVector.cpp
    CompactStar/Physics/Evolution/src/EvolutionSystem.cpp
    CompactStar/Physics/Evolution/src/StateLayout.cpp

    PARENT_SCOPE
)