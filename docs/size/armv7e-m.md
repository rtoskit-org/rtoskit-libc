# Size: armv7e-m

Release build of `linux-armv7e-m`, measured with `tools/scripts/size-report` (PLAN D51,
`docs/research/size-measurement.md`). The figures are in `baseline-armv7e-m.json`;
`tools/scripts/size-report diff` compares a build with them. Each phase 2 step
records its reductions, and justifies any growth, in a section below.

## P2.01 baseline (end of phase 1)

`libc.a` (743 members), release build:

| text | rodata | unwind | data | bss | tdata | tbss |
|---:|---:|---:|---:|---:|---:|---:|
| 112594 | 8863 | 0 | 116 | 672 | 8 | 396 |

Reference images (`test/size`): code is text+rodata+unwind, RAM is data+bss, TLS is tdata+tbss per thread; the libc and port shares come from the link map:

| Program | code | RAM | TLS | libc code | libc RAM | libc TLS | port code | libgcc code | main stack |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| hello-puts | 2595 | 356 | 8 | 1738 | 224 | 4 | 818 | 0 | 136* |
| hello-printf-int | 8577 | 356 | 8 | 6931 | 224 | 4 | 818 | 756 | 1268+ |
| hello-printf-float | 10562 | 356 | 8 | 6931 | 224 | 4 | 818 | 2704 | 1284+ |
| malloc-heavy | 2024 | 228 | 8 | 1106 | 144 | 4 | 716 | 0 | 184* |
| math-mix | 23900 | 200 | 4 | 20188 | 132 | 0 | 656 | 2504 | 896* |

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
| `cpow.o` | 2588 | 16 | 0 | 0 | 0 | 0 | 0 |
| `strerror.o` | 24 | 2371 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format.o` | 2200 | 96 | 0 | 0 | 0 | 0 | 0 |
| `__libc_atan2_core.o` | 1684 | 528 | 0 | 0 | 0 | 0 | 0 |
| `strftime.o` | 1918 | 285 | 0 | 0 | 0 | 0 | 0 |
| `__libc_exp_core.o` | 924 | 1024 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format_float.o` | 1854 | 90 | 0 | 0 | 0 | 0 | 0 |
| `__libc_scan.o` | 1728 | 0 | 0 | 0 | 0 | 0 | 0 |
| `__libc_rem_pio2.o` | 1458 | 160 | 0 | 0 | 0 | 0 | 0 |
| `atan2pi.o` | 1612 | 0 | 0 | 0 | 0 | 0 | 0 |
| `catanh.o` | 1512 | 0 | 0 | 0 | 0 | 0 | 0 |
| `atan2.o` | 1464 | 0 | 0 | 0 | 0 | 0 | 0 |

## P2.02 — build and structure levers

Decision D52, `docs/research/size-levers.md`. Before → after, release build.

`libc.a`: code 121457 → 121899 (+442), RAM 788 → 792 (+4), TLS 404 → 366 (-38).

| Program | code | RAM | TLS | libc code | main stack |
|---|---|---|---|---|---|
| hello-puts | 2595 → 2279 | 356 → 156 | 8 → 8 | 1738 → 1464 | 136 → 136 |
| hello-printf-int | 8577 → 8261 | 356 → 156 | 8 → 8 | 6931 → 6657 | 1268 → 1268 |
| hello-printf-float | 10562 → 10246 | 356 → 156 | 8 → 8 | 6931 → 6657 | 1284 → 1284 |
| malloc-heavy | 2024 → 1982 | 228 → 80 | 8 → 8 | 1106 → 1066 | 184 → 184 |
| math-mix | 23900 → 23460 | 200 → 20 | 4 → 0 | 20188 → 20148 | 896 → 896 |

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

`libc.a`: code 121899 → 121933 (+34), RAM 792 → 1292 (+500), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2279 → 1931 | 156 → 380 | 1464 → 1178 | 0 → 0 |
| hello-printf-int | 8261 → 7981 | 156 → 380 | 6657 → 6437 | 756 → 756 |
| hello-printf-float | 10246 → 9966 | 156 → 380 | 6657 → 6437 | 2704 → 2704 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 23460 → 23460 | 20 → 20 | 20148 → 20148 | 2504 → 2504 |
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

