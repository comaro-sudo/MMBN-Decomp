#!/usr/bin/env python3
"""MMBN4 Overhaul build script.

    python patch/build.py              build both versions -> build/*.gba + build/*.bps
    python patch/build.py bm           build Blue Moon only (bm | rs | all)
    python patch/build.py --hello      include the hello-world test patch (Cannon = 99 dmg)
    python patch/build.py --check      run the identity tests (no output ROMs kept)

Pipeline: verify ROM hash -> armips -> fix header checksum -> flips .bps
          -> re-apply the .bps to the original and confirm the result matches.
"""
from __future__ import annotations

import argparse
import hashlib
import subprocess
import sys
from pathlib import Path

PATCH = Path(__file__).resolve().parent   # patch/ (Phase 1 ROM patch project)
ROOT = PATCH.parent                        # repo root (roms/, tools/, ext/, build/)
TOOLS = ROOT / "tools"
ARMIPS = TOOLS / "armips" / "armips.exe"
FLIPS = TOOLS / "flips" / "flips.exe"
BUILD = ROOT / "build"
CHIP_TABLES = ROOT / "ext" / "mmbn-chip-tables"

VERSIONS = {
    "bm": {
        "name": "Blue Moon (USA)",
        "rom": ROOT / "roms" / "bn4_bm_us.gba",
        "sha1": "5e017c803ddc768efb8010314b46bc17e9757f71",
        "version_file": PATCH / "src" / "bm" / "version.asm",
        "chip_table": CHIP_TABLES / "bn4b_chip_table.asm",
    },
    "rs": {
        "name": "Red Sun (USA)",
        "rom": ROOT / "roms" / "bn4_rs_us.gba",
        "sha1": "a97e96a7da03abd70f7953328e511b9fa29179f1",
        "version_file": PATCH / "src" / "rs" / "version.asm",
        "chip_table": CHIP_TABLES / "bn4r_chip_table.asm",
    },
}


def sha1(path: Path) -> str:
    return hashlib.sha1(path.read_bytes()).hexdigest()


def fail(msg: str) -> None:
    print(f"ERROR: {msg}", file=sys.stderr)
    sys.exit(1)


def run(cmd: list, cwd: Path = ROOT) -> None:
    result = subprocess.run([str(c) for c in cmd], cwd=cwd, capture_output=True, text=True)
    lines = (result.stdout + result.stderr).splitlines()
    out = "\n".join(l for l in lines if l.strip() and not l.startswith("Please wait")).strip()
    if result.returncode != 0:
        fail(f"{Path(str(cmd[0])).name} failed ({result.returncode}):\n{out}")
    if out:
        print("   " + out.replace("\n", "\n   "))


def armips(asm: Path, rom_in: Path, rom_out: Path, version_file: Path, extra: list | None = None) -> None:
    cmd = [
        ARMIPS, asm,
        "-strequ", "ROM_IN", rom_in.as_posix(),
        "-strequ", "ROM_OUT", rom_out.as_posix(),
        "-strequ", "VERSION_FILE", version_file.as_posix(),
        *(extra or []),
    ]
    run(cmd, cwd=asm.parent)


def fix_header_checksum(rom: Path) -> None:
    data = bytearray(rom.read_bytes())
    data[0xBD] = (-sum(data[0xA0:0xBD]) - 0x19) & 0xFF
    rom.write_bytes(data)


def check_inputs(key: str) -> dict:
    v = VERSIONS[key]
    for tool in (ARMIPS, FLIPS):
        if not tool.exists():
            fail(f"missing tool {tool.relative_to(ROOT)} (see plan/01-setup-and-toolchain.md)")
    if not v["rom"].exists():
        fail(f"missing ROM {v['rom'].relative_to(ROOT)} (see roms/README.md)")
    got = sha1(v["rom"])
    if got != v["sha1"]:
        fail(f"{v['rom'].name}: SHA-1 {got} does not match expected {v['sha1']}")
    return v


def build(key: str, hello: bool) -> None:
    v = check_inputs(key)
    BUILD.mkdir(exist_ok=True)
    suffix = "_hello" if hello else ""
    out_rom = BUILD / f"bn4_{key}_overhaul{suffix}.gba"
    out_bps = BUILD / f"bn4_{key}_overhaul{suffix}.bps"
    print(f"== {v['name']}")

    armips(PATCH / "src" / "main.asm", v["rom"], out_rom, v["version_file"],
           ["-definelabel", "HELLO", "1" if hello else "0"])
    fix_header_checksum(out_rom)

    original = v["rom"].read_bytes()
    built = out_rom.read_bytes()
    changed = sum(1 for a, b in zip(original, built) if a != b) + abs(len(original) - len(built))
    print(f"   {out_rom.name}: {changed} byte(s) differ from original, SHA-1 {sha1(out_rom)}")

    if out_bps.exists():
        out_bps.unlink()
    run([FLIPS, "--create", "--bps", v["rom"], out_rom, out_bps])

    # Round-trip: applying the patch to the original must reproduce the build.
    roundtrip = BUILD / f".roundtrip_{key}.gba"
    run([FLIPS, "--apply", out_bps, v["rom"], roundtrip])
    if sha1(roundtrip) != sha1(out_rom):
        fail(f"{out_bps.name}: round-trip mismatch")
    roundtrip.unlink()
    print(f"   {out_bps.name}: {out_bps.stat().st_size} bytes, round-trip OK")


def check(key: str) -> None:
    v = check_inputs(key)
    BUILD.mkdir(exist_ok=True)
    print(f"== {v['name']} identity tests")
    tmp = BUILD / f".identity_{key}.gba"
    armips(PATCH / "test" / "chip_table_identity.asm", v["rom"], tmp, v["version_file"],
           ["-strequ", "CHIP_MACROS", (CHIP_TABLES / "bn4_chip_macros.asm").as_posix(),
            "-strequ", "CHIP_TABLE_FILE", v["chip_table"].as_posix()])
    ok = sha1(tmp) == v["sha1"]
    tmp.unlink()
    print(f"   chip table re-assembly: {'PASS (byte-identical)' if ok else 'FAIL'}")
    if not ok:
        sys.exit(1)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("version", nargs="?", default="all", choices=["all", *VERSIONS])
    ap.add_argument("--hello", action="store_true", help="include the hello-world test patch")
    ap.add_argument("--check", action="store_true", help="run identity tests only")
    args = ap.parse_args()

    keys = list(VERSIONS) if args.version == "all" else [args.version]
    for key in keys:
        check(key) if args.check else build(key, args.hello)


if __name__ == "__main__":
    main()
