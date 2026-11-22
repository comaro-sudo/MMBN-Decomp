#!/usr/bin/env python3
"""Report labels whose assembled position differs from the address in their name.

    python3 tools/drift.py [rs]

Labels are named after their original address (sub_XXXXXXXX / _XXXXXXXX), so
any difference means something before them assembled to a different size.
"""
import re
import subprocess
import sys
from pathlib import Path

DECOMP = Path(__file__).resolve().parents[1]


def main() -> None:
    v = sys.argv[1] if len(sys.argv) > 1 else "rs"
    out = subprocess.run(["arm-none-eabi-nm", "-n", str(DECOMP / "build" / v / "rom.o")],
                         capture_output=True, text=True, check=True).stdout
    shown = 0
    prev_ok = None
    for line in out.splitlines():
        parts = line.split()
        if len(parts) != 3 or parts[1] not in "tT":
            continue
        m = re.fullmatch(r"(?:sub)?_([0-9A-F]{8})", parts[2])
        if not m:
            continue
        pos = int(parts[0], 16) + 0x08000000
        want = int(m.group(1), 16)
        if pos == want:
            prev_ok = parts[2]
            continue
        print(f"{parts[2]}: assembled at 0x{pos:08X}, drift {pos - want:+d} (last good label: {prev_ok})")
        shown += 1
        if shown >= 5:
            break
    if not shown:
        print("no drift")


if __name__ == "__main__":
    main()