`libc.a`: code 121933 → 121548 (-385), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7981 → 7333 | 380 → 380 | 6437 → 6545 | 756 → 0 |
| hello-printf-float | 9966 → 9318 | 380 → 380 | 6437 → 6545 | 2704 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 23460 → 23460 | 20 → 20 | 20148 → 20148 | 2504 → 2504 |
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

`libc.a`: code 121548 → 120404 (-1144), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 23460 → 23256 | 20 → 20 | 20148 → 19956 | 2504 → 2504 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

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
- `difftime.o` 64 → 66 (+2): calls `__libc_i64_to_double`.
- Reduced: 66 members, -1486 bytes; the largest: `__libc_fromfp.o` 322 → 260, `rootn.o` 724 → 668, `__libc_exp_scale.o` 488 → 440, `lround.o` 132 → 88.

## P2.05b — single-precision layer: the exact float functions

Decision D56, `docs/research/libm-float-exact.md`. Before → after, release build.

`libc.a`: code 120404 → 121526 (+1122), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 23256 → 23256 | 20 → 20 | 19956 → 19956 | 2504 → 2504 |
| float-exact | 5056 → 2048 | 20 → 20 | 2484 → 1518 | 2040 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_remainderf.o` 0 → 392 (+392): new: the float remainder core of `fmodf`, `remainderf` and `remquof`.
- `__libc_fromfpf.o` 0 → 192 (+192): new: the float core of the four `fromfp` functions.
- `nexttowardf.o` 204 → 300 (+96): compares through widened bits instead of double arithmetic.
- `__libc_float_to_i64.o` 0 → 82 (+82): new: float to `int64_t` from the bits (libgcc's `__aeabi_f2lz` computes in double).
- `llroundf.o` 22 → 100 (+78): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lroundf.o` 22 → 92 (+70): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `rintf.o` 32 → 100 (+68): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `roundevenf.o` 32 → 96 (+64): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `ceilf.o` 32 → 92 (+60): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `floorf.o` 32 → 92 (+60): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `ilogbf.o` 22 → 80 (+58): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llogbf.o` 22 → 80 (+58): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `logbf.o` 32 → 80 (+48): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llrintf.o` 22 → 64 (+42): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lrintf.o` 22 → 64 (+42): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `roundf.o` 32 → 72 (+40): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `nearbyintf.o` 32 → 68 (+36): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `frexpf.o` 36 → 68 (+32): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `modff.o` 56 → 88 (+32): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `truncf.o` 32 → 48 (+16): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `cprojf.o` 74 → 84 (+10): tests the parts directly.
- Reduced: 10 members, -454 bytes; the largest: `conjf.o` 74 → 6, `crealf.o` 52 → 2, `remquof.o` 58 → 8, `fmodf.o` 56 → 8.

## P2.05c1 — float-float exponentials

Decision D57, `docs/research/libm-float-exp.md`. Before → after, release build.

`libc.a`: code 121526 → 123794 (+2268), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 23256 → 24340 | 20 → 20 | 19956 → 21034 | 2504 → 2504 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 18044 → 18824 | 20 → 20 | 15100 → 15882 | 2504 → 2504 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

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

`libc.a`: code 123794 → 125890 (+2096), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 24340 → 25488 | 20 → 20 | 21034 → 22178 | 2504 → 2504 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 18824 → 19784 | 20 → 20 | 15882 → 16834 | 2504 → 2504 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

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

`libc.a`: code 125890 → 129394 (+3504), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 25488 → 26212 | 20 → 20 | 22178 → 22898 | 2504 → 2504 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 19784 → 10408 | 20 → 20 | 16834 → 7944 | 2504 → 2040 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `powf.o` 52 → 592 (+540): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `rootnf.o` 40 → 520 (+480): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `pownf.o` 40 → 516 (+476): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `compoundnf.o` 40 → 496 (+456): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `powrf.o` 52 → 468 (+416): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cbrtf.o` 32 → 400 (+368): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `rsqrtf.o` 32 → 356 (+324): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `hypotf.o` 172 → 456 (+284): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_signed.o` 0 → 124 (+124): new: the end of every power function (thresholds, sign, one rounding).
- `compoundn.o` 320 → 348 (+28): the tiny-x case goes through the power core (D59 fix).
- `__libc_exp2f_scale.o` 168 → 176 (+8): keeps the sign of a result that rounds to zero (D59 fix).
- `math-mix` grows (justified): it links the float-float layer next to the double functions it also calls. `math-float` now links no double exp/log/pow core: 18044 → 10408 on armv7e-m since the start of P2.05; `sinf`/`cosf` (P2.05d) still pull the double sin/cos core and soft double.

## P2.05d1 — float-float sin, cos, tan and the pi-scaled forms

Decision D60, `docs/research/libm-float-trig.md`. Before → after, release build.

`libc.a`: code 129394 → 132154 (+2760), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 26212 → 27472 | 20 → 20 | 22898 → 24324 | 2504 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 10408 → 5426 | 20 → 20 | 7944 → 5014 | 2040 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_sinf_core.o` 0 → 734 (+734): new: the Tang sin core with sin/cos of j pi/32 (144 bytes).
- `__libc_rem_pio2f.o` 0 → 662 (+662): new: the exact integer argument reduction (Payne-Hanek, 32-byte 2/pi table).
- `tanpif.o` 32 → 620 (+588): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `sinpif.o` 32 → 416 (+384): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cospif.o` 32 → 328 (+296): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cosf.o` 32 → 136 (+104): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `tanf.o` 232 → 296 (+64): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 1 members, -72 bytes; the largest: `sinf.o` 236 → 164.
- `math-float` (float only) now links no double code at all: 18044 → 5426 bytes on armv7e-m (no libgcc) and 23064 → 9736 on armv6-m since the start of P2.05. `math-mix` grows (justified): it links the float-float layer next to the double functions it also calls.

