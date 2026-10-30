# Size: armv7e-m-dp

Release build of `linux-armv7e-m-dp`, measured with `tools/scripts/size-report` (PLAN D51,
`docs/research/size-measurement.md`). The figures are in `baseline-armv7e-m-dp.json`;
`tools/scripts/size-report diff` compares a build with them. Each phase 2 step
records its reductions, and justifies any growth, in a section below.

## P2.01 baseline (end of phase 1)

`libc.a` (743 members), release build:

| text | rodata | unwind | data | bss | tdata | tbss |
|---:|---:|---:|---:|---:|---:|---:|
| 77666 | 8879 | 0 | 116 | 672 | 8 | 396 |

Reference images (`test/size`): code is text+rodata+unwind, RAM is data+bss, TLS is tdata+tbss per thread; the libc and port shares come from the link map:

| Program | code | RAM | TLS | libc code | libc RAM | libc TLS | port code | libgcc code | main stack |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| hello-puts | 2595 | 356 | 8 | 1738 | 224 | 4 | 818 | 0 | 136* |
| hello-printf-int | 8612 | 356 | 8 | 6963 | 224 | 4 | 818 | 756 | 1268+ |
| hello-printf-float | 8636 | 356 | 8 | 6963 | 224 | 4 | 818 | 756 | 1276+ |
| malloc-heavy | 2024 | 228 | 8 | 1106 | 144 | 4 | 716 | 0 | 184* |
| math-mix | 13708 | 200 | 4 | 11706 | 132 | 0 | 656 | 888 | 672* |

Worst stack depth in bytes. `*`: without the frames of assembly or libgcc helpers (leaf routines that push a few registers); `+`: a lower bound because of an indirect call, recursion, a dynamic frame or an unknown callee:

| Function | depth | Function | depth | Function | depth |
|---|---:|---|---:|---|---:|
| `printf` | 1252+ | `fprintf` | 1248+ | `snprintf` | 1244+ |
| `sscanf` | 3168+ | `fscanf` | 3176+ | `strtod` | 1084+ |
| `strtof` | 1084+ | `strtol` | 88* | `malloc` | 72* |
| `calloc` | 88* | `realloc` | 88* | `free` | 72* |
| `qsort` | 68+ | `bsearch` | 32+ | `strftime` | 400+ |
| `mktime` | 400* | `localtime` | 292* | `gmtime` | 44* |
| `fopen` | 120* | `fclose` | 88* | `fputs` | 128* |
| `puts` | 128* | `fgets` | 120* | `fflush` | 112* |
| `exit` | 88+ | `abort` | 48+ | `thrd_create` | 104* |
| `mtx_lock` | 72* | `cnd_wait` | 96* | `exp` | 144 |
| `log` | 168 | `pow` | 248 | `sin` | 240* |
| `cos` | 240* | `tan` | 288* | `atan2` | 560 |
| `lgamma` | 896 | `tgamma` | 896 | `erfc` | 304 |
| `cpow` | 1296* | `sinf` | 280* | `expf` | 152 |
| `powf` | 256 |  |  |  |  |

Largest members (text+rodata+unwind+data+bss+tdata+tbss), top 15:

| Member | text | rodata | unwind | data | bss | tdata | tbss |
|---|---:|---:|---:|---:|---:|---:|---:|
| `strerror.o` | 24 | 2371 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format.o` | 2200 | 96 | 0 | 0 | 0 | 0 | 0 |
| `strftime.o` | 1918 | 285 | 0 | 0 | 0 | 0 | 0 |
| `__libc_erfc_table.o` | 348 | 1824 | 0 | 0 | 0 | 0 | 0 |
| `__libc_log_core.o` | 652 | 1472 | 0 | 0 | 0 | 0 | 0 |
| `__libc_lgamma_core.o` | 1610 | 392 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format_float.o` | 1854 | 90 | 0 | 0 | 0 | 0 | 0 |
| `__libc_scan.o` | 1728 | 0 | 0 | 0 | 0 | 0 | 0 |
| `cpow.o` | 1474 | 16 | 0 | 0 | 0 | 0 | 0 |
| `__libc_exp_core.o` | 396 | 1024 | 0 | 0 | 0 | 0 | 0 |
| `__libc_atan2_core.o` | 752 | 528 | 0 | 0 | 0 | 0 | 0 |
| `__libc_strtofloat.o` | 1114 | 17 | 0 | 0 | 0 | 0 | 0 |
| `__libc_scan_float.o` | 1080 | 25 | 0 | 0 | 0 | 0 | 0 |
| `__libc_rem_pio2.o` | 934 | 160 | 0 | 0 | 0 | 0 | 0 |
| `__libc_exact_fma.o` | 992 | 0 | 0 | 0 | 0 | 0 | 0 |

