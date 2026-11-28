#!/usr/bin/env python3
"""Violet Eclipse patch smoke test: drives the PC port headlessly and checks a few packages end to end.

  python patch/test/smoke/smoke.py            # run from MMBN4-Decomp/ (or anywhere)
  python patch/test/smoke/smoke.py --list     # list the checks
  python patch/test/smoke/smoke.py -k status  # only checks whose name contains "status"
  python patch/test/smoke/smoke.py --exe PATH # use another mmbn4.exe

Needs (all gitignored, read at run time; nothing from them is written into the repo):
- the patched ROMs `build/bn4_{rs,bm}_overhaul.gba` (`python patch/build.py`) and the PC port
  `build/pc-arm64/Release/mmbn4.exe` or `build/pc-x64/Release/mmbn4.exe` (`python ports/pc/build_pc.py`);
- the player's ROMs `roms/bn4_{rs,bm}_us.gba` (the vanilla side of each comparison);
- the community saves in `ref/saves/*.sps` (`ref/saves/index.md`). A check whose save is missing is skipped.

Every run happens in a temporary folder that is deleted at the end: converted saves, edited saves, EWRAM dumps
and screenshots never reach the repo. Checks:
- shop-table: the new-game shop stock table (BN4_SHOP_STOCK_INIT, read from the ROM files) has HP Memory
  at 40% (207,700Z -> 83,000Z) and the other WP-A3 lines at 40%, and nothing else changed;
- shop-migrate: an existing save loaded in the patched ROM has its HP Memory prices at 40% of vanilla's;
- status-pages: on the MegaMan status screen R/L change the Soul page (vanilla: R does nothing);
- tuner: talking to the Frequency Tuner in Higsby's sets flag 0x0E58 (vanilla: nothing happens); RS and BM;

- mystery-data: the Blue Mystery Data of WP-D2 (Z-Saver in Undernet 1, GP Power in ACDC Area 2): a save placed
  next to each one, walk onto it, open it; its taken flag (0x0E04 / 0x0D0B) is set (vanilla: not); RS and BM.
  It checks the flag only, not the chip that goes into the pack.

The inputs are frame-timed (`--input`), so a check can fail when the ROM boots at a different speed; the
timings have margin. Exit code 0 = all run checks passed (skips are listed), 1 = a check failed.
"""
import argparse
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]            # MMBN4-Decomp/
sys.path.insert(0, str(ROOT / "patch" / "tools"))
import savetool  # noqa: E402

VERSIONS = {"rs": "Red Sun", "bm": "Blue Moon"}
SHOP_STOCK_INIT = {"rs": 0x0804ADA0, "bm": 0x0804ADA8}   # docs/re/shops.md
SHOP_STOCK_SIZE = 0xDC0
SAVE_FILES = {                                           # ref/saves/index.md
    ("rs", "early"): "mega-man-battle-network-4.4440.sps",       # playthrough 1, HP Memories still in the shops
    ("bm", "early"): "mega-man-battle-network-4.5779.sps",       # playthrough 3
    ("rs", "souls"): "mega-man-battle-network-4.22565.sps",      # 11 Souls
    ("bm", "souls"): "mega-man-battle-network-4.4891.sps",       # 9 Souls
}
# From the title screen to the overworld: START, then A through Continue and the load screens.
BOOT = "600=START,606=,700=A,706=,800=A,806=,900=A,906=,1000=A,1006="
BOOT_FRAMES = 1300
HIGSBYS = (0x00, 0x06)                                   # map group, id (wp_b2_frequency_tuner.asm)
# Game-state bytes that put Lan in Higsby's: the map (+4, +5), the real-world mode (+2) and Lan's position
# (+0x38 X, +0x3C Y, 16.16 fixed point, saved with the game state). The Red Sun "souls" save is already
# in ACDC Town at (-22.25, 26), which is a spot inside Higsby's too; the Blue Moon one was saved in the net,
# so its X/Y are net coordinates and the room loads but Lan stands outside it (an "empty room": only the
# background colour and Lan). Setting the same X/Y for both makes the one walk script work in both.
HIGSBYS_STATE = ((2, 0), (4, HIGSBYS[0]), (5, HIGSBYS[1])) + tuple(
    (0x38 + i, b) for i, b in enumerate(struct.pack("<ii", int(-22.25 * 65536), 26 * 65536)))