## P2.05d2–d3 — float-float inverse trigonometric and hyperbolic functions

Decisions D61 and D62, `docs/research/libm-float-atan.md` and `libm-float-hyperbolic.md`. Before → after, release build.

`libc.a`: code 132154 → 138552 (+6398), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27472 | 20 → 20 | 24324 → 24324 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5426 | 20 → 20 | 5014 → 5014 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_atan2f_core.o` 0 → 854 (+854): new: the atan2 core shared by every inverse trigonometric function.
- `atan2pif.o` 52 → 764 (+712): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `asinpif.o` 32 → 580 (+548): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `acoshf.o` 32 → 578 (+546): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `tanhf.o` 236 → 708 (+472): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atan2f.o` 204 → 656 (+452): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
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

`libc.a`: code 138552 → 139896 (+1344), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27472 | 20 → 20 | 24324 → 24324 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5426 | 20 → 20 | 5014 → 5014 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `erfcf.o` 32 → 648 (+616): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_erff_core.o` 0 → 488 (+488): new: the erf series with positive terms, shared by erff and erfcf.
- `erff.o` 32 → 272 (+240): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e2 — float-float gamma functions

Decision D64, `docs/research/libm-float-gamma.md`. Before → after, release build.

`libc.a`: code 139896 → 143134 (+3238), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27472 | 20 → 20 | 24324 → 24324 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5426 | 20 → 20 | 5014 → 5014 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_lgammaf_core.o` 0 → 2790 (+2790): new: the lgamma core (zeta series, Stirling, reflection) with the Taylor expansions at the 14 negative zeros (728 bytes of the table).
- `tgammaf.o` 32 → 340 (+308): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `lgammaf.o` 32 → 172 (+140): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e3a — float complex layer: exponential, logarithm, powers and roots