## P2.02 — build and structure levers

Decision D52, `docs/research/size-levers.md`. Before → after, release build.

`libc.a`: code 86545 → 86987 (+442), RAM 788 → 792 (+4), TLS 404 → 366 (-38).

| Program | code | RAM | TLS | libc code | main stack |
|---|---|---|---|---|---|
| hello-puts | 2595 → 2279 | 356 → 156 | 8 → 8 | 1738 → 1464 | 136 → 136 |
| hello-printf-int | 8612 → 8296 | 356 → 156 | 8 → 8 | 6963 → 6689 | 1268 → 1268 |
| hello-printf-float | 8636 → 8320 | 356 → 156 | 8 → 8 | 6963 → 6689 | 1276 → 1276 |
| malloc-heavy | 2024 → 1982 | 228 → 80 | 8 → 8 | 1106 → 1066 | 184 → 184 |
| math-mix | 13708 → 13244 | 200 → 20 | 4 → 0 | 11706 → 11666 | 672 → 672 |

Members that grew, justified:

- `__libc_asctime.o` 294 → 502 (+208): formats by hand instead of calling `snprintf` (on Arm the `asctime` probe image shrinks by 5.7–6.2 KB).
- `__libc_assert_fail.o` 79 → 253 (+174): formats by hand instead of calling `snprintf` (on Arm the `assert` probe image shrinks by 6.4–6.6 KB).
- `__libc_atexit_run.o` 0 → 144 (+144): new: the atexit table (moved from `__libc_atexit_handlers.o`) and its runner.
- `__libc_quick_exit_handlers.o` 0 → 132 (+132): renamed from `__libc_atexit_handlers.o`, now only the `at_quick_exit` table.
- `raise.o` 60 → 188 (+128): now holds the signal table (moved from `__libc_signal_handlers.o`).
- `__libc_file_new.o` 84 → 108 (+24): closes the streams it made at exit (moved from `__libc_stdio_exit.o`).
- `abort.o` 12 → 20 (+8): tests the weak reference to `raise`.
- `__libc_stdio_flush_all.o` 130 → 134 (+4): tests the weak references to `stdout` and `stderr`.
- `exit.o` 44 → 48 (+4): tests the weak reference to `__libc_atexit_run`.
- `__libc_file_write.o` 280 → 282 (+2): calls `__libc_stdio_flush_all` directly.
- Removed: `__libc_atexit_handlers.o`, `__libc_signal_handlers.o`, `__libc_stdio_exit.o`.

The worst stack depth of `exit` drops from 88 to 32 bytes.

## P2.03 — stdio and printf flavours

Decision D53, `docs/research/stdio-flavours.md`. Before → after, release build, linked against `libc.a` unless the name says otherwise.

`libc.a`: code 86987 → 87021 (+34), RAM 792 → 1292 (+500), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2279 → 1931 | 156 → 380 | 1464 → 1178 | 0 → 0 |
| hello-printf-int | 8296 → 8016 | 156 → 380 | 6689 → 6469 | 756 → 756 |
| hello-printf-float | 8320 → 8040 | 156 → 380 | 6689 → 6469 | 756 → 756 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 13244 → 13244 | 20 → 20 | 11666 → 11666 | 888 → 888 |
| hello-printf-int-nofloat (new) | 5159 | 380 | 3617 | 756 |
| hello-printf-int-minimal (new) | 4271 | 380 | 3487 | 0 |

Members that grew or are new, justified:

