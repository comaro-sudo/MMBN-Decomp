#!/usr/bin/env python3
"""DC-5 C helper: compile a C file to a relocatable Thumb assembly fragment
that can be .included directly inside decomp/asm/*.s (.section .rom).

Uses pret/agbcc (GCC 2.9, matching Nintendo's original GBA SDK compiler) when
`--agbcc` is passed, or `arm-none-eabi-gcc` otherwise.

    python3 tools/c_to_inc.py src/bios_swi.c build/rs/c/bios_swi.s
    python3 tools/c_to_inc.py --agbcc src/m4a_cmds.c build/rs/c/m4a_cmds.s
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]
AGBCC = DECOMP.parent / "tools" / "agbcc" / "tools" / "agbcc" / "bin" / "agbcc"

SKIP_PREFIXES = (
    ".cpu ", ".arch ", ".fpu ", ".eabi_attribute ", ".file ",
    ".text", ".section ", ".ident ", ".code ", ".syntax ",
    ".size ", ".type ", ".bss", ".data", ".gcc2_compiled",
)


def main() -> None:
    args = sys.argv[1:]
    use_agbcc = False
    if args and args[0] == "--agbcc":
        use_agbcc = True
        args = args[1:]
    if len(args) != 2:
        sys.exit("usage: c_to_inc.py [--agbcc] <in.c> <out.s>")
    src, dst = Path(args[0]), Path(args[1])
    dst.parent.mkdir(parents=True, exist_ok=True)

    if use_agbcc:
        if not AGBCC.exists():
            sys.exit(f"missing agbcc at {AGBCC}")
        tmp_i = dst.with_suffix(".i")
        tmp_s = dst.with_suffix(".raw.s")
        subprocess.run(["arm-none-eabi-cpp", "-nostdinc", str(src), "-o", str(tmp_i)], check=True)
        subprocess.run([str(AGBCC), "-O2", "-mthumb-interwork", "-fhex-asm", str(tmp_i), "-o", str(tmp_s)], check=True)
        raw_asm = tmp_s.read_text(encoding="utf-8")
        tmp_i.unlink(missing_ok=True)
        tmp_s.unlink(missing_ok=True)
    else:
        cmd = [
            "arm-none-eabi-gcc",
            "-mcpu=arm7tdmi", "-mthumb", "-mthumb-interwork",
            "-O2", "-fno-ident", "-fomit-frame-pointer",
            "-S", "-o", "-", str(src),
        ]
        res = subprocess.run(cmd, check=True, capture_output=True, text=True)
        raw_asm = res.stdout

    compiler = "agbcc" if use_agbcc else "arm-none-eabi-gcc"
    out_lines = [f"@ Compiled from {src} by {compiler} (DC-5 C)", "    .thumb"]
    if use_agbcc:
        out_lines.append("    .syntax divided")
    for line in raw_asm.splitlines():
        s = line.strip()
        if not s or s.startswith("@"):
            continue
        if any(s.startswith(p) for p in SKIP_PREFIXES):
            continue
        if not use_agbcc and (s.startswith(".align ") or s.startswith(".p2align ")):
            continue
        out_lines.append(line)
    if use_agbcc:
        out_lines.append("    .syntax unified")
    dst.write_text("\n".join(out_lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
