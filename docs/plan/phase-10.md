# Phase 10 — POSIX threads, and `<threads.h>` on top

Prerequisite: issue #126; PLAN D43 (the thread seam), D128 (the POSIX
subset), D145 (`call_once` and exceptions), D148 (this phase).
Targets: every architecture in `ARCHS`; the Linux test port and `port/none`.

**Goal:** `<pthread.h>` in `libc.a`, written once over the RTOS seam, and
C11 `<threads.h>` as a thin layer over it, so pi-pico can drop its own
`rtos/pthread` and `rtos/threads`, and the toolchain's libstdc++ can use the
`posix` thread model.

**Out of scope** (D148): cancellation (`pthread_cancel` and cancellation
points), `pthread_kill` and signals, `pthread_atfork`, process-shared
objects (`PTHREAD_PROCESS_SHARED` returns `ENOTSUP`), robust mutexes.

**Every step's definition of done:**
- the research note's edge cases for the step in `test/unit/pthread` (new),
  on the Linux port (x86_64 and qemu-arm) and `port/none`;
- `test/unit/threads` and `test/unit/none` pass unchanged; no test is
  weakened;
- `make check` everywhere and libgcc-check clean;
- `size-report diff` and both budgets: a program that uses neither threads
  API pays nothing, and growth in one that uses `<threads.h>` is recorded
  and justified in `docs/size/`;
- one function per file, `<pthread.h>` parses as `gnu++98` (D131) and is in
  the standalone header test (D122).

---

## P10.01 — Research and decisions  `[x]`
- [x] `docs/research/pthreads.md`: why, sources (POSIX via the man-pages,
  musl, GCC 16 `gthr-posix.h`, pi-pico's `rtos/pthread`, FreeRTOS-Plus-POSIX,
  Zephyr), findings, edge cases and test vectors.
- [x] PLAN D148: the phase, its layering and scope (supersedes D128's
  "`<pthread.h>` stays pi-pico's" and D43's direct `<threads.h>`).
- [x] Open questions for the seam: Q45 (priorities), Q46 (the clock of a
  timed wait), Q47 (cancellation for pi-pico), answered by D149, D150 and
  D151.

## P10.02 — Seam: attributes, exit value, priorities, clocks  `[x]`
Q45 and Q46 answered: D149 (a range per policy, real round-robin), D150 (`__rtos_futex_wait` takes a time base).
- [x] `__rtos_thread_create(thread, attr, entry, arg)` takes a
  `struct __rtos_thread_attr` (stack size, a caller's stack, policy,
  priority, inherit); a null attribute is the port's default and inherits
  the creator's scheduling.
- [x] The exit value is `void *` (`__rtos_thread_exit`, `__rtos_thread_join`,
  which accepts a null result pointer); C11's `int` travels as `intptr_t`.
- [x] Priorities (D149): `__rtos_sched_priority_range(policy, &min, &max)`,
  `__rtos_thread_get_sched`/`set_sched` in POSIX numbers. The Linux test
  port reports Linux's ranges (`SCHED_FIFO` and `SCHED_RR` 1–99,
  `SCHED_OTHER` 0) and records the setting without applying it: it runs
  unprivileged, where the kernel refuses the real-time policies.
  `port/none` has `SCHED_OTHER` at 0 only.
- [x] `__rtos_futex_wait(address, expected, base, deadline)` (D150): the Linux
  port waits with `FUTEX_WAIT_BITSET` on `CLOCK_REALTIME` for `TIME_UTC` and
  `CLOCK_MONOTONIC` for `TIME_MONOTONIC`; both ports reject another base and
  a bad `tv_nsec` with `-EINVAL`.
- [x] Both ports; `<threads.h>` adapted to the new seam with no change in
  behaviour; `include/rtos/rtos-port.h` and PLAN §4 updated.
- [x] Tests: `test/unit/pthread` (new; the seam's ranges, inherited and
  explicit scheduling, a refused priority, the caller's stack, the `void *`
  result, both time bases), `test/unit/none` (the one-thread answers).

## P10.03 — `<pthread.h>` and the thread functions  `[ ]`
- [ ] `<pthread.h>` with every type sized for the largest core of a triple
  (D15), and the `<limits.h>`/`<unistd.h>` constants and option macros for
  what is provided (`PTHREAD_KEYS_MAX`, `PTHREAD_DESTRUCTOR_ITERATIONS`,
  `PTHREAD_STACK_MIN`, `_POSIX_THREADS`, `_POSIX_TIMEOUTS`, and
  `_POSIX_THREAD_PRIORITY_SCHEDULING` if Q45 provides it).
- [ ] `pthread_create`, `join`, `detach`, `exit`, `self`, `equal`; the
  `pthread_attr_*` set; `pthread_setschedparam`/`getschedparam`,
  `pthread_setschedprio`; `sched_get_priority_max`/`min`.
- [ ] `pthread_once`, exception-safe as D145's `__libc_call_once_eh`.

## P10.04 — Mutexes, condition variables, keys  `[ ]`
- [ ] `PTHREAD_MUTEX_NORMAL`, `RECURSIVE`, `ERRORCHECK`, `DEFAULT`; `lock`,
  `trylock`, `timedlock`, `clocklock`, `unlock`, `init`, `destroy`, static
  initialisers; `pthread_mutexattr_*`.
- [ ] `pthread_cond_wait`, `timedwait`, `clockwait`, `signal`, `broadcast`;
  `pthread_condattr_setclock`/`getclock`.
- [ ] `pthread_key_create`, `delete`, `getspecific`, `setspecific`, with
  destructor passes up to `PTHREAD_DESTRUCTOR_ITERATIONS`.

## P10.05 — Priority-inheritance mutexes  `[ ]`
- [ ] A seam pair for PI locking (Linux `FUTEX_LOCK_PI`/`UNLOCK_PI`;
  `port/none` trivially), and `PTHREAD_PRIO_INHERIT` through
  `pthread_mutexattr_setprotocol` (`ENOTSUP` where the port has none).

## P10.06 — Read-write locks, barriers, spin locks  `[ ]`
- [ ] `pthread_rwlock_*` (with `timedrdlock`/`timedwrlock`/`clock*`),
  `pthread_barrier_*` (`PTHREAD_BARRIER_SERIAL_THREAD`), `pthread_spin_*`.

## P10.07 — `<threads.h>` over `<pthread.h>`  `[ ]`
- [ ] `thrd_*`, `mtx_*`, `cnd_*`, `tss_*` and `call_once` as wrappers with
  C11's return codes; the ABI change recorded (toolchain and pi-pico
  rebuild together).
- [ ] Size: `<threads.h>` programs measured against phase 9's baselines.

## P10.08 — Consumers and wrap-up  `[ ]`
- [ ] The libstdc++ side for rtoskit-gcc, recorded here and in
  `docs/research/libstdcxx-features.md`: `--enable-threads=posix`,
  `_GLIBCXX_GTHREAD_USE_WEAK 0`, and `<rtoskit-libc.h>` features for
  `pthread_cond_clockwait`/`pthread_mutex_clocklock`.
- [ ] What pi-pico drops (`rtos/pthread`, `rtos/threads`, the cancellation
  declarations, D151) and what its port must implement.
- [ ] Phase exit: PLAN's phase table, decisions, size and docs.