- `__libc_stdin.o` 36 → 288 (+252): holds its static `BUFSIZ` buffer (`.bss`), so reading `stdin` needs no allocator.
- `__libc_stdout.o` 36 → 288 (+252): holds its static `BUFSIZ` buffer (`.bss`), so writing to `stdout` needs no allocator; the heap buffer it replaces was not counted.
- `__libc_format_digits.o` 0 → 194 (+194): new: the digit conversion, moved out of `__libc_format.o`.
- `__libc_format_argument.o` 0 → 172 (+172): new: the argument fetch, moved out of `__libc_format.o` so the minimal flavour can replace it.
- `__libc_scan_store.o` 0 → 62 (+62): new: the integer store, moved out of `__libc_scan.o`.
- `__libc_format_count.o` 0 → 58 (+58): new: `%n`, moved out of `__libc_format.o`.
- `setvbuf.o` 114 → 154 (+40): tracks a caller's buffer and keeps the stream's own one.
- `__libc_file_setup.o` 70 → 76 (+6): tests the weak reference to `malloc`.
- `__libc_scan_count.o` 0 → 6 (+6): new: `%n` for `scanf`, which calls the store.
- Reduced: `__libc_format.o` 2296 → 1938, `__libc_scan.o` 1728 → 1604, `__libc_file_link.o` 44 → 36, `__libc_file_getc.o` 128 → 124, `__libc_scan_float.o` 1105 → 1101, `__libc_stderr.o` 36 → 32, `feof.o` 26 → 24, `ferror.o` 26 → 24, `ungetc.o` 78 → 76.
- The RAM of the stdio images grows by the static `stdout` buffer, which used to come from the heap at the first write; the FILE objects shrink by a pointer each.

## P2.04 — stdlib, allocator, string and time

Decision D54, `docs/research/stdlib-time-size.md`. Before → after, release build.

`libc.a`: code 87021 → 86636 (-385), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 8016 → 7368 | 380 → 380 | 6469 → 6577 | 756 → 0 |
| hello-printf-float | 8040 → 7392 | 380 → 380 | 6469 → 6577 | 756 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 13244 → 13244 | 20 → 20 | 11666 → 11666 | 888 → 888 |
| hello-printf-int-nofloat | 5159 → 4511 | 380 → 380 | 3617 → 3723 | 756 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny (new) | 1922 | 76 | 1006 | 0 |

Members that grew or are new, justified:

- `__libc_udivmod64.o` 0 → 98 (+98): new: the shared 64-bit division by a small divisor that replaces libgcc's.
- `__libc_strtoint.o` 400 → 412 (+12): computes the overflow cutoff with the helper.
- `strftime.o` 2203 → 2215 (+12): calls the division helper; `%C` keeps truncating division.
- `__libc_tz_is_dst.o` 562 → 572 (+10): floor division through the helper.
- `__libc_format_digits.o` 194 → 202 (+8): calls the division helper.
- `__libc_decimal_set.o` 146 → 148 (+2): calls the division helper.
- Reduced: `strerror.o` 2395 → 1968, `__libc_secs_to_tm.o` 496 → 418, `__libc_asctime.o` 502 → 484, `__libc_strtofloat.o` 1131 → 1127.
- The tiny allocator's objects are not in `libc.a`; they replace nano's in `libc-tinymalloc.a` (`malloc-heavy-tiny`).

## P2.05a — soft-float helper audit

Decision D55, `docs/research/libm-softfloat.md`. Before → after, release build.

