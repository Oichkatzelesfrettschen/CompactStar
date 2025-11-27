set(CompactStar_ChemicalHeating_headers
    SigmaOmegaRho_npemu.hpp
    Config.hpp
    Envelope.hpp
    State.hpp
    EvolutionSystem.hpp
    GeometryCache.hpp
    IntegratorGSL.hpp
    Microphysics.hpp
    Observers.hpp
    RateChannels.hpp
    StarContext.hpp
)

install(FILES ${CompactStar_ChemicalHeating_headers} DESTINATION include/CompactStar/ChemicalHeating)

set(CompactStar_ChemicalHeating_sources

    CompactStar/ChemicalHeating/src/SigmaOmegaRho_npemu.cpp
    # CompactStar/ChemicalHeating/src/Config.cpp
    # CompactStar/ChemicalHeating/src/Envelope.cpp
    # CompactStar/ChemicalHeating/src/EvolutionState.cpp
    # CompactStar/ChemicalHeating/src/EvolutionSystem.cpp
    # CompactStar/ChemicalHeating/src/GeometryCache.cpp
    # CompactStar/ChemicalHeating/src/IntegratorGSL.cpp
    # CompactStar/ChemicalHeating/src/Microphysics.cpp
    # CompactStar/ChemicalHeating/src/Observers.cpp
    # CompactStar/ChemicalHeating/src/RateChannels.cpp
    # CompactStar/ChemicalHeating/src/StarContext.cpp
    PARENT_SCOPE
)