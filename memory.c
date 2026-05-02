/* ipc.c – Inter-Process Communication using POSIX Message Queues */
#include "banking.h"
#include <sys/wait.h>

#define NUM_CUSTOMERS_IPC  5
#define BANK_SERVER_TYPE   1L
#define RESPONSE_BASE_TYPE 100L

static int msgid = -1;

/* ─── Message format for IPC ─── */
typedef struct {
    long   mtype;
    int    customer_id;
    int    account_id;
    int    transaction_type;
    double amount;
    char   name[32];
} BankMessage;

typedef struct {
    long mtype;
    int  customer_id;
    int  success;
    double new_balance;
    char   message[64];
} ResponseMessage;

/* ─── Bank Server Process ─── */
static void bank_server_process(void) {
    log_event("[BANK SERVER] Started, listening on queue %d", msgid);

    int served = 0;
    while (served < NUM_CUSTOMERS_IPC) {
        BankMessage req;
        if (msgrcv(msgid, &req, sizeof(req) - sizeof(long),
                   BANK_SERVER_TYPE, 0) == -1) {
            if (errno == EINTR) continue;
            break;
        }

        log_event("[BANK SERVER] Received: %s from %s (Account %d, Amount %.2f)",
                  transaction_type_str(req.transaction_type),
                  req.name, req.account_id, req.amount);

        ResponseMessage resp;
        resp.mtype       = RESPONSE_BASE_TYPE + req.customer_id;
        resp.customer_id = req.customer_id;

        int result;
        if (req.transaction_type == DEPOSIT)
            result = deposit(req.account_id, req.amount, req.name);
        else
            result = withdraw(req.account_id, req.amount, req.name);

        if (result == 0) {
            resp.success = 1;
            Account *acc = find_account(req.account_id);
            resp.new_balance = acc ? acc->balance : 0.0;
            snprintf(resp.message, sizeof(resp.message),
                     "Transaction Successful. Balance: %.2f", resp.new_balance);
        } else {
            resp.success     = 0;
            resp.new_balance = 0.0;
            snprintf(resp.message, sizeof(resp.message), "Transaction FAILED");
        }

        msgsnd(msgid, &resp, sizeof(resp) - sizeof(long), 0);
        log_event("[BANK SERVER] Response sent to Customer %d: %s",
                  req.customer_id, resp.message);
        served++;
    }
    log_event("[BANK SERVER] Shutting down after serving %d requests", served);
    exit(0);
}

/* ─── Customer Process ─── */
typedef struct {
    int    customer_id;
    int    account_id;
    int    transaction_type;
    double amount;
    char   name[32];
    int    delay_ms;
} IPCCustomer;

static void customer_process(IPCCustomer *cust) {
    ms_sleep(cust->delay_ms);
    log_event("[CUSTOMER %d] %s sending %s request for %.2f",
              cust->customer_id, cust->name,
              transaction_type_str(cust->transaction_type), cust->amount);

    BankMessage msg;
    msg.mtype            = BANK_SERVER_TYPE;
    msg.customer_id      = cust->customer_id;
    msg.account_id       = cust->account_id;
    msg.transaction_type = cust->transaction_type;
    msg.amount           = cust->amount;
    strncpy(msg.name, cust->name, sizeof(msg.name) - 1);

    msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0);

    /* Wait for response */
    ResponseMessage resp;
    long resp_type = RESPONSE_BASE_TYPE + cust->customer_id;
    if (msgrcv(msgid, &resp, sizeof(resp) - sizeof(long), resp_type, 0) != -1) {
        log_event("[CUSTOMER %d] Response: %s", cust->customer_id, resp.message);
    }
    exit(0);
}

void run_ipc_demo(void) {
    printf("\n╔═══════════════════════════════════════════════╗\n");
    printf("║     MODULE 4 – IPC: Message Queue Demo        ║\n");
    printf("╚═══════════════════════════════════════════════╝\n");

    /* Create message queue */
    msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        printf("  [IPC] Cannot create message queue. Skipping IPC demo.\n");
        printf("  (Run as root or ensure IPC is enabled on this system)\n");
        return;
    }

    /* Define customer requests */
    IPCCustomer customers[NUM_CUSTOMERS_IPC] = {
        {1, 1001, DEPOSIT,    500.0, "Alice",   50},
        {2, 1002, WITHDRAWAL, 200.0, "Bob",    100},
        {3, 1003, DEPOSIT,   1000.0, "Charlie",  0},
        {4, 1004, DEPOSIT,   3000.0, "AcmeCorp",200},
        {5, 1005, WITHDRAWAL,5000.0, "Diana",   30},
    };

    /* Fork bank server */
    pid_t server_pid = fork();
    if (server_pid == 0) {
        bank_server_process();
        /* never returns */
    }

    log_event("[IPC DEMO] Bank server PID=%d", server_pid);

    /* Fork customer processes */
    pid_t cpids[NUM_CUSTOMERS_IPC];
    for (int i = 0; i < NUM_CUSTOMERS_IPC; i++) {
        cpids[i] = fork();
        if (cpids[i] == 0) {
            customer_process(&customers[i]);
            /* never returns */
        }
    }

    /* Wait for all customers */
    for (int i = 0; i < NUM_CUSTOMERS_IPC; i++)
        waitpid(cpids[i], NULL, 0);

    /* Wait for server */
    waitpid(server_pid, NULL, 0);

    /* Clean up message queue */
    msgctl(msgid, IPC_RMID, NULL);
    msgid = -1;

    log_event("[IPC DEMO] Message queue removed. IPC demo complete.");
    printf("\n── Producer-Consumer Summary ──\n");
    printf("  Producers: %d customer processes (sent requests)\n", NUM_CUSTOMERS_IPC);
    printf("  Consumer:  1 bank server process (processed requests)\n");
    printf("  IPC Channel: POSIX System V Message Queue\n\n");
}