Decision D65, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 143134 → 146548 (+3414), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27472 | 20 → 20 | 24324 → 24324 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5426 | 20 → 20 | 5014 → 5014 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `clogf.o` 74 → 1050 (+976): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cpowf.o` 112 → 1034 (+922): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `csqrtf.o` 74 → 816 (+742): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cexpf.o` 74 → 416 (+342): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_times.o` 0 → 260 (+260): new: 2^z m rounded once, shared by the complex functions.
- `__libc_sincosf_core.o` 0 → 256 (+256): new: float-float sin and cos of a float-float angle, shared by the complex functions.
- Reduced: 2 members, -84 bytes; the largest: `cabsf.o` 52 → 4, `cargf.o` 52 → 16.

## P2.05e3b — float complex trigonometric and hyperbolic functions

Decision D66, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 146548 → 149038 (+2490), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27472 | 20 → 20 | 24324 → 24324 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5426 | 20 → 20 | 5014 → 5014 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `ctanhf.o` 74 → 872 (+798): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_sinhcoshf_core.o` 0 → 722 (+722): new: float-float sinh and cosh, shared by csinhf, ccoshf and ctanhf.
- `ccoshf.o` 74 → 584 (+510): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `csinhf.o` 74 → 532 (+458): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_ff_round_product.o` 0 → 276 (+276): new: a float-float product rounded once with tiny factors prescaled, shared by the complex functions.
- `csinh.o` 612 → 632 (+20): the sign fix beyond 709 (one multiply).
- `ccosh.o` 704 → 720 (+16): the sign fix beyond 709 (one multiply).
- Reduced: 4 members, -310 bytes; the largest: `__libc_exp2f_times.o` 260 → 92, `ccosf.o` 74 → 16, `csinf.o` 74 → 32, `ctanf.o` 74 → 32.

## P2.05e3c — float complex inverse functions

Decision D67, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 149038 → 155102 (+6064), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27472 | 20 → 20 | 24324 → 24324 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5426 | 20 → 20 | 5014 → 5014 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `cacosf.o` 74 → 1442 (+1368): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `casinhf.o` 74 → 1430 (+1356): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `catanhf.o` 74 → 1338 (+1264): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_csqrtf_core.o` 0 → 760 (+760): new: float-float sqrt(a + ib), shared by cacosf and casinhf.
- `__libc_asinhf_core.o` 0 → 678 (+678): new: asinh of a float-float argument.
- `__libc_clogf_modulus.o` 0 → 368 (+368): new: log|z| as float-float, shared by the inverse functions.
- `__libc_log1pf_core.o` 0 → 368 (+368): new: log1p of a float-float argument.
- Reduced: 3 members, -98 bytes; the largest: `casinf.o` 74 → 32, `catanf.o` 74 → 32, `cacoshf.o` 74 → 60.

## P2.05e4 — float kernel sharing and table sizes

Decision D68, `docs/research/libm-float-kernels.md`. Before → after, release build.

`libc.a`: code 155102 → 154356 (-746), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27472 → 27328 | 20 → 20 | 24324 → 24180 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5426 → 5282 | 20 → 20 | 5014 → 4870 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- `exp2m1f.o` 420 → 524 (+104): the exact-square expm1 series inlined.
- `expm1f.o` 328 → 432 (+104): the exact-square expm1 series inlined.
- `exp10m1f.o` 440 → 536 (+96): the exact-square expm1 series inlined.
- Reduced: 5 members, -1050 bytes; the largest: `asinhf.o` 678 → 104, `clogf.o` 1050 → 874, `__libc_exp2f_core.o` 592 → 448, `__libc_lgammaf_core.o` 2790 → 2698.

## P2.06a — Thumb-1 string kernels

Decision D69, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 154356 → 154356 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1931 | 380 → 380 | 1178 → 1178 | 0 → 0 |
| hello-printf-int | 7333 → 7333 | 380 → 380 | 6545 → 6545 | 0 → 0 |
| hello-printf-float | 9318 → 9318 | 380 → 380 | 6545 → 6545 | 1948 → 1948 |
| malloc-heavy | 1982 → 1982 | 80 → 80 | 1066 → 1066 | 0 → 0 |
| math-mix | 27328 → 27328 | 20 → 20 | 24180 → 24180 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5282 → 5282 | 20 → 20 | 4870 → 4870 | 0 → 0 |
| hello-printf-int-nofloat | 4511 → 4511 | 380 → 380 | 3723 → 3723 | 0 → 0 |
| hello-printf-int-minimal | 4271 → 4271 | 380 → 380 | 3487 → 3487 | 0 → 0 |
| malloc-heavy-tiny | 1922 → 1922 | 76 → 76 | 1006 → 1006 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P2.06b — the string kernels on the Thumb-2 cores

