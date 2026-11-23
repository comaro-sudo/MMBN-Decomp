# Story Soul grants & Blue Moon Soul keys (2026-11-23)

Static analysis of where the story grants all 12 Double Souls in Red Sun and Blue Moon, where Blue Moon stores its Soul pseudo-chip keys, and what a Violet Eclipse patch must call to grant any of the 12 Souls cleanly in either ROM.

**V** = verified in disassembly (`git show HEAD:MMBN4-Decomp/decomp/asm/*.s`). **R** = checked against US ROM bytes (`bn4_rs_us.gba`, `bn4_bm_us.gba`) and community saves (`4429`, `4569`, `4891`, `22565`).

---

## 1. Where Blue Moon keeps its Soul pseudo-chip keys (SOLVED — V, R)

- **Exact same location as Red Sun:** `[eToolkit + 0x84][chip_id]` (save block offset `+0x37CC + chip_id`, for Soul pseudo-chips `353..364` = `0x161..0x16C`).
- **Why earlier checks missed them in Blue Moon saves (`soul-merge-experiment.md`):**
  - `WriteChipKey` (`0x080063B4` in both RS and BM) and `BN4_CHIP_GATE` (`0x080063C8` in both RS and BM) XOR `[0x02000670 + id]` with the version-specific constant **`SAVE_XOR_KEY_1`**:
    - **Red Sun:** `SAVE_XOR_KEY_1 = 0x64`
    - **Blue Moon:** `SAVE_XOR_KEY_1 = 0x47`
- **Confirmed against 4 community saves (R):**
  - `4429.sps` (RS): flag `0x18` (GutsSoul) set; `[eToolkit+0x84][354] == [0x02000670+354] ^ 0x64`.
  - `4569.sps` (BM): flags `0x1D`–`0x22` (all 6 BM Souls) set; `[eToolkit+0x84][359..364] == [0x02000670+id] ^ 0x47`.
  - `4891.sps` (BM): 9 Souls (all 6 BM + 3 RS traded) set; all 9 keys match `[0x02000670+id] ^ 0x47`.
  - `22565.sps` (RS): 11 Souls set; all 11 keys match `[0x02000670+id] ^ 0x64`.
- **Consequence:** Calling `WriteChipKey` (`0x080063B4`) automatically uses the running ROM's `SAVE_XOR_KEY_1` (`0x64` in RS, `0x47` in BM) — no version-specific XOR math is needed in a patch. **V**

---

## 2. How the story grants all 12 Souls (V, R)

There are **two script engines** in `code_02` that share the exact same underlying primitive handlers:

| Engine | Opcode table (RS / BM) | `SetEventFlag` (`sub_0802ACB4` RS / `0x0802ACB8` BM) | `WriteChipKey` (`sub_0802CFDE` RS / `0x0802CFE2` BM) |
|---|---|---|---|
| **Map-script engine** (`map_scripts_commands`) | `0x0802A7D0` / `0x0802A7D4` | opcode **`0x1C`** (`1C FF <lo> <hi>`) | opcode **`0x3F`** (`3F <lo> <hi>`) |
| **Cutscene / event-script engine** | `0x0802C140` / `0x0802C144` | opcode **`0x26`** (`26 FF <lo> <hi>`) | opcode **`0x66`** (`66 <lo> <hi>`) |

Both `sub_0802ACB4` (calling `SetEventFlag` `0x0802504C`) and `sub_0802CFDE` (calling `WriteChipKey` `0x080063B4`) are shared across both tables.

### 2.1 The mandatory starter Soul (Map-script engine)

In the Den Battle Tournament Round 1 Double Soul tutorial (right before launching tutorial battle `0x07`), each ROM grants its version's starter Soul:

- **Red Sun (`0x08059EAE`, `code_04`):**
  - `1C FF 14 00` — set flag `0x0014`
  - `1C FF 15 00` — set flag `0x0015` (**Double Soul system unlocked**)
  - `1C FF 18 00` — set flag **`0x0018` (GutsSoul)**
  - `3F 62 01` — `WriteChipKey(0x0162 = 354)` (**GutsSoul pseudo-chip key**)
  - `1C FF 84 00` — set flag `0x0084`
- **Blue Moon (`0x08059EBA`, `code_04`):**
  - `1C FF 14 00` — set flag `0x0014`
  - `1C FF 15 00` — set flag `0x0015` (**Double Soul system unlocked**)
  - `1C FF 21 00` — set flag **`0x0021` (AquaSoul)**
  - `3F 6B 01` — `WriteChipKey(0x016B = 363)` (**AquaSoul pseudo-chip key**)
  - `1C FF 84 00` — set flag `0x0084`

### 2.2 The other 10 Souls (Cutscene / event-script engine — present in BOTH ROMs!)

**Key finding (V, R):** **Both Red Sun and Blue Moon contain the exact same 10 Soul-grant cutscene scripts** (5 native + 5 cross-version!), shifted by `+0x0C` in BM. Every single one executes the exact 4-instruction sequence:

```text
26 FF 15 00          @ SetEventFlag(0x0015)  — Double Soul unlocked
26 FF <flag> 00      @ SetEventFlag(<flag>)  — Soul event flag
66 <cid_lo> 01       @ WriteChipKey(<cid>)   — Soul pseudo-chip key
26 FF 84 00          @ SetEventFlag(0x0084)  — post-Soul-grant flag
```

| Soul # | Soul | Event flag | Pseudo-chip ID | Engine | RS grant address (`flag` / `key`) | BM grant address (`flag` / `key`) |
|---|---|---|---|---|---|---|
| 1 | **RollSoul** | `0x0017` | `353` (`0x161`) | Cutscene (`26`/`66`) | `0x08093471` / `0x08093475` | `0x0809347D` / `0x08093481` |
| 2 | **GutsSoul** | `0x0018` | `354` (`0x162`) | Map-script (`1C`/`3F`) | `0x08059EB6` / `0x08059EBA` | *(AquaSoul in BM)* |
| 3 | **WindSoul** | `0x0019` | `355` (`0x163`) | Cutscene (`26`/`66`) | `0x08093E80` / `0x08093E84` | `0x08093E8C` / `0x08093E90` |
| 4 | **SrchSoul** | `0x001A` | `356` (`0x164`) | Cutscene (`26`/`66`) | `0x0809AEBC` / `0x0809AEC0` | `0x0809AEC8` / `0x0809AECC` |
| 5 | **FireSoul** | `0x001B` | `357` (`0x165`) | Cutscene (`26`/`66`) | `0x0808EA06` / `0x0808EA0A` | `0x0808EA12` / `0x0808EA16` |
| 6 | **ThunSoul** | `0x001C` | `358` (`0x166`) | Cutscene (`26`/`66`) | `0x08097BAC` / `0x08097BB0` | `0x08097BB8` / `0x08097BBC` |
| 7 | **ProtSoul** | `0x001D` | `359` (`0x167`) | Cutscene (`26`/`66`) | `0x0809B9CF` / `0x0809B9D3` | `0x0809B9DB` / `0x0809B9DF` |
| 8 | **NumSoul** | `0x001E` | `360` (`0x168`) | Cutscene (`26`/`66`) | `0x0809244A` / `0x0809244E` | `0x08092456` / `0x0809245A` |
| 9 | **MetlSoul** | `0x001F` | `361` (`0x169`) | Cutscene (`26`/`66`) | `0x0809445F` / `0x08094463` | `0x0809446B` / `0x0809446F` |
| 10 | **JunkSoul** | `0x0020` | `362` (`0x16A`) | Cutscene (`26`/`66`) | `0x080988E2` / `0x080988E6` | `0x080988EE` / `0x080988F2` |
| 11 | **AquaSoul** | `0x0021` | `363` (`0x16B`) | Map-script (`1C`/`3F`) | *(GutsSoul in RS)* | `0x08059EC2` / `0x08059EC6` |
| 12 | **WoodSoul** | `0x0022` | `364` (`0x16C`) | Cutscene (`26`/`66`) | `0x08095CC8` / `0x08095CCC` | `0x08095CD4` / `0x08095CD8` |

