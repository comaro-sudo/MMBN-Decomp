# Chip Names & Descriptions: Text Pipeline Design (Round 10, Task 7, 2026-11-23)

Exact TextPet archive locations, empirical single-script test results, build-time merge recipe, and exact `.tpl` source blocks (our lines only) for **Duo (`0x137`)**, **Grand Prix Power (`0x138`)**, and **DuoSoul (`0x16D`)**. Verified against both `bn4_rs_us.gba` and `bn4_bm_us.gba` and tested live with `tools/textpet/TextPet.exe` on a scratch copy of `bn4_rs_us.gba` outside the repo.

---

## 1. Archive Locations & TextPet Index Entries (V, R)

All three chip-text archives are **already present** in TextPet's shipped US indexes (`tools/textpet/indexes/mmbn4rs-us.tpi` and `mmbn4bm-us.tpi`), with every ROM pointer listed:

| Archive | Purpose | RS Offset : Size | BM Offset : Size | `@size` (scripts) | Index Pointers (RS / BM) |
|---|---|---|---|---:|---|
| **Chip Names Bank 0** | Chips `0x000`–`0x0FF` | `0x7481D8:0xA3A` | `0x747CCC:0xA3A` | 256 | 8 pointers (in `.tpi`) |
| **Chip Names Bank 1** | Chips `0x100`–`0x184` | **`0x748C14:0x4CC`** | **`0x748708:0x4CC`** | **133** | RS: `0x01FDA0,0x021D14,0x02F1B8,0x02F3E4,0x03813C,0x03CFAC,0x04A7C8,0x04FB78`<br>BM: `0x01FDA0,0x021D18,0x02F1BC,0x02F3E8,0x038144,0x03CFB4,0x04A7D0,0x04FB84` |
| **Chip Descriptions Bank 1** | Chips `0x100`–`0x170` | **`0x74F03C:0xBEC`** | **`0x74EACC:0xBEC`** | **113** | RS: `0x01FDE4,0x106C64,0x106F4C,0x109A48,0x109E34`<br>BM: `0x01FDE4,0x106C70,0x106F58,0x109A54,0x109E40` |

### Current entries in both US ROMs (verified by TextPet extraction)
- **`0x137` (`Duo`, Bank 1 script `55`):**
  - Name (`748C14` / `748708`, `script 55 mmbn4s`): `ereaderName { data = 0 }; end` (`FF 00 00 E5`) → **blank in US ROMs; needs text**.
  - Desc (`74F03C` / `74EACC`, `script 55 mmbn4`): `msgOpenQuick; textSpeed { delay = 0 }; ereaderDesc { data = 0 }; keyWait { any = true }; end` → **blank in US ROMs; needs text**.
- **`0x138` (`Grand Prix Power`, Bank 1 script `56`):**
  - Name (`script 56 mmbn4s`): `ereaderName { data = 1 }; end` (`FF 00 01 E5`) → **blank in US ROMs; needs text**.
  - Desc (`script 56 mmbn4`): `msgOpenQuick; textSpeed { delay = 0 }; ereaderDesc { data = 1 }; keyWait { any = true }; end` → **blank in US ROMs; needs text**.
- **`0x16D` (`DuoSoul`, Bank 1 script `109`):**
  - Name (`script 109 mmbn4s`): **`"DuoSoul"`** — **ALREADY PRESENT in both US ROMs!**
  - Desc (`script 109 mmbn4`): **`"DuoSoul"`** — **ALREADY PRESENT in both US ROMs!** *(Matches `0x161..0x16C`, which all just display their Soul name as description. No text patch needed for `0x16D`!)*

---

## 2. Empirical TextPet Test: Single-Script Patch vs. Full Archive (V)

Tested on scratch copies of `bn4_rs_us.gba` (`scratch/textpet_test/`):

1. **Test A (passing a 2-script `.tpl` WITHOUT reading the ROM archive first):**
   ```
   TextPet.exe load-plugins plugins game mmbn4 load-file rom.gba load-file-index mmbn4rs-us.tpi \
     read-text-archives patch_names.tpl --format tpl write-text-archives rom.gba --free-space 0x7FB000
   ```
   **Result: OVERWRITES THE ENTIRE ARCHIVE!** Scripts `0..54` and `57..132` are wiped to empty, leaving only scripts `55` and `56`.