Decision D70, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 154356 → 154198 (-158), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 1931 → 1903 | 380 → 380 | 1178 → 1148 | 0 → 0 |
| hello-printf-int | 7333 → 7185 | 380 → 380 | 6545 → 6397 | 0 → 0 |
| hello-printf-float | 9318 → 9170 | 380 → 380 | 6545 → 6397 | 1948 → 1948 |
| malloc-heavy | 1982 → 1934 | 80 → 80 | 1066 → 1016 | 0 → 0 |
| math-mix | 27328 → 27312 | 20 → 20 | 24180 → 24160 | 2344 → 2344 |
| float-exact | 2048 → 2048 | 20 → 20 | 1518 → 1518 | 0 → 0 |
| math-float | 5282 → 5282 | 20 → 20 | 4870 → 4870 | 0 → 0 |
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
| hello-printf-int | 7185 → 7169 | 380 → 340 | 8 → 8 |
| hello-printf-float | 9170 → 9154 | 380 → 340 | 8 → 8 |
| malloc-heavy | 1934 → 1914 | 80 → 44 | 8 → 8 |
| hello-printf-int-nofloat | 4471 → 4455 | 380 → 340 | 8 → 8 |
| hello-printf-int-minimal | 4231 → 4215 | 380 → 340 | 8 → 8 |
| malloc-heavy-tiny | 1874 → 1854 | 76 → 40 | 8 → 8 |

Other programs are unchanged. The port drops `__rtos_thread_self` from the lock path and the owner pointer from every lock (4 bytes on 32-bit cores, 8 on x86-64), and gains the thread-local token.

## P3.05b — float-float products by fma

Decision D82, `docs/research/libm-fma.md`. Before → after, release build: `__libc_ff_product` takes the exact error of a
float product from one `VFMA.F32` instead of Dekker's split products.

`libc.a` text: 144216 → 139904 (-4312).

| Program | code |
|---|---|
| math-mix | 27312 → 26844 |
| math-float | 5282 → 4818 |

Members that grew or are new, justified: none.

- Reduced: the float-float members, -4312 bytes of text in all; the largest: `__libc_asinhf_core.o` 678 → 568, `__libc_erff_core.o` 488 → 378, `__libc_lgammaf_core.o` 2698 → 2592, `acospif.o` 460 → 356.

## P3.07 — introsort qsort (speed flavour)

Decision D90, `docs/research/qsort-speed.md`. The release build is unchanged. In the speed build, `qsort.o` grows by
about 625 bytes (the introsort keeps the heapsort as its fallback), and `qsort`'s stack depth by about 440 bytes, most
of it the stack of waiting ranges (one per bit of `size_t`). No reference program links `qsort`; every image is
unchanged and both budgets hold.

## D99 — our own double-precision helpers (Q24)

Decision D99, `docs/research/softdouble.md`. The `soft-dp` layer replaces every libgcc double helper, so no libgcc floating-point member is linked. Release build, code (text + rodata):

| Program | code |
|---|---|
| hello-printf-float | 9154 → 8486 (−668) |
| math-mix | 26844 → 26688 (−156) |

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
| `__muldc3.o` | 1112 | 1200 |
| `__divdc3.o` | 1384 | 2012 |
| `__divsc3.o` | 884 | 724 |
| `__powidf2.o` | 116 | 116 |

A program using them no longer links libgcc's double objects as well, which were 888–2344 bytes more.

The faster `__divdc3` and `__divsc3` (see `docs/perf/armv7e-m.md`, D100) add 140–144 and 16 bytes for the range test and the reciprocal; the table shows the final sizes.

## D101 — the comparisons in Thumb-2 assembly

`math-mix` grows by 24 bytes. `__aeabi_dcmpgt` and `__aeabi_dcmpge` exchange their operands before calling the shared ordering (+12 and +8 bytes); `__libc_softdouble_compare` is 106 bytes against 100 in C. That pays for comparisons 34 % faster. The size budget holds; no other image changes.


## D104 — `.preinit_array` moves to the port

`__libc_start_main` runs only `.init_array` (28 bytes smaller); the port's start-up code walks `.preinit_array`. Every
reference image changes by −4 bytes.

