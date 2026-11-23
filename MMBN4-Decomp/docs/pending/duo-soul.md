# Plan: a Duo Soul (13th Double Soul) — owner request, 2026-11-23

**Status: idea → research.** The owner wants to add the Duo Soul that Capcom planned but cut for cartridge space. Wiki trivia: *"A Duo Soul was planned to be added to Mega Man Battle Network 4, but there wasn't enough space on the game cartridge. Within the game files, the framework for the soul still exists."*

## What we already know

- The chip table has an internal record **`0x16D` `DuoSoul`** (codes `A B C *`), like the 12 Souls' pseudo-chips `0x161`–`0x16C` (`build/inventory/chips.csv`, local). So at least the pseudo-chip exists.
- The Soul flags run `0x17`–`0x22`. A 13th Soul would need flag `0x23` (to check it's free) and its key `WriteChipKey(0x16D)`. `FC 0C` (`patch/src/common/wp_d4_grant_soul.asm`) is limited to idx 0–11 today and would need extending.
- The patch already shows 12 Souls on the status screen (two pages of 6), checks 12 for the completion star, and lets 12 unite. A 13th means a third status page, and so on.

## What a working Soul needs (to check, per part)

| Part | Question |
|---|---|
| Pseudo-chip | `0x16D` exists: is its record complete (name, picture, icon), or blank like Duo's chip? |
| Unite rule | Which chips a Soul is united with (by class or element) and the unite tables (`BN4_UNITE_CHECK` `0x0801FF84`, `BN4_SOUL_FLAGS` `0x08020018`, the Soul kinds `0x08020008`): are there 12 entries or room for 13, and is there a Duo slot? |
| **Form graphics** | MegaMan's Soul-form sprites and palettes: is there a Duo form? **Likely cut.** If so, it needs new original art: a full battle sprite set, the biggest cost |
| Abilities | Each Soul's charge shot and passive effect (code dispatched by Soul id): any Duo entry, or what an entry would need |
| UI | Custom Screen unite list, status screen (a 3rd page), the Soul's name and description text (TextPet pipeline) |
| How to get it | Owner to decide later. Natural fits: beating DuoΩ or SP+ in the Meteor Comp rematch, or a Duo friendly-match-style challenge |

## Ideas to reduce the art cost (if the form is missing)

- A **palette swap** of an existing Soul form in Duo's colours, with Duo-themed abilities.
- Reuse **Duo's own battle sprites** for the charge-shot effect (Duo's fist, laser), which are in both ROMs.

## Owner decisions (2026-11-23), after Gemini's research (`decomp/notes/duo-soul-research.md`)

- **Scope (b): a recolour of MetalSoul** (`list[0][9]`) in Duo's colours, with Duo-themed abilities.
  - The framework already exists in both ROMs: pseudo-chip `0x16D`, 16-entry Soul battle tables with entry 13 = DuoSoul, free flag `0x0023`, free unite class `0x0D`.
  - To build:
    - a hook in `sub_0800B90A` redirecting Soul id 13 to MetalSoul's sprite with a 32-byte Duo palette (otherwise it reads past the end of `list[0]`);
    - the `0x16D` icon (`0x80` bytes);
    - 13-entry `BN4_SOUL_KINDS` / `BN4_SOUL_FLAGS` in free space (class `0x0D`, flag `0x0023`) with the unite loop raised to 12 (`0x0801FFC6`);
    - the passive (`BN4_FORM_ENTERS[13]`) and charge shot (`BN4_FORM_LOADOUTS[13]`, e.g. Duo's meteor fist or laser);
    - which chips get unite class `0x0D`;
    - a 3rd status page; `FC 0C` extended to idx 12.
- **How to get it: by beating the first version of Duo** (V1, `0xE0`).
  - That's the story's final battle, which skips the results screen, so the grant goes in the post-Duo hook Gemini found (`sub_080C4780`, RS `0x080C4792` / BM `0x080C479E`, where the Duo-defeated flag `0x006F` is set).
  - A Duo V1 win in the Meteor Comp rematch grants it too (if not yet owned).
- Still open: the exact Duo palette colours, the passive and charge-shot choice, and which chips get unite class `0x0D`. Claude proposes defaults when building.

## Built (Claude, 2026-11-23, `patch/src/common/wp_d4_duo_soul.asm` + `wp_d2_duo_story_grant.asm`)

- Unite (class `0x0D`, flag `0x0023`, 13-Soul loop), MetalSoul's sprite for form 13, MetalFist at 180 as the charge shot, MetalSoul's icon, ten unite chips (Guard1–3, Barrier, HolyPanl, AntiNavi, AntiDmg, AntiRecv, LifeAura, Duo), `FC 0C 0C`.
- **Grant:** on any Duo deletion (story or rematch), if not owned. Duo's `r5[0x04]` is its version, and the original only sets flag `0x006F` for α/β/Ω.
- Checks on Gemini's design: `0x0800E1CA` is GutsSoul's +30 to Null/break chips in the hand, **not SuperArmor**; MetalFist's `[r7,#0xC]` word is `0x06050601`, not `0x0808`. So the passive is left out for now.
- **Still to do:** the Duo palette (where the battle loads the form's OBJ palette; Duo's own init uses `BN4_SPRITE_PALETTE_SET` `0x08002DEC` with an index, maybe the way in), a passive (default to propose), the 3rd status page. In-game test of everything.

## Next

Gemini's research (round 9, `decomp/notes/task-gemini-round9.md`), then an owner decision on scope: full Soul / palette-swap Soul / drop the idea.
