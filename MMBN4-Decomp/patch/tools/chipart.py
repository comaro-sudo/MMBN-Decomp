#!/usr/bin/env python3
"""Convert chip art (56 x 48 PNG) into BN4's chip-picture format, and make a matching 16 x 16 icon.

    python patch/tools/chipart.py <rom.gba> <icon_pal_addr> <out_dir> <name>=<png> [<name>=<png> ...]

For each name, writes <out_dir>/<name>_image.bin (7 x 6 tiles, 4bpp, 0x540 bytes), <name>_pal.bin
(16 colours, BGR555, 0x20 bytes) and <name>_icon.bin (2 x 2 tiles, 4bpp, 0x80 bytes).

Formats (verified by decoding Z-Saver's record from the US ROM, docs/completion/chips.md section 3):
- picture: uncompressed 4bpp tiles in row order, its own 16-colour palette (low nibble = left pixel);
- icon: 4bpp, the shared chip-icon palette BN4_CHIP_ICON_PAL (RS 0x08747FF8 / BM 0x08747AEC).
The icon keeps the standard frame of an existing icon (Z-Saver's, read from the player's ROM) and fills its
12 x 12 inside with the picture scaled down and mapped to the icon palette. No ROM bytes are stored in the
repo: the frame and the palette are read from the player's ROM at build time.

The source PNGs are kept local (ref/chip-art/, gitignored): they are not in any ROM. See the distribution
caveat in docs/completion/chips.md.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

from PIL import Image

CHIP_TABLE = 0x197EC          # file offset of the chip table (same in RS and BM)
CHIP_ENTRY = 0x2C
FRAME_CHIP = 0x0DA            # Z-Saver: a Mega chip whose icon frame is reused


def bgr555(rgb: tuple) -> int:
    r, g, b = rgb[:3]
    return (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10)


def from555(c: int) -> tuple:
    return ((c & 31) << 3, ((c >> 5) & 31) << 3, ((c >> 10) & 31) << 3)


def to_tiles(idx: list, w: int, h: int) -> bytes:
    """idx: row-major palette indices, w x h pixels (multiples of 8) -> 4bpp tiles in row order."""
    out = bytearray()
    for ty in range(h // 8):
        for tx in range(w // 8):
            for y in range(8):
                for x in range(0, 8, 2):
                    a = idx[(ty * 8 + y) * w + tx * 8 + x]
                    b = idx[(ty * 8 + y) * w + tx * 8 + x + 1]
                    out.append((a & 15) | ((b & 15) << 4))
    return bytes(out)


def from_tiles(data: bytes, w: int, h: int) -> list:
    idx = [0] * (w * h)
    t = 0
    for ty in range(h // 8):
        for tx in range(w // 8):
            for y in range(8):
                for x in range(8):
                    b = data[t * 32 + y * 4 + x // 2]
                    idx[(ty * 8 + y) * w + tx * 8 + x] = (b >> 4) if x & 1 else (b & 15)
            t += 1
    return idx


def nearest(pal: list, rgb: tuple, skip0: bool = True) -> int:
    best, bi = None, 0
    for i, p in enumerate(pal):
        if skip0 and i == 0:
            continue
        d = sum((a - b) ** 2 for a, b in zip(p, rgb[:3]))
        if best is None or d < best:
            best, bi = d, i
    return bi


def convert(rom: bytes, icon_pal_off: int, png: Path) -> tuple:
    img = Image.open(png).convert("RGB")
    if img.size != (56, 48):
        sys.exit(f"{png}: expected 56 x 48, got {img.size[0]} x {img.size[1]}")
    # Picture: 16 colours of its own.
    q = img.quantize(colors=16, method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
    qpal = (q.getpalette() or [])[:48]
    qpal += [0] * (48 - len(qpal))          # fewer than 16 colours: pad with black
    pal = [tuple(qpal[i * 3:i * 3 + 3]) for i in range(16)]
    image_bin = to_tiles([q.getpixel((x, y)) for y in range(48) for x in range(56)], 56, 48)
    pal_bin = struct.pack("<16H", *(bgr555(p) for p in pal))
    # Icon: Z-Saver's frame, the picture's centre scaled to 12 x 12 inside it, mapped to the icon palette.
    ipal = [from555(c) for c in struct.unpack_from("<16H", rom, icon_pal_off)]
    icon_ptr = struct.unpack_from("<I", rom, CHIP_TABLE + FRAME_CHIP * CHIP_ENTRY + 0x20)[0] - 0x08000000
    frame = from_tiles(rom[icon_ptr:icon_ptr + 0x80], 16, 16)
    inner = img.crop((8, 0, 56 - 8, 48)).resize((12, 12), Image.LANCZOS)
    icon = list(frame)
    for y in range(12):
        for x in range(12):
            icon[(y + 2) * 16 + x + 2] = nearest(ipal, inner.getpixel((x, y)))
    return image_bin, pal_bin, to_tiles(icon, 16, 16)


def main() -> None:
    if len(sys.argv) < 5:
        sys.exit(__doc__)
    rom = Path(sys.argv[1]).read_bytes()
    icon_pal_off = int(sys.argv[2], 16) - 0x08000000
    out = Path(sys.argv[3])
    out.mkdir(parents=True, exist_ok=True)
    for spec in sys.argv[4:]:
        name, png = spec.split("=", 1)
        image_bin, pal_bin, icon_bin = convert(rom, icon_pal_off, Path(png))
        (out / f"{name}_image.bin").write_bytes(image_bin)
        (out / f"{name}_pal.bin").write_bytes(pal_bin)
        (out / f"{name}_icon.bin").write_bytes(icon_bin)
        print(f"   chip art: {name} -> picture 0x{len(image_bin):X}, palette 0x{len(pal_bin):X}, icon 0x{len(icon_bin):X}")


if __name__ == "__main__":
    main()
