/* scheduling.c – FCFS, Priority Scheduling, Round Robin with Gantt charts */
#include "banking.h"

/* ─── helpers ─── */
static void compute_fcfs(Customer c[], int n) {
    /* sort by arrival time */
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++)
            if (c[j].arrival_time < c[i].arrival_time) {
                Customer tmp = c[i]; c[i] = c[j]; c[j] = tmp;
            }

    int clock = 0;
    for (int i = 0; i < n; i++) {
        if (clock < c[i].arrival_time) clock = c[i].arrival_time;
        c[i].start_time      = clock;
        c[i].finish_time     = clock + c[i].burst_time;
        c[i].waiting_time    = c[i].start_time - c[i].arrival_time;
        c[i].turnaround_time = c[i].finish_time - c[i].arrival_time;
        clock                = c[i].finish_time;
    }
}

static void compute_priority(Customer c[], int n) {
    /* sort by priority DESC, then arrival ASC */
    for (int i = 0; i < n-1; i++)
        for (int j = i+1; j < n; j++) {
            int swap = 0;
            if (c[j].priority > c[i].priority) swap = 1;
            else if (c[j].priority == c[i].priority &&
                     c[j].arrival_time < c[i].arrival_time) swap = 1;
            if (swap) { Customer tmp = c[i]; c[i] = c[j]; c[j] = tmp; }
        }

    int clock = 0;
    for (int i = 0; i < n; i++) {
        if (clock < c[i].arrival_time) clock = c[i].arrival_time;
        c[i].start_time      = clock;
        c[i].finish_time     = clock + c[i].burst_time;
        c[i].waiting_time    = c[i].start_time - c[i].arrival_time;
        c[i].turnaround_time = c[i].finish_time - c[i].arrival_time;
        clock                = c[i].finish_time;
    }
}

/* Round Robin – non-preemptive slice version with order tracking */
typedef struct { int cust_idx; int start; int end; } Slice;
#define MAX_SLICES 200
static Slice rr_slices[MAX_SLICES];
static int   rr_nslices;

static void compute_rr(Customer c[], int n) {
    rr_nslices = 0;
    int remaining[MAX_ACCOUNTS];
    int done[MAX_ACCOUNTS];
    for (int i = 0; i < n; i++) {
        remaining[i] = c[i].burst_time;
        done[i]      = 0;
        c[i].start_time = -1;
    }

    int clock   = 0;
    int finished = 0;
    while (finished < n) {
        int progressed = 0;
        for (int i = 0; i < n; i++) {
            if (done[i] || c[i].arrival_time > clock) continue;
            if (remaining[i] <= 0) { done[i] = 1; continue; }

            progressed = 1;
            if (c[i].start_time == -1) c[i].start_time = clock;

            int run = (remaining[i] < TIME_QUANTUM) ? remaining[i] : TIME_QUANTUM;
            if (rr_nslices < MAX_SLICES) {
                rr_slices[rr_nslices].cust_idx = i;
                rr_slices[rr_nslices].start    = clock;
                rr_slices[rr_nslices].end      = clock + run;
                rr_nslices++;
            }
            clock        += run;
            remaining[i] -= run;

            if (remaining[i] <= 0) {
                done[i]              = 1;
                c[i].finish_time     = clock;
                c[i].waiting_time    = c[i].finish_time - c[i].arrival_time - c[i].burst_time;
                c[i].turnaround_time = c[i].finish_time - c[i].arrival_time;
                finished++;
            }
        }
        if (!progressed) clock++; /* idle CPU */
    }
}

/* ─── Gantt chart (ASCII) ─── */
void print_gantt(Customer c[], int n, const char *algo) {
    printf("\n══════════════════════════════════════════════\n");
    printf("  Gantt Chart – %s\n", algo);
    printf("══════════════════════════════════════════════\n");

    if (strcmp(algo, "Round Robin") == 0) {
        /* top bar */
        printf("|");
        for (int i = 0; i < rr_nslices; i++)
            printf(" %-6s |", c[rr_slices[i].cust_idx].name);
        printf("\n");
        /* time row */
        printf("%d", rr_slices[0].start);
        for (int i = 0; i < rr_nslices; i++)
            printf("        %d", rr_slices[i].end);
        printf("\n\n");
    } else {
        printf("|");
        for (int i = 0; i < n; i++)
            printf(" %-6s |", c[i].name);
        printf("\n");
        printf("%d", c[0].arrival_time > c[0].start_time ?
               c[0].arrival_time : c[0].start_time);
        for (int i = 0; i < n; i++)
            printf("        %d", c[i].finish_time);
        printf("\n\n");
    }
}

/* ─── Metrics table ─── */
void print_metrics(Customer c[], int n, const char *algo) {
    printf("┌──────────────────────────────────────────────────────────────────┐\n");
    printf("│  Scheduling Metrics – %-44s│\n", algo);
    printf("├──────────┬────────────┬────────────┬────────────┬────────────────┤\n");
    printf("│ Customer │  Priority  │  Arrival   │  Waiting   │  Turnaround    │\n");
    printf("├──────────┼────────────┼────────────┼────────────┼────────────────┤\n");

    double total_wt = 0, total_tat = 0;
    for (int i = 0; i < n; i++) {
        printf("│ %-8s │ %-10d │ %-10d │ %-10d │ %-14d │\n",
               c[i].name, c[i].priority, c[i].arrival_time,
               c[i].waiting_time, c[i].turnaround_time);
        total_wt  += c[i].waiting_time;
        total_tat += c[i].turnaround_time;
    }
    printf("├──────────┴────────────┴────────────┼────────────┼────────────────┤\n");
    printf("│                           AVERAGE   │ %-10.2f │ %-14.2f │\n",
           total_wt/n, total_tat/n);
    printf("└──────────────────────────────────────────────────────────────────┘\n\n");

    log_event("[%s] Avg Waiting=%.2f ms  Avg Turnaround=%.2f ms",
              algo, total_wt/n, total_tat/n);
}

/* ─── Public run functions ─── */
void run_fcfs(Customer c[], int n) {
    Customer local[MAX_ACCOUNTS];
    memcpy(local, c, n * sizeof(Customer));
    compute_fcfs(local, n);
    print_gantt (local, n, "FCFS");
    print_metrics(local, n, "FCFS");
}

void run_priority(Customer c[], int n) {
    Customer local[MAX_ACCOUNTS];
    memcpy(local, c, n * sizeof(Customer));
    compute_priority(local, n);
    print_gantt (local, n, "Priority Scheduling");
    print_metrics(local, n, "Priority Scheduling");
}

void run_rr(Customer c[], int n) {
    Customer local[MAX_ACCOUNTS];
    memcpy(local, c, n * sizeof(Customer));
    compute_rr(local, n);
    print_gantt (local, n, "Round Robin");
    print_metrics(local, n, "Round Robin");
}
