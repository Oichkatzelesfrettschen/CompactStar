add_subdirectory(Integrator)
add_subdirectory(Observers)

set(CompactStar_Physics_Evolution_headers
    GeometryCache.hpp
    Graph.hpp
    StarContext.hpp
    System.hpp
)

install(FILES ${CompactStar_Physics_Evolution_headers} DESTINATION include/CompactStar/Physics/Evolution)

set(CompactStar_Physics_Evolution_sources

    PARENT_SCOPE
)