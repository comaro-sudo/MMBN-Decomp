#!/usr/bin/env python3
"""Generate original, copyright-free Violet Eclipse WUHB icon and splash PNGs.
Uses only Python stdlib (struct + zlib).
"""
from __future__ import annotations

import math
import struct
import zlib
from pathlib import Path

OUT = Path(__file__).resolve().parent / "assets"


def write_png(path: Path, w: int, h: int, rgb_bytes: bytes) -> None:
    def chunk(tag: bytes, data: bytes) -> bytes:
        c = tag + data
        return struct.pack(">I", len(data)) + c + struct.pack(">I", zlib.crc32(c) & 0xFFFFFFFF)

    raw = bytearray()
    stride = w * 3
    for y in range(h):
        raw.append(0)
        raw.extend(rgb_bytes[y * stride:(y + 1) * stride])

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b"")
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(png)


def render_eclipse(w: int, h: int) -> bytes:
    buf = bytearray(w * h * 3)
    cx, cy = w * 0.5, h * 0.5
    r_outer = min(w, h) * 0.30
    r_inner = r_outer * 0.78
    grid = max(16, min(w, h) // 12)

    for y in range(h):
        gy = (y % grid) == 0
        for x in range(w):
            # Deep cyber-indigo background + subtle grid
            r, g, b = 10, 18, 42
            if gy or (x % grid) == 0:
                r, g, b = 18, 42, 80

            dx, dy = x - cx, y - cy
            dist = math.hypot(dx, dy)
            # Violet corona ring + cyan accent
            if r_inner <= dist <= r_outer * 1.15:
                t = 1.0 - abs(dist - r_outer) / (r_outer * 0.35)
                if t > 0:
                    r = min(255, int(r + 165 * t))
                    g = min(255, int(g + 70 * t))
                    b = min(255, int(b + 245 * t))
            elif dist < r_inner:
                r, g, b = 12, 10, 28
                # Inner cyan diamond emblem
                if abs(dx) + abs(dy) < r_inner * 0.42:
                    r, g, b = 0, 210, 255

            # Outer border
            if x < 3 or x >= w - 3 or y < 3 or y >= h - 3:
                r, g, b = 0, 200, 255

            idx = (y * w + x) * 3
            buf[idx] = r
            buf[idx + 1] = g
            buf[idx + 2] = b
    return bytes(buf)


def main() -> None:
    write_png(OUT / "icon.png", 128, 128, render_eclipse(128, 128))
    write_png(OUT / "tv_splash.png", 1280, 720, render_eclipse(1280, 720))
    write_png(OUT / "drc_splash.png", 854, 480, render_eclipse(854, 480))
    print(f"generated icon.png, tv_splash.png, drc_splash.png in {OUT}")


if __name__ == "__main__":
    main()
