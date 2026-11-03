# Phase 7 — The minimal POSIX subset

Prerequisite: PLAN D128 (the subset and its reasons, including capture-only
`std::stacktrace`), issue #106 (what a hosted libstdc++ needs), Q37 and Q38.
Targets: every architecture in `ARCHS`.

**Goal:** give pi-pico and a hosted libstdc++ the POSIX names they need, and
nothing more, with every operation reaching the OS through the existing RTOS
seam.

**Every step's definition of done (DoD)** is the Phase 1 DoD, plus:

- `test/unit` cases for every name the step adds, on every architecture
  (D126);
- a research note (`docs/research/posix-subset.md`, or a section of it);
- both size budgets holding for the reference images, none of which uses the
  subset.

---

## P7.01 — Research and the descriptor model  `[x]`
- [x] `docs/research/posix-subset.md`:
  - POSIX.1-2024 for each header and function in D128;
  - how newlib and picolibc provide them (issue #106's survey);
  - what pi-pico's VFS (`posix/posix-io.c`) and libstdc++'s `basic_file_stdio.cc` expect.
- [x] Answer Q37 (the seam handle as the descriptor; 0, 1 and 2 for the standard streams) and Q38 (whether the library defines `open`, `close`, `read` and `write` weakly, or only declares them): D129, D130.
- [x] Re-run the pi-pico scan against `origin/main` (`a4055c6`: unchanged), and read libstdc++'s configure and I/O backend (GCC 16 branch) to confirm the D128 list. A real configure run needs the toolchain patch of Q39 and moves to P7.05.

## P7.02 — Headers  `[x]`
- [x] `<sys/types.h>`: `off_t`, `ssize_t`, `mode_t`, `pid_t` and `clockid_t`, with ABI-identical types on every core of a triple (D15, D131).
- [x] `<unistd.h>`:
  - `STDIN_FILENO`, `STDOUT_FILENO`, `STDERR_FILENO` and the `SEEK_*` constants;
  - declarations of `read`, `write`, `close` and `lseek`. The optional functions are declared in their own headers (`<time.h>`, `<sched.h>`) when P7.03 implements them.
- [x] `<fcntl.h>`: the D128 `O_*` flags (Linux values) and `open`. `fcntl` comes with its `F_*` commands in P7.04.
- [x] `<strings.h>` (`strcasecmp`, `strncasecmp`), `<sched.h>` (`struct sched_param`, the `SCHED_*` policies, `sched_yield`).
- [x] The extra `errno` codes (`EBADFD`, `EFTYPE`, `EHOSTDOWN`, `ENOTUNIQ`, `ESHUTDOWN`, `ESOCKTNOSUPPORT`), with `strerror` messages (+181 bytes, `docs/size/<arch>.md`).
- [x] Each header in `test/unit/api`'s standalone `-pedantic-errors` compile, its types and values checked there, and as C++17 and `gnu++98` in `test/unit/cxx` (D120, D131).
- [x] Fix #106 item 1: `errno`, and every header a C++98 program reaches, parses as `gnu++98` (D131).

## P7.03 — Library functions  `[x]`
- [x] `fileno`, `fdopen` and `lseek` over the seam handle (D129). `__libc_file_new` no longer closes the handle when it fails, so a failed `fdopen` leaves the descriptor to its caller.
- [x] `strnlen`, `strcasecmp`, `strncasecmp` (new `libc/strings`), `reallocarray` (`libc/malloc`, beside `calloc`), `sincos`, `sincosf` (over `sin` and `cos`).
- [x] The optional functions over the existing seam:
  - `clock_gettime` and `clock_getres` (`__rtos_clock_gettime`, `__rtos_clock_getres`), with `clockid_t` and the `CLOCK_*` clocks in `<time.h>` (pi-pico uses `CLOCK_MONOTONIC`);
  - `nanosleep` (`__rtos_thread_sleep`);
  - `sched_yield` (`__rtos_thread_yield`, new `libc/sched`);
  - `getentropy` is not provided: no port has an entropy source, and the seam would need a new call; `isatty` is deferred until a consumer needs it (P7.01 found none).
- [x] `open`, `close`, `read` and `write` over the seam, one per file (new `libc/fcntl` and `libc/unistd`), replaceable by a port through archive order (D130); `ioctl` and `poll` stay the port's own.
- [x] The ISO headers declare the non-ISO names only outside strict C23, or when a feature test macro asks (D132).
- [x] Tests: `test/unit/posix` (new), `test/unit/none`, `test/unit/api` (`posix.c`, and the strict-mode namespace check).

## P7.04 — `std::stacktrace`, capture only  `[x]`
Frames are captured as addresses on the target, then symbolized on the host
with `addr2line` against the ELF, as pi-pico's `sys/backtrace` output is
today. libstdc++'s libbacktrace is built against the library, but never opens
the ELF on the target (D128).
- [x] Research section in `docs/research/posix-subset.md`:
  - how libstdc++ builds `<stacktrace>` (`--enable-libstdcxx-backtrace`, libbacktrace in `libstdc++exp.a`, `_Unwind_Backtrace`);
  - which libc names libbacktrace needs to compile and link;
  - what happens when it finds no executable to read (no `/proc/self/exe`, no `getexecname`).
  *Against this library libstdc++'s configure selects libbacktrace's `alloc.c` (malloc) and `read.c` (no `<sys/mman.h>`), so it needs `open`, `close`, `read`, `lseek`, `fstat`, `fcntl` (`F_SETFD`, `HAVE_FCNTL` is forced for target builds) and `getpid` (`fileline.c`, always), besides string, stdio and malloc functions the library has. The GCC 16 libbacktrace sources (`alloc`, `read`, `elf`, `posix`, `fileline`, `dwarf`, `state`, `backtrace`, `simple`, `sort`, `atomic`) compile against the library's headers in GCC's default C mode. Linking and running them was not done: running externally fetched code needs the user's permission in this session. Without an executable, `backtrace_pcinfo` and `backtrace_syminfo` report an error once and return, and capture is unaffected.*
- [x] `<sys/stat.h>`: `struct stat` (every POSIX field; `fstat` fills `st_mode`, `st_size` and `st_nlink`), the `S_IS*` macros and `S_I*` bits, and `fstat`. A handle that seeks is a regular file whose size is its end; one that cannot (`ESPIPE`) is a character device of size 0. `<fcntl.h>` gains `O_CLOEXEC`, `F_GETFD`, `F_SETFD`, `FD_CLOEXEC` and `fcntl` (descriptor flags only, with no effect), `<unistd.h>` `getpid` (1), and `<sys/types.h>` `struct stat`'s field types (D134).
- [x] An optional unwind-tables build of the library: `LIBC_UNWIND_TABLES=1` adds `-funwind-tables` to `LIBC_CFLAGS` (`tools/makefiles/common.mk`). The default build, and with it both size budgets and the baselines, stays without unwind tables. Cost, debug images on armv7e-m: libgcc's EHABI unwinder and personality routine (about 3.4 KB of code, and `abort`, so even a freestanding program needs `__rtos_abort`) plus the tables (1.4 KB for `smoke`, 4.4 KB for `stdio`). `make check` passes with the option on armv7e-m and armv6-m.
- [x] `__exidx_start` and `__exidx_end`: come from the default linker script for the Linux test port. `port/none` has no linker script of its own (the board's provides the memory map), so `port/none/rtos-none.h` states the contract and the section statement a board script needs.
- [x] `test/unit/unwind` (Arm), in place of the planned `test/unit/cxx` stack trace (D134):
  - `_Unwind_Backtrace`, as `std::stacktrace::current()` calls it, returns the test's own frames;
  - through `qsort`'s comparison callback the walk reaches `main` in a `LIBC_UNWIND_TABLES=1` build, and stops cleanly at the library frame (`EXIDX_CANTUNWIND`) in the default one;
  - `__exidx_start` < `__exidx_end`.

  The toolchain's `libstdc++exp.a` has `<stacktrace>`, but its headers are configured for newlib and need a complete `<wchar.h>`, which is outside this library's scope (§1), so `#include <stacktrace>` cannot compile against the library's headers. The `std::stacktrace` test moves to P7.05, with the libstdc++ built against the library.
- [x] The toolchain side, `--enable-libstdcxx-backtrace` in sgstreet/rtoskit-gcc#7: moved to P7.05 with the rest of the hosted libstdc++ configure.

## P7.05 — Consumers  `[ ]`
- [ ] **pi-pico:** a build against the library, with its stdio glue rewritten onto the seam (in pi-pico, not here). Record any name the build still lacks as a Q entry.
- [ ] **Hosted libstdc++:** configure and build it against the library with the `stdio` backend and Q39's configure route (the toolchain side is sgstreet/rtoskit-gcc#7), and record which optional features configure turned on.
- [ ] **`std::stacktrace`** (from P7.04, D134): with that libstdc++, `--enable-libstdcxx-backtrace`, a `test/unit/cxx` case linked with `-lstdc++exp` and a `LIBC_UNWIND_TABLES=1` library: `std::stacktrace::current()` returns the test's frames and one inside the library (a `qsort` callback), and `to_string()` runs without symbol information. Reported as skipped without `libstdc++exp`, as `bitint` is (D127).
- [ ] **Pre-C23 C modes** (Q40): pi-pico builds with `-std=gnu11`.
- [ ] Close issue #106.
