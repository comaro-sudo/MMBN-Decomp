#!/usr/bin/env python3
"""Package the Wii U port into a distributable zip (no ROMs included).

    python3 ports/wiiu/package_wiiu.py       # after build_wiiu.py

Packs build/wiiu/mmbn4.wuhb with the third-party licence notices (mGBA MPL-2.0 and
its linked bundled libraries) read from the gitignored ext/mgba. Output:
build/mmbn4-wiiu.zip. The devkitPro libraries (WUT, wiiu-sdl2) are not covered here:
their licences are still unconfirmed (docs/credits.md row 14).
"""
from __future__ import annotations

import importlib.util
import sys
import zipfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def _pc_packager():
    spec = importlib.util.spec_from_file_location("package_pc", ROOT / "ports" / "pc" / "package_pc.py")
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def main() -> None:
    wuhb = ROOT / "build" / "wiiu" / "mmbn4.wuhb"
    if not wuhb.exists():
        sys.exit(f"missing {wuhb} - run: python3 ports/wiiu/build_wiiu.py")
    pc = _pc_packager()
    out_zip = ROOT / "build" / "mmbn4-wiiu.zip"
    with zipfile.ZipFile(out_zip, "w", compression=zipfile.ZIP_DEFLATED) as zf:
        zf.write(wuhb, "mmbn4/mmbn4.wuhb")
        readme = ROOT / "ports" / "wiiu" / "README.md"
        if readme.exists():
            zf.write(readme, "mmbn4/README.md")
        pc.add_notices(zf, "mmbn4", sdl=False)
    print(f"packaged {out_zip} ({out_zip.stat().st_size:,} bytes)")


if __name__ == "__main__":
    main()
