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

## D125 — the libm vectors' accuracy fixes (Q36)

Correctness fixes, so the growth is accepted: `cbrt.o` +24, `tgamma.o` +152 and `pow.o` +96 bytes (`log1pf.o` −8). `cbrt` gains an exact-residual Halley step, `tgamma` a double-double reciprocal for tiny arguments, and `pow` the tiny-exponent branch. `math-mix` and `math-float` grow by 96 bytes, both from `pow` (`powf` is `pow` rounded on this core). `math-float`'s size budget is re-locked at 9526 bytes (from 9494).

## D131 — the POSIX subset's `errno` codes (P7.02)

`strerror.o` grows by 181 bytes of message text for `ENOTUNIQ`, `EBADFD`, `ESOCKTNOSUPPORT`, `ESHUTDOWN`, `EHOSTDOWN` and `EFTYPE`. No reference image links `strerror`, so every image and both budgets are unchanged.

## D132 — the POSIX subset's functions (P7.03)

The new members (`open`, `read`, `write`, `close`, `lseek`, `fileno`, `fdopen`, `strnlen`, `strcasecmp`, `strncasecmp`, `reallocarray`, `sincos`, `sincosf`, `clock_gettime`, `clock_getres`, `nanosleep`, `sched_yield`) are linked only by programs that call them. `__libc_file_new` no longer closes the handle when it fails, so that a failed `fdopen` leaves the descriptor to its caller: `__libc_file_new.o` shrinks by 12 bytes, and its callers close the handle themselves, `fopen.o` +12 and `tmpfile.o` +24 bytes. `fopen`'s stack depth grows by 8 bytes. No reference image links any of them, so every image and both budgets are unchanged.

## D134 — `fstat`, `fcntl` and `getpid` (P7.04)

Three new members (`fstat`, `fcntl`, `getpid`), linked only by programs that call them; no other member, no image and neither budget changes. The optional `LIBC_UNWIND_TABLES=1` build is not measured here: the baselines and budgets are without unwind tables (D134; its cost is in `docs/research/posix-subset.md`).

## P8.02 — One reduction for `sincos` and `sincosf`

`sincos.o` 40 → 420 and `sincosf.o` 40 → 352 bytes; `sinf.o` −16 and `cosf.o` −32 (their hard-case tables move to `__libc_sinf_cases.o` and `__libc_cosf_cases.o`, shared with `sincosf`): each now holds the entry tests, one reduction and both cores' calls instead of two calls. `sincos.o` is still smaller than `sin.o` + `cos.o`, so a program that calls only `sincos` shrinks; one that also calls `sin` and `cos` grows by the difference. No reference image links them; every image and both budgets are unchanged.

## D144 — `stat`, `isatty` and `dup` (issue #113)

Three new members (`stat`, `isatty`, `dup`), linked only by programs that call them; `jmp_buf`'s named struct and `unreachable()` in C only change no object. No other member, no image and neither budget changes.

## Issue #122 — `clock_gettime` and `timespec_get` zero the result on failure

Members that grew, justified: `clock_gettime.o` 36 → 48 (+12) and `timespec_get.o` 36 → 48 (+12), the two stores that zero the result when the clock read fails (libstdc++'s clocks ignore the status); each frame 8 → 16 bytes, to keep the pointer across the seam call. No image and neither budget changes.

## D145 — `call_once` for C++ (issue #121)

Members that grew, justified: `__libc_call_once_eh.o` 0 → 204 (+204), new: the exception-safe `call_once` that the toolchain's `__gthread_once` calls, with its unwind table (the only library object built with `-fexceptions`); linked only by C++ programs that use `std::call_once`, which already link the unwinder. `call_once.o` 130 → 126 (−4), now sharing `libc/internal/once.h`. New frame: `__libc_call_once_eh` 24 bytes. No image and neither budget changes. Building `call_once.c` itself with `-fexceptions` would instead have added 4,426 bytes (libgcc's unwinder) to every C program that calls `call_once` (armv7e-m `test/unit/none`), which D145 avoids.

## D146 — `getentropy` (issue #127)

