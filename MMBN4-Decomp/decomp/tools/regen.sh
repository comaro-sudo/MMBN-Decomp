#!/usr/bin/env bash
# Rebuild the shared asm/ tree from the ROMs: disassemble the segments for RS and BM,
# merge the BM view, run every in-place emitter until nothing changes, then check that
# both ROMs still build byte-identical.
#
#   tools/regen.sh [--no-emit] [seg ...]     (from decomp/, in WSL)
#
# Default segments: all except the hand-maintained ones, intr_table (carved by hand) and
# code_10 (the matching C in src/*.c is wired in by hand as .include "c/*.s"). Name them
# explicitly to regenerate them anyway (you then redo those hand edits).
#
# Proven 2026-11-27 on a scratch copy: from the ROMs alone, both builds are byte-identical
# and segments match the committed asm except for name comments (disasm.py applies
# names.txt at every call and label).
# Claude, 2026-11-27.
set -euo pipefail
cd "$(dirname "$0")/.."
PY=${PY:-~/.venvs/mmbn4-decomp/bin/python}

EMIT=1
SEGS=()
for a in "$@"; do
    case "$a" in
        --no-emit) EMIT=0 ;;
        *) SEGS+=("$a") ;;
    esac
done
if [ ${#SEGS[@]} -eq 0 ]; then
    SEGS=(crt0 code_00 code_01 code_02 code_03 code_04 code_04b code_04c code_04d
          code_05 code_06 code_07 code_08 code_09)
fi

# Pass 1 (fast, unverified) makes the segments raw and consistent: disasm.py's verify
# loop builds the full ROM, which fails while other segments still hold emitted labels
# and macros. Pass 2 then verifies each segment (its loop rewrites non-canonical
# encodings as .hword). FAST=1 skips pass 2 (output builds, but doesn't match).
for s in "${SEGS[@]}"; do
    echo "== disasm (fast) $s"
    $PY tools/disasm.py rs "$s" --no-verify >/dev/null
    $PY tools/disasm.py bm "$s" --no-verify >/dev/null
done
if [ "${FAST:-0}" != 1 ]; then
    for s in "${SEGS[@]}"; do
        echo "== disasm (verified) $s"
        $PY tools/disasm.py rs "$s" >/dev/null
        $PY tools/disasm.py bm "$s" >/dev/null
    done
fi
$PY tools/merge_bm.py "${SEGS[@]}"

if [ $EMIT -eq 1 ]; then
    # Gemini's emitters seed from labels the others create, so run them until the tree
    # stops changing. emit_spawn_lists goes first: the cutscene emitters would otherwise
    # claim its lists.
    for pass in 1 2 3 4; do
        before=$(cat asm/*.s | md5sum)
        for t in emit_spawn_lists emit_cutscenes emit_map_scripts emit_cutscene_scripts \
                 emit_ptr_tables emit_npc_scripts emit_desc_tables emit_versioned_npcs; do
            echo "== pass $pass: $t"
            $PY "tools/$t.py"
        done
        [ "$(cat asm/*.s | md5sum)" = "$before" ] && break
    done
    # Claude's single-table emitters: each asserts its table is still incrom.
    for t in emit_enemy_held_chips emit_formations emit_chip_code_lists emit_event_spawns \
             emit_flag_props emit_u16_curves emit_spawn_records emit_font_widths; do
        [ -f "tools/$t.py" ] || continue
        echo "== $t"
        $PY "tools/$t.py" || echo "   ($t: table not in its incrom form, skipped)"
    done
fi

make -B compare VERSION=rs 2>&1 | tail -1
make -B compare VERSION=bm 2>&1 | tail -1
