/*
 * Portable C Cutscene Script VM State Stepper (Phase C3).
 *
 * Models a lightweight portable interpreter over MMBN4's cutscene bytecode
 * core control ops (0x00 End, 0x02 WaitFrames, 0x04 SetFlag-like, 0x14 Jump)
 * for native script testing and future engine ports.
 */
#include "modern_api.h"

/*
 * Initialize a cutscene VM thread on bytecode buffer `code[0..len)`.
 */
void CutsceneVmInit(struct CutsceneVmState *vm, const u8 *code, u32 len) {
    vm->code = code;
    vm->len = len;
    vm->pc = 0u;
    vm->waitFrames = 0u;
    vm->halted = (code == 0 || len == 0u) ? 1u : 0u;
    vm->lastOp = 0u;
}

/*
 * Advance the cutscene VM by 1 frame.
 *   - While waitFrames > 0, decrements waitFrames and returns 0 (waiting).
 *   - Op 0x00 / 0x01 / 0x10: halt thread, return 1 (halted).
 *   - Op 0x02 <u16 frames>: set waitFrames, return 0.
 *   - Op 0x14 <u16 relPc>: jump pc to relPc (within bounds), return 0.
 *   - Any other single-byte no-op advances pc by 1.
 */
u32 CutsceneVmStep(struct CutsceneVmState *vm) {
    u8 op;

    if (vm == 0 || vm->halted) {
        return 1u;
    }
    if (vm->waitFrames > 0u) {
        vm->waitFrames--;
        return 0u;
    }
    if (vm->pc >= vm->len) {
        vm->halted = 1u;
        return 1u;
    }

    op = vm->code[vm->pc];
    vm->lastOp = op;

    if (op == 0x00u || op == 0x01u || op == 0x10u) {
        vm->pc++;
        vm->halted = 1u;
        return 1u;
    }
    if (op == 0x02u && vm->pc + 3u <= vm->len) {
        vm->waitFrames = (u16)((u16)vm->code[vm->pc + 1u]
                             | ((u16)vm->code[vm->pc + 2u] << 8));
        vm->pc += 3u;
        return 0u;
    }
    if (op == 0x14u && vm->pc + 3u <= vm->len) {
        u16 target = (u16)((u16)vm->code[vm->pc + 1u]
                         | ((u16)vm->code[vm->pc + 2u] << 8));
        if ((u32)target < vm->len) {
            vm->pc = (u32)target;
        } else {
            vm->halted = 1u;
            return 1u;
        }
        return 0u;
    }

    vm->pc++;
    return 0u;
}

/*
 * Return 1 if the cutscene VM thread has halted, 0 if still running.
 */
u32 CutsceneVmIsHalted(const struct CutsceneVmState *vm) {
    return (vm == 0 || vm->halted) ? 1u : 0u;
}

/*
 * Force-stop a cutscene VM thread immediately.
 */
void CutsceneVmHalt(struct CutsceneVmState *vm) {
    if (vm != 0) {
        vm->halted = 1u;
        vm->waitFrames = 0u;
    }
}

/*
 * Return remaining wait frames on a cutscene VM thread.
 */
u16 CutsceneVmGetWaitFrames(const struct CutsceneVmState *vm) {
    return (vm != 0) ? vm->waitFrames : 0u;
}

/*
 * Reset a cutscene VM thread back to PC=0 to replay its script.
 */
void CutsceneVmReset(struct CutsceneVmState *vm) {
    if (vm != 0) {
        vm->pc = 0u;
        vm->waitFrames = 0u;
        vm->halted = (vm->code == 0 || vm->len == 0u) ? 1u : 0u;
        vm->lastOp = 0u;
    }
}
