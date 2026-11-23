#!/usr/bin/env python3
"""MMBN4 Overhaul build script.

    python patch/build.py              build both versions -> build/*.gba + build/*.bps
    python patch/build.py bm           build Blue Moon only (bm | rs | all)
    python patch/build.py --hello      include the hello-world test patch (Cannon = 99 dmg)
    python patch/build.py --check      run the identity tests (no output ROMs kept)
    python patch/build.py --public     leave out local-only third-party art (ref/chip-art) for a shareable patch

Pipeline: verify ROM hash -> armips -> TextPet (patch/src/scripts/text/*.tpl, if any)
          -> fix header checksum -> flips .bps
          -> re-apply the .bps to the original and confirm the result matches.

Text (plan/text-insertion.md): patch/src/scripts/text/ holds only OUR scripts, as TextPet
.tpl files (@archive <NAME> + script blocks; UTF-8 without BOM), merged into the archives with --patch;
in place when they still fit, otherwise into FS_TEXT (patch/src/common/defines.asm).
"""
from __future__ import annotations

import argparse
import hashlib
import re
import subprocess
import sys
from pathlib import Path

PATCH = Path(__file__).resolve().parent   # patch/ (Phase 1 ROM patch project)
ROOT = PATCH.parent                        # repo root (roms/, tools/, ext/, build/)
TOOLS = ROOT / "tools"
ARMIPS = TOOLS / "armips" / "armips.exe"
FLIPS = TOOLS / "flips" / "flips.exe"
TEXTPET_DIR = TOOLS / "textpet"
TEXTPET = TEXTPET_DIR / "TextPet.exe"
BUILD = ROOT / "build"
CHIP_TABLES = ROOT / "ext" / "mmbn-chip-tables"

VERSIONS = {
    "bm": {
        "name": "Blue Moon (USA)",
        "rom": ROOT / "roms" / "bn4_bm_us.gba",
        "sha1": "5e017c803ddc768efb8010314b46bc17e9757f71",
        "version_file": PATCH / "src" / "bm" / "version.asm",
        "chip_table": CHIP_TABLES / "bn4b_chip_table.asm",
        "reward_rows": 0x080F5F5C,      # BN4_REWARD_ROWS (RE-06): 276 rows x 40 bytes
        "icon_pal": 0x08747AEC,         # BN4_CHIP_ICON_PAL (CE)
        # TextPet archives our .tpl files patch (offsets from tools/textpet/indexes/mmbn4bm-us.tpi)
        "text_archives": {"CHIP_NAMES_1": 0x748708, "CHIP_DESCS_1": 0x74EACC},
    },
    "rs": {
        "name": "Red Sun (USA)",
        "rom": ROOT / "roms" / "bn4_rs_us.gba",
        "sha1": "a97e96a7da03abd70f7953328e511b9fa29179f1",
        "version_file": PATCH / "src" / "rs" / "version.asm",
        "chip_table": CHIP_TABLES / "bn4r_chip_table.asm",
        "reward_rows": 0x080F5F50,
        "icon_pal": 0x08747FF8,
        "text_archives": {"CHIP_NAMES_1": 0x748C14, "CHIP_DESCS_1": 0x74F03C},
    },
}
REWARD_ROWS_SIZE = 276 * 40


CHIP_ART_DIR = ROOT / "ref" / "chip-art"      # local only (gitignored): art that is in no ROM
CHIP_ART = {"DUO": "duo_image.png", "GPP": "gpp_image.png"}


