# Wii U CMake toolchain wrapper around devkitPro's devkitPPC.cmake + WUT.
# Avoids requiring wiiu-pkg-config when building static libmgba + SDL2 via CMake.
cmake_minimum_required(VERSION 3.13)

if(NOT CMAKE_SYSTEM_NAME)
    set(CMAKE_SYSTEM_NAME CafeOS)
endif()

set(DEVKITPRO "/opt/devkitpro")
include(${DEVKITPRO}/cmake/devkitPPC.cmake)

set(WUT_ROOT ${DEVKITPRO}/wut)
set(DKP_INSTALL_PREFIX_INIT ${DEVKITPRO}/portlibs/wiiu)

__dkp_platform_prefix(
    ${DEVKITPRO}/portlibs/wiiu
    ${DEVKITPRO}/portlibs/ppc
    ${WUT_ROOT}
)

find_program(WUT_ELF2RPL_EXE NAMES elf2rpl HINTS "${DEVKITPRO}/tools/bin")
find_program(WUT_WUHBTOOL_EXE NAMES wuhbtool HINTS "${DEVKITPRO}/tools/bin")
find_program(WUT_RPLEXPORTGEN_EXE NAMES rplexportgen HINTS "${DEVKITPRO}/tools/bin")
find_program(WUT_RPLIMPORTGEN_EXE NAMES rplimportgen HINTS "${DEVKITPRO}/tools/bin")
