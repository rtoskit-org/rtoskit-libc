# Phase 7 — The minimal POSIX subset

Prerequisite: PLAN D128 (the subset and its reasons), issue #106 (what a hosted
libstdc++ needs), Q37 and Q38.
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

## P7.01 — Research and the descriptor model  `[ ]`
- [ ] `docs/research/posix-subset.md`:
  - POSIX.1-2024 for each header and function in D128;
  - how newlib and picolibc provide them (issue #106's survey);
  - what pi-pico's VFS (`posix/posix-io.c`) and libstdc++'s `basic_file_stdio.cc` expect.
- [ ] Answer Q37 (the seam handle as the descriptor; 0, 1 and 2 for the standard streams) and Q38 (whether the library defines `open`, `close`, `read` and `write` weakly, or only declares them).
- [ ] Re-run the pi-pico scan against `origin/main`, and configure a hosted libstdc++ against the headers, to confirm the D128 list.

## P7.02 — Headers  `[ ]`
- [ ] `<sys/types.h>`: `off_t`, `ssize_t`, `mode_t`, `pid_t`, with ABI-identical types on every core of a triple (D15).
- [ ] `<unistd.h>`:
  - `STDIN_FILENO`, `STDOUT_FILENO`, `STDERR_FILENO` and the `SEEK_*` constants;
  - declarations of `read`, `write`, `close`, `lseek` and the optional functions.
- [ ] `<fcntl.h>`: the D128 `O_*` flags, `open`, `fcntl` declared.
- [ ] `<strings.h>`, `<sched.h>` (the `SCHED_*` policies, `sched_yield`).
- [ ] The extra `errno` codes (`EBADFD`, `EFTYPE`, `EHOSTDOWN`, `ENOTUNIQ`, `ESHUTDOWN`, `ESOCKTNOSUPPORT`).
- [ ] Each header in `test/unit/api`'s standalone `-pedantic-errors` compile, and as C++ in `test/unit/cxx` (D120).
- [ ] Fix #106 item 1: `errno` must parse in C++98.

## P7.03 — Library functions  `[ ]`
- [ ] `fileno`, `fdopen` and `lseek` over the seam handle (D128, Q37).
- [ ] `strnlen`, `strcasecmp`, `reallocarray`, `sincos`, `sincosf`.
- [ ] The optional functions over the existing seam:
  - `clock_gettime` (`__rtos_clock_gettime`);
  - `nanosleep` (`__rtos_thread_sleep`);
  - `sched_yield` (`__rtos_thread_yield`);
  - `isatty` and `getentropy`, with a seam addition only if Q37 or P7.01 shows one is needed.
- [ ] `open`, `close`, `read` and `write` as Q38 decides (`ioctl` and `poll` stay the port's own).

## P7.04 — Consumers  `[ ]`
- [ ] **pi-pico:** a build against the library, with its stdio glue rewritten onto the seam (in pi-pico, not here). Record any name the build still lacks as a Q entry.
- [ ] **Hosted libstdc++:** configure and build it against the library (the toolchain side is sgstreet/rtoskit-gcc#7), and record which optional features configure turned on.
- [ ] Close issue #106.
