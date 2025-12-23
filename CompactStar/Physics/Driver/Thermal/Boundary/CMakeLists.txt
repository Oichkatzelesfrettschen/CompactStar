set(CompactStar_Physics_Driver_Thermal_Boundary_headers

	EnvelopePotekhin1997.hpp
	EnvelopePotekhin2003.hpp
	IEnvelope.hpp
	SurfaceGravity.hpp
	TbDefinition.hpp
)

install(FILES ${CompactStar_Physics_Driver_Thermal_Boundary_headers} DESTINATION include/CompactStar/Physics/Driver/Thermal/Boundary)

set(CompactStar_Physics_Driver_Thermal_Boundary_sources

	CompactStar/Physics/Driver/Thermal/Boundary/src/SurfaceGravity.cpp
	CompactStar/Physics/Driver/Thermal/Boundary/src/TbDefinition.cpp
	CompactStar/Physics/Driver/Thermal/Boundary/src/EnvelopePotekhin1997.cpp
	CompactStar/Physics/Driver/Thermal/Boundary/src/EnvelopePotekhin2003.cpp

	PARENT_SCOPE
)