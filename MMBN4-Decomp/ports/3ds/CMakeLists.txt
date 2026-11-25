cmake_minimum_required(VERSION 3.20)
project(mmbn4_3ds C)

# Built by ports/3ds/build_3ds.py with devkitARM + libctru + citro2d:
#   MGBA_DIR    ext/mgba         (mGBA 0.10.5 source, MPL-2.0)
#   MGBA_BUILD  build/mgba-3ds   (static libmgba.a, GBA core only)
set(MGBA_DIR "" CACHE PATH "mGBA source dir")
set(MGBA_BUILD "" CACHE PATH "libmgba 3DS build dir")

add_executable(mmbn4
    ../common/bench.c
    ../common/emu_mgba.c
    ../common/game.c
    ../common/pet_model.c
    ../common/pet_ui.c
    ${MGBA_DIR}/src/util/vfs/vfs-fd.c
    ${MGBA_DIR}/src/util/vfs/vfs-dirent.c
    src/ui_draw_c2d.c
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
    __3DS__
)

target_compile_options(mmbn4 PRIVATE
    -O2 -Wall -Wno-unused-function
    -include ${MGBA_BUILD}/include/mgba/flags.h
)

target_link_libraries(mmbn4 PRIVATE
    ${MGBA_BUILD}/libmgba.a
    citro2d citro3d ctru m
)

ctr_generate_smdh(mmbn4.smdh
    NAME "MMBN4 Violet Eclipse"
    DESCRIPTION "Mega Man Battle Network 4 - Violet Eclipse"
    AUTHOR "MMBN-Decomp"
)
ctr_create_3dsx(mmbn4 SMDH mmbn4.smdh)
