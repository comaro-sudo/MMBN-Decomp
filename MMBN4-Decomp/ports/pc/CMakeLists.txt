cmake_minimum_required(VERSION 3.20)
project(mmbn4_pc C)

# Built by ports/pc/build_pc.py, which passes:
#   MGBA_DIR    ext/mgba             (mGBA 0.10.5 source, MPL-2.0)
#   MGBA_BUILD  build/mgba-<arch>    (static libmgba, GBA core only)
#   SDL3_DIR    tools/sdl3/SDL3-x.y.z/cmake
set(MGBA_DIR "" CACHE PATH "mGBA source dir")
set(MGBA_BUILD "" CACHE PATH "libmgba build dir")

find_package(SDL3 REQUIRED CONFIG)

add_executable(mmbn4 WIN32
    ../common/bench.c
    ../common/emu_mgba.c
    ../common/game.c
    ../common/pet_model.c
    ../common/pet_ui.c
    src/main.c
    src/pet.c
    src/ui_draw_sdl3.c
)

set_target_properties(mmbn4 PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED ON)

target_include_directories(mmbn4 PRIVATE
    ../common
    ${MGBA_DIR}/include
    ${MGBA_BUILD}/include
)

# Must match how libmgba was built (see build_pc.py).
target_compile_definitions(mmbn4 PRIVATE
    BUILD_STATIC
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
)
if(MSVC)
    # libmgba's generated flags.h (M_CORE_GBA, ...) is force-included so every
    # mGBA header sees the same configuration the library was compiled with.
    target_compile_options(mmbn4 PRIVATE /W3 /utf-8 "/FI${MGBA_BUILD}/include/mgba/flags.h")
    # WIN32 subsystem, but keep a normal main().
    target_link_options(mmbn4 PRIVATE /ENTRY:mainCRTStartup)
endif()

target_link_libraries(mmbn4 PRIVATE
    ${MGBA_BUILD}/Release/mgba.lib
    SDL3::SDL3
    ws2_32 shlwapi
)

# SDL3.dll next to the exe.
add_custom_command(TARGET mmbn4 POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:mmbn4>
)
