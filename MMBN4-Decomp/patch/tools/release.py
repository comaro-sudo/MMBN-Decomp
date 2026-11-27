#!/usr/bin/env python3
"""Violet Eclipse release packager (docs/release-checklist.md section 2).

    python patch/tools/release.py              build --public for both versions, check, package
    python patch/tools/release.py --no-zip     same, but leave build/release/ unzipped
    python patch/tools/release.py --name NAME  zip name (default: violet-eclipse-<date>)

Steps: build.build(key, public=True) for rs and bm -> build.check(key) -> collect into build/release/:
  bn4_rs_overhaul.bps, bn4_bm_overhaul.bps   the patches (they hold no complete ROM)
  RELEASE-SHA1.txt                          SHA-1 of each .bps, of each patched ROM, and of each input ROM
  README.md                                  copy of docs/player-readme-draft.md
then zips the folder to build/<name>.zip. The patched .gba files are NOT put in the release (they are
whole Capcom ROMs); only their SHA-1s are recorded. Everything lands in the gitignored build/.
"""
from __future__ import annotations

import argparse
import datetime
import hashlib
import importlib.util
import shutil
import sys
import zipfile
from pathlib import Path

PATCH = Path(__file__).resolve().parent.parent
spec = importlib.util.spec_from_file_location("bn4_build", PATCH / "build.py")
bn4_build = importlib.util.module_from_spec(spec)
sys.modules["bn4_build"] = bn4_build
spec.loader.exec_module(bn4_build)

ROOT = bn4_build.ROOT
BUILD = bn4_build.BUILD
README_SRC = ROOT / "docs" / "player-readme-draft.md"
OUT_DIR = BUILD / "release"


def sha1(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--no-zip", action="store_true", help="leave build/release/ unzipped")
    ap.add_argument("--name", default=f"violet-eclipse-{datetime.date.today().isoformat()}",
                    help="zip name without extension")
    args = ap.parse_args()

    if not README_SRC.exists():
        bn4_build.fail(f"missing {README_SRC.relative_to(ROOT)}")
    keys = list(bn4_build.VERSIONS)

    for key in keys:                       # --public builds (no local-only art), then the identity tests
        bn4_build.build(key, hello=False, public=True)
    for key in keys:
        bn4_build.check(key)

    if OUT_DIR.exists():
        shutil.rmtree(OUT_DIR)
    OUT_DIR.mkdir(parents=True)

    lines = ["Mega Man Battle Network 4: Violet Eclipse - release checksums (SHA-1)", ""]
    for key in keys:
        v = bn4_build.VERSIONS[key]
        bps = BUILD / f"bn4_{key}_overhaul.bps"
        rom = BUILD / f"bn4_{key}_overhaul.gba"
        shutil.copy2(bps, OUT_DIR / bps.name)
        lines += [
            f"[{v['name']}]",
            f"  input ROM (yours, must match): {v['sha1']}  {v['rom'].name}",
            f"  patch:                         {sha1(bps)}  {bps.name}",
            f"  patched ROM (expected result): {sha1(rom)}  {rom.name}",
            "",
        ]
    (OUT_DIR / "RELEASE-SHA1.txt").write_text("\n".join(lines), encoding="utf-8", newline="\n")
    shutil.copy2(README_SRC, OUT_DIR / "README.md")

    print("== release contents")
    for f in sorted(OUT_DIR.iterdir()):
        print(f"   {f.name}  ({f.stat().st_size} bytes)")
    print("\n".join("   " + l for l in lines))

    if not args.no_zip:
        zpath = BUILD / f"{args.name}.zip"
        with zipfile.ZipFile(zpath, "w", zipfile.ZIP_DEFLATED) as z:
            for f in sorted(OUT_DIR.iterdir()):
                z.write(f, f"{args.name}/{f.name}")
        print(f"== {zpath.relative_to(ROOT)} ({zpath.stat().st_size} bytes, sha1 {sha1(zpath)})")


if __name__ == "__main__":
    main()
