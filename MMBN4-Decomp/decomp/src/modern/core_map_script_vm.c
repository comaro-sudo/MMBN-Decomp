/*
 * Portable C map_scripts_commands Bytecode Stepper (Phase C3).
 *
 * Implements a portable interpreter over MMBN4's core map-init script
 * opcodes (dat_0802A7D0 / include/cutscene_macros.inc):
 *   - 0x00 ms_end
 *   - 0x1C ms_set_flag <bank> <u16 flag>
 *   - 0x1D ms_clear_flag <bank> <u16 flag>
 */
#include "modern_api.h"

static u16 MsReadLe16(const u8 *p) {
    return (u16)((u16)p[0] | ((u16)p[1] << 8));
}

void MapScriptVmInit(struct MapScriptVmState *vm, const u8 *code, u32 len) {
    vm->code = code;
    vm->len = len;
    vm->pc = 0u;
    vm->halted = (code == 0 || len == 0u) ? 1u : 0u;
    vm->flagsTouched = 0u;
}

u32 MapScriptVmStep(struct MapScriptVmState *vm, struct Toolkit *tk) {
    u8 op;

    if (vm == 0 || vm->halted || vm->pc >= vm->len) {
        if (vm != 0) {
            vm->halted = 1u;
        }
        return 1u;
    }

    op = vm->code[vm->pc];
    if (op == 0x00u) {
        vm->pc++;
        vm->halted = 1u;
        return 1u;
    }
    if (op == 0x1Cu && vm->pc + 4u <= vm->len) {
        if (tk != 0) {
            EventFlagSet(tk, MsReadLe16(&vm->code[vm->pc + 2u]));
        }
        vm->flagsTouched++;
        vm->pc += 4u;
        return 0u;
    }
    if (op == 0x1Du && vm->pc + 4u <= vm->len) {
        if (tk != 0) {
            EventFlagClear(tk, MsReadLe16(&vm->code[vm->pc + 2u]));
        }
        vm->flagsTouched++;
        vm->pc += 4u;
        return 0u;
    }

    vm->pc++;
    return 0u;
}

u32 MapScriptVmRun(struct MapScriptVmState *vm, struct Toolkit *tk, u32 maxSteps) {
    u32 steps = 0u;
    while (steps < maxSteps && vm != 0 && !vm->halted) {
        steps++;
        if (MapScriptVmStep(vm, tk)) {
            break;
        }
    }
    return steps;
}
