# Size: armv8-m.main

Release build of `linux-armv8-m.main`, measured with `tools/scripts/size-report` (PLAN D51,
`docs/research/size-measurement.md`). The figures are in `baseline-armv8-m.main.json`;
`tools/scripts/size-report diff` compares a build with them. Each phase 2 step
records its reductions, and justifies any growth, in a section below.

## P2.01 baseline (end of phase 1)

`libc.a` (743 members), release build:

| text | rodata | unwind | data | bss | tdata | tbss |
|---:|---:|---:|---:|---:|---:|---:|
| 112114 | 8863 | 0 | 116 | 672 | 8 | 396 |

Reference images (`test/size`): code is text+rodata+unwind, RAM is data+bss, TLS is tdata+tbss per thread; the libc and port shares come from the link map:

| Program | code | RAM | TLS | libc code | libc RAM | libc TLS | port code | libgcc code | main stack |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| hello-puts | 2579 | 356 | 8 | 1730 | 224 | 4 | 806 | 0 | 136* |
| hello-printf-int | 8565 | 356 | 8 | 6929 | 224 | 4 | 806 | 758 | 1268+ |
| hello-printf-float | 10550 | 356 | 8 | 6929 | 224 | 4 | 806 | 2706 | 1284+ |
| malloc-heavy | 2008 | 228 | 8 | 1100 | 144 | 4 | 704 | 0 | 184* |
| math-mix | 23888 | 200 | 4 | 20180 | 132 | 0 | 644 | 2504 | 896* |

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
| `mtx_lock` | 72* | `cnd_wait` | 96* | `exp` | 224* |
| `log` | 296* | `pow` | 416* | `sin` | 328* |
| `cos` | 328* | `tan` | 376* | `atan2` | 784* |
| `lgamma` | 1112* | `tgamma` | 1088* | `erfc` | 456* |
| `cpow` | 1568* | `sinf` | 376* | `expf` | 232* |
| `powf` | 440* |  |  |  |  |

Largest members (text+rodata+unwind+data+bss+tdata+tbss), top 15:

| Member | text | rodata | unwind | data | bss | tdata | tbss |
|---|---:|---:|---:|---:|---:|---:|---:|
| `__libc_lgamma_core.o` | 2920 | 392 | 0 | 0 | 0 | 0 | 0 |
| `__libc_log_core.o` | 1364 | 1472 | 0 | 0 | 0 | 0 | 0 |
| `__libc_erfc_table.o` | 844 | 1824 | 0 | 0 | 0 | 0 | 0 |
| `cpow.o` | 2596 | 16 | 0 | 0 | 0 | 0 | 0 |
| `strerror.o` | 24 | 2371 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format.o` | 2196 | 96 | 0 | 0 | 0 | 0 | 0 |
| `__libc_atan2_core.o` | 1684 | 528 | 0 | 0 | 0 | 0 | 0 |
| `strftime.o` | 1920 | 285 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format_float.o` | 1866 | 90 | 0 | 0 | 0 | 0 | 0 |
| `__libc_exp_core.o` | 924 | 1024 | 0 | 0 | 0 | 0 | 0 |
| `__libc_scan.o` | 1724 | 0 | 0 | 0 | 0 | 0 | 0 |
| `__libc_rem_pio2.o` | 1450 | 160 | 0 | 0 | 0 | 0 | 0 |
| `atan2pi.o` | 1604 | 0 | 0 | 0 | 0 | 0 | 0 |
| `catanh.o` | 1512 | 0 | 0 | 0 | 0 | 0 | 0 |
| `atan2.o` | 1464 | 0 | 0 | 0 | 0 | 0 | 0 |

## P2.02 — build and structure levers

Decision D52, `docs/research/size-levers.md`. Before → after, release build.

`libc.a`: code 120977 → 121423 (+446), RAM 788 → 792 (+4), TLS 404 → 366 (-38).

| Program | code | RAM | TLS | libc code | main stack |
|---|---|---|---|---|---|
| hello-puts | 2579 → 2263 | 356 → 156 | 8 → 8 | 1730 → 1456 | 136 → 136 |
| hello-printf-int | 8565 → 8249 | 356 → 156 | 8 → 8 | 6929 → 6655 | 1268 → 1268 |
| hello-printf-float | 10550 → 10234 | 356 → 156 | 8 → 8 | 6929 → 6655 | 1284 → 1284 |
| malloc-heavy | 2008 → 1966 | 228 → 80 | 8 → 8 | 1100 → 1060 | 184 → 184 |
| math-mix | 23888 → 23448 | 200 → 20 | 4 → 0 | 20180 → 20140 | 896 → 896 |