def chip_art(key: str, public: bool) -> list:
    """Duo and Grand Prix Power chip art (docs/completion/chips.md section 3). Their art is in no ROM, so it
    comes from the owner's local ref/chip-art/ PNGs, converted by patch/tools/chipart.py. A --public build
    (or a missing PNG) leaves it out: the chips then keep their original (blank) art."""
    pngs = {k: CHIP_ART_DIR / f for k, f in CHIP_ART.items()}
    out = BUILD / "chipart" / key
    out.mkdir(parents=True, exist_ok=True)
    enabled = not public and all(p.exists() for p in pngs.values())
    if enabled:
        v = VERSIONS[key]
        run([sys.executable, PATCH / "tools" / "chipart.py", v["rom"], f"0x{v['icon_pal']:08X}", out,
             *(f"{k.lower()}={p}" for k, p in pngs.items())])
    else:
        # armips resolves .incbin names even inside a false .if, so they still need a file to name
        (out / "none.bin").write_bytes(b"")
    args = ["-definelabel", "CHIP_ART", "1" if enabled else "0"]
    for k in pngs:
        for part in ("image", "pal", "icon"):
            f = out / (f"{k.lower()}_{part}.bin" if enabled else "none.bin")
            args += ["-strequ", f"{k}_{part.upper()}_BIN", f.as_posix()]
    return args


def other_reward_rows(key: str) -> Path:
    """The other version's drop table, read from the player's own other ROM into build/ (never committed).
    WP-D2 random drops (patch/src/common/wp_d2_random_version_drops.asm) pick between the two tables."""
    other = "rs" if key == "bm" else "bm"
    o = check_inputs(other)
    data = o["rom"].read_bytes()
    off = o["reward_rows"] - 0x08000000
    out = BUILD / f"reward_rows_from_{other}.bin"
    out.write_bytes(data[off:off + REWARD_ROWS_SIZE])
    return out


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


def compile_new_text() -> list:
    """Compile our new dialogue archives (patch/src/scripts/new/<name>.tpl) to build/text/<name>.bin, the game's
    archive format (u16 offset table + encoded scripts), for the asm to .incbin as <NAME>_BIN. Version-neutral.
    Returns the extra armips arguments."""
    src = PATCH / "src" / "scripts" / "new"
    tpls = sorted(src.glob("*.tpl")) if src.is_dir() else []
    if not tpls:
        return []
    if not TEXTPET.exists():
        fail(f"missing tool {TEXTPET.relative_to(ROOT)} (see plan/text-insertion.md)")
    out_dir = BUILD / "text"
    out_dir.mkdir(parents=True, exist_ok=True)
    args = []
    for t in tpls:
        if t.read_bytes()[:3] == b"\xef\xbb\xbf":
            fail(f"{t.relative_to(ROOT)} starts with a UTF-8 BOM; TextPet rejects it (save as UTF-8 without BOM)")
        out = out_dir / f"{t.stem}.bin"
        if out.exists():
            out.unlink()
        cmd = [TEXTPET, "load-plugins", "plugins", "game", "mmbn4",
               "read-text-archives", t, "--format", "tpl", "write-text-archives", out, "--format", "bin"]
        result = subprocess.run([str(c) for c in cmd], cwd=TEXTPET_DIR, capture_output=True, text=True)
        text = result.stdout + result.stderr
        if result.returncode != 0 or "FATAL" in text or not out.exists():
            fail(f"TextPet could not compile {t.name}:\n{text[-1500:]}")
        args += ["-strequ", f"{t.stem.upper()}_BIN", out.as_posix()]
    print(f"   text: compiled {len(tpls)} new dialogue archive(s)")
    return args


