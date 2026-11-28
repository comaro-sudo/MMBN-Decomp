#!/usr/bin/env python3
"""Package the PC port into a distributable zip (no ROMs included).

    python ports/pc/package_pc.py              # packages build/pc-arm64/Release
    python ports/pc/package_pc.py --arch x64   # packages build/pc-x64/Release
"""
from __future__ import annotations

import argparse
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
GPL_TEXT = ROOT / "ports" / "LICENSE"  # official GPL-3.0 text, unmodified

MGBA_SRC = ROOT / "ext" / "mgba"
MGBA_URL = "https://github.com/mgba-emu/mgba"
PROJECT_URL = "https://github.com/comaro-sudo/MMBN-Decomp"

NOTICE = """Third-party notices (binary release of the MMBN4 Violet Eclipse port)

This package contains no ROM. The licence texts are in the LICENSES folder.

The port's own code is licensed under the GNU GPL v3.0.
  Licence: LICENSES/GPL-3.0.txt
  Source:  {project_url}

mGBA 0.10.5 (MPL-2.0), statically linked and unmodified
  Copyright 2013-2022 Jeffrey Pfau and mGBA contributors.
  Licence: LICENSES/mGBA-MPL-2.0.txt
  Source:  {mgba_url} (tag 0.10.5). The port's own source, including the
  build scripts that link it, is at {project_url}.

Libraries bundled inside mGBA that are linked into this build:
  inih (BSD-3-Clause), Copyright 2009-2020 Ben Hoyt.
    Licence: LICENSES/inih-BSD-3-Clause.txt
  blip-buf (LGPL-2.1), Copyright 2003-2009 Shay Green.
    Licence: LICENSES/blip-buf-LGPL-2.1.txt
    Source:  {mgba_url}/tree/0.10.5/src/third-party/blip_buf
  (mGBA's zlib, libpng, LZMA, SQLite and Discord code is not built in: the
  port compiles mGBA with DISABLE_DEPS and MINIMAL_CORE.)
{sdl}"""

SDL_NOTICE = """
SDL3 (zlib licence), {how}
  Copyright (C) 1997-2026 Sam Lantinga and SDL contributors.
  Licence: LICENSES/SDL3-zlib.txt
"""


def add_notices(zf: zipfile.ZipFile, top: str, sdl: bool) -> list[str]:
    """Add the third-party licence files and NOTICE.txt under <top>/.
    Reads them from the gitignored ext/ and tools/ copies. Returns the names added.
    Exits if a required licence file is missing."""
    files = [
        (MGBA_SRC / "LICENSE", "LICENSES/mGBA-MPL-2.0.txt"),
        (MGBA_SRC / "src" / "third-party" / "inih" / "LICENSE.txt", "LICENSES/inih-BSD-3-Clause.txt"),
        (MGBA_SRC / "src" / "third-party" / "blip_buf" / "license.txt", "LICENSES/blip-buf-LGPL-2.1.txt"),
    ]
    if sdl:
        found = sorted((ROOT / "tools" / "sdl3").glob("SDL3-*/LICENSE.txt"))
        files.append((found[-1] if found else ROOT / "tools" / "sdl3" / "SDL3-?" / "LICENSE.txt",
                      "LICENSES/SDL3-zlib.txt"))
    files.insert(0, (GPL_TEXT, "LICENSES/GPL-3.0.txt"))
    missing = [str(src) for src, _ in files if not src.exists()]
    if missing:
        sys.exit("missing licence files (ext/ and tools/ must be present): " + ", ".join(missing))
    added = []
    for src, name in files:
        zf.write(src, f"{top}/{name}")
        added.append(name)
    text = NOTICE.format(
        mgba_url=MGBA_URL, project_url=PROJECT_URL,
        sdl=SDL_NOTICE.format(how="shipped as SDL3.dll") if sdl else "")
    zf.writestr(f"{top}/NOTICE.txt", text)
    added.append("NOTICE.txt")
    return added


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--arch", default="ARM64", choices=["ARM64", "x64"])
    args = ap.parse_args()

    arch = args.arch.lower()
    rel = ROOT / "build" / f"pc-{arch}" / "Release"
    exe = rel / "mmbn4.exe"
    dll = rel / "SDL3.dll"
    if not exe.exists() or not dll.exists():
        sys.exit(f"missing {exe} or {dll} — run: python ports/pc/build_pc.py --arch {args.arch}")

    out_zip = ROOT / "build" / f"mmbn4-pc-{arch}.zip"
    with zipfile.ZipFile(out_zip, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        zf.write(exe, "mmbn4/mmbn4.exe")
        zf.write(dll, "mmbn4/SDL3.dll")
        readme = ROOT / "ports" / "pc" / "README.md"
        if readme.exists():
            zf.write(readme, "mmbn4/README.md")
        add_notices(zf, "mmbn4", sdl=True)
    print(f"packaged {out_zip} ({out_zip.stat().st_size:,} bytes)")


if __name__ == "__main__":
    main()