Members that grew, justified:

- `__libc_asctime.o` 298 → 506 (+208): formats by hand instead of calling `snprintf` (on Arm the `asctime` probe image shrinks by 5.7–6.2 KB).
- `__libc_assert_fail.o` 79 → 257 (+178): formats by hand instead of calling `snprintf` (on Arm the `assert` probe image shrinks by 6.4–6.6 KB).
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

`libc.a`: code 121423 → 121453 (+30), RAM 792 → 1292 (+500), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2263 → 1915 | 156 → 380 | 1456 → 1170 | 0 → 0 |
| hello-printf-int | 8249 → 7965 | 156 → 380 | 6655 → 6431 | 758 → 758 |
| hello-printf-float | 10234 → 9950 | 156 → 380 | 6655 → 6431 | 2706 → 2706 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 23448 → 23448 | 20 → 20 | 20140 → 20140 | 2504 → 2504 |
| hello-printf-int-nofloat (new) | 5135 | 380 | 3601 | 758 |
| hello-printf-int-minimal (new) | 4243 | 380 | 3471 | 0 |

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
- Reduced: `__libc_format.o` 2292 → 1930, `__libc_scan.o` 1724 → 1600, `__libc_file_link.o` 44 → 36, `__libc_file_getc.o` 128 → 124, `__libc_scan_float.o` 1105 → 1101, `__libc_stderr.o` 36 → 32, `feof.o` 26 → 24, `ferror.o` 26 → 24, `ungetc.o` 78 → 76.
- The RAM of the stdio images grows by the static `stdout` buffer, which used to come from the heap at the first write; the FILE objects shrink by a pointer each.

## P2.04 — stdlib, allocator, string and time

Decision D54, `docs/research/stdlib-time-size.md`. Before → after, release build.

`libc.a`: code 121453 → 121046 (-407), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7965 → 7313 | 380 → 380 | 6431 → 6539 | 758 → 0 |
| hello-printf-float | 9950 → 9298 | 380 → 380 | 6431 → 6539 | 2706 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 23448 → 23448 | 20 → 20 | 20140 → 20140 | 2504 → 2504 |
| hello-printf-int-nofloat | 5135 → 4479 | 380 → 380 | 3601 → 3707 | 758 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny (new) | 1906 | 76 | 1000 | 0 |

Members that grew or are new, justified:

- `__libc_udivmod64.o` 0 → 98 (+98): new: the shared 64-bit division by a small divisor that replaces libgcc's.
- `strftime.o` 2205 → 2217 (+12): calls the division helper; `%C` keeps truncating division.
- `__libc_format_digits.o` 194 → 202 (+8): calls the division helper.
- `__libc_strtoint.o` 400 → 408 (+8): computes the overflow cutoff with the helper.
- `__libc_decimal_set.o` 146 → 148 (+2): calls the division helper.
- `__libc_tz_is_dst.o` 570 → 572 (+2): floor division through the helper.
- Reduced: `strerror.o` 2395 → 1968, `__libc_secs_to_tm.o` 496 → 410, `__libc_asctime.o` 506 → 486, `__libc_strtofloat.o` 1131 → 1127.
- The tiny allocator's objects are not in `libc.a`; they replace nano's in `libc-tinymalloc.a` (`malloc-heavy-tiny`).

## P2.05a — soft-float helper audit

Decision D55, `docs/research/libm-softfloat.md`. Before → after, release build.