## D121 — the C++ ABI hooks (issue #98)

New members: `__cxa_atexit` (code and the first static block of 32 registrations), `__cxa_finalize`, `__cxa_thread_atexit`, `__libc_cxa_thread_run` and `__dso_handle`. `__libc_atexit_run` and `thrd_exit` each gain one weak call (12–16 bytes) that runs the C++ `thread_local` destructors when they are linked. A C program links none of the new members, and every reference image is unchanged.

## D124 — `(float)` of a NaN keeps its sign (issue #95)

The libm vectors found that `__libc_softdouble_d2f` set a NaN's sign from the double's exponent bits, so every narrowed NaN came out negative. Masking the payload adds 4 bytes to `__libc_softdouble_d2f.o`; no reference image changes.

## D125 — the libm vectors' accuracy fixes (Q36)

Correctness fixes, so the growth is accepted: `cbrt.o` +84, `tgamma.o` +520 and `pow.o` +168 bytes. `cbrt` gains an exact-residual Halley step, `tgamma` a double-double reciprocal for tiny arguments, and `pow` the tiny-exponent branch. `math-mix` grows by 168 bytes from `pow`; the size budget holds.

## D131 — the POSIX subset's `errno` codes (P7.02)

`strerror.o` grows by 181 bytes of message text for `ENOTUNIQ`, `EBADFD`, `ESOCKTNOSUPPORT`, `ESHUTDOWN`, `EHOSTDOWN` and `EFTYPE`. No reference image links `strerror`, so every image and both budgets are unchanged.

## D132 — the POSIX subset's functions (P7.03)

The new members (`open`, `read`, `write`, `close`, `lseek`, `fileno`, `fdopen`, `strnlen`, `strcasecmp`, `strncasecmp`, `reallocarray`, `sincos`, `sincosf`, `clock_gettime`, `clock_getres`, `nanosleep`, `sched_yield`) are linked only by programs that call them. `__libc_file_new` no longer closes the handle when it fails, so that a failed `fdopen` leaves the descriptor to its caller: `__libc_file_new.o` shrinks by 12 bytes, and its callers close the handle themselves, `fopen.o` +12 and `tmpfile.o` +24 bytes. `fopen`'s stack depth grows by 8 bytes. No reference image links any of them, so every image and both budgets are unchanged.

## D134 — `fstat`, `fcntl` and `getpid` (P7.04)

Three new members (`fstat`, `fcntl`, `getpid`), linked only by programs that call them; no other member, no image and neither budget changes. The optional `LIBC_UNWIND_TABLES=1` build is not measured here: the baselines and budgets are without unwind tables (D134; its cost is in `docs/research/posix-subset.md`).

## P8.02 — One reduction for `sincos` and `sincosf`

`sincos.o` grows to about 520–545 bytes and `sincosf.o` (the `sp-nearest` file) to 184–200 bytes: each now holds the entry tests, one reduction and both cores' calls instead of two calls. `sincos.o` is still smaller than `sin.o` + `cos.o`, so a program that calls only `sincos` shrinks; one that also calls `sin` and `cos` grows by the difference. No reference image links them; every image and both budgets are unchanged.

## P8.03 — Fixed-point double `sin`, `cos` and `sincos` (speed flavour, D136)

Speed flavour only: `__libc_trig_fixed.o`, `__libc_trig_dd.o` and `__libc_trig_fixed_round.o` (1,352 + 560 + 92 bytes) are new, and the speed `sin.o`, `cos.o` and `sincos.o` shrink by 110–165 bytes each (their double-double code moves to `__libc_trig_dd.o`). A program that calls `sin` or `cos` links the kernel and keeps the double-double cores for the fallback, about 2 KB more; the speed budgets hold (`size-report budget --flavour speed`). The size flavour and its baselines are unchanged.

## P9.03 — `tan` through the D136 kernel (speed flavour, D142)

The speed flavour's `tan` adds `__libc_tan_fixed.o` and `__libc_tan_dd.o` (tan.c's double-double path, now its fallback). The speed budget holds within its margin, and the size flavour is unchanged.

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

