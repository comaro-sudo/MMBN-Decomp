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
        mgba_lic = ROOT / "ext" / "mgba" / "LICENSE"
        if mgba_lic.exists():
            zf.write(mgba_lic, "mmbn4/LICENSES/mGBA-MPL-2.0.txt")
    print(f"packaged {out_zip} ({out_zip.stat().st_size:,} bytes)")


if __name__ == "__main__":
    main()
