/* banker.c – Deadlock Avoidance via Banker's Algorithm */
#include "banking.h"

BankerState banker;

void init_banker(void) {
    /*
     * Resources: [Credit Units, Tellers, Vaults]
     * 3 resources, 5 customers
     *
     * Allocation matrix: currently held
     * Max matrix: maximum they'll ever need
     */
    banker.num_customers  = 5;
    banker.num_resources  = 3;

    /* Available resources */
    int avail[] = {3, 3, 2};
    for (int r = 0; r < 3; r++) banker.available[r] = avail[r];

    /* Max need matrix */
    int max[5][3] = {
        {7, 5, 3},  /* Regular   – Alice   */
        {3, 2, 2},  /* Premium   – Bob     */
        {9, 0, 2},  /* Loan      – Charlie */
        {2, 2, 2},  /* Corporate – Acme    */
        {4, 3, 3},  /* VIP       – Diana   */
    };
    /* Allocation matrix */
    int alloc[5][3] = {
        {0, 1, 0},
        {2, 0, 0},
        {3, 0, 2},
        {2, 1, 1},
        {0, 0, 2},
    };

    for (int i = 0; i < banker.num_customers; i++) {
        for (int r = 0; r < banker.num_resources; r++) {
            banker.allocation[i][r] = alloc[i][r];
            banker.max_need  [i][r] = max [i][r];
            banker.need      [i][r] = max[i][r] - alloc[i][r];
        }
    }
    log_event("Banker's Algorithm initialised (%d customers, %d resources)",
              banker.num_customers, banker.num_resources);
}

/* Safety Algorithm – returns 1 if safe, 0 if unsafe */
int is_safe_state(BankerState *b) {
    int work[MAX_RESOURCES], finish[MAX_CUSTOMERS];
    int safe_seq[MAX_CUSTOMERS];
    int count = 0;

    for (int r = 0; r < b->num_resources; r++)  work[r]   = b->available[r];
    for (int i = 0; i < b->num_customers; i++)  finish[i] = 0;

    while (count < b->num_customers) {
        int found = 0;
        for (int i = 0; i < b->num_customers; i++) {
            if (finish[i]) continue;
            /* check if need[i] <= work */
            int can = 1;
            for (int r = 0; r < b->num_resources; r++)
                if (b->need[i][r] > work[r]) { can = 0; break; }
            if (can) {
                for (int r = 0; r < b->num_resources; r++)
                    work[r] += b->allocation[i][r];
                finish[i]         = 1;
                safe_seq[count++] = i;
                found             = 1;
            }
        }
        if (!found) break;  /* stuck – unsafe */
    }

    if (count == b->num_customers) {
        printf("  Safe Sequence: ");
        for (int i = 0; i < count; i++)
            printf("P%d%s", safe_seq[i], (i < count-1) ? " → " : "");
        printf("\n");
        return 1;
    }
    return 0;
}

/* Resource-Request Algorithm */
int request_loan(BankerState *b, int cust, int request[]) {
    const char *names[] = {"Alice", "Bob", "Charlie", "Acme", "Diana"};

    printf("\n  [Loan Request] %s (P%d) requests [", names[cust], cust);
    for (int r = 0; r < b->num_resources; r++)
        printf("%d%s", request[r], (r < b->num_resources-1) ? "," : "");
    printf("]\n");

    /* Step 1: request <= need? */
    for (int r = 0; r < b->num_resources; r++) {
        if (request[r] > b->need[cust][r]) {
            printf("  DENIED: Request exceeds maximum need.\n");
            log_event("LOAN DENIED [%s]: exceeds max need", names[cust]);
            return 0;
        }
    }
    /* Step 2: request <= available? */
    for (int r = 0; r < b->num_resources; r++) {
        if (request[r] > b->available[r]) {
            printf("  DENIED: Insufficient resources – process must wait.\n");
            log_event("LOAN DENIED [%s]: insufficient available resources", names[cust]);
            return 0;
        }
    }

    /* Step 3: tentatively allocate */
    for (int r = 0; r < b->num_resources; r++) {
        b->available  [r]       -= request[r];
        b->allocation [cust][r] += request[r];
        b->need       [cust][r] -= request[r];
    }

    /* Step 4: safety check */
    if (is_safe_state(b)) {
        printf("  APPROVED: System remains in safe state.\n");
        log_event("LOAN APPROVED [%s]", names[cust]);
        return 1;
    } else {
        /* Roll back */
        for (int r = 0; r < b->num_resources; r++) {
            b->available  [r]       += request[r];
            b->allocation [cust][r] -= request[r];
            b->need       [cust][r] += request[r];
        }
        printf("  DENIED: Would create UNSAFE state – potential deadlock!\n");
        log_event("LOAN DENIED [%s]: unsafe state", names[cust]);
        return 0;
    }
}

static void print_banker_state(BankerState *b) {
    const char *names[] = {"Alice", "Bob", "Charlie", "Acme", "Diana"};
    printf("\n  %-10s %-18s %-18s %-18s\n",
           "Process", "Allocation", "Max Need", "Current Need");
    printf("  %-10s %-18s %-18s %-18s\n",
           "─────────","─────────────────","─────────────────","─────────────────");
    for (int i = 0; i < b->num_customers; i++) {
        char al[32], mx[32], nd[32];
        snprintf(al, sizeof(al), "[%d,%d,%d]",
                 b->allocation[i][0], b->allocation[i][1], b->allocation[i][2]);
        snprintf(mx, sizeof(mx), "[%d,%d,%d]",
                 b->max_need  [i][0], b->max_need  [i][1], b->max_need  [i][2]);
        snprintf(nd, sizeof(nd), "[%d,%d,%d]",
                 b->need      [i][0], b->need      [i][1], b->need      [i][2]);
        printf("  %-10s %-18s %-18s %-18s\n", names[i], al, mx, nd);
    }
    printf("  Available: [%d,%d,%d]\n",
           b->available[0], b->available[1], b->available[2]);
}

void run_banker_demo(void) {
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║    MODULE 3 – Banker's Algorithm / Deadlock   ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    print_banker_state(&banker);

    printf("\n── Initial Safety Check ──\n");
    if (is_safe_state(&banker))
        printf("  System is in a SAFE state.\n");
    else
        printf("  System is in an UNSAFE state!\n");

    /* Test Case 1: Safe request from Bob (P1) */
    printf("\n── Test Case 1: Safe Request ──\n");
    int req1[] = {1, 0, 2};
    request_loan(&banker, 1, req1);

    /* Test Case 2: Unsafe request from Charlie (P2) that would cause deadlock */
    printf("\n── Test Case 2: Unsafe Request (should be denied) ──\n");
    int req2[] = {3, 3, 0};
    request_loan(&banker, 2, req2);

    /* Test Case 3: Request exceeding need */
    printf("\n── Test Case 3: Request Exceeds Need ──\n");
    int req3[] = {9, 9, 9};
    request_loan(&banker, 0, req3);

    /* Test Case 4: Safe request from VIP Diana (P4) */
    printf("\n── Test Case 4: VIP Loan Request ──\n");
    int req4[] = {2, 1, 0};
    request_loan(&banker, 4, req4);

    printf("\n── Final Banker State ──\n");
    print_banker_state(&banker);
}
