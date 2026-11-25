#!/usr/bin/env python3
"""DC-5 C helper: compile a C file to a relocatable Thumb assembly fragment
that can be .included directly inside decomp/asm/*.s (.section .rom).

    python3 tools/c_to_inc.py src/bios_swi.c build/rs/c/bios_swi.s
"""
from __future__ import annotations

import subprocess
import sys
from pathlib import Path

SKIP_PREFIXES = (
    ".cpu ", ".arch ", ".fpu ", ".eabi_attribute ", ".file ",
    ".text", ".section ", ".ident ", ".code ", ".syntax ",
    ".align ", ".p2align ", ".size ", ".type ", ".bss", ".data",
)


def main() -> None:
    if len(sys.argv) != 3:
        sys.exit("usage: c_to_inc.py <in.c> <out.s>")
    src, dst = Path(sys.argv[1]), Path(sys.argv[2])
    dst.parent.mkdir(parents=True, exist_ok=True)

    cmd = [
        "arm-none-eabi-gcc",
        "-mcpu=arm7tdmi", "-mthumb", "-mthumb-interwork",
        "-O2", "-fno-ident", "-fomit-frame-pointer",
        "-S", "-o", "-", str(src),
    ]
    res = subprocess.run(cmd, check=True, capture_output=True, text=True)

    out_lines = [f"@ Compiled from {src} by arm-none-eabi-gcc (DC-5 C)", "    .thumb"]
    for line in res.stdout.splitlines():
        s = line.strip()
        if not s or any(s.startswith(p) for p in SKIP_PREFIXES):
            continue
        out_lines.append(line)
    dst.write_text("\n".join(out_lines) + "\n", encoding="utf-8")


if __name__ == "__main__":
    main()