def insert_text(key: str, rom: Path) -> None:
    """Patch our scripts (patch/src/scripts/text/*.tpl) into existing text archives of the built ROM.

    The .tpl files hold only our scripts and name their archive symbolically (`@archive CHIP_NAMES_1`);
    VERSIONS[key]["text_archives"] gives each version's offset. TextPet gets an index with only those
    archives, reads them from the ROM, merges our scripts in with --patch (every other script is kept),
    and writes them back, moving any that grew to FS_TEXT and repointing them (Gemini,
    decomp/notes/chip-text-design.md). No Capcom text is committed."""
    tpls = sorted((PATCH / "src" / "scripts" / "text").glob("*.tpl"))
    if not tpls:
        return
    if not TEXTPET.exists():
        fail(f"missing tool {TEXTPET.relative_to(ROOT)} (see plan/text-insertion.md)")
    archives = VERSIONS[key]["text_archives"]
    out = BUILD / "text" / key
    out.mkdir(parents=True, exist_ok=True)
    used, patched = set(), []
    for t in tpls:
        raw = t.read_bytes()
        if raw[:3] == b"\xef\xbb\xbf":
            fail(f"{t.relative_to(ROOT)} starts with a UTF-8 BOM; TextPet rejects it (save as UTF-8 without BOM)")
        def sub(m):
            name = m.group(1)
            if name not in archives:
                fail(f"{t.relative_to(ROOT)}: unknown archive {name} (add it to VERSIONS[...]['text_archives'])")
            used.add(archives[name])
            return f"@archive {archives[name]:X}"
        text = re.sub(r"^@archive\s+([A-Z0-9_]+)\s*$", sub, raw.decode("utf-8"), flags=re.M)
        dst = out / t.name
        dst.write_bytes(text.encode("utf-8"))
        patched.append(dst)
    index_lines = (TEXTPET_DIR / "indexes" / f"mmbn4{key}-us.tpi").read_text(encoding="utf-8").splitlines()
    picked = [l for l in index_lines if l.split(":")[0].strip().lower() in {f"0x{a:x}" for a in used}]
    if len(picked) != len(used):
        fail(f"TextPet index lacks some archives: {sorted(hex(a) for a in used)}")
    index = out / "patch.tpi"
    index.write_text("\n".join(picked) + "\n", encoding="utf-8")
    defines = (PATCH / "src" / "common" / "defines.asm").read_text(encoding="utf-8")
    m = re.search(r"^FS_TEXT\s+equ\s+0x([0-9A-Fa-f]+)", defines, re.M)
    if not m:
        fail("FS_TEXT not found in patch/src/common/defines.asm")
    free = int(m.group(1), 16) - 0x08000000
    cmd = [TEXTPET, "load-plugins", "plugins", "game", "mmbn4", "load-file", rom.resolve(),
           "load-file-index", index.resolve(), "read-text-archives", rom.resolve()]
    for d in patched:
        cmd += ["read-text-archives", d.resolve(), "--format", "tpl", "--patch"]
    cmd += ["write-text-archives", rom.resolve(), "--free-space", f"0x{free:X}"]
    result = subprocess.run([str(c) for c in cmd], cwd=TEXTPET_DIR, capture_output=True, text=True)
    out_txt = result.stdout + result.stderr
    if result.returncode != 0 or "FATAL" in out_txt:
        fail(f"TextPet failed:\n{out_txt[-1500:]}")
    print(f"   text: {len(patched)} script file(s) patched into {len(used)} archive(s) with TextPet")


def build(key: str, hello: bool, public: bool = False) -> None:
    v = check_inputs(key)
    BUILD.mkdir(exist_ok=True)
    suffix = "_hello" if hello else ""
    out_rom = BUILD / f"bn4_{key}_overhaul{suffix}.gba"
    out_bps = BUILD / f"bn4_{key}_overhaul{suffix}.bps"
    print(f"== {v['name']}")

    armips(PATCH / "src" / "main.asm", v["rom"], out_rom, v["version_file"],
           ["-definelabel", "HELLO", "1" if hello else "0",
            "-strequ", "OTHER_REWARD_ROWS", other_reward_rows(key).as_posix(),
            *compile_new_text(), *chip_art(key, public)])
    insert_text(key, out_rom)
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
    ap.add_argument("--public", action="store_true",
                    help="leave out local-only third-party art (ref/chip-art) for a shareable patch")
    args = ap.parse_args()

    keys = list(VERSIONS) if args.version == "all" else [args.version]
    for key in keys:
        check(key) if args.check else build(key, args.hello, args.public)


if __name__ == "__main__":
    main()