2. **Test C (1-entry `.tpi` index + `read-text-archives rom.gba` + `read-text-archives patch.tpl --format tpl --patch`):**
   - Generate a temporary `.tpi` containing **only the 2 lines** for the archives we are patching (copied from `mmbn4rs-us.tpi` or `mmbn4bm-us.tpi` at build time).
   - Run:
     ```
     TextPet.exe load-plugins plugins game mmbn4 \
       load-file rom.gba \
       load-file-index build/text/chip_text.tpi \
       read-text-archives rom.gba \
       read-text-archives build/text/chip_text_patch.tpl --format tpl --patch \
       write-text-archives rom.gba --free-space 0x7FB000
     ```
   - **Verified result:**
     - TextPet reads **only the listed archive(s)** from the player's ROM (`Reading 1 text archive...`), merges **only our patched scripts** via `--patch`, and writes **only those archive(s)** back!
     - Because the patched archive grew slightly (`"Duo"` / `"GP Power"` vs 4-byte `ereaderName`), TextPet moved `0x748C14` cleanly to `0x087FB000` and updated **all 8 ROM pointers** (`0x01FDA0` → `0x087FB000`, etc.).
     - Total ROM diff: **1,256 bytes** (`0x4D0` archive bytes at `0x7FB000` + 8 × 4-byte pointers). **Zero other archives touched!**
     - Re-extracting from `0x7FB000` confirmed all 133 scripts intact: `script 0` still `"WoodMnDS"`, `script 55` = `"Duo"`, `script 56` = `"GP Power"`, `script 109` still `"DuoSoul"`.

### Recommended Build-Time Pipeline for `patch/build.py`
- Commit **only** the version-neutral 4-script `.tpl` source below (e.g. `patch/src/text/chip_text.tpl`, using placeholder `@archive CHIP_NAMES_1` / `@archive CHIP_DESCS_1`, or two tiny files).
- At build time (for each version `rs`/`bm`), `build.py`:
  1. Looks up the 2 lines (`748C14`/`74F03C` for RS; `748708`/`74EACC` for BM) in `tools/textpet/indexes/mmbn4{ver}-us.tpi` and writes `build/text/chip_text_{ver}.tpi` (2 lines).
  2. Substitutes the version's `@archive` hex offset into `build/text/chip_text_{ver}.tpl`.
  3. Runs the single TextPet `--patch` command above with `--free-space <FS_CHIP_TEXT>` (from `defines.asm`; reserve `~0x1200` bytes for both moved archives: `~0x4E0` names + `~0xC40` descriptions).
  4. **Zero Capcom text is ever committed to the repo.**

---

## 3. Exact `.tpl` Content (Our Lines Only, UTF-8 without BOM)

Chip names in BN4 have an **8-character UI width limit** (`"GP Power"` = 8 chars; `"Duo"` = 3 chars). Chip descriptions use **3 lines × up to 9 characters per line** (matching vanilla `74F03C.tpl`, e.g. `"WoodTowr\npierce in\nfront"`).

### Chip Names Bank 1 (`RS @archive 748C14` / `BM @archive 748708`, `@size 133`)
```tpl
@archive 748C14
@size 133

script 55 mmbn4s {
	"Duo"
	end
}
script 56 mmbn4s {
	"GP Power"
	end
}
```
*(For Blue Moon, replace `748C14` with `748708`.)*

### Chip Descriptions Bank 1 (`RS @archive 74F03C` / `BM @archive 74EACC`, `@size 113`)
```tpl
@archive 74F03C
@size 113

script 55 mmbn4 {
	msgOpenQuick
	textSpeed
		delay = 0
	"""
	Fist of
	Justice:
	MetKnukl!
	"""
	keyWait
		any = true
	end
}
script 56 mmbn4 {
	msgOpenQuick
	textSpeed
		delay = 0
	"""
	Speedy
	trio
	attack!
	"""
	keyWait
		any = true
	end
}
```
*(For Blue Moon, replace `74F03C` with `74EACC`. Note: `"Fist of\nJustice:\nMetKnukl!"` wraps the owner's `"Fist of Justice: MetKnukl!"` cleanly into BN4's 3-line × 9-char chip description box — 7, 8, and 9 chars per line! And `DuoSoul` `script 109` already exists in both archives as `"DuoSoul"`, so it does not need to be patched.)*