Members that grew, justified: `getentropy.o` 0 → 52 (+52), new, linked only by programs that call it (libstdc++'s `std::random_device`). New frames: `getentropy` 8 bytes; the Linux port's `__rtos_entropy` 40 bytes (the port, not `libc.a`). No image and neither budget changes.

## `clock_getres` accepts a null result pointer

Members that grew, justified: `clock_getres.o` 36 → 52 (+16), the local the seam fills and the copy out when `res` is not null (POSIX allows a null `res`; the Linux port wrote through it). Frame: `clock_getres` 8 → 24 bytes, the local `struct timespec`. No image and neither budget changes.

## Phase 10 — POSIX threads (P10.02–P10.07, D148–D157)

Members that grew, justified: 106 members, nearly all new (`libc/pthread`, `sched_get_priority_max`/`min`): `<pthread.h>`, linked only by programs that use threads; the seam's futex time base (D150) adds an argument at each wait. Reduced: 15 members, -690 bytes, mostly the C11 functions that are now wrappers over `<pthread.h>` (D156) and the removed `__libc_mutex_lock`, `__libc_mutex_unlock`, `__libc_cond_wait` and `__libc_thread_start`. Images: the seven programs that link the Linux test port's lock grow by 16 bytes each, with the library's share unchanged: the port's `__rtos_futex_wait` rejects an unknown time base (D150) and `__rtos_lock_acquire` passes one. Both budgets hold.

## P11.02a — The `fast` build type (D162, D164)

No member, image or stack depth changed: the size and speed flavours build the same objects, and the fast
flavour's `libc.a` has the speed flavour's members, byte for byte but the debug sections
(`size-report --build-type fast same --as speed`). No `libc/math/fast/` member yet. The fast flavour is budgeted
as the speed flavour (`docs/size/budget.json`).

## P11.03a — Fast `expf`, `exp2f`, `exp10f` (D168)

Release and speed unchanged (`size-report diff`; the speed budget holds). Fast members: `expf.o` 16 → 288, `exp2f.o` 92 → 272, `exp10f.o` 16 → 288, and `__libc_fast_exp2_table.o` 256 (the speed `expf` and `exp10f` call the double functions). The fast flavour's
budget holds.

## P11.03b — Fast `logf`, `log2f`, `log10f` (D169)

Release and speed unchanged (`size-report diff`); the fast budget holds. Fast members (code and constants, speed →
fast, bytes): `logf.o` 116 → 360, `log2f.o` 16 → 360, `log10f.o` 84 → 376.

## P11.03c — Fast `powf` (D170)

Release and speed: no growth (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed → fast, bytes): `powf.o` 20 → 924 (speed calls `pow`).

## P11.03d — Fast m1 and p1 forms, `cbrtf` (D171)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes; several speed members call a shared core or the double function, not counted): `expm1f.o` 168 → 500, `exp2m1f.o` 16 → 348, `exp10m1f.o` 84 → 372, `log1pf.o` 300 → 544, `log2p1f.o` 92 → 400, `log10p1f.o` 92 → 416, `cbrtf.o` 16 → 312.

## P11.03e — The cores without an FPU: fixed point without the test (D172)

Unchanged here (`size-report diff`; the budgets hold): `nofp/fast/` applies to the cores without an FPU only.

## P11.03f — The cores without an FPU: fixed-point m1 forms (D173)

Unchanged here (`size-report diff`; the budgets hold).

## P11.03g — The cores without an FPU: fixed-point p1 forms, `cbrtf` (D174)

Unchanged here (`size-report diff`; the budgets hold).

## P11.04a — Fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D175)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed →
fast, bytes; speed members that call the double functions or a shared core not counted): `sinf.o` 252 → 500, `cosf.o` 76 → 332, `sincosf.o` 352 → 612, `tanf.o` 192 → 492, `sinpif.o` 16 → 380, `cospif.o` 16 → 400, `tanpif.o` 16 → 448.

## P11.04b — Fast `asinf`, `acosf`, `atanf`, `atan2f` and the π forms (D176)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed →
fast, bytes; the speed files call the double functions): `atanf.o` 268 → 404, `atanpif.o` 92 → 252, `asinf.o` 192 → 452, `asinpif.o` 16 → 324, `acosf.o` 92 → 320, `acospif.o` 16 → 336, `atan2f.o` 140 → 392, `atan2pif.o` 20 → 336, `__libc_fast_atan_table.o` new → 72.

## P11.04c — Fast `sinhf`, `coshf`, `tanhf`, `asinhf`, `acoshf`, `atanhf` (D177)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes): `sinhf.o` 260 → 532, `coshf.o` 16 → 252, `tanhf.o` 200 → 532, `asinhf.o` 300 → 524, `acoshf.o` 92 → 312, `atanhf.o` 192 → 512 (the speed `coshf` and `acoshf` call the double functions).

## P11.04d1 — Fast `erff`, and `erfcf` on the double-FPU cores (D178)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes): `erff.o` 16 → 536, `erfcf.o` 84 → 596 (the speed files call the double functions, which `erfcf` still calls for its fallback), `__libc_fast_erfc_table.o` new → 432.

## P11.04d2 — Fast `erfcf` on the single-precision FPU cores (D179)

Unchanged here (`size-report diff`; the budgets hold): the double-FPU cores' `erfcf` is P11.04d1.