# Blue Mystery Data added by WP-D2 (patch/src/common/wp_d2_*_md.asm): net map (group, id), the MD's own
# position, and its "taken" event flag.
MYSTERY_DATA = (
    ("Z-Saver, Undernet 1", (0x94, 0x00), (-282, 56), 0x0E04),
    ("GP Power, ACDC Area 2", (0x90, 0x01), (106, 326), 0x0D0B),
)
# A save that was already in the net, so the game-state handler (+0x34) fits a net map: the Red Sun "souls"
# save was in ACDC Town (real world) and a net map edited into it does not work.
NET_SAVE = {"rs": "early", "bm": "souls"}


class Skip(Exception):
    pass


class Smoke:
    def __init__(self, exe, work):
        self.exe, self.work = exe, Path(work)
        self.n = 0

    # ---------- files ----------
    def rom(self, ver, patched):
        p = ROOT / "build" / f"bn4_{ver}_overhaul.gba" if patched else ROOT / "roms" / f"bn4_{ver}_us.gba"
        if not p.exists():
            raise Skip(f"missing {p.relative_to(ROOT)}")
        return p

    def save(self, ver, kind, flags=(), state=()):
        """A temporary .sav made from a community save, optionally with event flags set and game-state bytes
        changed (checksum fixed). Returns the path of a fresh copy for each call."""
        src = ROOT / "ref" / "saves" / SAVE_FILES[(ver, kind)]
        if not src.exists():
            raise Skip(f"missing ref/saves/{src.name}")
        sram, _ = savetool.read_sram(src)
        sram = bytearray(sram[:0x8000])
        if flags or state:
            sram = bytearray(edit_save(bytes(sram), flags, state))
        self.n += 1
        dst = self.work / f"s{self.n}.sav"
        dst.write_bytes(sram)
        return dst

    # ---------- emulator ----------
    def run(self, rom, sav, frames, inp, shot=False):
        """Boots `rom` with `sav`, runs `frames` frames with the input script; returns (EWRAM dump, BMP path)."""
        self.n += 1
        dump = self.work / f"r{self.n}.ram"
        bmp = self.work / f"r{self.n}.bmp"
        cmd = [str(self.exe), str(rom), "--save", str(sav), "--test", str(frames), "--input", inp, "--dump", str(dump)]
        if shot:
            cmd += ["--shot", str(bmp)]
        r = subprocess.run(cmd, capture_output=True, text=True, timeout=300)
        if r.returncode != 0 or "dump=" not in r.stdout:
            raise RuntimeError(f"mmbn4.exe failed ({r.returncode}): {r.stdout[-200:]} {r.stderr[-200:]}")
        return dump.read_bytes(), bmp


# ---------- save editing ----------
def edit_save(sram, flags, state):
    """Set event flags and game-state bytes in a decoded save image, fix the checksum, re-scramble.
    Format: docs/re/save-and-flags.md §1.2 (same code path as savetool.decode)."""
    dec, blk, ver, _ = savetool.decode(sram)
    if not ver:
        raise RuntimeError("save checksum invalid")
    dec = bytearray(dec)
    for f in flags:
        dec[blk + 0x118 + (f >> 3)] |= 0x80 >> (f & 7)
    for off, val in state:
        dec[blk + off] = val
    prog = blk + savetool.PROGRESS
    dec[prog + 0x34:prog + 0x38] = b"\0\0\0\0"
    const = {"Red Sun": 0x16, "Blue Moon": 0x22}[ver]
    dec[prog + 0x34:prog + 0x38] = struct.pack("<I", sum(dec) + const)
    key = sram[savetool.KEY_AT]
    out = bytearray(sram)
    for i in range(savetool.SRAM_USED):
        out[i] = dec[i] ^ key
    out[savetool.KEY_AT:savetool.KEY_AT + 4] = sram[savetool.KEY_AT:savetool.KEY_AT + 4]
    return bytes(out)


