/* Portable headless frame-time benchmark shared across PC, Wii U, and 3DS.
 * Measures unthrottled emu_run_frame() latency (mean / p90 / p99 / max ms). */
#ifndef MMBN4_BENCH_H
#define MMBN4_BENCH_H

#include <stdbool.h>
#include <stdint.h>

#include "emu_backend.h"

typedef struct {
    int frames;
    double mean_ms;
    double p90_ms;
    double p99_ms;
    double max_ms;
    double fps;
} BenchResult;

typedef uint64_t (*BenchNowNsFn)(void);

/* Runs `frames` unthrottled frames (after optionally loading `state_path` if
 * non-NULL and not "-") and populates `out`. Returns false on error. */
bool bench_run(Emu* emu, const char* state_path, int frames, BenchNowNsFn now_ns, BenchResult* out);

#endif
