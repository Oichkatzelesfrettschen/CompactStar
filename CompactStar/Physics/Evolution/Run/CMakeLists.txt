set(CompactStar_Physics_Evolution_Run_headers
	RunObservers.hpp
	RunPaths.hpp
	RunBuilder.hpp
)

install(FILES ${CompactStar_Physics_Evolution_Run_headers} DESTINATION include/CompactStar/Physics/Evolution/Run)

set(CompactStar_Physics_Evolution_Run_sources
	CompactStar/Physics/Evolution/Run/src/RunObservers.cpp
	CompactStar/Physics/Evolution/Run/src/RunPaths.cpp
	CompactStar/Physics/Evolution/Run/src/RunBuilder.cpp

	PARENT_SCOPE
)