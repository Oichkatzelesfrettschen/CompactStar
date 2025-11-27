set(CompactStar_Microphysics_BNV_Analysis_headers
    BNV_Analysis.hpp
    Decay_Analysis.hpp
    BNV_Sequence.hpp
)

install(FILES ${CompactStar_Microphysics_BNV_Analysis_headers} DESTINATION include/CompactStar/Microphysics/BNV/Analysis)

set(CompactStar_Microphysics_BNV_Analysis_sources

    CompactStar/Microphysics/BNV/Analysis/src/BNV_Analysis.cpp
    CompactStar/Microphysics/BNV/Analysis/src/Decay_Analysis.cpp
    CompactStar/Microphysics/BNV/Analysis/src/BNV_Sequence.cpp

    PARENT_SCOPE
)