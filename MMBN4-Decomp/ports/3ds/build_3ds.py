#!/usr/bin/env python3
"""Build the New 3DS port (.3dsx) using devkitARM + libctru + citro2d in WSL.

    python3 ports/3ds/build_3ds.py

Needs: devkitARM, libctru, citro2d, citro3d in /opt/devkitpro (3ds-dev), and ext/mgba.
Output: build/3ds/mmbn4.3dsx.
"""
from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
MGBA_SRC = ROOT / "ext" / "mgba"
DKP = Path(os.environ.get("DEVKITPRO", "/opt/devkitpro"))
TOOLCHAIN = DKP / "cmake" / "3DS.cmake"

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
        sys.exit(f"missing 3DS toolchain at {TOOLCHAIN} — install devkitPro 3ds-dev (devkitARM + libctru + citro2d)")

    env = os.environ.copy()
    env["DEVKITPRO"] = str(DKP)
    env["DEVKITARM"] = str(DKP / "devkitARM")
    venv_bin = Path.home() / ".venvs" / "mmbn4-decomp" / "bin"
    extra = f"{DKP}/tools/bin:{DKP}/devkitARM/bin"
    if venv_bin.exists():
        extra = f"{venv_bin}:{extra}"
    env["PATH"] = extra + ":" + env.get("PATH", "")

    mgba_build = ROOT / "build" / "mgba-3ds"
    app_build = ROOT / "build" / "3ds"
    jobs = str(os.cpu_count() or 4)

    if not (mgba_build / "libmgba.a").exists():
        run(["cmake", "-S", str(MGBA_SRC), "-B", str(mgba_build),
             f"-DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN}",
             "-DCMAKE_MAKE_PROGRAM=/usr/bin/make", *MGBA_FLAGS], env)
        run(["cmake", "--build", str(mgba_build), "-j", jobs], env)

    run(["cmake", "-S", str(ROOT / "ports" / "3ds"), "-B", str(app_build),
         f"-DCMAKE_TOOLCHAIN_FILE={TOOLCHAIN}",
         "-DCMAKE_MAKE_PROGRAM=/usr/bin/make",
         "-DCMAKE_BUILD_TYPE=Release",
         f"-DMGBA_DIR={MGBA_SRC}",
         f"-DMGBA_BUILD={mgba_build}"], env)
    run(["cmake", "--build", str(app_build), "-j", jobs], env)
    print(f"built {app_build / 'mmbn4.3dsx'}")


if __name__ == "__main__":
    main()
