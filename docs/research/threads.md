# Research: `<threads.h>`, `<stdatomic.h>`, real locks, `port/none`

Step: P1.12. Decisions: PLAN D4, D6, D10, D13, D15, D31, D43.

## Sources reviewed

- U. Drepper, *Futexes Are Tricky* (2011,
  <https://www.ic.unicamp.br/~islene/2s2013-mo806/futex/futex-are-tricky.pdf>):
  why a two-state futex mutex wakes needlessly and races, and the three-state
  mutex (0 unlocked, 1 locked, 2 locked with waiters) that only enters the
  kernel under contention; condition variables built on a sequence counter.
- Lockless Inc., *Mutexes and Condition Variables using Futexes*
  (<https://www.locklessinc.com/articles/mutex_cv_futex/>): the same designs,
  the thundering herd of a broadcast and why requeueing is an optimisation
  only.
- Fuchsia Zircon `sync/mutex.c`
  (<https://fuchsia.googlesource.com/fuchsia/+/HEAD/zircon/system/ulib/sync/mutex.c>):
  unlocking with an exchange instead of a decrement.
- Linux `Documentation/arm/kernel_user_helpers.rst`
  (<https://lxr.missinglinkelectronics.com/linux/Documentation/arm/kernel_user_helpers.rst>):
  `__kuser_cmpxchg` at `0xffff0fc0`, a 32-bit compare-and-swap with barriers
  for cores without exclusive access (helper version ≥ 2, Linux 2.6.12).
- Picolibc `doc/locking.md`
  (<https://codex.ac6.fr/Zephyr/xref/picolibc-3.5.0-3.4.0/doc/locking.md>) and
  Zephyr's `lib/libc/picolibc/locks.c`: retargetable recursive locks the
  RTOS supplies, with no-op stubs for single-threaded images; one global lock
  for malloc, atexit and the time zone; TLS `errno`.
- Newlib lock glue and FreeRTOS `configUSE_NEWLIB_REENTRANT`
  (<https://sourceware.org/pipermail/newlib/2018/016432.html>): the
  `__retarget_lock_*` hooks and the per-task `struct _reent` swapped at each
  context switch, the cost that `thread_local` state avoids (D6, D10).
- musl `src/thread/` (structure only): C11 threads as thin wrappers over the
  pthread core, `tss` on top of `pthread_key` with a generation-free table,
  `call_once` over `pthread_once`, and `__clone` in assembly per arch.
- GCC libatomic and the reports of missing `__atomic_*_N` on Cortex-M0
  (RIOT `core/atomic_c11.c`, the Raspberry Pi Pico SDK `pico_atomic.c`, GNU Arm
  Embedded bug 1898209, the GCC `bitint-100.c` test discussion): armv6-m has
  no `LDREX`/`STREX`, so GCC calls `__atomic_compare_exchange_4`,
  `__atomic_fetch_add_4`, ...; bare-metal runtimes provide them by masking
  interrupts.
- The standard: C23 7.17 (`<stdatomic.h>`), 7.28 (`<threads.h>`), 7.24
  (`call_once` and `once_flag` also in `<stdlib.h>`).

Measured with GCC 16.2 (`arm-none-eabi`): armv6-m calls the `__atomic_*`
library for every read-modify-write of every size (`__GCC_ATOMIC_*_LOCK_FREE`
is 1); armv7e-m and armv8-m.main inline 1-, 2- and 4-byte operations with
`LDREX`/`STREX` but call the library for 8-byte ones. The library call of a
compare-exchange takes five arguments (pointer, expected, desired, success
order, failure order; no `weak`). `qemu-arm -cpu max` implements the kuser
helpers (checked with a stand-alone program).

## The thread seam

| Choice | Chosen | Rejected |
|---|---|---|
| Blocking primitive | A futex-like wait/wake pair: `__rtos_futex_wait(address, expected, deadline)` blocks while `*address == expected` (deadline: absolute `TIME_UTC`, or none), `__rtos_futex_wake(address, count)`. Mutexes, condition variables and `call_once` are written once in libc on top of it and atomics. | Native RTOS mutex/condition objects behind the seam: more seam functions, `mtx_t` would hold a pointer to port-allocated storage (allocation in `mtx_init`), and every port would reimplement condition variables, which few RTOSes have. Any RTOS can implement wait/wake with a wait queue keyed by address (rtoskit's scheduler is futex based, CMSIS-RTOS2 ports can use thread flags). |
| Threads | `__rtos_thread_create(&thread, entry, arg)`, `__rtos_thread_exit(result)`, `join`, `detach`, `self`, `yield`, `sleep(duration, remaining)`. `struct __rtos_thread` is opaque; `thrd_t` is a pointer to it, one size for every port (D15). The port allocates the stack and the TLS block (`__libc_tls_block_size`/`__libc_tls_init`, D10). | Exposing stack sizes or priorities: C has no way to pass them. |
| Seam locks (D31) | Unchanged; the Linux port now implements them as recursive futex mutexes, `port/none` keeps no-ops. | — |
| Atomics library | libc provides the `__atomic_*` library functions (sizes 1, 2, 4, 8 and the generic ones) for operations the compiler cannot inline. Sizes 1, 2 and 4 use a port word compare-and-swap, `__rtos_atomic_cas(address, expected, desired)`, on the aligned word holding the object; 8-byte and odd-sized operations run inside a port critical section, `__rtos_atomic_enter()`/`__rtos_atomic_exit(state)`. | Linking GCC's libatomic: not built for `arm-none-eabi`, and it uses pthread locks. A lock for every size: wrong on armv6-m, see below. Per-address lock tables: more RAM, no gain with one core. |

### Mutex, condition variable, `call_once`

- Mutex word: 0 unlocked, 1 locked, 2 locked with possible waiters. Lock:
  compare-exchange 0→1; otherwise exchange to 2 and wait while it reads 2,
  until an exchange returns 0. Unlock: exchange to 0; wake one waiter if the
  old value was 2. Timed lock passes the deadline to the wait and gives up on
  `-ETIMEDOUT`. Recursive mutexes keep an owner (`thrd_current()`) and a
  count; plain ones keep the owner too, for `cnd_wait` and diagnostics.
- Condition variable: a sequence counter. Wait reads it, unlocks the mutex,
  waits while it is unchanged, relocks. Signal and broadcast increment it and
  wake one or all. A wake can be spurious (C allows "an unspecified reason").
  The counter wraps after 2^32 signals; a waiter that sleeps through exactly
  2^32 signals misses one wake, which is harmless for condition variables.
- `call_once`: 0 not run, 1 running, 2 running with waiters, 3 done. The
  caller that moves 0→1 runs the function, stores 3 with release order and
  wakes all if it saw 2.

### Thread exit and thread-specific storage

- `thrd_create` passes the port a libc trampoline and a small heap record
  holding `func` and `arg`; returning from `func` is `thrd_exit(result)`.
- `thrd_exit` runs the `tss` destructors (at most `TSS_DTOR_ITERATIONS`, 4,
  passes) and decrements the count of live threads; when it reaches zero the
  program calls `exit(EXIT_SUCCESS)` (7.28.5.5p5), otherwise the port ends
  the thread.
- `tss_t` is an index into a table of 32 keys. Each key has a generation
  (odd while in use) and a destructor; each thread has a `thread_local`
  value and generation per key. A value whose generation differs from the
  key's reads as null, which gives "null in all existing threads" for a new
  key without touching other threads. The destructor pass is linked only
  when `tss_create` is (a weak reference from `thrd_exit`, D13).

## `<stdatomic.h>`

Built on the GCC `__atomic` builtins (D8): every type of 7.17.6 including
`atomic_char8_t`, the lock-free macros from `__GCC_ATOMIC_*_LOCK_FREE`,
`memory_order` from `__ATOMIC_*`, generic functions as macros (allowed by
7.17.1p7), and real functions for `atomic_thread_fence`,
`atomic_signal_fence` and the `atomic_flag` functions. `ATOMIC_VAR_INIT` was
removed in C23 and is not defined.

Out-of-line atomics policy (armv6-m, and 8-byte operations everywhere on Arm).
GCC inlines 1-, 2- and 4-byte atomic loads and stores on armv6-m (plain
accesses with `DMB`) but calls the library for every read-modify-write. A
lock-based read-modify-write is then not atomic against another thread's
inline store: an `atomic_flag_clear` that lands between a library
test-and-set's read and write is lost, and the flag stays set. The first
version used one lock for every size; `test/unit/atomic` hung on
armv6-m under qemu (whose vCPUs run in parallel) exactly this way. So:

- 1, 2 and 4 bytes: a compare-and-swap loop on the naturally aligned word
  holding the object (other bytes of the word only cause a retry), through
  `__rtos_atomic_cas`, which must be atomic against plain stores. Linux: the
  kernel's `__kuser_cmpxchg` on Arm, `lock cmpxchg` on x86_64. `port/none`:
  `PRIMASK` masking on Arm (correct on one core; a multi-core armv6-m part
  would need a hardware spin lock and still could not order against plain
  stores of the other core), `lock cmpxchg` on x86_64.
- 8 bytes and odd sizes: the compiler never inlines any access to them on
  these targets, so a lock is enough: the critical section. `port/none` masks
  interrupts (`PRIMASK`, `cli`), so an interrupt handler using a 64-bit
  atomic cannot deadlock against the thread; the Linux port uses one spin
  lock (kuser compare-and-swap on Arm, an inline exchange on x86_64) and
  yields while it is taken.

`__atomic_is_lock_free` reports true for naturally aligned 1-, 2- and 4-byte
objects and false otherwise. A signal handler that interrupts a library
8-byte atomic and performs another can deadlock on Linux; signals are a
test-port concern only.

## Ports

- **Linux** (test port): `clone` with `CLONE_VM | CLONE_FS | CLONE_FILES |
  CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM | CLONE_SETTLS |
  CLONE_CHILD_CLEARTID`, one `mmap` per thread holding the stack (256 KiB),
  the thread record and the TLS block. Join waits on the kernel-cleared tid
  (a shared futex, which is what `CLONE_CHILD_CLEARTID` wakes), then unmaps.
  A detached thread cannot unmap its own stack, so it parks its record on a
  list that the next create or join reclaims. Futexes are `futex` (x86_64)
  and `futex_time64` (Arm) with `FUTEX_WAIT_BITSET | FUTEX_CLOCK_REALTIME`
  for absolute deadlines; sleep is `clock_nanosleep` (`_time64` on Arm).
- **none** (bare metal, single thread): `__rtos_none_start(image)` places the
  one TLS block in a static 1 KiB buffer, installs the thread pointer
  (`__rtos_none_set_tp`: a variable `__aeabi_read_tp` returns on Arm,
  `wrfsbase` on x86_64) and runs `__libc_start_main`. Thread creation fails
  (`-ENOSYS` → `thrd_error`); a futex wait returns at once if the value
  changed, spins otherwise (only an interrupt handler could change it), and
  times out at once without a clock; `thrd_sleep` busy-waits on the board's
  monotonic clock if there is one. `test/unit/none` runs such a program as a
  Linux process: it supplies `_start`, output, exit and a heap as a board
  would, and replaces the privileged hooks (`PRIMASK`, `cli`, `wrfsbase`)
  that a process cannot execute.

## Edge cases and test vectors

- Thread-local `errno`, `strtok`, `rand` and aligned, initialised
  `thread_local` objects in new threads.
- Results through `thrd_join`; `thrd_detach` before and after the thread
  ends; many short-lived detached threads (reclamation).
- Mutex counters under contention, recursion depth, `mtx_trylock` busy from
  another thread, `mtx_timedlock` timing out.
- Condition variables: producer/consumer, broadcast to all waiters,
  `cnd_timedwait` timing out.
- `call_once` raced by several threads; `tss` destructors that set values
  again (bounded passes), `tss_delete` and key reuse.
- 8-byte and odd-sized atomics from several threads (library path on every
  target); `atomic_flag` as a spin lock.
- `thrd_exit` from `main` while other threads run: the program ends with
  status 0 after the last one, running the `atexit` handlers.
- Concurrent `malloc`/`free` and concurrent `fprintf` to one stream (lines
  never interleave).
- `port/none`: a program linked with `librtos-none.a` runs, prints through
  `__rtos_none_output`, allocates, and gets `thrd_error` from `thrd_create`.
- A TLS image whose address is not a multiple of its alignment
  (`p_vaddr % p_align != 0`). GNU ld always aligns the TLS segment, so
  `test/unit/threads` builds such an image by hand for `__libc_tls_init`.

## `call_once` and exceptions (issue #121, PLAN D145)

C++ requires that an exception from `std::call_once`'s callable propagates to
the caller and leaves the flag unset, so a later call runs the callable again
([thread.once.callonce]); glibc's `pthread_once` does this with a cancellation
cleanup handler, and libstdc++'s own futex-based `call_once` (Linux) resets its
state in a destructor. With `--enable-threads=c11`, libstdc++ calls the C
library's `call_once` through `__gthread_once`, so the exception unwinds
through a C frame.

- **Unwinding through C** needs the frame's unwind table, and a cleanup needs
  a landing pad: `-fexceptions` with `__attribute__((cleanup))`. The landing
  pad calls `_Unwind_Resume` and the frame names `__gcc_personality_v0`
  (libgcc's `unwind-c.o`), both strong references.
- **Cost if `call_once` carried them:** every C program that calls it links
  libgcc's unwinder, measured +4,426 bytes of code on armv7e-m
  (`test/unit/none`) and +4,428 (`test/unit/threads`); on x86_64 the host
  libgcc keeps the unwinder in `libgcc_eh.a`, which the test link lacks.
- **Weak references do not work:** a weak `__gcc_personality_v0` resolves to
  null in a C++ program, since libsupc++ is C++ and never references the C
  personality (checked with `nm` on GCC 16.2's `libsupc++.a`), so the unwinder
  would call address 0 when it reaches the frame.
- **Chosen:** a second entry point, `__libc_call_once_eh`, the only object
  built with `-fexceptions`; the toolchain's `__gthread_once` calls it. Both
  entry points use one state machine (`libc/internal/once.h`): a waiter that
  wakes to find the flag back at NOT_RUN claims it and runs the function.
- **`longjmp`** out of the function runs no cleanup; the flag stays RUNNING.
  C23 does not address it, and POSIX leaves `pthread_once` the same way.

Test vectors (`test/unit/cxx`, `regression_call_once_exception`): the first of
three calls throws and the second runs the function (two tries in all); a
thread blocked on the flag (state 2, through plain `call_once`) while the
runner throws is woken and runs the function itself. Without `-fexceptions` on
`__libc_call_once_eh` the test program aborts, as the issue reported.
