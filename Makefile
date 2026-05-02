/* main.c – Concurrent Banking System: Main Entry Point */
#include "banking.h"

/* ─── Scheduling test data ─── */
static Customer make_customer(int id, const char *name, int type,
                               int arrival, int burst) {
    Customer c = {0};
    c.customer_id      = id;
    c.customer_type    = type;
    c.priority         = type;   /* customer type value == priority */
    c.arrival_time     = arrival;
    c.burst_time       = burst;
    c.remaining_time   = burst;
    strncpy(c.name, name, sizeof(c.name) - 1);
    return c;
}

int main(void) {
    /* Open log file */
    log_fp = fopen("logs/banking_system.log", "w");
    if (!log_fp) {
        log_fp = fopen("/tmp/banking_system.log", "w");
    }

    printf("\n");
    printf("╔══════════════════════════════════════════════════════════╗\n");
    printf("║        CONCURRENT BANKING SYSTEM SIMULATION              ║\n");
    printf("║    OS Concepts: Scheduling | Sync | Deadlock | IPC | MM  ║\n");
    printf("║    FAST-NUCES  |  CL2006 OS Lab  |  Spring 2026          ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");

    /* ════════════════════════════════════════════
       Initialise shared resources
       ════════════════════════════════════════════ */
    init_accounts();
    init_banker();

    printf("\n── Initial Account State ──\n");
    print_accounts();

    /* ════════════════════════════════════════════
       MODULE 1 – Scheduling
       ════════════════════════════════════════════ */
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║       MODULE 1 – CPU Scheduling Demo          ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    /*
     * 6 customers with different types (= priorities) and burst times.
     * Arrival times staggered to illustrate scheduling differences.
     */
    Customer sched_customers[6] = {
        make_customer(1, "Alice",  REGULAR,   0, 8),
        make_customer(2, "Bob",    PREMIUM,   2, 5),
        make_customer(3, "Charlie",LOAN,      4, 3),
        make_customer(4, "Acme",   CORPORATE, 1, 6),
        make_customer(5, "Diana",  VIP,       3, 4),
        make_customer(6, "Eve",    REGULAR,   5, 7),
    };
    int n = 6;

    printf("\nCustomers for scheduling:\n");
    printf("%-10s %-10s %-10s %-10s\n","Name","Priority","Arrival","Burst");
    printf("%-10s %-10s %-10s %-10s\n","────","────────","───────","─────");
    for (int i = 0; i < n; i++)
        printf("%-10s %-10d %-10d %-10d\n",
               sched_customers[i].name,
               sched_customers[i].priority,
               sched_customers[i].arrival_time,
               sched_customers[i].burst_time);

    run_fcfs    (sched_customers, n);
    run_priority(sched_customers, n);
    run_rr      (sched_customers, n);

    /* ════════════════════════════════════════════
       MODULE 2 – Synchronization
       ════════════════════════════════════════════ */
    run_sync_demo();

    /* ════════════════════════════════════════════
       MODULE 3 – Deadlock (Banker's Algorithm)
       ════════════════════════════════════════════ */
    run_banker_demo();

    /* ════════════════════════════════════════════
       MODULE 4 – IPC
       ════════════════════════════════════════════ */
    run_ipc_demo();

    /* ════════════════════════════════════════════
       MODULE 5 – Memory Management
       ════════════════════════════════════════════ */
    run_memory_demo();

    /* ════════════════════════════════════════════
       Final summary
       ════════════════════════════════════════════ */
    printf("\n╔══════════════════════════════════════════════════════════╗\n");
    printf("║                    SIMULATION COMPLETE                   ║\n");
    printf("╠══════════════════════════════════════════════════════════╣\n");
    printf("║  Module 1: Scheduling (FCFS, Priority, Round Robin)  ✓  ║\n");
    printf("║  Module 2: Synchronization (Mutex, Semaphore)        ✓  ║\n");
    printf("║  Module 3: Deadlock Avoidance (Banker's Algorithm)   ✓  ║\n");
    printf("║  Module 4: IPC (Message Queues, Producer-Consumer)   ✓  ║\n");
    printf("║  Module 5: Memory Management (FIFO, LRU)             ✓  ║\n");
    printf("╚══════════════════════════════════════════════════════════╝\n");
    printf("\nLog saved to: logs/banking_system.log\n\n");

    if (log_fp) fclose(log_fp);
    sem_destroy(&deposit_sem);
    return 0;
}
