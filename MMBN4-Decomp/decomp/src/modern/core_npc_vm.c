/*
 * Portable C NpcRunScript Bytecode Stepper (Phase C3).
 *
 * Implements a portable interpreter over MMBN4's core NpcRunScript opcodes
 * (sub_080A8598 / include/npc_macros.inc):
 *   - 0x00 npc_end, 0x03 npc_hide, 0x06 npc_set_flag, 0x07 npc_clear_flag,
 *     0x14 npc_set_pos, 0x16 npc_set_facing, 0x17 npc_set_sprite,
 *     0x18 npc_set_talk, 0x34 npc_stop
 */
#include "modern_api.h"

static u16 NpcReadLe16(const u8 *p) {
    return (u16)((u16)p[0] | ((u16)p[1] << 8));
}

/*
 * Initialize an NPC script thread on bytecode buffer `code[0..len)`.
 */
void NpcVmInit(struct NpcVmState *vm, const u8 *code, u32 len) {
    vm->code = code;
    vm->len = len;
    vm->pc = 0u;
    vm->x = 0;
    vm->y = 0;
    vm->z = 0;
    vm->facing = 0u;
    vm->spriteId = 0u;
    vm->talkIdx = 0u;
    vm->hidden = 0u;
    vm->halted = (code == 0 || len == 0u) ? 1u : 0u;
}

/*
 * Step one NpcRunScript instruction, applying flag ops to `tk` if non-NULL.
 * Returns 1 if the script has halted/ended, 0 if still running.
 */
u32 NpcVmStep(struct NpcVmState *vm, struct Toolkit *tk) {
    u8 op;

    if (vm == 0 || vm->halted || vm->pc >= vm->len) {
        if (vm != 0) {
            vm->halted = 1u;
        }
        return 1u;
    }

    op = vm->code[vm->pc];
    switch (op) {
    case 0x00u: /* npc_end */
    case 0x34u: /* npc_stop */
        vm->pc++;
        vm->halted = 1u;
        return 1u;
    case 0x03u: /* npc_hide */
        vm->hidden = 1u;
        vm->pc++;
        return 0u;
    case 0x06u: /* npc_set_flag <u16> */
        if (vm->pc + 3u > vm->len) { vm->halted = 1u; return 1u; }
        if (tk != 0) { EventFlagSet(tk, NpcReadLe16(&vm->code[vm->pc + 1u])); }
        vm->pc += 3u;
        return 0u;
    case 0x07u: /* npc_clear_flag <u16> */
        if (vm->pc + 3u > vm->len) { vm->halted = 1u; return 1u; }
        if (tk != 0) { EventFlagClear(tk, NpcReadLe16(&vm->code[vm->pc + 1u])); }
        vm->pc += 3u;
        return 0u;
    case 0x14u: /* npc_set_pos <s16 x, s16 y, s16 z> */
        if (vm->pc + 7u > vm->len) { vm->halted = 1u; return 1u; }
        vm->x = (s16)NpcReadLe16(&vm->code[vm->pc + 1u]);
        vm->y = (s16)NpcReadLe16(&vm->code[vm->pc + 3u]);
        vm->z = (s16)NpcReadLe16(&vm->code[vm->pc + 5u]);
        vm->pc += 7u;
        return 0u;
    case 0x16u: /* npc_set_facing <u8> */
        if (vm->pc + 2u > vm->len) { vm->halted = 1u; return 1u; }
        vm->facing = vm->code[vm->pc + 1u];
        vm->pc += 2u;
        return 0u;
    case 0x17u: /* npc_set_sprite <u8> */
        if (vm->pc + 2u > vm->len) { vm->halted = 1u; return 1u; }
        vm->spriteId = vm->code[vm->pc + 1u];
        vm->pc += 2u;
        return 0u;
    case 0x18u: /* npc_set_talk <u8> */
        if (vm->pc + 2u > vm->len) { vm->halted = 1u; return 1u; }
        vm->talkIdx = vm->code[vm->pc + 1u];
        vm->pc += 2u;
        return 0u;
    default:
        vm->pc++;
        return 0u;
    }
}

/*
 * Run an NpcRunScript to completion (capped at `maxSteps` to prevent infinite loops).
 * Returns the number of instructions executed.
 */
u32 NpcVmRun(struct NpcVmState *vm, struct Toolkit *tk, u32 maxSteps) {
    u32 steps = 0u;
    while (steps < maxSteps && vm != 0 && !vm->halted) {
        steps++;
        if (NpcVmStep(vm, tk)) {
            break;
        }
    }
    return steps;
}
