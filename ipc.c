/* utils.c – Logging and utility helpers */
#include "banking.h"
#include <stdarg.h>

FILE *log_fp = NULL;

void log_event(const char *fmt, ...) {
    va_list args;
    time_t  now = time(NULL);
    struct  tm *t = localtime(&now);
    char    ts[32];
    strftime(ts, sizeof(ts), "%H:%M:%S", t);

    /* stdout */
    printf("[%s] ", ts);
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
    printf("\n");

    /* file */
    if (log_fp) {
        fprintf(log_fp, "[%s] ", ts);
        va_start(args, fmt);
        vfprintf(log_fp, fmt, args);
        va_end(args);
        fprintf(log_fp, "\n");
        fflush(log_fp);
    }
}

void ms_sleep(int ms) {
    struct timespec ts;
    ts.tv_sec  = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
}

const char *customer_type_str(int type) {
    switch (type) {
        case REGULAR:   return "Regular";
        case PREMIUM:   return "Premium";
        case LOAN:      return "LoanApplicant";
        case CORPORATE: return "Corporate";
        case VIP:       return "VIP";
        default:        return "Unknown";
    }
}

const char *transaction_type_str(int type) {
    switch (type) {
        case DEPOSIT:    return "Deposit";
        case WITHDRAWAL: return "Withdrawal";
        case LOAN_REQ:   return "LoanRequest";
        case PAYROLL:    return "Payroll";
        default:         return "Unknown";
    }
}
