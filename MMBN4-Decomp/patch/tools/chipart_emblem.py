#!/usr/bin/env python3
"""PROTOTYPE (not used by build.py): original placeholder chip art for Duo and Grand Prix Power.

    python patch/tools/chipart_emblem.py <rom.gba> <icon_pal_addr> <out_dir>      # same args as chipart.py minus the PNGs
    python patch/tools/chipart_emblem.py --preview <out_dir>                      # PNGs only, no ROM needed

Draws two simple geometric emblems from code (no Capcom art, nothing read from a ROM except the standard icon
frame/palette that chipart.py already uses for icons) and writes, per name (duo, gpp), the same files as
chipart.py: <name>_image.bin (56 x 48, 7 x 6 tiles 4bpp, 0x540), <name>_pal.bin (16 x BGR555, 0x20) and
<name>_icon.bin (16 x 16, 0x80). Palette index 0 is the background (transparent in BN4 chip pictures).
Docs: docs/pending/public-chip-art.md.
"""
from __future__ import annotations

import math
import struct
import sys
from pathlib import Path

from PIL import Image, ImageDraw

sys.path.insert(0, str(Path(__file__).parent))
import chipart as ca  # noqa: E402

W, H = 56, 48
# 16-colour palette shared by both emblems: 0 background, 1 outline, 2-4 gold ramp, 5-7 blue ramp, 8-10 red ramp, 11 white
PAL = [(0, 0, 0), (24, 24, 56), (120, 80, 0), (216, 160, 24), (255, 224, 96),
       (24, 72, 160), (64, 128, 232), (160, 200, 255), (136, 24, 24), (224, 64, 48), (255, 144, 112),
       (255, 255, 255), (96, 96, 112), (160, 160, 176), (0, 0, 0), (0, 0, 0)]


def star(cx, cy, r_out, r_in, rot=-math.pi / 2):
    pts = []
    for i in range(10):
        r = r_out if i % 2 == 0 else r_in
        a = rot + i * math.pi / 5
        pts.append((cx + r * math.cos(a), cy + r * math.sin(a)))
    return pts


def duo() -> Image.Image:
    """Twin interlocked rings (blue + red) around a gold diamond: 'duo'."""
    im = Image.new("P", (W, H), 0)
    d = ImageDraw.Draw(im)
    for box, outer, inner in (((6, 9, 34, 37), 5, 6), ((22, 9, 50, 37), 8, 9)):
        d.ellipse(box, fill=1)
        d.ellipse((box[0] + 2, box[1] + 2, box[2] - 2, box[3] - 2), fill=outer)
        d.ellipse((box[0] + 5, box[1] + 5, box[2] - 5, box[3] - 5), fill=1)
        d.ellipse((box[0] + 7, box[1] + 7, box[2] - 7, box[3] - 7), fill=0)
    # re-cross the rings so they look interlocked
    d.arc((6, 9, 34, 37), -50, 50, fill=6, width=3)
    d.polygon([(28, 10), (40, 23), (28, 36), (16, 23)], fill=1)
    d.polygon([(28, 13), (37, 23), (28, 33), (19, 23)], fill=3)
    d.polygon([(28, 13), (37, 23), (28, 23)], fill=4)
    d.polygon([(28, 23), (37, 23), (28, 33)], fill=2)
    d.rectangle((14, 41, 41, 43), fill=1)
    d.rectangle((15, 42, 40, 42), fill=3)
    return im


def gpp() -> Image.Image:
    """Gold star on a red-and-blue roundel: 'grand prix'."""
    im = Image.new("P", (W, H), 0)
    d = ImageDraw.Draw(im)
    d.ellipse((8, 2, 48, 42), fill=1)
    d.ellipse((10, 4, 46, 40), fill=9)
    d.ellipse((13, 7, 43, 37), fill=1)
    d.ellipse((15, 9, 41, 35), fill=5)
    d.pieslice((15, 9, 41, 35), 200, 340, fill=6)
    d.polygon(star(28, 22, 13, 5.5), fill=1)
    d.polygon(star(28, 22, 11, 4.2), fill=3)
    d.polygon(star(28, 22, 5, 2.2, rot=-math.pi / 2 + math.pi / 5), fill=4)
    d.polygon([(14, 38), (20, 46), (24, 40)], fill=8)
    d.polygon([(42, 38), (36, 46), (32, 40)], fill=8)
    d.rectangle((20, 40, 35, 44), fill=1)
    d.rectangle((21, 41, 34, 43), fill=3)
    return im


def to_files(im: Image.Image, rom: bytes | None, icon_pal_off: int | None) -> tuple:
    idx = list(im.tobytes())
    image_bin = ca.to_tiles(idx, W, H)
    pal_bin = struct.pack("<16H", *(ca.bgr555(p) for p in PAL))
    icon_bin = b""
    if rom is not None:
        ipal = [ca.from555(c) for c in struct.unpack_from("<16H", rom, icon_pal_off)]
        ptr = struct.unpack_from("<I", rom, ca.CHIP_TABLE + ca.FRAME_CHIP * ca.CHIP_ENTRY + 0x20)[0] - 0x08000000
        icon = ca.from_tiles(rom[ptr:ptr + 0x80], 16, 16)
        rgb = Image.new("RGB", (W, H), (0, 0, 0))
        rgb.putdata([PAL[i] for i in idx])
        inner = rgb.crop((8, 0, 48, 48)).resize((12, 12), Image.LANCZOS)
        for y in range(12):
            for x in range(12):
                icon[(y + 2) * 16 + x + 2] = ca.nearest(ipal, inner.getpixel((x, y)))
        icon_bin = ca.to_tiles(icon, 16, 16)
    return image_bin, pal_bin, icon_bin


def main() -> None:
    args = sys.argv[1:]
    preview = bool(args) and args[0] == "--preview"
    if (preview and len(args) != 2) or (not preview and len(args) != 3):
        sys.exit(__doc__)
    out = Path(args[-1])
    out.mkdir(parents=True, exist_ok=True)
    rom = None if preview else Path(args[0]).read_bytes()
    off = None if preview else int(args[1], 16) - 0x08000000
    for name, im in (("duo", duo()), ("gpp", gpp())):
        image_bin, pal_bin, icon_bin = to_files(im, rom, off)
        if preview:
            flat = Image.new("RGB", (W, H), (0, 128, 128))   # teal stands in for the transparent index 0
            flat.putdata([PAL[i] if i else (0, 128, 128) for i in im.tobytes()])
            flat.resize((W * 6, H * 6), Image.NEAREST).save(out / f"{name}_emblem.png")
            continue
        (out / f"{name}_image.bin").write_bytes(image_bin)
        (out / f"{name}_pal.bin").write_bytes(pal_bin)
        (out / f"{name}_icon.bin").write_bytes(icon_bin)
        print(f"   emblem: {name} -> picture 0x{len(image_bin):X}, palette 0x{len(pal_bin):X}, icon 0x{len(icon_bin):X}")


if __name__ == "__main__":
    main()
