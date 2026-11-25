# Open Questions: Tier A Leftovers & QoL (saved for later, 2026-11-25)

Source: [`plan/03-phase1-rom-overhaul.md`](../../plan/03-phase1-rom-overhaul.md).

Already built and verified in-game:
- **WP-A1** — WoodMan softlock fix (Prof. 9's v2.0)
- **WP-A2** — Battle Zenny ×1.5
- **WP-A4** — 40% key-item & upgrade shop prices (34 slots + existing-save `SaveLoad` sync)

## Remaining Tier A / QoL items to decide for 1.0 vs defer/skip

1. **WP-A3 — Rewards on scripted/story battles** *(Size: M)*
   - In vanilla BN4, scenario battles and tournament gauntlets skip the results screen and drop nothing.
   - Enabling results screens on them gives chips/Zenny, *but* each scripted battle has to be tested so the results screen doesn't break a cutscene or script flow.
2. **WP-A5 — Cutscene / bracket-animation skip with START** *(Size: M)*
   - Hook the text/event script interpreter so holding/pressing START fast-forwards cutscenes and tournament bracket animations without skipping event flags.
3. **WP-A6 — ToyRobo slide: 1 run instead of 3** *(Size: S)*
   - In the ToyRobo scenario, reduce the repetitive slide puzzle from 3 runs down to 1.
4. **Remaining non-Soul boss rematches** *(Size: S–M)*
   - The 12 Soul Navis (lobby) and Duo (Meteor Comp 1) already have full V1 → SP/Ω → SP+ rematches, and the 10 roaming SP ghosts (TopManΩ through BassΩ/BassXX) already have SP+ support.
   - **Question:** Do you want dedicated lobby/fixed rematch NPCs for the non-Soul bosses (TopMan, ShadeMan, BurnMan, ColdMan, SparkMan, LaserMan, KendoMan, VideoMan, Bass), or are their vanilla roaming ghosts + SP+ enough?

## Decision needed when revisited

For each item above, pick: **include in 1.0**, **defer to post-1.0**, or **skip**.
