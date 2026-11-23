#!/usr/bin/env python3
"""inventory.py: the Violet Eclipse chip inventory (plan/DEFINITIVE-EDITION.md §2, criterion C2).

Reads both US ROMs' chip tables (0x080197EC, 0x2C bytes a record, 389 records) and, when present, local third-party
references, then writes build/inventory/chips.csv: one row per chip with its class, codes per version, and every source
the references list per code, classified as in-game RS / in-game BM / link-or-event only / unknown.

  python patch/tools/inventory.py            # writes build/inventory/chips.csv, prints a summary of counts

Inputs:
  roms/bn4_rs_us.gba, roms/bn4_bm_us.gba          required, SHA-1 checked (see roms/README.md)
  ext/mmbn-chip-tables/bn4r_chip_table.asm         optional: chip names (comments ';XXXX - Name')
  ref/guides/bn4-battle-chip-guide-dx_xetrov.txt   optional: per-code sources ([CHLOC] section)
  ref/maps/legend.csv                              optional: Mystery Data, shops and traders by area

The CSV contains game text (chip names) and third-party facts, so it stays in build/ (gitignored). Only this script and
the counts in docs/completion/README.md are tracked."""
import csv
import hashlib
import re
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
ROMS = {
    "rs": (ROOT / "roms" / "bn4_rs_us.gba", "a97e96a7da03abd70f7953328e511b9fa29179f1"),
    "bm": (ROOT / "roms" / "bn4_bm_us.gba", "5e017c803ddc768efb8010314b46bc17e9757f71"),
}
CHIP_TABLE, CHIP_SIZE, CHIP_COUNT = 0x197EC, 0x2C, 389
CODES = "ABCDEFGHIJKLMNOPQRSTUVWXYZ*"
LIBRARY = {0: "standard", 1: "mega", 2: "giga", 3: "none", 4: "pa"}
FLAG_DARK, FLAG_NAVI, FLAG_IN_LIBRARY = 0x20, 0x04, 0x40
SOUL_CHIPS = range(0x161, 0x16D)
# Which version fights each Soul Navi (BN4_SOUL_FLAGS: Souls 1-6 Red Sun, 7-12 Blue Moon); their Navi chips follow.
NAVI_VERSION = {"RS": ("ROLL", "GUTS", "WIND", "SERCH", "SRCH", "FIRE", "THUN"),
                "BM": ("PROTO", "NUMB", "METL", "METAL", "JUNK", "AQUA", "WOOD")}        # Soul n = chip 0x160 + n (docs/re/completion-and-new-game-plus.md §2)

# Source tokens in the guide's [CHLOC] lines -> kind of source.
IN_GAME = ("BMD", "GMD", "PMD", "BGMD", "ND", "BFD", "HCT", "HSCT", "BFT", "CO", "FLDR", "SHOP", "MD")


def load_rom(key):
    path, want = ROMS[key]
    if not path.exists():
        sys.exit(f"missing {path.relative_to(ROOT)} (see roms/README.md)")
    data = path.read_bytes()
    got = hashlib.sha1(data).hexdigest()
    if got != want:
        sys.exit(f"{path.name}: SHA-1 {got}, expected {want}")
    return data


def chip_records(rom):
    out = []
    for i in range(CHIP_COUNT):
        r = rom[CHIP_TABLE + i * CHIP_SIZE:][:CHIP_SIZE]
        codes = "".join(CODES[c] for c in r[0:4] if c < len(CODES))
        out.append({
            "codes": codes, "library": LIBRARY.get(r[8], str(r[8])), "flags": r[9],
            "number": struct.unpack_from("<H", r, 0x1C)[0], "damage": struct.unpack_from("<H", r, 0x1A)[0],
        })
    return out


def chip_names():
    p = ROOT / "ext" / "mmbn-chip-tables" / "bn4r_chip_table.asm"
    names = {}
    if p.exists():
        for m in re.finditer(r"^;([0-9A-F]{4}) - (.*)$", p.read_text(encoding="utf-8", errors="replace"), re.M):
            names[int(m.group(1), 16)] = m.group(2).strip()
    return names