Release and speed unchanged (`size-report diff`; the speed budget holds). Fast members: `expf.o` 184 → 308, `exp2f.o` 100 → 308, `exp10f.o` 192 → 348 bytes of code and constants (each file inlines `__libc_fast_expf_kernel`; the speed ones call shared float-float cores). The fast flavour's
budget holds.

## P11.03b — Fast `logf`, `log2f`, `log10f` (D169)

Release and speed unchanged (`size-report diff`); the fast budget holds. Fast members (code and constants, speed →
fast, bytes): `logf.o` 164 → 312, `log2f.o` 112 → 336, `log10f.o` 164 → 356, `__libc_fast_logf_invc.o` new → 128, `__libc_fast_logf_ln.o` new → 256, `__libc_fast_logf_log2.o` new → 256, `__libc_fast_logf_log10.o` new → 256.

## P11.03c — Fast `powf` (D170)

Release and speed: no growth (`size-report diff`; the strict `pow.o` 1,576 → 1,568); the speed and fast budgets hold. Fast members (code and constants, speed → fast, bytes): `powf.o` 540 → 1,096.

## P11.03d — Fast m1 and p1 forms, `cbrtf` (D171)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes; several speed members call a shared core or the double function, not counted): `expm1f.o` 332 → 528, `exp2m1f.o` 436 → 492, `exp10m1f.o` 446 → 636, `log1pf.o` 204 → 424, `log2p1f.o` 252 → 496, `log10p1f.o` 304 → 516, `cbrtf.o` 308 → 312.

## P11.03e — The cores without an FPU: fixed point without the test (D172)

Unchanged here (`size-report diff`; the budgets hold): `nofp/fast/` applies to the cores without an FPU only.

## P11.03f — The cores without an FPU: fixed-point m1 forms (D173)

Unchanged here (`size-report diff`; the budgets hold).

## P11.03g — The cores without an FPU: fixed-point p1 forms, `cbrtf` (D174)

Unchanged here (`size-report diff`; the budgets hold).

## P11.04a — Fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D175)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed →
fast, bytes; speed members that call the double functions or a shared core not counted): `sinf.o` 164 → 528, `cosf.o` 136 → 508, `sincosf.o` 200 → 608, `tanf.o` 248 → 632, `sinpif.o` 362 → 544, `cospif.o` 276 → 500, `tanpif.o` 520 → 708, `__libc_fast_sinf_table.o` new → 144.

## P11.04b — Fast `asinf`, `acosf`, `atanf`, `atan2f` and the π forms (D176)

Release and speed: `__libc_atan2f_core.o` 740 → 736 (the strict index fix, D176; `size-report diff`); the speed and fast budgets
hold. Fast members (code and constants, speed → fast, bytes; the speed files share the 736-byte float-float core, which stays
in libc.a for the complex functions): `atanf.o` 144 → 388, `atanpif.o` 336 → 464, `asinf.o` 292 → 340, `asinpif.o` 530 → 416, `acosf.o` 268 → 360, `acospif.o` 356 → 400, `atan2f.o` 556 → 696, `atan2pif.o` 668 → 828, `__libc_fast_atanf_table.o` new → 72.

## P11.04c — Fast `sinhf`, `coshf`, `tanhf`, `asinhf`, `acoshf`, `atanhf` (D177)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. The release baselines take P11.04b's 4-byte `__libc_atan2f_core.o` (`size-report report`). Fast members (code and constants,
speed → fast, bytes): `sinhf.o` 440 → 524, `coshf.o` 180 → 412, `tanhf.o` 658 → 668, `asinhf.o` 104 → 688, `acoshf.o` 478 → 652, `atanhf.o` 534 → 452 (the speed files share `__libc_sinhcoshf_core.o` 636 and `__libc_asinhf_core.o` 568, which stay in libc.a for the complex functions).

## P11.04d1 — Fast `erff`, and `erfcf` on the double-FPU cores (D178)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes): `erff.o` 220 → 784 (the speed file shares `__libc_erff_core.o` 378, which stays for `erfcf`), `__libc_fast_erfcf_table.o` new → 216; `erfcf.o` unchanged.