`libc.a`: code 86636 → 85598 (-1038), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 13244 → 12326 | 20 → 20 | 11666 → 11634 | 888 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_double_to_i64.o` 0 → 56 (+56): new: the integral double to `int64_t` conversion shared by `llrint`, `llround`, `setpayload` and `setpayloadsig`, without `__aeabi_d2lz`/`__aeabi_d2ulz`.
- `__libc_i64_to_double.o` 0 → 40 (+40): new: as `__libc_u64_to_double`.
- `__libc_u64_to_double.o` 0 → 40 (+40): new: 64-bit conversion; two exact 32-bit conversions on fpv5-dp, the native one elsewhere.
- `__libc_math_raise_inexact.o` 0 → 34 (+34): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_underflow.o` 0 → 32 (+32): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_math_raise_overflow.o` 0 → 28 (+28): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_underflow.o` 0 → 28 (+28): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_invalid.o` 0 → 26 (+26): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_math_raise_divbyzero.o` 0 → 26 (+26): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_invalid.o` 0 → 26 (+26): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_divbyzero.o` 0 → 22 (+22): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_math_invalidf.o` 0 → 22 (+22): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `pown.o` 272 → 280 (+8): inlines `__libc_pow_integer`'s conversion call; layout.
- Reduced: 71 members, -1426 bytes; the largest: `__libc_fromfp.o` 258 → 208, `rootn.o` 432 → 384, `tgamma.o` 484 → 436, `lround.o` 144 → 104.

## P2.05b — single-precision layer: the exact float functions

Decision D56, `docs/research/libm-float-exact.md`. Before → after, release build.

`libc.a`: code 85598 → 86908 (+1310), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 2002 → 1688 | 20 → 20 | 1508 → 1198 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_remainderf.o` 0 → 392 (+392): new: the float remainder core of `fmodf`, `remainderf` and `remquof`.
- `__libc_fromfpf.o` 0 → 192 (+192): new: the float core of the four `fromfp` functions.
- `nexttowardf.o` 160 → 304 (+144): compares through widened bits instead of double arithmetic.
- `llroundf.o` 8 → 100 (+92): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lroundf.o` 8 → 92 (+84): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `__libc_float_to_i64.o` 0 → 82 (+82): new: float to `int64_t` from the bits (libgcc's `__aeabi_f2lz` computes in double).
- `ilogbf.o` 8 → 80 (+72): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llogbf.o` 8 → 80 (+72): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `logbf.o` 16 → 80 (+64): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `cprojf.o` 28 → 84 (+56): tests the parts directly.
- `llrintf.o` 8 → 64 (+56): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lrintf.o` 8 → 64 (+56): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `modff.o` 34 → 88 (+54): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `frexpf.o` 16 → 68 (+52): computed in float (the `sp` layer) instead of a wrapper of the double function.
- Reduced: 10 members, -158 bytes; the largest: `conjf.o` 28 → 6, `crealf.o` 24 → 2, `cimagf.o` 24 → 6, `fmodf.o` 24 → 8.

## P2.05c1 — float-float exponentials

Decision D57, `docs/research/libm-float-exp.md`. Before → after, release build.

`libc.a`: code 86908 → 86908 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:


## P2.05c2 — float-float logarithms

Decision D58, `docs/research/libm-float-log.md`. Before → after, release build.

`libc.a`: code 86908 → 86908 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:


## P2.05c3 — float-float powers and roots

Decision D59, `docs/research/libm-float-pow.md`. Before → after, release build.

`libc.a`: code 86908 → 86932 (+24), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `compoundn.o` 280 → 304 (+24): the tiny-x case goes through the power core (D59 fix).

## P2.05d1 — float-float sin, cos, tan and the pi-scaled forms

Decision D60, `docs/research/libm-float-trig.md`. Before → after, release build.

`libc.a`: code 86932 → 86932 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:


## P2.05d2–d3 — float-float inverse trigonometric and hyperbolic functions

Decisions D61 and D62, `docs/research/libm-float-atan.md` and `libm-float-hyperbolic.md`. Before → after, release build.

`libc.a`: code 86932 → 86932 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e1 — float-float error functions

Decision D63, `docs/research/libm-float-erf.md`. Before → after, release build.

`libc.a`: code 86932 → 86932 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e2 — float-float gamma functions

Decision D64, `docs/research/libm-float-gamma.md`. Before → after, release build.

`libc.a`: code 86932 → 86932 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e3a — float complex layer: exponential, logarithm, powers and roots

Decision D65, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 86932 → 86932 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e3b — float complex trigonometric and hyperbolic functions

Decision D66, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 86932 → 86948 (+16), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `ccosh.o` 392 → 400 (+8): the sign fix beyond 709 (one multiply).
- `csinh.o` 352 → 360 (+8): the sign fix beyond 709 (one multiply).
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e3c — float complex inverse functions

Decision D67, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 86948 → 86948 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e4 — float kernel sharing and table sizes

Decision D68, `docs/research/libm-float-kernels.md`. Before → after, release build.

`libc.a`: code 86948 → 86948 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.06a — Thumb-1 string kernels

Decision D69, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 86948 → 86948 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7368 → 7368 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| hello-printf-float | 7392 → 7392 | 380 → 380 | 6577 → 6577 | 0 → 0 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 12326 → 12326 | 20 → 20 | 11634 → 11634 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9510 | 20 → 20 | 9068 → 9068 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.06b — the string kernels on the Thumb-2 cores

Decision D70, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 86948 → 86790 (-158), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1903 | 380 → 380 | 1178 → 1148 | 0 → 0 |
| hello-printf-int | 7368 → 7220 | 380 → 380 | 6577 → 6429 | 0 → 0 |
| hello-printf-float | 7392 → 7244 | 380 → 380 | 6577 → 6429 | 0 → 0 |
| malloc-heavy | 1982 → 1934 | 80 → 80 | 1066 → 1016 | 0 → 0 |
| math-mix | 12326 → 12302 | 20 → 20 | 11634 → 11614 | 0 → 0 |
| float-exact | 1688 → 1688 | 20 → 20 | 1198 → 1198 | 0 → 0 |
| math-float | 9510 → 9494 | 20 → 20 | 9068 → 9048 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4471 | 380 → 380 | 3723 → 3683 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4231 | 380 → 380 | 3487 → 3447 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1874 | 76 → 76 | 1006 → 956 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 4 members, -158 bytes; the largest: `memmove.o` 168 → 60, `memcpy.o` 80 → 60, `memset.o` 68 → 48, `strlen.o` 54 → 44.

## P3.04a — the lock word fast path in the port

Decision D79, `docs/research/locks.md`. Before → after, release build; `libc.a` is unchanged, the change is in
`port/linux` (the size images' port).

| Program | code | RAM | TLS |
|---|---|---|---|
| hello-puts | 1903 → 1887 | 380 → 340 | 8 → 8 |
| hello-printf-int | 7220 → 7204 | 380 → 340 | 8 → 8 |
| hello-printf-float | 7244 → 7228 | 380 → 340 | 8 → 8 |
| malloc-heavy | 1934 → 1914 | 80 → 44 | 8 → 8 |
| hello-printf-int-nofloat | 4471 → 4455 | 380 → 340 | 8 → 8 |
| hello-printf-int-minimal | 4231 → 4215 | 380 → 340 | 8 → 8 |
| malloc-heavy-tiny | 1874 → 1854 | 76 → 40 | 8 → 8 |

Other programs are unchanged. The port drops `__rtos_thread_self` from the lock path and the owner pointer from every lock (4 bytes on 32-bit cores, 8 on x86-64), and gains the thread-local token.

## P3.05c — double-double products by fma

Decision D83, `docs/research/libm-fma.md`. Before → after, release build: `__libc_dd_product` takes the exact error of a
double product from one `VFMA.F64` instead of Dekker's split products.

`libc.a` text: 78384 → 75006 (-3378).

| Program | code |
|---|---|
| math-mix | 12302 → 11662 |
| math-float | 9494 → 9078 |

Grew, justified: `asinh`'s frame, 200 → 208 bytes (register allocation around the shorter products); no entry point's depth grows.

- Reduced: the double-double members, -3378 bytes of text in all; the largest: `cpow.o` 1490 → 1356, `__libc_atan2_core.o` 1280 → 1170, `acos.o` 416 → 312, `acosh.o` 592 → 488.

## P3.07 — introsort qsort (speed flavour)

Decision D90, `docs/research/qsort-speed.md`. The release build is unchanged. In the speed build, `qsort.o` grows by
about 625 bytes (the introsort keeps the heapsort as its fallback), and `qsort`'s stack depth by about 440 bytes, most
of it the stack of waiting ranges (one per bit of `size_t`). No reference program links `qsort`; every image is
unchanged and both budgets hold.

## D98 — Correctly rounded float functions on the double path (Q19)

The 18 float functions with hard cases carry a table of them (8 bytes per
case) and the inline halfway test, and share the out-of-line lookup
`__libc_float_case` (88 bytes). `libc.a` grows by 1136 bytes of text and
392 of rodata. `math-float` and `math-mix` grow by 352 and 360 bytes (it
links `cosf`, `logf` and `sinf`), which leaves `math-float` 64 bytes
inside the size budget. No other image changes; the soft-float cores do not
use these wrappers.


## D104 — `.preinit_array` moves to the port

`__libc_start_main` runs only `.init_array` (28 bytes smaller); the port's start-up code walks `.preinit_array`. Every
reference image changes by −4 bytes.

## D121 — the C++ ABI hooks (issue #98)

New members: `__cxa_atexit` (code and the first static block of 32 registrations), `__cxa_finalize`, `__cxa_thread_atexit`, `__libc_cxa_thread_run` and `__dso_handle`. `__libc_atexit_run` and `thrd_exit` each gain one weak call (12–16 bytes) that runs the C++ `thread_local` destructors when they are linked. A C program links none of the new members, and every reference image is unchanged.