def guide_sources():
    """[{'cls', 'number', 'name', 'tag', 'codes': {code: text}}] from the guide's [CHLOC] section."""
    p = ROOT / "ref" / "guides" / "bn4-battle-chip-guide-dx_xetrov.txt"
    if not p.exists():
        return []
    lines = p.read_text(encoding="utf-8", errors="replace").splitlines()
    start = next(i for i, l in enumerate(lines) if l.strip() == "Keyword: [CHLOC]")
    section = {"STANDARD": "standard", "MEGA": "mega", "GIGA": "giga", "SECRET": "secret", "DARK": "dark"}
    cls, entry, codes, out = None, None, [], []
    for l in lines[start + 1:]:
        if l.startswith("Keyword: ["):
            break
        m = re.match(r"~~~ (\w+) CHIPS ~~~", l.strip())
        if m:
            cls = section.get(m.group(1)); entry = None; continue
        m = re.match(r"^(\((RS|BM)\) )?0*(\d+): (.+?)\s*$", l)
        if m and cls:
            entry = {"cls": cls, "number": int(m.group(3)), "name": m.group(4), "tag": m.group(2) or "", "codes": {}}
            out.append(entry); codes = []; continue
        m = re.match(r"^((?:[A-Z*](?:, ?)?)*)- (.*)$", l)
        if m and entry is not None:
            codes = re.findall(r"[A-Z*]", m.group(1)) or ["any"]
            for c in codes:
                entry["codes"][c] = m.group(2).strip()
            continue
        if l.strip() and codes and entry is not None and not l.startswith("~"):
            for c in codes:
                entry["codes"][c] += " " + l.strip()     # wrapped source line
        if not l.strip():
            codes = []
    return out


def map_sources():
    p = ROOT / "ref" / "maps" / "legend.csv"
    out = defaultdict(list)
    if p.exists():
        for row in csv.DictReader(p.open(encoding="utf-8")):
            out[row["item"].upper().replace(" ", "")].append(f"{row['source']} {row['area']}{' ' + row['code_or_amount'] if row['code_or_amount'] else ''}")
    return out


def norm(name):
    return re.sub(r"[^A-Z0-9+]", "", name.upper())


ALIASES = {"PRIXPOWR": "GRANDPRIXPOWER"}   # ROM short name -> the guide's name
LINK_WORDS = ("PROMOTION", "JPN ONLY", "NAVI SOUL DATA", "LINK", "TRADE", "ZERO 3", "ZERO3", "BOKTAI", "EVENT", "E-READER", "DISTRIBUT",
              "OPPOSING", "OTHER VERSION", "OTHER CART")


def classify(text):
    """One code's guide text -> (in_rs, in_bm, link). A '(RS)' or '(BM)' tag limits the source it is attached to.
    Any source is taken as in-game unless it names a link, trade or event (LINK_WORDS)."""
    if not text:
        return False, False, False
    if "NAVI SOUL DATA" in text.upper():          # Free Tournament secret chips: need the other cart's Soul data
        return False, False, True
    rs = bm = link = False
    for part in re.split(r",\s*(?![^()]*\))", text):
        up = part.upper()
        if any(w in up for w in LINK_WORDS):
            link = True
            continue
        if not part.replace("(RS)", "").replace("(BM)", "").strip():
            continue
        tag_rs, tag_bm = "(RS)" in up or " RS)" in up, "(BM)" in up or " BM)" in up
        if tag_rs and not tag_bm:
            rs = True
        elif tag_bm and not tag_rs:
            bm = True
        else:
            rs = bm = True
    return rs, bm, link


