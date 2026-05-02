# Concurrent Banking System

**CL2006 – Operating System Lab | Spring 2026 | FAST-NUCES Chiniot-Faisalabad | BS-4-B**

A full-scale C simulation of a concurrent banking environment built to demonstrate five foundational Operating System concepts: CPU Scheduling, Process Synchronization, Deadlock Avoidance, Inter-Process Communication, and Memory Management. Every module produces measurable output — Gantt charts, scheduling metrics, Banker's Algorithm traces, IPC logs, and page replacement comparisons.

---

## Read the Full Write-up

A detailed technical breakdown of every module, design decision, and result is published on Medium.

[Read on Medium →](https://medium.com/@zoha-ashraf)

---

## Modules

### 1. CPU Scheduling
Implements three classical scheduling algorithms on a set of customers with varying priorities and burst times.

- **FCFS** — processes served in order of arrival
- **Priority Scheduling** — VIP (priority 5) executes before Regular (priority 1)
- **Round Robin** — fixed time quantum of 2 ms, cycling until completion

Each algorithm outputs an ASCII Gantt chart and a full metrics table with waiting time, turnaround time, and averages.

### 2. Synchronization
Shared account balances are protected against race conditions using two POSIX primitives.

- **Mutex lock** — ensures only one thread modifies a balance at a time; demonstrated with two concurrent withdrawals from the same account
- **Semaphore (value = 2)** — throttles concurrent deposits to a maximum of two at once; the third blocks until a slot frees
- **Corporate payroll** — 10 threads deposit simultaneously into a single account; mutex ensures the final balance is always correct

### 3. Deadlock Avoidance — Banker's Algorithm
Loan requests are treated as resource allocation problems across three resource types: Credit Units, Tellers, and Vault Access.

- Safety Algorithm finds a safe execution sequence before any allocation is committed
- Resource-Request Algorithm tentatively allocates then rolls back if the resulting state is unsafe
- Four test cases demonstrated: two approved (safe state preserved) and two denied (exceeds max need or creates unsafe state)

### 4. IPC — Message Queues
Customer processes communicate with a bank server process using POSIX System V message queues, implementing the Producer-Consumer pattern.

- Five customer processes (producers) send transaction requests to the queue
- One bank server process (consumer) reads, processes, updates balances, and sends responses back
- Full request/response flow is logged with timestamps

### 5. Memory Management — Page Replacement
Customer account data is modeled as memory pages. When frames are full, a replacement algorithm evicts an existing page.

- **FIFO** — evicts the oldest loaded page
- **LRU** — evicts the least recently used page
- Compared across a 20-reference string on 3 frames and 4 frames, with page fault counts and hit ratios reported

---

## Results Summary

| Algorithm | Avg Waiting (ms) | Avg Turnaround (ms) |
|---|---|---|
| FCFS | 12.50 | 18.00 |
| Priority | 12.67 | 18.17 |
| Round Robin | 18.17 | 23.67 |

| Page Replacement | Faults (3 frames) | Hit Ratio |
|---|---|---|
| FIFO | 14 | 30.0% |
| LRU | 12 | 40.0% |

---

## How to Run

Requires GCC and a Linux environment (WSL works).

```bash
# Clone the repository
git clone https://github.com/ZohaAshraf/ConcurrentBankingSystem
cd ConcurrentBankingSystem

# Install dependencies (Ubuntu/WSL)
sudo apt update && sudo apt install gcc make -y

# Compile and run
make run

# View the runtime log
cat logs/banking_system.log
```

---

## Project Structure

```
ConcurrentBankingSystem_Group/
├── src/
│   ├── banking.h        # Shared types, constants, prototypes
│   ├── main.c           # Entry point — runs all modules
│   ├── accounts.c       # Account management with mutex/semaphore
│   ├── scheduling.c     # FCFS, Priority, Round Robin + Gantt charts
│   ├── sync.c           # Thread synchronization demos
│   ├── banker.c         # Banker's Algorithm (safety + request)
│   ├── ipc.c            # Message queues + fork-based processes
│   ├── memory.c         # FIFO and LRU page replacement
│   └── utils.c          # Logging and shared utilities
├── logs/                # Runtime log output
├── report/
│   └── Report.md        # Full technical report
└── Makefile
```

---

## Technologies

C · POSIX Threads · POSIX IPC · GCC · Linux · Make

---

## Author

**Zoha Ashraf**
BS Computer Science — FAST-NUCES Chiniot-Faisalabad

[LinkedIn](https://www.linkedin.com/in/zoha-ashraf) · [GitHub](https://github.com/ZohaAshraf) · [Medium](https://medium.com/@zoha-ashraf)
