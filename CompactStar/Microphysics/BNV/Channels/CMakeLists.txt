set(CompactStar_Microphysics_BNV_Channels_headers
    BNV_B_Chi_Photon.hpp
    BNV_B_Chi_Transition.hpp
    BNV_B_Chi_Combo.hpp
    BNV_B_Psi_Pion.hpp
)

install(FILES ${CompactStar_Microphysics_BNV_Channels_headers} DESTINATION include/CompactStar/BNV/Channels)

set(CompactStar_Microphysics_BNV_Channels_sources

    CompactStar/Microphysics/BNV/Channels/src/BNV_B_Chi_Combo.cpp
    CompactStar/Microphysics/BNV/Channels/src/BNV_B_Chi_Photon.cpp
    CompactStar/Microphysics/BNV/Channels/src/BNV_B_Chi_Transition.cpp
    CompactStar/Microphysics/BNV/Channels/src/BNV_B_Psi_Pion.cpp

    PARENT_SCOPE
)