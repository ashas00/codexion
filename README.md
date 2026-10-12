*This project has been created as part of the 42 curriculum by olprymas.*

# Codexion

## Description

Codexion is a multithreaded C simulation of concurrent resource sharing. Coders
sit around a circular co-working hub and must acquire two shared **USB dongles**
(one on each side) to compile quantum code. Each coder repeatedly compiles,
debugs and refactors; a coder that fails to start a new compile before its
burnout deadline is considered burned out, and the simulation stops.

The goal of the project is to practice POSIX thread programming without any
global variables: mutual exclusion, condition variables, deadlock avoidance,
fair scheduling (FIFO and Earliest-Deadline-First) and precise, race-free
logging.

## Instructions

### Build

```
make        # builds the codexion binary
make clean  # removes object files
make fclean # removes object files and the binary
make re     # fclean + all
```

Compiled with `cc -Wall -Wextra -Werror -pthread`.

### Run

```
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Meaning |
|---|---|
| `number_of_coders` | number of coders, and also the number of dongles |
| `time_to_burnout` | ms since the start of a coder's last compile before it burns out |
| `time_to_compile` | ms spent compiling, holding both dongles |
| `time_to_debug` | ms spent debugging |
| `time_to_refactor` | ms spent refactoring |
| `number_of_compiles_required` | the simulation stops once every coder has reached this many compiles |
| `dongle_cooldown` | ms a dongle stays unavailable after being released |
| `scheduler` | `fifo` or `edf` — arbitration policy used when several coders want the same dongle |

Example:

```
$ ./codexion 5 800 200 200 200 7 100 edf
0 1 has taken a dongle
0 1 has taken a dongle
0 1 is compiling
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
0 5 has taken a dongle
200 1 is debugging
...
```

All eight arguments are mandatory and validated. Non-numeric or negative
values, a missing argument, and any `scheduler` value other than `fifo` or
`edf` are all rejected with an explicit error printed on stderr.

## Resources

- `man pthread_create`, `man pthread_mutex_init`, `man pthread_cond_init`,
  `man pthread_cond_timedwait` — the POSIX threading API used throughout the
  project.
- The **Dining Philosophers problem** — the classic formulation this project
  is modeled after (coders and dongles instead of philosophers and forks).
- E. G. Coffman, M. Elphick, A. Shoshani, *"System Deadlocks"*, ACM Computing
  Surveys, 1971 — the four necessary conditions for deadlock, used as a
  checklist when designing dongle acquisition.
- C. L. Liu, J. Layland, *"Scheduling Algorithms for Multiprogramming in a
  Hard-Real-Time Environment"*, JACM, 1973 — background on Earliest-Deadline-
  First scheduling.

**AI usage:** Claude (Anthropic) was used throughout this project for:

- Running `valgrind` (`--leak-check=full`, `--tool=helgrind`) and
  `norminette` and reporting the results.
- Drafting this README.

## Blocking cases handled

- **Deadlock prevention (Coffman's conditions).** A coder needing two dongles
  always acquires them in a fixed global order — by dongle id, lowest first —
  which breaks the *circular wait* condition: no cycle of coders each holding
  one dongle while waiting for another's can form. If the second dongle
  cannot be obtained (for example because the simulation is stopping), the
  first one is released immediately instead of being held indefinitely.
- **Starvation prevention.** Every dongle keeps its own list of waiting
  requests ordered by scheduling priority — arrival order for `fifo`, or
  deadline (`last_compile_start + time_to_burnout`) for `edf`. A coder is
  only granted the dongle once it reaches the front of that ordered list, so
  no coder can be indefinitely passed over in favour of others.
- **Cooldown handling.** Each dongle records the timestamp of its last
  release. Before re-granting it, the acquiring coder waits on
  `pthread_cond_timedwait` with a deadline computed from that timestamp plus
  `dongle_cooldown`, so a dongle is never re-issued early — while the waiting
  coder is still woken immediately, rather than only after the cooldown, if
  it stops being at the front of the queue or the simulation ends.
- **Precise burnout detection.** A dedicated monitor thread polls every 5 ms
  and, for every coder that has not yet met `number_of_compiles_required`,
  checks whether `now - last_compile_start >= time_to_burnout`. The burnout
  message is printed and the global `stop` flag is set inside the same
  `log_mutex` critical section, so the log line appears essentially at the
  moment the burnout is detected, and no coder thread can print a
  contradicting message afterwards.
- **Log serialization.** Every log line is printed while holding a single
  global `log_mutex`, so two messages from different threads can never
  interleave on the same line, and the timestamp printed is always taken
  while holding that lock, immediately before the `printf`.

## Thread synchronization mechanisms

| Mutex / condvar | Protects | Used by |
|---|---|---|
| `t_dongle.mutex` / `t_dongle.cond` (one pair per dongle) | `in_use`, `cooldown`, and the waiting list of *that* dongle | coder threads acquiring or releasing that dongle |
| `log_mutex` | every `printf` call, and the burnout message + `stop` flag together | every coder thread, the monitor thread |
| `stop_mutex` | the `stop` flag that tells every thread the simulation has ended | coder threads, monitor thread |
| `progress_mutex` | each coder's `compiles_done` / `last_compile_start`, and the shared `finished` counter | coder threads (write), monitor thread (read) |
| `seq_mutex` | the monotonically increasing `request_seq` used to order/tie-break scheduling requests | any coder computing a FIFO or EDF priority |

Each dongle is its own independent critical section: a coder wanting two
dongles takes them one at a time, each under its own `mutex`/`cond` pair, so
contention on one dongle never blocks unrelated dongles elsewhere in the
ring. `pthread_cond_wait` / `pthread_cond_timedwait` are used instead of
busy-waiting, so a coder queued for a dongle consumes no CPU while it waits.

The monitor thread and the coder threads communicate exclusively through
mutex-protected shared state — never through a signal or an unlocked read of
shared memory. `last_compile_start` is written by a coder thread right
before it starts compiling and read by the monitor thread on every poll,
both always under `progress_mutex`, so the monitor never observes a
half-written value. Likewise `stop` is only ever read or written under
`stop_mutex`, so a coder thread checking "should I keep going?" and the
monitor thread requesting a stop can never race on that flag. One mutex per
logically independent piece of shared state, and condition variables for
waiting instead of sleep-based polling loops (with the exception of the
monitor's own coarse-grained 5 ms scan, which is what bounds
burnout-detection latency) — this is what keeps every shared resource
(dongles, the log, per-coder progress) race-free without resorting to a
single global lock.

printf("%ld REQUEST coder=%d dongle=%d prio=%ld\n", diff_ms(&data->start_time), coder->id, dongle->id, priority);