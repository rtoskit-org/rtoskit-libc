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

## P7.03 — Library functions  `[ ]`
- [ ] `fileno`, `fdopen` and `lseek` over the seam handle (D129).
- [ ] `strnlen`, `strcasecmp`, `strncasecmp`, `reallocarray`, `sincos`, `sincosf`.
- [ ] The optional functions over the existing seam:
  - `clock_gettime` (`__rtos_clock_gettime`), with `CLOCK_REALTIME` and `CLOCK_MONOTONIC` in `<time.h>` (pi-pico uses `CLOCK_MONOTONIC`);
  - `nanosleep` (`__rtos_thread_sleep`);
  - `sched_yield` (`__rtos_thread_yield`);
  - `getentropy` only if a port supplies an entropy source (libstdc++'s `random_device`); `isatty` is deferred until a consumer needs it (P7.01 found none).
- [ ] `open`, `close`, `read` and `write` over the seam, one per file, replaceable by a port through archive order (D130); `ioctl` and `poll` stay the port's own.

## P7.04 — `std::stacktrace`, capture only  `[ ]`
Frames are captured as addresses on the target, then symbolized on the host
with `addr2line` against the ELF, as pi-pico's `sys/backtrace` output is
today. libstdc++'s libbacktrace is built against the library, but never opens
the ELF on the target (D128).
- [ ] Research section in `docs/research/posix-subset.md`:
  - how libstdc++ builds `<stacktrace>` (`--enable-libstdcxx-backtrace`, libbacktrace in `libstdc++exp.a`, `_Unwind_Backtrace`);
  - which libc names libbacktrace needs to compile and link;
  - what happens when it finds no executable to read (no `/proc/self/exe`, no `getexecname`).
- [ ] `<sys/stat.h>`: `struct stat` (at least `st_mode` and `st_size`), `mode_t`, the `S_IS*` macros and `S_I*` bits, and `fstat` declared. `fstat` reports the size from `__rtos_seek` to the end, or fails with `ENOSYS` if Q37 leaves no handle for it. `<fcntl.h>` gains `O_CLOEXEC`, `F_GETFD`, `F_SETFD` and `FD_CLOEXEC`, which libbacktrace's open path uses.
- [ ] An optional unwind-tables build of the library: a build variable (e.g. `LIBC_UNWIND_TABLES=1`, not `#if`) that adds `-funwind-tables` to `LIBC_CFLAGS`, so traces can pass through library frames (`qsort` callbacks, `abort`, `__assert_fail`). The default build, and with it both size budgets and the baselines, stays without unwind tables. The docs say what the option costs.
- [ ] `__exidx_start` and `__exidx_end`: come from the default linker script for the Linux test port; the `port/none` linker script gains them (D121's note).
- [ ] `test/unit/cxx` (Arm), built with unwind tables and linked with `-lstdc++exp` when the toolchain provides it:
  - `std::stacktrace::current()` returns at least the test's own frames;
  - the frames include one inside the library (a `qsort` comparison callback);
  - `to_string()` runs without symbol information.

  Without `libstdc++exp`, the test is reported as skipped, the same way as `bitint` (D127).
- [ ] The toolchain side, `--enable-libstdcxx-backtrace` in sgstreet/rtoskit-gcc#7: record the configure result.

## P7.05 — Consumers  `[ ]`
- [ ] **pi-pico:** a build against the library, with its stdio glue rewritten onto the seam (in pi-pico, not here). Record any name the build still lacks as a Q entry.
- [ ] **Hosted libstdc++:** configure and build it against the library with the `stdio` backend and Q39's configure route (the toolchain side is sgstreet/rtoskit-gcc#7), and record which optional features configure turned on.
- [ ] Close issue #106.