# ---------- RAM helpers ----------
def block(ram):
    return 0x2130 + (struct.unpack_from("<I", ram, 0x1550)[0] & 0x1FE)


def flag(ram, f):
    return bool(ram[block(ram) + 0x118 + (f >> 3)] & (0x80 >> (f & 7)))


def hp_memory_prices(ram):
    """(shop, slot, price) of every HP Memory (type 1, item 160) in the live shop stock (save slot +0x58)."""
    st = block(ram) + 0x164C
    out = []
    for shop in range(16):
        for slot in range(8):
            e = ram[st + shop * 0x40 + slot * 8: st + shop * 0x40 + slot * 8 + 8]
            if e[0] == 1 and struct.unpack_from("<H", e, 2)[0] == 160:
                out.append((shop, slot, struct.unpack_from("<H", e, 6)[0]))
    return out


def bmp_region(path, x0, y0, x1, y1):
    """Raw pixel bytes of a rectangle of a 24/32-bit uncompressed BMP (SDL_SaveBMP output)."""
    d = Path(path).read_bytes()
    off, w, h, bpp = struct.unpack_from("<I", d, 10)[0], *struct.unpack_from("<ii", d, 18), struct.unpack_from("<H", d, 28)[0]
    bpp //= 8
    stride = (w * bpp + 3) & ~3
    rows = []
    for y in range(y0, y1):
        ry = (h - 1 - y) if h > 0 else y
        rows.append(d[off + ry * stride + x0 * bpp: off + ry * stride + x1 * bpp])
    return b"".join(rows)


def presses(frame, keys, gap=20):
    """Input script text: tap each key in `keys` (a list) every `gap` frames, starting at `frame`."""
    out = []
    for i, k in enumerate(keys):
        out += [f"{frame + i * gap}={k}", f"{frame + i * gap + 6}="]
    return ",".join(out)


# ---------- the checks ----------
def check_shop_table(s):
    """New-game shop stock: read the table straight from the vanilla and the patched ROM files."""
    for ver in VERSIONS:
        a = s.rom(ver, False).read_bytes()
        b = s.rom(ver, True).read_bytes()
        at = SHOP_STOCK_INIT[ver] - 0x08000000
        va, vb = a[at:at + SHOP_STOCK_SIZE], b[at:at + SHOP_STOCK_SIZE]
        changed = 0
        hp_a = hp_b = 0
        for i in range(0, SHOP_STOCK_SIZE, 8):
            ea, eb = va[i:i + 8], vb[i:i + 8]
            pa, pb = struct.unpack_from("<H", ea, 6)[0], struct.unpack_from("<H", eb, 6)[0]
            if ea[:6] != eb[:6]:
                raise AssertionError(f"{ver}: stock entry {i // 8} changed in more than its price")
            if ea[0] == 1 and struct.unpack_from("<H", ea, 2)[0] == 160:
                hp_a += pa
                hp_b += pb
            if pa != pb:
                changed += 1
                # 40%, rounded the way the patch rounds (exact for these prices, within 1 otherwise)
                if abs(pb * 5 - pa * 2) > 5:
                    raise AssertionError(f"{ver}: entry {i // 8} price {pa} -> {pb} is not 40%")
        # 14 lines at 40%, each rounded to 100Z: the total is 83,000Z give or take the rounding
        if hp_a != 2077 or abs(hp_b - 830) > 7:
            raise AssertionError(f"{ver}: HP Memory total {hp_a * 100:,}Z -> {hp_b * 100:,}Z, expected 207,700Z -> about 83,000Z")
        # 14 HP Memories + 12 Zenny programs + 4 Unlockers + 4 LocEnemies = 34 slots (patch/README.md, WP-A3)
        if changed != 34:
            raise AssertionError(f"{ver}: {changed} prices changed, expected 34")
    return f"HP Memory 207,700Z -> {hp_b * 100:,}Z (about 40%) and {changed} of 34 WP-A3 prices at 40% (RS, BM)"