*(Raw Thumb `BL` scan of the entire ROM confirms `WriteChipKey` `0x080063B4` has only 7 `BL` callers in both RS and BM: `0x08007B82`, `0x08019142`, `0x0801917E`, `0x080191D0`, `0x0801976E`, `0x0801F4E8`, and `0x0802CFE8`/`0x0802CFEC` (`sub_0802CFDE`). There is no other code path that writes Soul pseudo-chip keys.)* **V, R**

---

## 3. What a Violet Eclipse patch calls to grant any of the 12 Souls (V)

Given a 0-based Soul index `idx` (`0..11`, matching `BN4_SOUL_FLAGS` order: `0=Roll, 1=Guts, 2=Wind, 3=Search, 4=Fire, 5=Thunder, 6=Proto, 7=Number, 8=Metal, 9=Junk, 10=Aqua, 11=Wood`):

1. **Ensure Double Soul is unlocked** (if granting before Den Battle Round 1):
   - `r0 = 0x0015`; `bl SetEventFlag` (RS `0x0802504C` / BM `0x08025050`).
2. **Set the Soul's event flag:**
   - `r0 = 0x0017 + idx`; `bl SetEventFlag` (RS `0x0802504C` / BM `0x08025050`).
3. **Write the Soul's pseudo-chip anti-cheat key:**
   - `r0 = 0x0161 + idx` (`353 + idx`); `bl WriteChipKey` (RS `0x080063B4` / BM `0x080063B4`).
   - Uses the running ROM's `SAVE_XOR_KEY_1` (`0x64` RS / `0x47` BM) and writes `[eToolkit + 0x84][r0]`.
4. **Optional (only if matching story state):** `r0 = 0x0084`; `bl SetEventFlag`.
5. **Required alongside the grant (already in `wp_d4_twelve_souls.asm`):** `BN4_SOUL_FLAGS` (`0x08020018` in both RS and BM) must contain `17 18 19 1A 1B 1C 1D 1E 1F 20 21 22` so `BN4_UNITE_CHECK` (`0x0801FF84`) accepts all 12 Souls.
6. **Note on UI/completion counts:** Remember to also update `BN4_STAR_SOUL_FLAGS` (`0x08000DB4`, 6 entries → 12), `dat_08034A78` (RS `0x08034A78` / BM `0x08034A7C`, 6-byte status-screen Soul list), and `checkDoubleSouls` (`sub_0804F494`) when all 12 Souls are enabled.

---

## 4. Proposed symbol names (`names.txt` format)

```text
0802C140 0802C144 data   GEMINI       BN4_CUTSCENE_SCRIPT_CMDS         Event/cutscene script command table (0x67 entries); op 0x26 = SetEventFlag (sub_0802ACB4), op 0x66 = WriteChipKey (sub_0802CFDE)
0802CFDE 0802CFE2 code   GEMINI       ScriptCmd_WriteChipKey           Cutscene/map-script opcode (0x3F map / 0x66 event): reads u16 chip id and calls WriteChipKey (0x080063B4); used for Soul grants
08034A30 08034A34 code   GEMINI       StatusCollectOwnedSouls          If flag 0x15 set, walks dat_08034A78 (version's 6 Soul flags) and writes owned Soul flags to 0x0201D000
08034A78 08034A7C data   GEMINI       BN4_STATUS_SOUL_FLAGS            0xFF-terminated u8 list of this version's 6 Soul flags (RS: 17..1C, BM: 1D..22)
```