`libc.a`: code 121046 → 119894 (-1152), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 23448 → 23244 | 20 → 20 | 20140 → 19940 | 2504 → 2504 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_double_to_i64.o` 0 → 118 (+118): new: the integral double to `int64_t` conversion shared by `llrint`, `llround`, `setpayload` and `setpayloadsig`, without `__aeabi_d2lz`/`__aeabi_d2ulz`.
- `__libc_math_divbyzero.o` 0 → 48 (+48): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_math_underflow.o` 0 → 48 (+48): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `clog.o` 1276 → 1300 (+24): calls the out-of-line divide-by-zero result.
- `__libc_math_invalid.o` 0 → 16 (+16): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_pow_integer.o` 796 → 812 (+16): calls `__libc_i64_to_double`.
- `__libc_i64_to_double.o` 0 → 12 (+12): new: as `__libc_u64_to_double`.
- `__libc_math_invalidf.o` 0 → 12 (+12): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_u64_to_double.o` 0 → 12 (+12): new: 64-bit conversion; two exact 32-bit conversions on fpv5-dp, the native one elsewhere.
- `exp.o` 288 → 296 (+8): calls an out-of-line exception helper instead of inline arithmetic; layout.
- `tanpi.o` 1440 → 1448 (+8): calls an out-of-line exception helper instead of inline arithmetic; layout.
- `lgamma.o` 920 → 924 (+4): calls an out-of-line exception helper instead of inline arithmetic; layout.
- `strtod.o` 284 → 288 (+4): calls `__libc_u64_to_double`.
- `__libc_math_raise_divbyzero.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_inexact.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_invalid.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_overflow.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_underflow.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `difftime.o` 66 → 68 (+2): calls `__libc_i64_to_double`.
- Reduced: 66 members, -1494 bytes; the largest: `__libc_fromfp.o` 322 → 260, `rootn.o` 724 → 668, `pow.o` 1460 → 1408, `__libc_exp_scale.o` 488 → 440.

## P2.05b — single-precision layer: the exact float functions

Decision D56, `docs/research/libm-float-exact.md`. Before → after, release build.

`libc.a`: code 119894 → 120672 (+778), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 23244 → 23244 | 20 → 20 | 19940 → 19940 | 2504 → 2504 |
| float-exact | 4552 → 1612 | 20 → 20 | 2014 → 1122 | 2040 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_remainderf.o` 0 → 388 (+388): new: the float remainder core of `fmodf`, `remainderf` and `remquof`.
- `__libc_fromfpf.o` 0 → 192 (+192): new: the float core of the four `fromfp` functions.
- `nexttowardf.o` 204 → 304 (+100): compares through widened bits instead of double arithmetic.
- `__libc_float_to_i64.o` 0 → 82 (+82): new: float to `int64_t` from the bits (libgcc's `__aeabi_f2lz` computes in double).
- `llroundf.o` 22 → 100 (+78): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lroundf.o` 22 → 92 (+70): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `ilogbf.o` 22 → 80 (+58): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llogbf.o` 22 → 80 (+58): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `logbf.o` 32 → 80 (+48): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llrintf.o` 22 → 64 (+42): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lrintf.o` 22 → 64 (+42): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `frexpf.o` 36 → 68 (+32): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `modff.o` 56 → 88 (+32): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `cprojf.o` 74 → 84 (+10): tests the parts directly.
- Reduced: 10 members, -454 bytes; the largest: `conjf.o` 74 → 6, `crealf.o` 52 → 2, `remquof.o` 58 → 8, `fmodf.o` 56 → 8.

## P2.05c1 — float-float exponentials

Decision D57, `docs/research/libm-float-exp.md`. Before → after, release build.

`libc.a`: code 120672 → 122940 (+2268), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 23244 → 24324 | 20 → 20 | 19940 → 21018 | 2504 → 2504 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 18024 → 18812 | 20 → 20 | 15084 → 15866 | 2504 → 2504 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_exp2f_core.o` 0 → 592 (+592): new: the base-2 float-float exponential core with its 2^(j/32) table (256 bytes).
- `exp10m1f.o` 32 → 440 (+408): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `exp2m1f.o` 32 → 420 (+388): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `exp10f.o` 32 → 244 (+212): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `expf.o` 32 → 236 (+204): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_scale.o` 0 → 160 (+160): new: one rounding of the core result, subnormal results included.
- `expm1f.o` 196 → 328 (+132): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2m1f_scale.o` 0 → 104 (+104): new: the core result minus one, rounded once.
- `exp2f.o` 32 → 100 (+68): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Transitional growth, justified: `math-mix` and `math-float` now link the float-float exponentials *and*, through `logf`, `powf`, `sinf` and `cosf` (still wrappers until P2.05c2–d), the double cores and soft double. A float program sheds soft double only when every float function it calls is float-only; the reductions are measured at the end of P2.05 (D57).

## P2.05c2 — float-float logarithms

Decision D58, `docs/research/libm-float-log.md`. Before → after, release build.

`libc.a`: code 122940 → 125036 (+2096), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 24324 → 25472 | 20 → 20 | 21018 → 22162 | 2504 → 2504 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 18812 → 19764 | 20 → 20 | 15866 → 16818 | 2504 → 2504 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_log2f_core.o` 0 → 860 (+860): new: the float-float log2 core with its 23-entry table (276 bytes).
- `log10p1f.o` 32 → 392 (+360): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_log2p1f_core.o` 0 → 344 (+344): new: log2(1 + x), the series near 0, the core elsewhere.
- `log2p1f.o` 32 → 304 (+272): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log10f.o` 32 → 216 (+184): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `logf.o` 32 → 216 (+184): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log2f.o` 32 → 112 (+80): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log1pf.o` 228 → 256 (+28): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_scale.o` 160 → 168 (+8): takes the subnormal path from the magnitude of the result (D58).
- Reduced: 1 members, -224 bytes; the largest: `logp1f.o` 228 → 4.
- Transitional growth, justified as in P2.05c1: `math-mix` and `math-float` still link the double cores through `powf`, `sinf` and `cosf` (P2.05c3, P2.05d) besides the new float-float logarithm (D58).

## P2.05c3 — float-float powers and roots

Decision D59, `docs/research/libm-float-pow.md`. Before → after, release build.

`libc.a`: code 125036 → 128480 (+3444), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 25472 → 26132 | 20 → 20 | 22162 → 22820 | 2504 → 2504 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 19764 → 10328 | 20 → 20 | 16818 → 7868 | 2504 → 2040 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `powf.o` 52 → 580 (+528): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `rootnf.o` 40 → 512 (+472): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `pownf.o` 40 → 504 (+464): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `compoundnf.o` 40 → 492 (+452): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `powrf.o` 52 → 464 (+412): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cbrtf.o` 32 → 400 (+368): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `rsqrtf.o` 32 → 352 (+320): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `hypotf.o` 172 → 448 (+276): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_signed.o` 0 → 116 (+116): new: the end of every power function (thresholds, sign, one rounding).
- `compoundn.o` 320 → 348 (+28): the tiny-x case goes through the power core (D59 fix).
- `__libc_exp2f_scale.o` 168 → 176 (+8): keeps the sign of a result that rounds to zero (D59 fix).
- `math-mix` grows (justified): it links the float-float layer next to the double functions it also calls. `math-float` now links no double exp/log/pow core: 18044 → 10408 on armv7e-m since the start of P2.05; `sinf`/`cosf` (P2.05d) still pull the double sin/cos core and soft double.

## P2.05d1 — float-float sin, cos, tan and the pi-scaled forms

Decision D60, `docs/research/libm-float-trig.md`. Before → after, release build.

`libc.a`: code 128480 → 131236 (+2756), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 26132 → 27400 | 20 → 20 | 22820 → 24246 | 2504 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 10328 → 5358 | 20 → 20 | 7868 → 4946 | 2040 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_sinf_core.o` 0 → 734 (+734): new: the Tang sin core with sin/cos of j pi/32 (144 bytes).
- `__libc_rem_pio2f.o` 0 → 662 (+662): new: the exact integer argument reduction (Payne-Hanek, 32-byte 2/pi table).
- `tanpif.o` 32 → 620 (+588): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `sinpif.o` 32 → 416 (+384): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cospif.o` 32 → 324 (+292): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cosf.o` 32 → 136 (+104): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `tanf.o` 232 → 296 (+64): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 1 members, -72 bytes; the largest: `sinf.o` 236 → 164.
- `math-float` (float only) now links no double code at all: 18044 → 5426 bytes on armv7e-m (no libgcc) and 23064 → 9736 on armv6-m since the start of P2.05. `math-mix` grows (justified): it links the float-float layer next to the double functions it also calls.

## P2.05d2–d3 — float-float inverse trigonometric and hyperbolic functions

Decisions D61 and D62, `docs/research/libm-float-atan.md` and `libm-float-hyperbolic.md`. Before → after, release build.

`libc.a`: code 131236 → 137602 (+6366), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27400 | 20 → 20 | 24246 → 24246 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5358 | 20 → 20 | 4946 → 4946 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_atan2f_core.o` 0 → 842 (+842): new: the atan2 core shared by every inverse trigonometric function.
- `atan2pif.o` 52 → 752 (+700): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `asinpif.o` 32 → 580 (+548): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `acoshf.o` 32 → 578 (+546): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `tanhf.o` 236 → 708 (+472): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atan2f.o` 204 → 648 (+444): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `asinhf.o` 236 → 678 (+442): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `acospif.o` 32 → 460 (+428): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atanhf.o` 232 → 636 (+404): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atanpif.o` 32 → 428 (+396): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `acosf.o` 32 → 360 (+328): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_expf_pair.o` 0 → 304 (+304): new: e^x and e^-x as float-float, shared by sinhf, coshf and tanhf.
- `sinhf.o` 232 → 532 (+300): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `asinf.o` 232 → 388 (+156): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `coshf.o` 32 → 180 (+148): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 1 members, -92 bytes; the largest: `atanf.o` 236 → 144.

## P2.05e1 — float-float error functions

Decision D63, `docs/research/libm-float-erf.md`. Before → after, release build.

`libc.a`: code 137602 → 138946 (+1344), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27400 | 20 → 20 | 24246 → 24246 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5358 | 20 → 20 | 4946 → 4946 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `erfcf.o` 32 → 648 (+616): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_erff_core.o` 0 → 488 (+488): new: the erf series with positive terms, shared by erff and erfcf.
- `erff.o` 32 → 272 (+240): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e2 — float-float gamma functions

Decision D64, `docs/research/libm-float-gamma.md`. Before → after, release build.

`libc.a`: code 138946 → 142180 (+3234), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27400 | 20 → 20 | 24246 → 24246 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5358 | 20 → 20 | 4946 → 4946 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_lgammaf_core.o` 0 → 2786 (+2786): new: the lgamma core (zeta series, Stirling, reflection) with the Taylor expansions at the 14 negative zeros (728 bytes of the table).
- `tgammaf.o` 32 → 340 (+308): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `lgammaf.o` 32 → 172 (+140): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e3a — float complex layer: exponential, logarithm, powers and roots

Decision D65, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 142180 → 145590 (+3410), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27400 | 20 → 20 | 24246 → 24246 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5358 | 20 → 20 | 4946 → 4946 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `clogf.o` 74 → 1050 (+976): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cpowf.o` 112 → 1034 (+922): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `csqrtf.o` 74 → 812 (+738): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cexpf.o` 74 → 416 (+342): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_times.o` 0 → 260 (+260): new: 2^z m rounded once, shared by the complex functions.
- `__libc_sincosf_core.o` 0 → 256 (+256): new: float-float sin and cos of a float-float angle, shared by the complex functions.
- Reduced: 2 members, -84 bytes; the largest: `cabsf.o` 52 → 4, `cargf.o` 52 → 16.

## P2.05e3b — float complex trigonometric and hyperbolic functions

Decision D66, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 145590 → 148072 (+2482), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27400 | 20 → 20 | 24246 → 24246 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5358 | 20 → 20 | 4946 → 4946 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `ctanhf.o` 74 → 864 (+790): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_sinhcoshf_core.o` 0 → 722 (+722): new: float-float sinh and cosh, shared by csinhf, ccoshf and ctanhf.
- `ccoshf.o` 74 → 584 (+510): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `csinhf.o` 74 → 532 (+458): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_ff_round_product.o` 0 → 276 (+276): new: a float-float product rounded once with tiny factors prescaled, shared by the complex functions.
- `csinh.o` 612 → 632 (+20): the sign fix beyond 709 (one multiply).
- `ccosh.o` 704 → 720 (+16): the sign fix beyond 709 (one multiply).
- Reduced: 4 members, -310 bytes; the largest: `__libc_exp2f_times.o` 260 → 92, `ccosf.o` 74 → 16, `csinf.o` 74 → 32, `ctanf.o` 74 → 32.

## P2.05e3c — float complex inverse functions

Decision D67, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 148072 → 154104 (+6032), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27400 | 20 → 20 | 24246 → 24246 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5358 | 20 → 20 | 4946 → 4946 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `casinhf.o` 74 → 1426 (+1352): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cacosf.o` 74 → 1422 (+1348): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `catanhf.o` 74 → 1338 (+1264): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_csqrtf_core.o` 0 → 752 (+752): new: float-float sqrt(a + ib), shared by cacosf and casinhf.
- `__libc_asinhf_core.o` 0 → 678 (+678): new: asinh of a float-float argument.
- `__libc_clogf_modulus.o` 0 → 368 (+368): new: log|z| as float-float, shared by the inverse functions.
- `__libc_log1pf_core.o` 0 → 368 (+368): new: log1p of a float-float argument.
- Reduced: 3 members, -98 bytes; the largest: `casinf.o` 74 → 32, `catanf.o` 74 → 32, `cacoshf.o` 74 → 60.

## P2.05e4 — float kernel sharing and table sizes

Decision D68, `docs/research/libm-float-kernels.md`. Before → after, release build.

`libc.a`: code 154104 → 153358 (-746), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27400 → 27256 | 20 → 20 | 24246 → 24102 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5358 → 5214 | 20 → 20 | 4946 → 4802 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- `exp2m1f.o` 420 → 524 (+104): the exact-square expm1 series inlined.
- `expm1f.o` 328 → 432 (+104): the exact-square expm1 series inlined.
- `exp10m1f.o` 440 → 536 (+96): the exact-square expm1 series inlined.
- Reduced: 5 members, -1050 bytes; the largest: `asinhf.o` 678 → 104, `clogf.o` 1050 → 874, `__libc_exp2f_core.o` 592 → 448, `__libc_lgammaf_core.o` 2786 → 2694.

## P2.06a — Thumb-1 string kernels

Decision D69, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 153358 → 153358 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1915 | 380 → 380 | 1170 → 1170 | 0 → 0 |
| hello-printf-int | 7313 → 7313 | 380 → 380 | 6539 → 6539 | 0 → 0 |
| hello-printf-float | 9298 → 9298 | 380 → 380 | 6539 → 6539 | 1948 → 1948 |
| malloc-heavy | 1966 → 1966 | 80 → 80 | 1060 → 1060 | 0 → 0 |
| math-mix | 27256 → 27256 | 20 → 20 | 24102 → 24102 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5214 → 5214 | 20 → 20 | 4802 → 4802 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4479 | 380 → 380 | 3707 → 3707 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4243 | 380 → 380 | 3471 → 3471 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1906 | 76 → 76 | 1000 → 1000 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.06b — the string kernels on the Thumb-2 cores

Decision D70, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 153358 → 153200 (-158), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1915 → 1883 | 380 → 380 | 1170 → 1140 | 0 → 0 |
| hello-printf-int | 7313 → 7165 | 380 → 380 | 6539 → 6391 | 0 → 0 |
| hello-printf-float | 9298 → 9150 | 380 → 380 | 6539 → 6391 | 1948 → 1948 |
| malloc-heavy | 1966 → 1914 | 80 → 80 | 1060 → 1010 | 0 → 0 |
| math-mix | 27256 → 27232 | 20 → 20 | 24102 → 24082 | 2344 → 2344 |
| float-exact | 1612 → 1612 | 20 → 20 | 1122 → 1122 | 0 → 0 |
| math-float | 5214 → 5214 | 20 → 20 | 4802 → 4802 | 0 → 0 |
| hello-printf-int-nofloat | 4479 → 4439 | 380 → 380 | 3707 → 3667 | 0 → 0 |
| hello-printf-int-minimal | 4243 → 4203 | 380 → 380 | 3471 → 3431 | 0 → 0 |
| malloc-heavy-tiny | 1906 → 1854 | 76 → 76 | 1000 → 950 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 4 members, -158 bytes; the largest: `memmove.o` 168 → 60, `memcpy.o` 80 → 60, `memset.o` 68 → 48, `strlen.o` 54 → 44.

## P3.04a — the lock word fast path in the port

Decision D79, `docs/research/locks.md`. Before → after, release build; `libc.a` is unchanged, the change is in
`port/linux` (the size images' port).

| Program | code | RAM | TLS |
|---|---|---|---|
| hello-puts | 1883 → 1871 | 380 → 340 | 8 → 8 |
| hello-printf-int | 7165 → 7153 | 380 → 340 | 8 → 8 |
| hello-printf-float | 9150 → 9138 | 380 → 340 | 8 → 8 |
| malloc-heavy | 1914 → 1898 | 80 → 44 | 8 → 8 |
| hello-printf-int-nofloat | 4439 → 4427 | 380 → 340 | 8 → 8 |
| hello-printf-int-minimal | 4203 → 4191 | 380 → 340 | 8 → 8 |
| malloc-heavy-tiny | 1854 → 1838 | 76 → 40 | 8 → 8 |

Other programs are unchanged. The port drops `__rtos_thread_self` from the lock path and the owner pointer from every lock (4 bytes on 32-bit cores, 8 on x86-64), and gains the thread-local token.

## P3.05b — float-float products by fma

Decision D82, `docs/research/libm-fma.md`. Before → after, release build: `__libc_ff_product` takes the exact error of a
float product from one `VFMA.F32` instead of Dekker's split products.

`libc.a` text: 143218 → 138902 (-4316).

| Program | code |
|---|---|
| math-mix | 27232 → 26772 |
| math-float | 5214 → 4750 |

Members that grew or are new, justified: none.

- Reduced: the float-float members, -4316 bytes of text in all; the largest: `__libc_asinhf_core.o` 678 → 564, `__libc_erff_core.o` 488 → 378, `__libc_lgammaf_core.o` 2694 → 2588, `acospif.o` 460 → 356.

## P3.07 — introsort qsort (speed flavour)

Decision D90, `docs/research/qsort-speed.md`. The release build is unchanged. In the speed build, `qsort.o` grows by
about 625 bytes (the introsort keeps the heapsort as its fallback), and `qsort`'s stack depth by about 440 bytes, most
of it the stack of waiting ranges (one per bit of `size_t`). No reference program links `qsort`; every image is
unchanged and both budgets hold.

## D99 — our own double-precision helpers (Q24)

Decision D99, `docs/research/softdouble.md`. The `soft-dp` layer replaces every libgcc double helper, so no libgcc floating-point member is linked. Release build, code (text + rodata):

| Program | code |
|---|---|
| hello-printf-float | 9138 → 8478 (−660) |
| math-mix | 26772 → 26616 (−156) |

- **Other images:** unchanged; their programs use no double arithmetic.
- **`libc.a`:** grows by the layer's members, about 3.4 KB of text, which a program links only for the operations it uses.
  - The largest: `__aeabi_dmul` (Thumb-2 assembly, 554 bytes, complete with subnormals and underflow), `__aeabi_dadd` (532), `__aeabi_ddiv` (480, C), `__libc_softdouble_d2f` (256, C, `d2f` outside float's normal range) and `__libc_softdouble_pack` (192, the division's rounding).
  - The 64-bit conversions are 6 to 172 bytes each, where libgcc's were 1.6–1.9 KB with the double arithmetic they called.
- **`__libc_float_case.o`:** also new in this baseline. It came with D98 and no armv7e-m-class image links it.
- **Stack depths:** the C helpers use 8–40 bytes of stack. The assembly pushes 24 or 28 bytes, which is not in the call-graph files.

## D100 — strong helpers, and libgcc's complex helpers replaced

Decision D100. The `soft-dp` helpers are strong symbols now, and the layer adds the four libgcc helpers that called them from inside libgcc. No reference program uses complex arithmetic or `__builtin_powi`, so every image is unchanged.

`libc.a` gains the four, about the size of libgcc's own:

| Member | ours | libgcc's |
|---|---:|---:|
| `__muldc3.o` | 1104 | 1184 |
| `__divdc3.o` | 1384 | 2012 |
| `__divsc3.o` | 876 | 724 |
| `__powidf2.o` | 116 | 116 |

A program using them no longer links libgcc's double objects as well, which were 888–2344 bytes more.

The faster `__divdc3` and `__divsc3` (see `docs/perf/armv8-m.main.md`, D100) add 140–144 and 16 bytes for the range test and the reciprocal; the table shows the final sizes.

## D101 — the comparisons in Thumb-2 assembly

`math-mix` grows by 24 bytes. `__aeabi_dcmpgt` and `__aeabi_dcmpge` exchange their operands before calling the shared ordering (+12 and +8 bytes); `__libc_softdouble_compare` is 106 bytes against 100 in C. That pays for comparisons 34 % faster. The size budget holds; no other image changes.


## D104 — `.preinit_array` moves to the port

`__libc_start_main` runs only `.init_array` (28 bytes smaller); the port's start-up code walks `.preinit_array`. Every
reference image changes by −4 bytes.