def check_shop_migrate(s):
    """An existing save loaded in the patched ROM gets its shop prices synced (SaveLoad hook, WP-A3)."""
    msg = []
    for ver in VERSIONS:
        van, _ = s.run(s.rom(ver, False), s.save(ver, "early"), BOOT_FRAMES, BOOT)
        pat, _ = s.run(s.rom(ver, True), s.save(ver, "early"), BOOT_FRAMES, BOOT)
        a, b = hp_memory_prices(van), hp_memory_prices(pat)
        if not a:
            raise AssertionError(f"{ver}: the save has no HP Memory in its shops (pick another save)")
        if [(x[0], x[1]) for x in a] != [(x[0], x[1]) for x in b]:
            raise AssertionError(f"{ver}: shop slots differ: {a} vs {b}")
        for (sh, sl, pa), (_, _, pb) in zip(a, b):
            if pb * 5 != pa * 2:
                raise AssertionError(f"{ver}: shop {sh} slot {sl}: {pa} -> {pb}, expected 40%")
        msg.append(f"{ver.upper()} {len(a)} HP Memory lines {sum(x[2] for x in a) * 100:,}Z -> {sum(x[2] for x in b) * 100:,}Z")
    return "; ".join(msg)


STATUS_MENU = ",1300=START,1306=,1360=DOWN,1366=,1380=DOWN,1386=,1400=DOWN,1406=,1430=A,1436="


def status_icons(s, ver, rom_patched, sav, keys):
    """Open Menu > MegaMan and press `keys` (list); returns the pixels of the Soul icon columns."""
    t = 1640
    ev = []
    for k in keys:
        ev += [f"{t}={k}", f"{t + 6}="]
        t += 40
    _, bmp = s.run(s.rom(ver, rom_patched), sav, t + 120, BOOT + STATUS_MENU + ",".join([""] + ev), shot=True)
    # the six icons are in two columns left and right of MegaMan's sprite
    return bmp_region(bmp, 8, 30, 44, 110) + bmp_region(bmp, 76, 30, 100, 110)


def check_status_pages(s):
    """The status screen shows the Souls in pages of 6: R next, L back, a third page after the second."""
    msg = []
    for ver in VERSIONS:
        sav = s.save(ver, "souls")
        van_0 = status_icons(s, ver, False, sav, [])
        van_r = status_icons(s, ver, False, sav, ["R"])
        if van_0 != van_r:
            raise AssertionError(f"{ver}: vanilla changed on R (the check is not valid)")
        p1 = status_icons(s, ver, True, sav, [])
        p2 = status_icons(s, ver, True, sav, ["R"])
        p3 = status_icons(s, ver, True, sav, ["R", "R"])
        back = status_icons(s, ver, True, sav, ["R", "L"])
        wrap = status_icons(s, ver, True, sav, ["R", "R", "R"])
        if p2 == p1:
            raise AssertionError(f"{ver}: R did not change the page")
        if p3 in (p1, p2):
            raise AssertionError(f"{ver}: the third page looks like page 1 or 2")
        if back != p1:
            raise AssertionError(f"{ver}: R then L did not return to page 1")
        if wrap != p1:
            raise AssertionError(f"{ver}: three R presses did not wrap to page 1")
        msg.append(f"{ver.upper()} 3 pages, L returns, wraps")
    return "; ".join(msg)


