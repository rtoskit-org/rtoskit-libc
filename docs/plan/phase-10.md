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

## P10.03 — `<pthread.h>` and the thread functions  `[x]`
- [x] `<pthread.h>` and `<__pthread_types.h>` (shared with `<sys/types.h>`),
  each type the same on every core of a triple (D15); `PTHREAD_KEYS_MAX` 32
  (below POSIX's 128, D152), `PTHREAD_DESTRUCTOR_ITERATIONS` 4,
  `PTHREAD_STACK_MIN` 512 in `<limits.h>`; `_POSIX_THREADS`,
  `_POSIX_TIMEOUTS`, `_POSIX_THREAD_ATTR_STACKADDR`/`STACKSIZE` and
  `_POSIX_THREAD_PRIORITY_SCHEDULING` in `<unistd.h>`.
- [x] `pthread_create`, `join`, `detach`, `exit`, `self`, `equal`; the
  `pthread_attr_*` set (detach state, stack size, a caller's stack,
  scheduling policy and parameter, inheritance, scope); `pthread_setschedparam`,
  `getschedparam`, `setschedprio`; `sched_get_priority_max`/`min`. `thrd_exit`
  and `pthread_exit` share `__libc_thread_exit`.
- [x] `pthread_once` on `call_once`'s state machine, table-free; the toolchain
  routes `std::call_once` to `__libc_call_once_eh` (D152, P10.08).
- [x] Tests: `test/unit/pthread` (create and join with a returned and a
  `pthread_exit` value; `EDEADLK`, `EINVAL` for joining a detached thread
  and detaching twice; every attribute's default and refused values; a
  detached-at-creation thread; explicit, out-of-range and inherited
  scheduling; `setschedprio`; the priority ranges; a caller's stack; four
  threads racing `pthread_once`; the option macros and limits),
  `test/unit/none` (`EAGAIN`, `EDEADLK`, `pthread_once`, the ranges),
  `<pthread.h>` in the standalone and `gnu++98` header tests.

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
