# Research: POSIX threads over the seam, and `<threads.h>` on top (issue #126, phase 10)

Step: P10.01. Decisions: PLAN D148 (this phase), D149–D151 (Q45–Q47: priorities, the futex clock, no cancellation). Builds on
D43 (the thread seam and `<threads.h>`, `docs/research/threads.md`), D128
(the POSIX subset), D121 (the C++ ABI hooks) and D145 (`call_once` and
exceptions).

## Why

- **One implementation, not two.** pi-pico (`origin/main` `a4055c6`) has its
  own `<pthread.h>` (`include/pthread.h`, `rtos/pthread/*.c`: 86 functions)
  over its scheduler, and its C11 `<threads.h>` (`rtos/threads/threads.c`)
  over that. D128 left `<pthread.h>` to pi-pico, so pi-pico would keep two
  thread layers next to this library's `<threads.h>`; every further port
  would write a third.
- **libstdc++'s `posix` thread model.** The toolchain's libstdc++ today has
  the `single` model (rtoskit-gcc `origin/main` `cdd8f82` passes no
  `--enable-threads`; `--enable-threads=c11` is proposed in rtoskit-gcc#2).
  With `posix`, `std::condition_variable` and `std::timed_mutex` wait on the
  steady clock through `pthread_cond_clockwait` and `pthread_mutex_clocklock`
  (configure's `GLIBCXX_CHECK_PTHREAD_COND_CLOCKWAIT` and
  `..._MUTEX_CLOCKLOCK` are compile tests against `<pthread.h>`), which
  issue #123 otherwise asks the C11 layer to grow.
- **What C11 cannot say:** thread stack size and priority, error-checking
  mutexes, priority-inheritance mutexes, read-write locks, barriers.

## Sources reviewed

- **POSIX.1-2024**, XBD `<pthread.h>` and XSH 2.9 (Threads). The Open Group
  pages are not reachable from this session (pubs.opengroup.org is outside
  the network policy); the requirements below come from the Linux
  man-pages (man7.org: `pthread_create(3)`, `pthread_mutex_lock(3)`,
  `pthread_key_create(3)`, `pthread_attr_setstacksize(3)`,
  `pthread_mutexattr_setprotocol(3)`, `pthread_once(3)`,
  `pthread_spin_init(3)`, `pthread_rwlock_rdlock(3)`,
  `pthread_setschedparam(3)`, `pthread_cancel(3)`), which cite it, and from
  the implementations. Each implementation step checks its function's
  errors against those pages again.
- **musl** (`src/thread/`, the kraj/musl mirror of git.musl-libc.org, read
  for structure only): C11 threads are wrappers over the pthread core
  (`thrd_create` calls `__pthread_create` with a C11 attribute and maps
  `EAGAIN` to `thrd_nomem`, anything else to `thrd_error`; `mtx_timedlock`
  maps `ETIMEDOUT` to `thrd_timedout`); keys are a fixed table of
  `PTHREAD_KEYS_MAX` destructors, and thread exit runs at most
  `PTHREAD_DESTRUCTOR_ITERATIONS` passes while values remain.
- **GCC 16 `libgcc/gthr-posix.h`**: the pthread functions libstdc++'s
  `posix` model calls (all through weak references when
  `GTHREAD_USE_WEAK`): `pthread_once`, `getspecific`, `setspecific`,
  `create`, `join`, `equal`, `self`, `detach`, `cancel`, `exit`,
  `key_create`, `key_delete`, `mutex_init/destroy/lock/trylock/timedlock/unlock`,
  `mutexattr_init/settype/destroy`, `cond_init/destroy/broadcast/signal/wait/timedwait`,
  `rwlock_rdlock/tryrdlock/wrlock/trywrlock/unlock`, `attr_init/destroy/setdetachstate`,
  `getschedparam`, `setschedparam`, `sched_get_priority_min/max`,
  `sched_yield`; plus `pthread_cond_clockwait` and
  `pthread_mutex_clocklock` when configure finds them.
  **`__gthread_active_p`**, outside glibc and Bionic, is true when a weak
  reference to `pthread_cancel` resolves. In a static link a weak
  reference pulls no archive member, so with this library a program would
  see threads as inactive (every libstdc++ mutex a no-op) unless something
  else linked `pthread_cancel`. The toolchain must therefore build
  libstdc++ with `_GLIBCXX_GTHREAD_USE_WEAK 0` (libstdc++'s
  `config/os/*/os_defines.h` sets it), which makes every call strong.
- **pi-pico** `rtos/pthread` (`a4055c6`, the consumer, read for its API):
  86 functions, including `pthread_cancel`/`setcancelstate`/`testcancel`,
  `pthread_setaffinity_np`, `pthread_setschedprio` and
  `PTHREAD_PRIO_INHERIT`; `PTHREAD_KEYS_MAX` 16,
  `PTHREAD_DESTRUCTOR_ITERATIONS` 4, `PTHREAD_STACK_MIN` 512. The thread
  control block lives in the thread's TLS block (`thread_local struct
  pthread`), joiners wait on a scheduler futex, absolute deadlines are
  turned into scheduler ticks, and memory comes from weak
  `_pthread_alloc`/`_pthread_release` hooks (default `calloc`). Outside
  `rtos/pthread` only `rtos/threads/threads.c` and the tests use pthreads;
  nothing calls `pthread_cancel`.
- **FreeRTOS-Plus-POSIX** `FreeRTOS_POSIX_pthread.c`: a heap-allocated
  control block per thread (`pvPortMalloc`), `xTaskCreate` with the
  attribute's stack size and priority, joinable threads parked on a
  semaphore until joined.
- **Zephyr** `lib/posix/pthread.c` (v3.7.0): a fixed pool of
  `CONFIG_POSIX_THREAD_THREADS_MAX` threads, stacks from
  `k_thread_stack_alloc`, and `posix_to_zephyr_priority` /
  `zephyr_to_posix_priority`, which map POSIX's "higher is more urgent"
  numbers onto the kernel's own range per policy.
- This library: `docs/research/threads.md` (D43), `include/rtos/rtos-port.h`,
  `port/linux` (a `clone`d thread per mapping, the `int` result in
  `struct __rtos_thread`), `port/none` (one thread).

## Findings

1. **The thread control block needs no allocation in libc.** If the seam
   carries a `void *` exit value instead of an `int`, `pthread_t` can stay
   the seam handle, as `thrd_t` is now: join returns the value, the
   detach state is the port's, and the per-thread data libc owns (keys)
   is already `thread_local`. C11's `int` result travels as
   `(void *)(intptr_t)`. FreeRTOS-Plus-POSIX and pi-pico allocate a block
   per thread; this avoids it.
2. **Attributes at creation.** `pthread_attr_t` needs stack size, a
   caller-provided stack, priority and policy (with
   `PTHREAD_EXPLICIT_SCHED`), and the detach state. The first four are the
   port's to apply, so `__rtos_thread_create` needs an attribute argument;
   the detach state can stay libc's (`create` then `detach`).
3. **Priorities.** POSIX numbers priorities so that higher is more urgent,
   within `sched_get_priority_min/max(policy)`; RTOSes differ (Zephyr maps
   both ways per policy). The seam needs the range and a get/set pair; the
   mapping is the port's (Q45).
4. **Clock for timed waits.** `pthread_cond_clockwait`,
   `pthread_mutex_clocklock` and `pthread_condattr_setclock` wait on
   `CLOCK_MONOTONIC` or `CLOCK_REALTIME`. `__rtos_futex_wait` takes a
   `TIME_UTC` deadline only, so the seam needs a clock base (Q46).
5. **Priority inheritance** needs the owner's identity in the lock word and
   a port that boosts it: Linux has `FUTEX_LOCK_PI`/`UNLOCK_PI` (the word
   holds the owner's TID), pi-pico's scheduler has PI futexes. A port
   without it reports `ENOTSUP` from
   `pthread_mutexattr_setprotocol(PTHREAD_PRIO_INHERIT)`.
6. **`pthread_once` and exceptions.** D145's `__libc_call_once_eh` is the
   pattern: `pthread_once` is what libstdc++'s `posix` model calls for
   `std::call_once`, so it must be the exception-safe one (built with
   `-fexceptions`), or C++ needs its own entry point as with C11.
7. **Cancellation** stays out (signals, cancellation points throughout
   stdio). libstdc++ does not need it once `_GLIBCXX_GTHREAD_USE_WEAK` is 0;
   pi-pico declares it but does not use it (Q47).
8. **ABI.** C11 types over pthread types change size and layout:
   `mtx_t`, `cnd_t`, `once_flag` and `thrd_t` (if `pthread_t` stops being
   the seam handle, which finding 1 avoids). The toolchain and pi-pico
   rebuild together; D15 holds (one layout for every core of a triple).

## Edge cases the tests must cover

- Mutex types: relocking a `PTHREAD_MUTEX_ERRORCHECK` mutex → `EDEADLK`;
  unlocking one the caller does not own → `EPERM`; recursive counts;
  `pthread_mutex_destroy` of a locked mutex → `EBUSY`; `trylock` → `EBUSY`.
- Timed waits: an absolute deadline already past → `ETIMEDOUT` at once; a
  bad `tv_nsec` → `EINVAL`; `clockwait` on `CLOCK_MONOTONIC` unaffected by
  a change of `CLOCK_REALTIME` (Linux port); an unsupported clock →
  `EINVAL`.
- Keys: `PTHREAD_KEYS_MAX` keys then `EAGAIN`; destructors run at most
  `PTHREAD_DESTRUCTOR_ITERATIONS` passes while a destructor sets values
  again; a deleted key reads as null in every thread.
- Threads: join of self → `EDEADLK`; join of a detached thread → `EINVAL`;
  `pthread_exit` from the initial thread lets the others finish (C11's
  last-`thrd_exit` rule, D43); a caller-provided stack; a stack below
  `PTHREAD_STACK_MIN` → `EINVAL`.
- `pthread_once` racing threads; an exception from the init routine (C++).
- `port/none`: `pthread_create` → `EAGAIN`; locks, keys, once and the
  timed waits work in the one thread.
- C11 through the new layer: `test/unit/threads` and `test/unit/none` pass
  unchanged.

## Plan

`docs/plan/phase-10.md`.
