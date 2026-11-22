#!/usr/bin/env python3
"""Build the PC port (Windows, MSVC).

    python ports/pc/build_pc.py            # ARM64 (native on the ThinkPad)
    python ports/pc/build_pc.py --arch x64 # x64 build for other PCs
    python ports/pc/build_pc.py --run      # build, then run from the repo root

Needs: VS 2022 Build Tools, CMake, ext/mgba (mGBA 0.10.5), tools/sdl3 (SDL3 VC package).
Output: build/pc-<arch>/Release/mmbn4.exe (+ SDL3.dll).
"""
from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MGBA_SRC = ROOT / "ext" / "mgba"
SDL3_CMAKE = next((ROOT / "tools" / "sdl3").glob("SDL3-*/cmake"), None)

MGBA_FLAGS = [
    "-DLIBMGBA_ONLY=ON", "-DBUILD_STATIC=ON", "-DBUILD_SHARED=OFF",
    "-DM_CORE_GB=OFF", "-DM_CORE_GBA=ON", "-DDISABLE_DEPS=ON",
    "-DUSE_DEBUGGERS=OFF", "-DUSE_GDB_STUB=OFF", "-DENABLE_SCRIPTING=OFF", "-DUSE_LUA=OFF",
    "-DUSE_EDITLINE=OFF", "-DUSE_EPOXY=OFF", "-DUSE_DISCORD_RPC=OFF",
    "-DBUILD_GL=OFF", "-DBUILD_GLES2=OFF", "-DBUILD_GLES3=OFF",
    "-DBUILD_QT=OFF", "-DBUILD_SDL=OFF", "-DBUILD_LTO=OFF",
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
]


def run(cmd: list) -> None:
    print("> " + " ".join(str(c) for c in cmd))
    r = subprocess.run([str(c) for c in cmd])
    if r.returncode != 0:
        sys.exit(r.returncode)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--arch", default="ARM64", choices=["ARM64", "x64"])
    ap.add_argument("--run", action="store_true")
    ap.add_argument("rom", nargs="?", help="ROM to run with --run")
    args = ap.parse_args()

    if not (MGBA_SRC / "CMakeLists.txt").exists():
        sys.exit("missing ext/mgba: git clone --depth 1 --branch 0.10.5 https://github.com/mgba-emu/mgba.git ext/mgba")
    if SDL3_CMAKE is None:
        sys.exit("missing tools/sdl3: download SDL3-devel-<ver>-VC.zip from libsdl-org/SDL releases")

    arch = args.arch.lower()
    mgba_build = ROOT / "build" / f"mgba-{arch}"
    app_build = ROOT / "build" / f"pc-{arch}"

    if not (mgba_build / "Release" / "mgba.lib").exists():
        run(["cmake", "-S", MGBA_SRC, "-B", mgba_build, "-G", "Visual Studio 17 2022", "-A", args.arch, *MGBA_FLAGS])
        run(["cmake", "--build", mgba_build, "--config", "Release", "-j"])

    run(["cmake", "-S", ROOT / "ports" / "pc", "-B", app_build, "-G", "Visual Studio 17 2022", "-A", args.arch,
         f"-DMGBA_DIR={MGBA_SRC.as_posix()}", f"-DMGBA_BUILD={mgba_build.as_posix()}",
         f"-DSDL3_DIR={SDL3_CMAKE.as_posix()}"])
    run(["cmake", "--build", app_build, "--config", "Release", "-j"])

    exe = app_build / "Release" / "mmbn4.exe"
    print(f"built {exe}")
    if args.run:
        subprocess.Popen([str(exe), *([args.rom] if args.rom else [])], cwd=ROOT)


if __name__ == "__main__":
    main()
