#!/usr/bin/env python3
"""Build the Wii U port (.rpx + .wuhb) using devkitPPC + wut in WSL.

    wsl.exe -d Ubuntu-24.04 -- bash -lc "cd /mnt/c/proyectos/MMBN-Decomp/MMBN4-Decomp && python3 ports/wiiu/build_wiiu.py"

Needs: devkitPPC, wut, wiiu-sdl2 in /opt/devkitpro, and ext/mgba (mGBA 0.10.5).
Output: build/wiiu/mmbn4.rpx and build/wiiu/mmbn4.wuhb.
"""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MGBA_SRC = ROOT / "ext" / "mgba"
DKP = Path(os.environ.get("DEVKITPRO", "/opt/devkitpro"))
TOOLCHAIN = Path(__file__).resolve().parent / "wiiu_toolchain.cmake"

MGBA_FLAGS = [
    "-DCMAKE_BUILD_TYPE=Release",
    "-DLIBMGBA_ONLY=ON", "-DBUILD_STATIC=ON", "-DBUILD_SHARED=OFF",
    "-DM_CORE_GB=OFF", "-DM_CORE_GBA=ON", "-DDISABLE_DEPS=ON",
    "-DUSE_DEBUGGERS=OFF", "-DUSE_GDB_STUB=OFF", "-DENABLE_SCRIPTING=OFF", "-DUSE_LUA=OFF",
    "-DUSE_EDITLINE=OFF", "-DUSE_EPOXY=OFF", "-DUSE_DISCORD_RPC=OFF",
    "-DBUILD_GL=OFF", "-DBUILD_GLES2=OFF", "-DBUILD_GLES3=OFF",
    "-DBUILD_QT=OFF", "-DBUILD_SDL=OFF", "-DBUILD_LTO=OFF",
    "-DUSE_PTHREADS=OFF", "-DUSE_ZLIB=OFF", "-DUSE_PNG=OFF",
    "-DUSE_MINIZIP=OFF", "-DUSE_LIBZIP=OFF",
    "-DMINIMAL_CORE=ON",
    "-DCMAKE_C_FLAGS=-D_GNU_SOURCE -D_DEFAULT_SOURCE -DMINIMAL_CORE=1",
    "-DCMAKE_POLICY_VERSION_MINIMUM=3.5",
]


def run(cmd: list[str], env: dict[str, str]) -> None:
    print("> " + " ".join(cmd))
    r = subprocess.run(cmd, env=env)
    if r.returncode != 0:
        sys.exit(r.returncode)


def main() -> None:
    if not (MGBA_SRC / "CMakeLists.txt").exists():
        sys.exit("missing ext/mgba")
    if not TOOLCHAIN.exists():
        sys.exit(f"missing Wii U toolchain at {TOOLCHAIN}")

    env = os.environ.copy()
    env["DEVKITPRO"] = str(DKP)
    env["DEVKITPPC"] = str(DKP / "devkitPPC")
    venv_bin = Path.home() / ".venvs" / "mmbn4-decomp" / "bin"
    extra = f"{DKP}/portlibs/wiiu/bin:{DKP}/tools/bin:{DKP}/devkitPPC/bin"
    if venv_bin.exists():
        extra = f"{venv_bin}:{extra}"
    env["PATH"] = extra + ":" + env.get("PATH", "")

    mgba_build = ROOT / "build" / "mgba-wiiu"
    app_build = ROOT / "build" / "wiiu"
    jobs = str(os.cpu_count() or 4)

    if not (mgba_build / "libmgba.a").exists():
        run(["cmake", "-S", str(MGBA_SRC), "-B", str(mgba_build),
             f"-DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN}",
             "-DCMAKE_MAKE_PROGRAM=/usr/bin/make",
             "-DPKG_CONFIG_EXECUTABLE=/usr/bin/false", *MGBA_FLAGS], env)
        run(["cmake", "--build", str(mgba_build), "-j", jobs], env)

    run(["cmake", "-S", str(ROOT / "ports" / "wiiu"), "-B", str(app_build),
         f"-DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN}",
         "-DCMAKE_MAKE_PROGRAM=/usr/bin/make",
         "-DPKG_CONFIG_EXECUTABLE=/usr/bin/false",
         "-DCMAKE_BUILD_TYPE=Release",
         f"-DMGBA_DIR={MGBA_SRC}",
         f"-DMGBA_BUILD={mgba_build}"], env)
    run(["cmake", "--build", str(app_build), "-j", jobs], env)

    print(f"built {app_build / 'mmbn4.wuhb'}")


if __name__ == "__main__":
    main()
