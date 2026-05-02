/* memory.c – Page Replacement: FIFO vs LRU with metrics */
#include "banking.h"

/* ─── FIFO Page Replacement ─── */
int fifo_replace(int pages[], int n, int frames[], int num_frames, int *faults) {
    int memory[NUM_FRAMES];
    int head  = 0;
    int hits  = 0;
    *faults   = 0;

    for (int f = 0; f < num_frames; f++) memory[f] = -1;

    printf("\n  FIFO Simulation (frames=%d):\n", num_frames);
    printf("  %-6s %-30s %-10s\n", "Page", "Frames", "Result");
    printf("  %-6s %-30s %-10s\n", "────", "──────────────────────────────", "──────");

    for (int i = 0; i < n; i++) {
        int page = pages[i];
        int hit  = 0;

        /* Check if already in memory */
        for (int f = 0; f < num_frames; f++) {
            if (memory[f] == page) { hit = 1; hits++; break; }
        }

        if (!hit) {
            memory[head] = page;
            head = (head + 1) % num_frames;
            (*faults)++;
        }

        /* Print frame state */
        char frame_str[64] = {0};
        char tmp[8];
        for (int f = 0; f < num_frames; f++) {
            if (memory[f] == -1) snprintf(tmp, sizeof(tmp), "[ ]");
            else                 snprintf(tmp, sizeof(tmp), "[%d]", memory[f]);
            strncat(frame_str, tmp, sizeof(frame_str) - strlen(frame_str) - 1);
            if (f < num_frames-1)
                strncat(frame_str, " ", sizeof(frame_str) - strlen(frame_str) - 1);
        }
        printf("  %-6d %-30s %-10s\n", page, frame_str,
               hit ? "HIT" : "PAGE FAULT");
    }
    return hits;
}

/* ─── LRU Page Replacement ─── */
int lru_replace(int pages[], int n, int frames[], int num_frames, int *faults) {
    int memory  [NUM_FRAMES];
    int last_use[NUM_FRAMES];
    int hits = 0;
    *faults  = 0;
    int timestamp = 0;

    for (int f = 0; f < num_frames; f++) {
        memory  [f] = -1;
        last_use[f] = 0;
    }

    printf("\n  LRU Simulation (frames=%d):\n", num_frames);
    printf("  %-6s %-30s %-10s\n", "Page", "Frames", "Result");
    printf("  %-6s %-30s %-10s\n", "────", "──────────────────────────────", "──────");

    for (int i = 0; i < n; i++) {
        int page = pages[i];
        int hit  = 0;
        int idx  = -1;
        timestamp++;

        /* Check hit */
        for (int f = 0; f < num_frames; f++) {
            if (memory[f] == page) {
                hit          = 1;
                hits++;
                last_use[f]  = timestamp;
                break;
            }
        }

        if (!hit) {
            /* Find free frame first */
            for (int f = 0; f < num_frames; f++) {
                if (memory[f] == -1) { idx = f; break; }
            }
            /* If no free, evict LRU */
            if (idx == -1) {
                int min_t = last_use[0];
                idx = 0;
                for (int f = 1; f < num_frames; f++) {
                    if (last_use[f] < min_t) { min_t = last_use[f]; idx = f; }
                }
            }
            memory  [idx] = page;
            last_use[idx] = timestamp;
            (*faults)++;
        }

        /* Print frame state */
        char frame_str[64] = {0};
        char tmp[8];
        for (int f = 0; f < num_frames; f++) {
            if (memory[f] == -1) snprintf(tmp, sizeof(tmp), "[ ]");
            else                 snprintf(tmp, sizeof(tmp), "[%d]", memory[f]);
            strncat(frame_str, tmp, sizeof(frame_str) - strlen(frame_str) - 1);
            if (f < num_frames-1)
                strncat(frame_str, " ", sizeof(frame_str) - strlen(frame_str) - 1);
        }
        printf("  %-6d %-30s %-10s\n", page, frame_str,
               hit ? "HIT" : "PAGE FAULT");
    }
    return hits;
}

void run_memory_demo(void) {
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║     MODULE 5 – Memory Management Demo         ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    /*
     * Reference string simulates customer account pages being accessed.
     * Each number = page id (e.g., page 1 = Alice's account data,
     * page 2 = Bob's transaction history, etc.)
     */
    int pages[] = {1, 2, 3, 4, 1, 2, 5, 1, 2, 3, 4, 5, 3, 2, 1, 4, 5, 2, 3, 1};
    int n       = (int)(sizeof(pages)/sizeof(pages[0]));
    int frames[NUM_FRAMES];
    int num_frames = 3;

    printf("\nPage Reference String: ");
    for (int i = 0; i < n; i++)
        printf("%d%s", pages[i], (i < n-1) ? " → " : "\n");
    printf("Number of Frames: %d\n", num_frames);

    /* ── FIFO ── */
    printf("\n── Algorithm: FIFO ──");
    int fifo_faults, fifo_hits;
    fifo_hits = fifo_replace(pages, n, frames, num_frames, &fifo_faults);

    /* ── LRU ── */
    printf("\n── Algorithm: LRU ──");
    int lru_faults, lru_hits;
    lru_hits = lru_replace(pages, n, frames, num_frames, &lru_faults);

    /* ── Comparison ── */
    printf("\n┌──────────────────────────────────────────────┐\n");
    printf("│         Page Replacement Comparison           │\n");
    printf("├───────────────┬───────────────┬───────────────┤\n");
    printf("│  Metric       │     FIFO      │     LRU       │\n");
    printf("├───────────────┼───────────────┼───────────────┤\n");
    printf("│  Page Faults  │ %-13d │ %-13d │\n", fifo_faults, lru_faults);
    printf("│  Page Hits    │ %-13d │ %-13d │\n", fifo_hits,   lru_hits);
    printf("│  Hit Ratio    │ %-12.1f%% │ %-12.1f%% │\n",
           (100.0 * fifo_hits) / n, (100.0 * lru_hits) / n);
    printf("│  Fault Rate   │ %-12.1f%% │ %-12.1f%% │\n",
           (100.0 * fifo_faults) / n, (100.0 * lru_faults) / n);
    printf("└───────────────┴───────────────┴───────────────┘\n");

    const char *winner = (lru_faults < fifo_faults) ? "LRU" :
                         (fifo_faults < lru_faults)  ? "FIFO" : "Tie";
    printf("\n  Conclusion: %s performs better with this reference string.\n", winner);

    log_event("Memory Demo – FIFO faults=%d, LRU faults=%d, Winner=%s",
              fifo_faults, lru_faults, winner);

    /* ── Test Case 2: Larger frame set ── */
    printf("\n── Bonus Test: 4 Frames ──\n");
    num_frames = 4;
    printf("FIFO with 4 frames:\n");
    fifo_hits = fifo_replace(pages, n, frames, num_frames, &fifo_faults);
    printf("LRU with 4 frames:\n");
    lru_hits  = lru_replace (pages, n, frames, num_frames, &lru_faults);
    printf("\n  FIFO Faults=%d  |  LRU Faults=%d\n\n",
           fifo_faults, lru_faults);
}