## P11.04d2 — Fast `erfcf` on the single-precision FPU cores (D179)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes): `erfcf.o` 598 → 1678 (with its own copy of the float-float code as the fallback; `__libc_erff_core.o` 378 stays), `__libc_fast_erfcf_r_table.o` new → 420, `__libc_fast_exp2f_table.o` new → 256.

## P11.04e1 — The cores without an FPU: fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D180)

Unchanged here (`size-report diff`; the budgets hold).

## P11.05c1 — Fast `exp`, `exp2` and `exp10` in 64-bit fixed point (D188)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (exp and pow) is 31292 bytes of code against 29984 in speed (+4.4 %, within the +20 % budget): the fixed-point `exp`, its kernel and its 512-byte table replace D187's double file and its 1 KiB table.

## P11.05c2 — Fast `expm1` in 64-bit fixed point (D189)

Release and speed unchanged (`size-report diff`); the fast budgets hold. No size program calls `expm1`, and `math-mix` loses 8 bytes in fast (the kernel's polynomial as an inline).

## P11.05d — Fast `log`, `log2`, `log10` and `log1p` in double (D190)

Release and speed unchanged (`size-report diff`). `math-mix` (log and pow) grows from 31284 to 34828 bytes of code in fast (+3544): the fast `log` and its 3 KiB table on top of the strict log core that pow still uses. The fast figures are re-locked (`size-report --flavour fast --lock`), as the maintainer decided for P11.05 (D187).

## P11.05e — Fast `log`, `log2`, `log10` and `log1p` in 64-bit fixed point (D192)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (log and pow) shrinks from 34828 to 34300 bytes of code: the 1440-byte table and the two kernels replace D190's file and its 3 KiB table. Smaller than the locked fast figure, so it is not re-locked (P11.06 does, D187, D191).

## P11.05f — Fast `pow` and `cbrt` in double (D193)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (log and pow) grows from 34300 to 35252 bytes of code (+952): pow runs the double file on this core's helpers until P11.05g, with the double logarithms' 3 KiB table beside the fixed-point one. The fast figures are re-locked (`size-report --flavour fast --lock`), as the maintainer decided for P11.05 (D187).

## P11.05g1 — Fast `pow` in fixed point (D194)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (log and pow) shrinks from 35252 to 32806 bytes of code (+9.4 % over speed's 29984, inside D191's +20 %): the fixed-point pow shares the logarithms' table and the exponentials' kernel, and D193's double table and the exp pairs leave this core. Not re-locked (P11.06 does, D187, D191).

## P11.05g2 — Fast `cbrt` in fixed point (D195)

Release and speed unchanged (`size-report diff`). In fast, no size program calls `cbrt`, and every budget holds; nothing is re-locked.

## P11.05i — Fast `sin`, `cos`, `sincos` and `tan` without the rounding test (D197)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (which calls `sin`, `cos` and `tan`) goes from 32806 to 32812 bytes of code (+6): the four files drop the rounding test but inline the 54th-bit rounding. Every budget holds; nothing is re-locked.

## P11.05j — Fast `asin`, `acos`, `atan` and `atan2` in double (D198)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (which calls `atan2`) grows from 32812 to 34364 bytes of code (+1552): until P11.05k this core runs the double file on its helpers, with the atan(j/16) table and the strict slow path beside it. That is +14 % over speed's 30038, inside D191's +20 % line; every budget holds and nothing is re-locked.

## P11.05k1 — Fast `atan` and `atan2` in fixed point (D199)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (which calls `atan2`) shrinks from 34364 to 34060 bytes of code (-304): the fixed-point kernel and its 136-byte Q64 table replace P11.05j's double file and its pairs on this core's helpers. That is +13.4 % over speed's 30038, inside D191's +20 % line; every budget holds and nothing is re-locked.

## P11.05k2 — Fast `asin` and `acos` in fixed point (D200)

Release and speed unchanged (`size-report diff`). In fast, no size program calls `asin` or `acos` (`math-mix` calls `atan2` only); every budget holds and nothing is re-locked.

## P11.05l — Fast hyperbolic functions in double (D201)

Release and speed unchanged (`size-report diff`). In fast, no size program calls a hyperbolic function; every budget holds and nothing is re-locked.