def main():
    rs, bm = chip_records(load_rom("rs")), chip_records(load_rom("bm"))
    names, guide, maps = chip_names(), guide_sources(), map_sources()
    out_dir = ROOT / "build" / "inventory"
    out_dir.mkdir(parents=True, exist_ok=True)
    rows, summary = [], Counter()
    for i in range(CHIP_COUNT):
        a, b = rs[i], bm[i]
        lib = a["library"]
        if i in SOUL_CHIPS:
            kind = "soul"
        elif lib == "pa":
            kind = "pa"
        elif not (a["flags"] & FLAG_IN_LIBRARY) and not (a["flags"] & FLAG_DARK):
            kind = "internal"
        elif a["flags"] & FLAG_DARK:
            kind = "dark"
        elif lib == "none":
            kind = "none"
        else:
            kind = lib
        # Match the guide by name first (its numbers repeat: RS's and BM's Navi chips share 19-36), then by number:
        # the record's +0x1C number is global (standard 1-150, 151+ secret, mega 200+, giga 280+, PA 300+).
        key = ALIASES.get(norm(names.get(i, "")), norm(names.get(i, "")))
        g = next((e for e in guide if norm(e["name"]) == key), None)
        base = {"standard": 0, "mega": 200, "giga": 280}.get(lib)
        if g is None and base is not None and not (lib == "standard" and a["number"] > 150):
            g = next((e for e in guide if e["cls"] == lib and e["number"] == a["number"] - base), None)
        if kind == "internal" and g is not None:
            kind = lib if lib in ("standard", "mega", "giga") else g["cls"]
        navi_version = next((v for v, ns in NAVI_VERSION.items() if any(key.startswith(n) for n in ns)), None)
        per_code, any_rs, any_bm, any_link, missing = [], False, False, False, []
        for code in sorted(set(a["codes"]) | set(b["codes"])):
            gc = (g or {}).get("codes", {})
            text = gc.get(code) or gc.get("any", "")
            crs, cbm, clink = classify(text)
            any_link |= clink
            tag = (g or {}).get("tag") or (navi_version if a["flags"] & FLAG_NAVI else None)
            if tag == "RS":
                cbm = False
            elif tag == "BM":
                crs = False
            any_rs |= crs; any_bm |= cbm
            if not (crs or cbm):
                missing.append(code)
            per_code.append(f"{code}: {text}" if text else f"{code}: ?")
        if kind == "soul":
            status = "soul (story; flag 0x17+n)"
        elif kind == "pa":
            status = "combination (no source needed)"
        elif kind == "internal":
            status = "internal record (not in the Library)"
        elif kind == "none" and not g:
            status = "not a library chip"
        elif not g:
            status = "unknown (no reference entry)"
        elif any_rs and any_bm and not missing:
            status = "in-game both"
        elif any_rs and not any_bm:
            status = "in-game RS only"
        elif any_bm and not any_rs:
            status = "in-game BM only"
        elif any_rs or any_bm:
            status = "partial: codes " + "".join(missing) + " have no listed source"
        elif any_link:
            status = "no in-game source (link/trade)"
        else:
            status = "no in-game source (none listed)"
        summary[(kind, status.split(":")[0])] += 1
        rows.append({
            "id": f"0x{i:03X}", "name": names.get(i, ""), "kind": kind, "library_no": a["number"],
            "codes_rs": a["codes"], "codes_bm": b["codes"], "same_record": a == b,
            "navi_chip": bool(a["flags"] & FLAG_NAVI), "in_library": bool(a["flags"] & FLAG_IN_LIBRARY),
            "status": status, "guide_sources": " | ".join(per_code),
            "map_sources": "; ".join(maps.get(names.get(i, "").upper().replace(" ", ""), [])),
        })
    path = out_dir / "chips.csv"
    with path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(f, fieldnames=list(rows[0]))
        w.writeheader(); w.writerows(rows)
    print(f"wrote {path.relative_to(ROOT)}: {len(rows)} chips"
          f" (names: {'yes' if names else 'no'}, guide: {'yes' if guide else 'no'}, maps: {'yes' if maps else 'no'})")
    for (kind, status), n in sorted(summary.items()):
        print(f"  {kind:9s} {status:40s} {n:4d}")
    diff = [r["id"] for r in rows if not r["same_record"]]
    print(f"  records that differ between RS and BM: {len(diff)}")


if __name__ == "__main__":
    main()
