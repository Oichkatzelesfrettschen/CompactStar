set(CompactStar_Physics_Evolution_Integrator_headers
	GSLIntegrator.hpp
)

install(FILES ${CompactStar_Physics_Evolution_Integrator_headers} DESTINATION include/CompactStar/Physics/Evolution/Integrator)

set(CompactStar_Physics_Evolution_Integrator_sources
	CompactStar/Physics/Evolution/Integrator/src/GSLIntegrator.cpp

	PARENT_SCOPE
)