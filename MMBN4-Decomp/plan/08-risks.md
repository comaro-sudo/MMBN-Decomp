# 08 — Risk Register

| ID | Risk | Likelihood | Impact | Mitigation |
|---|---|---|---|---|
| R1 | **Legal:** distributing Capcom code or assets | Med | High | Patches only (BPS). Native ports extract assets from the user's own ROM. No ROMs in git, ever. |
| R2 | The single-playthrough Soul rework (B1) conflicts with story scripts that are tied to specific opponents | High | High | Document the original structure first. Prototype on one tournament. Allow a scope fallback (for example, 2 playthroughs instead of 1). |
| R3 | Not enough free ROM space for new code or content | Med | Med | Map free space early (RE-17). Expand the ROM to 16 MB (GBA max 32 MB). |
| R4 | Offsets differ between RS and BM, so work doubles | High | Med | Symbol files per version, byte-pattern search scripts, BM-first development |
| R5 | Patches behave differently on hardware or hypervisors than in emulators | Med | Med | Hardware pass before releases. Avoid timing-sensitive hooks. |
| R6 | Decompilation never reaches completion | **High** | High (for Phases 3–4 only) | Phases 1–2 are independent and stand on their own. Check for and join any existing community decomp. Try the static recompilation spike. |
| R7 | 400px widescreen breaks maps, scripts and camera locks | High | Med | Widescreen for the overworld only. Per-map fixes. Fall back to 4:3 with side art. |
| R8 | Reference repos in the list are missing, outdated or misdescribed | Med | Low–Med | Verify in M0. Rely on mGBA and Ghidra for anything missing. |
| R9 | Scope creep from Tier C content and stretch features (Slot-2, online) | High | Med | Tiered releases. Stretch items are explicitly out of scope until 1.0 ships. |
| R10 | Save-compatibility breakage annoys players | Med | Med | New variables in unused save bytes. A version tag in the save. Document breakages. |
| R12 | Wii U: big-endian bugs in host-side code | Med | Med | Option E (mGBA handles it). Endian-safe `pal_read*` helpers used on every platform from the start |
| R13 | ~~Wii U: no way to test on the ARM64 laptop~~ **Resolved 2026-11-21:** a native ARM64 Cemu fork (rhemfur/Cemu) runs on this laptop with Vulkan, and is in `tools/cemu` | — | — | — |
| R14 | Wii U: needs Wii U hardware and a homebrew setup the owner may not have | ? | Med | Confirm before M6/M12 |
| R11 | Burnout (**confirmed solo project**) | High | High | Small releasable increments (Tier A ships early). Hard gates (G2/G3) so there is never more than one big track at a time. Reuse community work (WoodMan fix, existing RE notes, any existing decomp) instead of redoing it. Public progress may attract contributors later. |
