/* sync.c – Thread synchronization: mutex locks & semaphores */
#include "banking.h"

/* ─── Thread argument ─── */
typedef struct {
    int    account_id;
    double amount;
    char   name[32];
    int    op;         /* DEPOSIT or WITHDRAWAL */
    int    delay_ms;   /* simulate arrival time  */
} ThreadArg;

void *thread_deposit(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    ms_sleep(a->delay_ms);
    log_event("[THREAD] %s attempting deposit of %.2f", a->name, a->amount);
    deposit(a->account_id, a->amount, a->name);
    return NULL;
}

void *thread_withdraw(void *arg) {
    ThreadArg *a = (ThreadArg *)arg;
    ms_sleep(a->delay_ms);
    log_event("[THREAD] %s attempting withdrawal of %.2f", a->name, a->amount);
    withdraw(a->account_id, a->amount, a->name);
    return NULL;
}

/* Corporate payroll – spawns N threads, each pays one employee */
typedef struct {
    int    account_id;
    double salary;
    int    employee_id;
} PayrollArg;

static void *payroll_thread(void *arg) {
    PayrollArg *p = (PayrollArg *)arg;
    char who[32];
    snprintf(who, sizeof(who), "Payroll-Emp%d", p->employee_id);
    ms_sleep(10);  /* simulate short processing */
    deposit(p->account_id, p->salary, who);
    free(p);
    return NULL;
}

static void run_payroll_demo(void) {
    printf("\n── Corporate Payroll Demo (10 employee threads) ──\n");
    pthread_t tids[10];
    for (int i = 0; i < 10; i++) {
        PayrollArg *p = malloc(sizeof(PayrollArg));
        p->account_id  = 1004;   /* Acme Corp account */
        p->salary      = 500.0;
        p->employee_id = i + 1;
        pthread_create(&tids[i], NULL, payroll_thread, p);
    }
    for (int i = 0; i < 10; i++)
        pthread_join(tids[i], NULL);
    log_event("Payroll complete: 10 employees paid from Account 1004");
}

void run_sync_demo(void) {
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║      MODULE 2 – Synchronization Demo          ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    /* ── Test 1: Mutex – two simultaneous withdrawals from same account ── */
    printf("\n[Test 1] Mutex: Two concurrent withdrawals from Account 1001\n");
    pthread_t t1, t2;
    ThreadArg a1 = {1001, 1000.0, "Alice-T1", WITHDRAWAL, 0};
    ThreadArg a2 = {1001, 500.0,  "Alice-T2", WITHDRAWAL, 5};
    pthread_create(&t1, NULL, thread_withdraw, &a1);
    pthread_create(&t2, NULL, thread_withdraw, &a2);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("  -> Mutex ensures only one thread modifies balance at a time.\n");

    /* ── Test 2: Semaphore – three deposits, only two run concurrently ── */
    printf("\n[Test 2] Semaphore: Three deposits (semaphore value=2)\n");
    pthread_t t3, t4, t5;
    ThreadArg b1 = {1002, 200.0, "Bob-D1",   DEPOSIT, 0};
    ThreadArg b2 = {1002, 300.0, "Bob-D2",   DEPOSIT, 0};
    ThreadArg b3 = {1002, 400.0, "Charlie-D1",DEPOSIT, 0};
    pthread_create(&t3, NULL, thread_deposit, &b1);
    pthread_create(&t4, NULL, thread_deposit, &b2);
    pthread_create(&t5, NULL, thread_deposit, &b3);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    pthread_join(t5, NULL);
    printf("  -> Semaphore(2) blocked third deposit until a slot freed.\n");

    /* ── Test 3: Corporate payroll ── */
    run_payroll_demo();

    /* ── Test 4: VIP preempts regular (priority order) ── */
    printf("\n[Test 4] Priority: VIP withdrawal before Regular deposit\n");
    ThreadArg v1 = {1005, 2000.0, "Diana-VIP",  WITHDRAWAL, 0};
    ThreadArg r1 = {1001, 100.0,  "Eve-Regular", DEPOSIT,   5};
    pthread_t tv, tr;
    pthread_create(&tv, NULL, thread_withdraw, &v1);
    pthread_create(&tr, NULL, thread_deposit,  &r1);
    pthread_join(tv, NULL);
    pthread_join(tr, NULL);

    printf("\n── Final Account Balances after Sync Demo ──\n");
    print_accounts();
}