def check_tuner(s):
    """Talk to the Frequency Tuner NPC in Higsby's: Yes sets flag 0x0E58 (Auto -> Tier 1)."""
    msg = []
    walk = ",1200=LEFT,1500=DOWN,1700="                       # to the Tuner (the room's walls stop the player)
    talk = "," + presses(1750, ["A", "A", "A", "A"])
    for ver in VERSIONS:
        sav = s.save(ver, "souls", state=HIGSBYS_STATE)
        pat, _ = s.run(s.rom(ver, True), sav, 2000, BOOT + walk + talk)
        van, _ = s.run(s.rom(ver, False), sav, 2000, BOOT + walk + talk)
        idle, _ = s.run(s.rom(ver, True), sav, 2000, BOOT)
        if flag(idle, 0x0E58) or flag(idle, 0x0E59):
            raise AssertionError(f"{ver}: Tuner flags already set before the talk")
        if flag(van, 0x0E58) or flag(van, 0x0E59):
            raise AssertionError(f"{ver}: vanilla ROM set a Tuner flag")
        if not flag(pat, 0x0E58) or flag(pat, 0x0E59):
            raise AssertionError(f"{ver}: expected Tier 1 (0x0E58 set, 0x0E59 clear), got "
                                 f"{flag(pat, 0x0E58)}/{flag(pat, 0x0E59)}")
        msg.append(f"{ver.upper()} Auto -> Tier 1")
    return "; ".join(msg)


def check_mystery_data(s):
    """The two Blue Mystery Data (Z-Saver in Undernet 1, GP Power in ACDC Area 2): walk to each and open it."""
    msg = []
    for ver in VERSIONS:
        for name, (grp, mid), (x, y), fl in MYSTERY_DATA:
            # a save moved next to the Mystery Data (map, X, Y in 16.16 fixed point; 30 units left of it)
            state = ((4, grp), (5, mid)) + tuple(
                (0x38 + i, b) for i, b in enumerate(struct.pack("<ii", int((x - 30) * 65536), int(y * 65536))))
            sav = s.save(ver, NET_SAVE[ver], state=state)
            # walk right onto the data (frames 1300-1342), then press A (opens it, text boxes) 8 times
            walk = ",1300=RIGHT,1342=," + presses(1360, ["A"] * 8, gap=25)
            idle, _ = s.run(s.rom(ver, True), sav, 1700, BOOT)
            pat, _ = s.run(s.rom(ver, True), sav, 1700, BOOT + walk)
            van, _ = s.run(s.rom(ver, False), sav, 1700, BOOT + walk)
            if flag(idle, fl):
                raise AssertionError(f"{ver}: {name}: flag 0x{fl:04X} already set before the pickup")
            if flag(van, fl):
                raise AssertionError(f"{ver}: {name}: the vanilla ROM set flag 0x{fl:04X}")
            if not flag(pat, fl):
                raise AssertionError(f"{ver}: {name}: flag 0x{fl:04X} not set, the Mystery Data was not picked up")
        msg.append(f"{ver.upper()} Z-Saver and GP Power picked up (taken flags set)")
    return "; ".join(msg)


CHECKS = [
    ("shop-table", check_shop_table),
    ("shop-migrate", check_shop_migrate),
    ("status-pages", check_status_pages),
    ("tuner", check_tuner),
    ("mystery-data", check_mystery_data),
]


def find_exe():
    for a in ("arm64", "x64"):
        p = ROOT / "build" / f"pc-{a}" / "Release" / "mmbn4.exe"
        if p.exists():
            return p
    return None


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
    ap.add_argument("-k", help="only checks whose name contains this")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--exe")
    a = ap.parse_args()
    if a.list:
        for n, f in CHECKS:
            print(f"{n}: {(f.__doc__ or '').strip().splitlines()[0]}")
        return 0
    exe = Path(a.exe) if a.exe else find_exe()
    if not exe or not exe.exists():
        print("mmbn4.exe not found: run python ports/pc/build_pc.py (or pass --exe)")
        return 1
    work = tempfile.mkdtemp(prefix="ve-smoke-")
    s = Smoke(exe, work)
    fails = 0
    try:
        for name, fn in CHECKS:
            if a.k and a.k not in name:
                continue
            try:
                print(f"PASS {name}: {fn(s)}")
            except Skip as e:
                print(f"SKIP {name}: {e}")
            except Exception as e:  # noqa: BLE001
                fails += 1
                print(f"FAIL {name}: {e}")
    finally:
        shutil.rmtree(work, ignore_errors=True)
    print("smoke test:", "FAILED" if fails else "ok")
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
