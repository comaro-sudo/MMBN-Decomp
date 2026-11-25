cmake_minimum_required(VERSION 3.20)
project(mmbn4_wiiu C)

# Built by ports/wiiu/build_wiiu.py inside WSL with devkitPPC + wut:
#   MGBA_DIR    ext/mgba          (mGBA 0.10.5 source, MPL-2.0)
#   MGBA_BUILD  build/mgba-wiiu   (static libmgba.a, GBA core only)
set(MGBA_DIR "" CACHE PATH "mGBA source dir")
set(MGBA_BUILD "" CACHE PATH "libmgba Wii U build dir")

find_package(SDL2 REQUIRED)

add_executable(mmbn4
    ../common/bench.c
    ../common/emu_mgba.c
    ../common/game.c
    ../common/pet_model.c
    ../common/pet_ui.c
    ${MGBA_DIR}/src/util/vfs/vfs-fd.c
    ${MGBA_DIR}/src/util/vfs/vfs-dirent.c
    src/ui_draw_sdl2.c
    src/main.c
)

set_target_properties(mmbn4 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

target_include_directories(mmbn4 PRIVATE
    ../common
    ${MGBA_DIR}/include
    ${MGBA_BUILD}/include
)

target_compile_definitions(mmbn4 PRIVATE
    BUILD_STATIC
    MINIMAL_CORE=1
    _GNU_SOURCE
    _DEFAULT_SOURCE
    __WIIU__
)

target_compile_options(mmbn4 PRIVATE
    -O2 -Wall -Wno-unused-function
    -include ${MGBA_BUILD}/include/mgba/flags.h
)

target_link_libraries(mmbn4 PRIVATE
    ${MGBA_BUILD}/libmgba.a
    SDL2::SDL2-static
    m
)

wut_create_rpx(mmbn4)
wut_create_wuhb(mmbn4
    NAME "MMBN4 Violet Eclipse"
    SHORTNAME "MMBN4 VE"
    AUTHOR "MMBN-Decomp"
)
