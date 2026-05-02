#ifndef BANKING_H
#define BANKING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <errno.h>
#include <signal.h>

/* ─── Customer Types ─── */
#define REGULAR    1
#define PREMIUM    3
#define LOAN       2
#define CORPORATE  4
#define VIP        5

/* ─── Transaction Types ─── */
#define DEPOSIT    1
#define WITHDRAWAL 2
#define LOAN_REQ   3
#define PAYROLL    4

/* ─── Scheduling Algorithms ─── */
#define FCFS           0
#define PRIORITY_SCHED 1
#define ROUND_ROBIN    2
#define TIME_QUANTUM   2   /* ms for Round Robin */

/* ─── Banker's Algorithm ─── */
#define MAX_CUSTOMERS  10
#define MAX_RESOURCES   3
#define TOTAL_CREDIT   10  /* total credit units in bank */

/* ─── Memory Management ─── */
#define NUM_FRAMES      4
#define PAGE_SEQ_LEN   20

/* ─── IPC ─── */
#define MSG_KEY       0x1234
#define MAX_MSG_TEXT    128

/* ─── Account ─── */
#define MAX_ACCOUNTS   20

/* ══════════════════════════════════════════════════════════
   Data Structures
   ══════════════════════════════════════════════════════════ */

typedef struct {
    int    account_id;
    double balance;
    int    owner_type;      /* customer type */
    char   owner_name[32];
} Account;

typedef struct {
    int    customer_id;
    int    customer_type;   /* REGULAR, PREMIUM, VIP, LOAN, CORPORATE */
    int    transaction_type;
    double amount;
    int    account_id;
    int    arrival_time;    /* for scheduling simulation (ms) */
    int    burst_time;      /* processing time (ms)           */
    int    remaining_time;  /* for Round Robin                 */
    int    priority;        /* same as customer_type value     */
    int    start_time;
    int    finish_time;
    int    waiting_time;
    int    turnaround_time;
    char   name[32];
} Customer;

/* ─── Banker's Algorithm structures ─── */
typedef struct {
    int allocation[MAX_CUSTOMERS][MAX_RESOURCES];
    int max_need [MAX_CUSTOMERS][MAX_RESOURCES];
    int available[MAX_RESOURCES];
    int need     [MAX_CUSTOMERS][MAX_RESOURCES];
    int num_customers;
    int num_resources;
} BankerState;

/* ─── IPC message ─── */
typedef struct {
    long mtype;
    char mtext[MAX_MSG_TEXT];
    int  account_id;
    double amount;
    int  transaction_type;
    int  customer_id;
} Message;

/* ─── Memory page ─── */
typedef struct {
    int page_id;
    int last_used;   /* timestamp for LRU */
    int load_order;  /* for FIFO          */
} Frame;

/* ══════════════════════════════════════════════════════════
   Globals (defined in main.c)
   ══════════════════════════════════════════════════════════ */

extern Account         accounts[MAX_ACCOUNTS];
extern int             num_accounts;
extern pthread_mutex_t account_mutex;
extern sem_t           deposit_sem;      /* allows 2 concurrent deposits */
extern BankerState     banker;
extern FILE           *log_fp;

/* ─── Function Prototypes ─── */

/* accounts.c */
void   init_accounts(void);
Account *find_account(int account_id);
int    deposit(int account_id, double amount, const char *who);
int    withdraw(int account_id, double amount, const char *who);
void   print_accounts(void);

/* scheduling.c */
void   run_fcfs      (Customer customers[], int n);
void   run_priority  (Customer customers[], int n);
void   run_rr        (Customer customers[], int n);
void   print_gantt   (Customer customers[], int n, const char *algo);
void   print_metrics (Customer customers[], int n, const char *algo);

/* sync.c */
void  *thread_deposit  (void *arg);
void  *thread_withdraw (void *arg);
void   run_sync_demo   (void);

/* banker.c */
void   init_banker   (void);
int    is_safe_state (BankerState *b);
int    request_loan  (BankerState *b, int cust, int request[]);
void   run_banker_demo(void);

/* ipc.c */
void   run_ipc_demo  (void);

/* memory.c */
int    fifo_replace  (int pages[], int n, int frames[], int num_frames, int *faults);
int    lru_replace   (int pages[], int n, int frames[], int num_frames, int *faults);
void   run_memory_demo(void);

/* utils.c */
void   log_event     (const char *fmt, ...);
void   ms_sleep      (int ms);
const char *customer_type_str(int type);
const char *transaction_type_str(int type);

#endif /* BANKING_H */
