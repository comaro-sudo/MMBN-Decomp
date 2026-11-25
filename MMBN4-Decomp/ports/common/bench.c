#include "bench.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int cmp_u64(const void* a, const void* b) {
    uint64_t ua = *(const uint64_t*) a;
    uint64_t ub = *(const uint64_t*) b;
    return (ua > ub) - (ua < ub);
}

bool bench_run(Emu* emu, const char* state_path, int frames, BenchNowNsFn now_ns, BenchResult* out) {
    if (!emu || frames <= 0 || !now_ns || !out) return false;

    if (state_path && state_path[0] && strcmp(state_path, "-") != 0) {
        if (!emu_load_state(emu, state_path)) {
            fprintf(stderr, "bench: could not load state %s\n", state_path);
            return false;
        }
    }

    uint64_t* times = (uint64_t*) malloc((size_t) frames * sizeof(uint64_t));
    if (!times) return false;

    int16_t samples[4096 * 2];
    uint64_t total_ns = 0;
    for (int i = 0; i < frames; ++i) {
        uint64_t t0 = now_ns();
        emu_run_frame(emu);
        (void) emu_read_audio(emu, samples, 4096);
        uint64_t dt = now_ns() - t0;
        times[i] = dt;
        total_ns += dt;
    }

    qsort(times, (size_t) frames, sizeof(uint64_t), cmp_u64);

    int i90 = (int) ((long long) (frames - 1) * 90 / 100);
    int i99 = (int) ((long long) (frames - 1) * 99 / 100);

    out->frames  = frames;
    out->mean_ms = ((double) total_ns / (double) frames) / 1e6;
    out->p90_ms  = (double) times[i90] / 1e6;
    out->p99_ms  = (double) times[i99] / 1e6;
    out->max_ms  = (double) times[frames - 1] / 1e6;
    out->fps     = (total_ns > 0) ? ((double) frames * 1e9 / (double) total_ns) : 0.0;

    free(times);
    return true;
}
