# Size: armv6-m

Release build of `linux-armv6-m`, measured with `tools/scripts/size-report` (PLAN D51,
`docs/research/size-measurement.md`). The figures are in `baseline-armv6-m.json`;
`tools/scripts/size-report diff` compares a build with them. Each phase 2 step
records its reductions, and justifies any growth, in a section below.

## P2.01 baseline (end of phase 1)

`libc.a` (743 members), release build:

| text | rodata | unwind | data | bss | tdata | tbss |
|---:|---:|---:|---:|---:|---:|---:|
| 112594 | 8815 | 0 | 116 | 672 | 8 | 396 |

Reference images (`test/size`): code is text+rodata+unwind, RAM is data+bss, TLS is tdata+tbss per thread; the libc and port shares come from the link map:

| Program | code | RAM | TLS | libc code | libc RAM | libc TLS | port code | libgcc code | main stack |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| hello-puts | 2887 | 356 | 8 | 1996 | 224 | 4 | 852 | 0 | 176+ |
| hello-printf-int | 8829 | 356 | 8 | 7105 | 224 | 4 | 852 | 800 | 1336+ |
| hello-printf-float | 11374 | 356 | 8 | 7105 | 224 | 4 | 852 | 3304 | 1360+ |
| malloc-heavy | 2332 | 228 | 8 | 1390 | 144 | 4 | 752 | 0 | 232+ |
| math-mix | 30720 | 200 | 4 | 20198 | 132 | 0 | 688 | 9368 | 560* |

Worst stack depth in bytes. `*`: without the frames of assembly or libgcc helpers (leaf routines that push a few registers); `+`: a lower bound because of an indirect call, recursion, a dynamic frame or an unknown callee:

| Function | depth | Function | depth | Function | depth |
|---|---:|---|---:|---|---:|
| `printf` | 1320+ | `fprintf` | 1316+ | `snprintf` | 1304+ |
| `sscanf` | 3228+ | `fscanf` | 3236+ | `strtod` | 1112+ |
| `strtof` | 1112+ | `strtol` | 88* | `malloc` | 104+ |
| `calloc` | 120+ | `realloc` | 128+ | `free` | 104+ |
| `qsort` | 84+ | `bsearch` | 32+ | `strftime` | 424+ |
| `mktime` | 384* | `localtime` | 272* | `gmtime` | 80* |
| `fopen` | 160+ | `fclose` | 120+ | `fputs` | 168+ |
| `puts` | 168+ | `fgets` | 168+ | `fflush` | 144+ |
| `exit` | 120+ | `abort` | 56+ | `thrd_create` | 184+ |
| `mtx_lock` | 96+ | `cnd_wait` | 128+ | `exp` | 200* |
| `log` | 248* | `pow` | 368* | `sin` | 320* |
| `cos` | 320* | `tan` | 384* | `atan2` | 448* |
| `lgamma` | 816* | `tgamma` | 784* | `erfc` | 392* |
| `cpow` | 1168* | `sinf` | 360* | `expf` | 208* |
| `powf` | 384* |  |  |  |  |

Largest members (text+rodata+unwind+data+bss+tdata+tbss), top 15:

| Member | text | rodata | unwind | data | bss | tdata | tbss |
|---|---:|---:|---:|---:|---:|---:|---:|
| `__libc_lgamma_core.o` | 4002 | 360 | 0 | 0 | 0 | 0 | 0 |
| `__libc_log_core.o` | 1328 | 1472 | 0 | 0 | 0 | 0 | 0 |
| `cpow.o` | 2718 | 16 | 0 | 0 | 0 | 0 | 0 |
| `__libc_erfc_table.o` | 864 | 1824 | 0 | 0 | 0 | 0 | 0 |
| `__libc_atan2_core.o` | 1868 | 528 | 0 | 0 | 0 | 0 | 0 |
| `strerror.o` | 24 | 2371 | 0 | 0 | 0 | 0 | 0 |
| `strftime.o` | 1922 | 285 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format.o` | 2042 | 96 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format_float.o` | 1880 | 90 | 0 | 0 | 0 | 0 | 0 |
| `__libc_rem_pio2.o` | 1750 | 160 | 0 | 0 | 0 | 0 | 0 |
| `__libc_exp_core.o` | 880 | 1024 | 0 | 0 | 0 | 0 | 0 |
| `__libc_scan.o` | 1714 | 0 | 0 | 0 | 0 | 0 | 0 |
| `asinh.o` | 1544 | 0 | 0 | 0 | 0 | 0 | 0 |
| `catanh.o` | 1476 | 0 | 0 | 0 | 0 | 0 | 0 |
| `atan2pi.o` | 1468 | 0 | 0 | 0 | 0 | 0 | 0 |

## P2.02 — build and structure levers

Decision D52, `docs/research/size-levers.md`. Before → after, release build.

`libc.a`: code 121409 → 121875 (+466), RAM 788 → 792 (+4), TLS 404 → 366 (-38).

| Program | code | RAM | TLS | libc code | main stack |
|---|---|---|---|---|---|
| hello-puts | 2887 → 2579 | 356 → 156 | 8 → 8 | 1996 → 1730 | 176 → 176 |
| hello-printf-int | 8829 → 8521 | 356 → 156 | 8 → 8 | 7105 → 6839 | 1336 → 1336 |
| hello-printf-float | 11374 → 11066 | 356 → 156 | 8 → 8 | 7105 → 6839 | 1360 → 1360 |
| malloc-heavy | 2332 → 2286 | 228 → 80 | 8 → 8 | 1390 → 1346 | 232 → 232 |
| math-mix | 30720 → 30024 | 200 → 20 | 4 → 0 | 20198 → 19940 | 560 → 560 |

Members that grew, justified:

- `__libc_asctime.o` 298 → 498 (+200): formats by hand instead of calling `snprintf` (on Arm the `asctime` probe image shrinks by 5.7–6.2 KB).
- `__libc_assert_fail.o` 79 → 265 (+186): formats by hand instead of calling `snprintf` (on Arm the `assert` probe image shrinks by 6.4–6.6 KB).
- `__libc_atexit_run.o` 0 → 148 (+148): new: the atexit table (moved from `__libc_atexit_handlers.o`) and its runner.
- `__libc_quick_exit_handlers.o` 0 → 132 (+132): renamed from `__libc_atexit_handlers.o`, now only the `at_quick_exit` table.
- `raise.o` 60 → 188 (+128): now holds the signal table (moved from `__libc_signal_handlers.o`).
- `__libc_file_new.o` 88 → 116 (+28): closes the streams it made at exit (moved from `__libc_stdio_exit.o`).
- `__libc_stdio_flush_all.o` 132 → 144 (+12): tests the weak references to `stdout` and `stderr`.
- `abort.o` 12 → 24 (+12): tests the weak reference to `raise`.
- `exit.o` 44 → 48 (+4): tests the weak reference to `__libc_atexit_run`.
- `__libc_file_write.o` 292 → 294 (+2): calls `__libc_stdio_flush_all` directly.
- Removed: `__libc_atexit_handlers.o`, `__libc_signal_handlers.o`, `__libc_stdio_exit.o`.

The worst stack depth of `exit` drops from 120 to 48 bytes.

## P2.03 — stdio and printf flavours

Decision D53, `docs/research/stdio-flavours.md`. Before → after, release build, linked against `libc.a` unless the name says otherwise.

`libc.a`: code 121875 → 122011 (+136), RAM 792 → 1292 (+500), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2579 → 2211 | 156 → 380 | 1730 → 1428 | 0 → 0 |
| hello-printf-int | 8521 → 8249 | 156 → 380 | 6839 → 6631 | 800 → 800 |
| hello-printf-float | 11066 → 10794 | 156 → 380 | 6839 → 6631 | 3304 → 3304 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 30024 → 30024 | 20 → 20 | 19940 → 19940 | 9368 → 9368 |
| hello-printf-int-nofloat (new) | 5351 | 380 | 3735 | 800 |
| hello-printf-int-minimal (new) | 4779 | 380 | 3593 | 372 |

Members that grew or are new, justified:

- `__libc_stdin.o` 36 → 288 (+252): holds its static `BUFSIZ` buffer (`.bss`), so reading `stdin` needs no allocator.
- `__libc_stdout.o` 36 → 288 (+252): holds its static `BUFSIZ` buffer (`.bss`), so writing to `stdout` needs no allocator; the heap buffer it replaces was not counted.
- `__libc_format_argument.o` 0 → 194 (+194): new: the argument fetch, moved out of `__libc_format.o` so the minimal flavour can replace it.
- `__libc_format_digits.o` 0 → 186 (+186): new: the digit conversion, moved out of `__libc_format.o`.
- `__libc_scan_store.o` 0 → 62 (+62): new: the integer store, moved out of `__libc_scan.o`.
- `__libc_format_count.o` 0 → 60 (+60): new: `%n`, moved out of `__libc_format.o`.
- `setvbuf.o` 116 → 164 (+48): tracks a caller's buffer and keeps the stream's own one.
- `__libc_scan_count.o` 0 → 10 (+10): new: `%n` for `scanf`, which calls the store.
- `__libc_file_setup.o` 74 → 80 (+6): tests the weak reference to `malloc`.
- Reduced: `__libc_format.o` 2138 → 1792, `__libc_scan.o` 1714 → 1658, `__libc_file_link.o` 44 → 36, `__libc_file_getc.o` 132 → 128, `__libc_file_unlink.o` 64 → 60, `__libc_scan_float.o` 1147 → 1143, `__libc_stderr.o` 36 → 32, `ungetc.o` 84 → 80, `feof.o` 26 → 24, `ferror.o` 26 → 24.
- The RAM of the stdio images grows by the static `stdout` buffer, which used to come from the heap at the first write; the FILE objects shrink by a pointer each.

## P2.04 — stdlib, allocator, string and time

Decision D54, `docs/research/stdlib-time-size.md`. Before → after, release build.

`libc.a`: code 122011 → 121640 (-371), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 8249 → 7945 | 380 → 380 | 6631 → 6709 | 800 → 420 |
| hello-printf-float | 10794 → 10550 | 380 → 380 | 6631 → 6709 | 3304 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 30024 → 30024 | 20 → 20 | 19940 → 19940 | 9368 → 9368 |
| hello-printf-int-nofloat | 5351 → 5047 | 380 → 380 | 3735 → 3811 | 800 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny (new) | 2206 | 76 | 1266 | 0 |

Members that grew or are new, justified:

- `__libc_udivmod64.o` 0 → 76 (+76): new: the shared 64-bit division by a small divisor that replaces libgcc's.
- `strftime.o` 2207 → 2247 (+40): calls the division helper; `%C` keeps truncating division.
- `__libc_strtoint.o` 416 → 444 (+28): computes the overflow cutoff with the helper.
- `__libc_tm_to_secs.o` 304 → 320 (+16): floor division through the helper.
- `__libc_tz_is_dst.o` 612 → 618 (+6): floor division through the helper.
- `__libc_decimal_set.o` 144 → 146 (+2): calls the division helper.
- Reduced: `strerror.o` 2395 → 1968, `__libc_secs_to_tm.o` 580 → 486, `__libc_asctime.o` 498 → 480.
- The tiny allocator's objects are not in `libc.a`; they replace nano's in `libc-tinymalloc.a` (`malloc-heavy-tiny`).

## P2.05a — soft-float helper audit

Decision D55, `docs/research/libm-softfloat.md`. Before → after, release build.

`libc.a`: code 121640 → 120330 (-1310), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 30024 → 27864 | 20 → 20 | 19940 → 19732 | 9368 → 7412 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_double_to_i64.o` 0 → 96 (+96): new: the integral double to `int64_t` conversion shared by `llrint`, `llround`, `setpayload` and `setpayloadsig`, without `__aeabi_d2lz`/`__aeabi_d2ulz`.
- `__libc_math_divbyzero.o` 0 → 28 (+28): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_math_underflow.o` 0 → 20 (+20): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__aeabi_dsub.o` 0 → 14 (+14): new (nofp layer): subtraction as addition of the negation, replacing libgcc's 1444-byte `subdf3.o`.
- `__aeabi_fsub.o` 0 → 14 (+14): new (nofp layer): replaces libgcc's 764-byte `subsf3.o`.
- `__libc_math_invalid.o` 0 → 12 (+12): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_i64_to_double.o` 0 → 8 (+8): new: as `__libc_u64_to_double`.
- `__libc_math_invalidf.o` 0 → 8 (+8): new: out-of-line special-case result (a constant where `<fenv.h>` records nothing, so no soft division).
- `__libc_u64_to_double.o` 0 → 8 (+8): new: 64-bit conversion; two exact 32-bit conversions on fpv5-dp, the native one elsewhere.
- `__libc_math_raise_divbyzero.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_inexact.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_invalid.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_overflow.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- `__libc_math_raise_underflow.o` 0 → 2 (+2): new: out-of-line exception raising (empty where `<fenv.h>` records nothing).
- Reduced: 73 members, -1528 bytes; the largest: `rootn.o` 756 → 692, `powr.o` 990 → 932, `tgamma.o` 588 → 540, `pow.o` 1360 → 1316.

## P2.05b — single-precision layer: the exact float functions

Decision D56, `docs/research/libm-float-exact.md`. Before → after, release build.

`libc.a`: code 120330 → 121780 (+1450), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 27864 → 27864 | 20 → 20 | 19732 → 19732 | 7412 → 7412 |
| float-exact | 8440 → 3908 | 20 → 20 | 2302 → 1444 | 5618 → 1946 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_remainderf.o` 0 → 384 (+384): new: the float remainder core of `fmodf`, `remainderf` and `remquof`.
- `__libc_fromfpf.o` 0 → 208 (+208): new: the float core of the four `fromfp` functions.
- `nexttowardf.o` 164 → 316 (+152): compares through widened bits instead of double arithmetic.
- `rintf.o` 16 → 100 (+84): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `roundevenf.o` 16 → 96 (+80): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `__libc_float_to_i64.o` 0 → 68 (+68): new: float to `int64_t` from the bits (libgcc's `__aeabi_f2lz` computes in double).
- `ilogbf.o` 12 → 76 (+64): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llogbf.o` 12 → 76 (+64): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `roundf.o` 16 → 80 (+64): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `ceilf.o` 16 → 76 (+60): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `floorf.o` 16 → 76 (+60): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llroundf.o` 12 → 72 (+60): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lroundf.o` 12 → 70 (+58): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `logbf.o` 16 → 72 (+56): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `frexpf.o` 20 → 72 (+52): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `modff.o` 38 → 84 (+46): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `llrintf.o` 12 → 54 (+42): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `lrintf.o` 12 → 52 (+40): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `nearbyintf.o` 16 → 54 (+38): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `truncf.o` 16 → 48 (+32): computed in float (the `sp` layer) instead of a wrapper of the double function.
- `cprojf.o` 68 → 96 (+28): tests the parts directly.
- Reduced: 10 members, -290 bytes; the largest: `conjf.o` 68 → 12, `crealf.o` 40 → 2, `cimagf.o` 40 → 4, `remainderf.o` 42 → 12.

## P2.05c1 — float-float exponentials

Decision D57, `docs/research/libm-float-exp.md`. Before → after, release build.

`libc.a`: code 121780 → 124908 (+3128), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 27864 → 29368 | 20 → 20 | 19732 → 21178 | 7412 → 7472 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 23064 → 24296 | 20 → 20 | 15224 → 16394 | 7412 → 7472 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_exp2f_core.o` 0 → 864 (+864): new: the base-2 float-float exponential core with its 2^(j/32) table (256 bytes).
- `exp10m1f.o` 16 → 560 (+544): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `exp2m1f.o` 16 → 496 (+480): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `exp10f.o` 16 → 316 (+300): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `expf.o` 16 → 300 (+284): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `expm1f.o` 164 → 432 (+268): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_scale.o` 0 → 172 (+172): new: one rounding of the core result, subnormal results included.
- `__libc_exp2m1f_scale.o` 0 → 132 (+132): new: the core result minus one, rounded once.
- `exp2f.o` 16 → 100 (+84): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Transitional growth, justified: `math-mix` and `math-float` now link the float-float exponentials *and*, through `logf`, `powf`, `sinf` and `cosf` (still wrappers until P2.05c2–d), the double cores and soft double. A float program sheds soft double only when every float function it calls is float-only; the reductions are measured at the end of P2.05 (D57).

## P2.05c2 — float-float logarithms

Decision D58, `docs/research/libm-float-log.md`. Before → after, release build.

`libc.a`: code 124908 → 128256 (+3348), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 29368 → 31508 | 20 → 20 | 21178 → 22806 | 7472 → 7984 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 24296 → 26256 | 20 → 20 | 16394 → 17834 | 7472 → 7984 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_log2f_core.o` 0 → 1232 (+1232): new: the float-float log2 core with its 23-entry table (276 bytes).
- `__libc_log2p1f_core.o` 0 → 576 (+576): new: log2(1 + x), the series near 0, the core elsewhere.
- `log10p1f.o` 16 → 496 (+480): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log2p1f.o` 16 → 364 (+348): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log10f.o` 16 → 324 (+308): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `logf.o` 16 → 324 (+308): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log1pf.o` 184 → 340 (+156): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `log2f.o` 16 → 128 (+112): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_scale.o` 172 → 176 (+4): takes the subnormal path from the magnitude of the result (D58).
- Reduced: 1 members, -176 bytes; the largest: `logp1f.o` 184 → 8.
- Transitional growth, justified as in P2.05c1: `math-mix` and `math-float` still link the double cores through `powf`, `sinf` and `cosf` (P2.05c3, P2.05d) besides the new float-float logarithm (D58).

## P2.05c3 — float-float powers and roots

Decision D59, `docs/research/libm-float-pow.md`. Before → after, release build.

`libc.a`: code 128256 → 133280 (+5024), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 31508 → 32780 | 20 → 20 | 22806 → 23960 | 7984 → 8104 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 26256 → 16360 | 20 → 20 | 17834 → 9432 | 7984 → 6496 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `rootnf.o` 24 → 700 (+676): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `powf.o` 34 → 688 (+654): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `pownf.o` 24 → 656 (+632): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `compoundnf.o` 24 → 628 (+604): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cbrtf.o` 16 → 580 (+564): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `powrf.o` 34 → 588 (+554): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `hypotf.o` 176 → 620 (+444): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `rsqrtf.o` 16 → 456 (+440): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `sqrtf.o` 16 → 320 (+304): float-only correctly rounded square root in the `sp` layer (integer root and sticky bit) instead of a wrapper of sqrt.
- `__libc_exp2f_signed.o` 0 → 140 (+140): new: the end of every power function (thresholds, sign, one rounding).
- `__libc_exp2f_scale.o` 176 → 184 (+8): keeps the sign of a result that rounds to zero (D59 fix).
- `compoundn.o` 308 → 312 (+4): the tiny-x case goes through the power core (D59 fix).
- `math-mix` grows (justified): it links the float-float layer next to the double functions it also calls. `math-float` now links no double exp/log/pow core: 18044 → 10408 on armv7e-m since the start of P2.05; `sinf`/`cosf` (P2.05d) still pull the double sin/cos core and soft double.

## P2.05d1 — float-float sin, cos, tan and the pi-scaled forms

Decision D60, `docs/research/libm-float-trig.md`. Before → after, release build.

`libc.a`: code 133280 → 136892 (+3612), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 32780 → 34272 | 20 → 20 | 23960 → 25802 | 8104 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 16360 → 9736 | 20 → 20 | 9432 → 6600 | 6496 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_sinf_core.o` 0 → 1064 (+1064): new: the Tang sin core with sin/cos of j pi/32 (144 bytes).
- `__libc_rem_pio2f.o` 0 → 724 (+724): new: the exact integer argument reduction (Payne-Hanek, 32-byte 2/pi table).
- `tanpif.o` 16 → 712 (+696): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `sinpif.o` 16 → 508 (+492): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cospif.o` 16 → 384 (+368): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `tanf.o` 184 → 396 (+212): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `cosf.o` 16 → 124 (+108): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 1 members, -52 bytes; the largest: `sinf.o` 196 → 144.
- `math-float` (float only) now links no double code at all: 18044 → 5426 bytes on armv7e-m (no libgcc) and 23064 → 9736 on armv6-m since the start of P2.05. `math-mix` grows (justified): it links the float-float layer next to the double functions it also calls.

## P2.05d2–d3 — float-float inverse trigonometric and hyperbolic functions

Decisions D61 and D62, `docs/research/libm-float-atan.md` and `libm-float-hyperbolic.md`. Before → after, release build.

`libc.a`: code 136892 → 146998 (+10106), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34272 | 20 → 20 | 25802 → 25802 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9736 | 20 → 20 | 6600 → 6600 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_atan2f_core.o` 0 → 1324 (+1324): new: the atan2 core shared by every inverse trigonometric function.
- `acoshf.o` 16 → 972 (+956): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atan2pif.o` 34 → 884 (+850): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `asinhf.o` 196 → 984 (+788): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atanhf.o` 184 → 972 (+788): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `tanhf.o` 196 → 974 (+778): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `asinpif.o` 16 → 792 (+776): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `acospif.o` 16 → 708 (+692): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `sinhf.o` 184 → 776 (+592): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `acosf.o` 16 → 590 (+574): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atan2f.o` 220 → 780 (+560): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `atanpif.o` 16 → 508 (+492): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_expf_pair.o` 0 → 416 (+416): new: e^x and e^-x as float-float, shared by sinhf, coshf and tanhf.
- `asinf.o` 184 → 592 (+408): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `coshf.o` 16 → 216 (+200): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 1 members, -88 bytes; the largest: `atanf.o` 196 → 108.

## P2.05e1 — float-float error functions

Decision D63, `docs/research/libm-float-erf.md`. Before → after, release build.

`libc.a`: code 146998 → 148982 (+1984), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34272 | 20 → 20 | 25802 → 25802 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9736 | 20 → 20 | 6600 → 6600 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `erfcf.o` 16 → 972 (+956): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_erff_core.o` 0 → 728 (+728): new: the erf series with positive terms, shared by erff and erfcf.
- `erff.o` 16 → 316 (+300): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e2 — float-float gamma functions

Decision D64, `docs/research/libm-float-gamma.md`. Before → after, release build.

`libc.a`: code 148982 → 152582 (+3600), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34272 | 20 → 20 | 25802 → 25802 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9736 | 20 → 20 | 6600 → 6600 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `__libc_lgammaf_core.o` 0 → 3052 (+3052): new: the lgamma core (zeta series, Stirling, reflection) with the Taylor expansions at the 14 negative zeros (728 bytes of the table).
- `tgammaf.o` 16 → 408 (+392): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- `lgammaf.o` 16 → 172 (+156): computed in float-float (the `sp-nearest` layer) instead of a wrapper of the double function.
- Reduced: 0 members, 0 bytes; the largest: .

## P2.05e3a — float complex layer: exponential, logarithm, powers and roots

Decision D65, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 152582 → 157014 (+4432), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34272 | 20 → 20 | 25802 → 25802 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9736 | 20 → 20 | 6600 → 6600 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `clogf.o` 68 → 1384 (+1316): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cpowf.o` 110 → 1246 (+1136): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `csqrtf.o` 68 → 1032 (+964): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `cexpf.o` 68 → 524 (+456): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_exp2f_times.o` 0 → 372 (+372): new: 2^z m rounded once, shared by the complex functions.
- `__libc_sincosf_core.o` 0 → 246 (+246): new: float-float sin and cos of a float-float angle, shared by the complex functions.
- Reduced: 2 members, -58 bytes; the largest: `cabsf.o` 40 → 8, `cargf.o` 40 → 14.

## P2.05e3b — float complex trigonometric and hyperbolic functions

Decision D66, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 157014 → 160268 (+3254), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34272 | 20 → 20 | 25802 → 25802 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9736 | 20 → 20 | 6600 → 6600 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `ctanhf.o` 68 → 1136 (+1068): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_sinhcoshf_core.o` 0 → 912 (+912): new: float-float sinh and cosh, shared by csinhf, ccoshf and ctanhf.
- `ccoshf.o` 68 → 724 (+656): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `csinhf.o` 68 → 664 (+596): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_ff_round_product.o` 0 → 376 (+376): new: a float-float product rounded once with tiny factors prescaled, shared by the complex functions.
- `csinh.o` 572 → 596 (+24): the sign fix beyond 709 (one multiply).
- Reduced: 5 members, -378 bytes; the largest: `__libc_exp2f_times.o` 372 → 96, `ccosf.o` 68 → 34, `csinf.o` 68 → 36, `ctanf.o` 68 → 36.

## P2.05e3c — float complex inverse functions

Decision D67, `docs/research/libm-float-complex.md`. Before → after, release build.

`libc.a`: code 160268 → 168122 (+7854), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34272 | 20 → 20 | 25802 → 25802 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9736 | 20 → 20 | 6600 → 6600 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `cacosf.o` 68 → 1720 (+1652): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `casinhf.o` 68 → 1680 (+1612): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `catanhf.o` 68 → 1668 (+1600): computed in float (the complex `sp-nearest` layer) instead of a wrapper of the double function.
- `__libc_csqrtf_core.o` 0 → 956 (+956): new: float-float sqrt(a + ib), shared by cacosf and casinhf.
- `__libc_asinhf_core.o` 0 → 952 (+952): new: asinh of a float-float argument.
- `__libc_log1pf_core.o` 0 → 608 (+608): new: log1p of a float-float argument.
- `__libc_clogf_modulus.o` 0 → 568 (+568): new: log|z| as float-float, shared by the inverse functions.
- Reduced: 3 members, -94 bytes; the largest: `casinf.o` 68 → 36, `catanf.o` 68 → 36, `cacoshf.o` 68 → 38.

## P2.05e4 — float kernel sharing and table sizes

Decision D68, `docs/research/libm-float-kernels.md`. Before → after, release build.

`libc.a`: code 168122 → 167320 (-802), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2211 | 380 → 380 | 1428 → 1428 | 0 → 0 |
| hello-printf-int | 7945 → 7945 | 380 → 380 | 6709 → 6709 | 420 → 420 |
| hello-printf-float | 10550 → 10550 | 380 → 380 | 6709 → 6709 | 2984 → 2984 |
| malloc-heavy | 2286 → 2286 | 80 → 80 | 1346 → 1346 | 0 → 0 |
| math-mix | 34272 → 34176 | 20 → 20 | 25802 → 25706 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9736 → 9640 | 20 → 20 | 6600 → 6504 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 5047 | 380 → 380 | 3811 → 3811 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4779 | 380 → 380 | 3593 → 3593 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2206 | 76 → 76 | 1266 → 1266 | 0 → 0 |

Members that grew or are new, justified:

- `exp2m1f.o` 496 → 684 (+188): the exact-square expm1 series inlined.
- `expm1f.o` 432 → 588 (+156): the exact-square expm1 series inlined.
- `exp10m1f.o` 560 → 690 (+130): the exact-square expm1 series inlined.
- Reduced: 5 members, -1276 bytes; the largest: `asinhf.o` 984 → 100, `clogf.o` 1384 → 1196, `__libc_exp2f_core.o` 864 → 768, `__libc_atan2f_core.o` 1324 → 1260.

## P2.06a — Thumb-1 string kernels

Decision D69, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 167320 → 167130 (-190), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2211 → 2163 | 380 → 380 | 1428 → 1380 | 0 → 0 |
| hello-printf-int | 7945 → 7769 | 380 → 380 | 6709 → 6533 | 420 → 420 |
| hello-printf-float | 10550 → 10374 | 380 → 380 | 6709 → 6533 | 2984 → 2984 |
| malloc-heavy | 2286 → 2214 | 80 → 80 | 1346 → 1274 | 0 → 0 |
| math-mix | 34176 → 34116 | 20 → 20 | 25706 → 25646 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9640 → 9640 | 20 → 20 | 6504 → 6504 | 2708 → 2708 |
| hello-printf-int-nofloat | 5047 → 4987 | 380 → 380 | 3811 → 3751 | 420 → 420 |
| hello-printf-int-minimal | 4779 → 4719 | 380 → 380 | 3593 → 3533 | 372 → 372 |
| malloc-heavy-tiny | 2206 → 2134 | 76 → 76 | 1266 → 1194 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 5 members, -190 bytes; the largest: `memmove.o` 176 → 60, `memcpy.o` 96 → 60, `memset.o` 72 → 48, `strlen.o` 60 → 48.

## P2.06b — the string kernels on the Thumb-2 cores

Decision D70, `docs/research/string-asm.md`. Before → after, release build.

`libc.a`: code 167130 → 167130 (+0), RAM 1292 → 1292 (+0), TLS 366 → 366 (+0).

| Program | code | RAM | libc code | libgcc code |
|---|---|---|---|---|
| hello-puts | 2163 → 2163 | 380 → 380 | 1380 → 1380 | 0 → 0 |
| hello-printf-int | 7769 → 7769 | 380 → 380 | 6533 → 6533 | 420 → 420 |
| hello-printf-float | 10374 → 10374 | 380 → 380 | 6533 → 6533 | 2984 → 2984 |
| malloc-heavy | 2214 → 2214 | 80 → 80 | 1274 → 1274 | 0 → 0 |
| math-mix | 34116 → 34116 | 20 → 20 | 25646 → 25646 | 7752 → 7752 |
| float-exact | 3908 → 3908 | 20 → 20 | 1444 → 1444 | 1946 → 1946 |
| math-float | 9640 → 9640 | 20 → 20 | 6504 → 6504 | 2708 → 2708 |
| hello-printf-int-nofloat | 4987 → 4987 | 380 → 380 | 3751 → 3751 | 420 → 420 |
| hello-printf-int-minimal | 4719 → 4719 | 380 → 380 | 3533 → 3533 | 372 → 372 |
| malloc-heavy-tiny | 2134 → 2134 | 76 → 76 | 1194 → 1194 | 0 → 0 |

Members that grew or are new, justified:

- Reduced: 0 members, 0 bytes; the largest: .

## P3.04a — the lock word fast path in the port

Decision D79, `docs/research/locks.md`. Before → after, release build; `libc.a` is unchanged, the change is in
`port/linux` (the size images' port).

| Program | code | RAM | TLS |
|---|---|---|---|
| hello-puts | 2163 → 2155 | 380 → 340 | 8 → 8 |
| hello-printf-int | 7769 → 7761 | 380 → 340 | 8 → 8 |
| hello-printf-float | 10374 → 10366 | 380 → 340 | 8 → 8 |
| malloc-heavy | 2214 → 2206 | 80 → 44 | 8 → 8 |
| hello-printf-int-nofloat | 4987 → 4979 | 380 → 340 | 8 → 8 |
| hello-printf-int-minimal | 4719 → 4711 | 380 → 340 | 8 → 8 |
| malloc-heavy-tiny | 2134 → 2126 | 76 → 40 | 8 → 8 |

Other programs are unchanged. The port drops `__rtos_thread_self` from the lock path and the owner pointer from every lock (4 bytes on 32-bit cores, 8 on x86-64), and gains the thread-local token.

Grew, justified: `__rtos_lock_acquire`'s frame, 32 → 40 bytes, and so the depth of every entry point that takes a lock by 8 bytes (`malloc` 104 → 112, `calloc` 120 → 128, `realloc` 128 → 136, `free` 104 → 112, …). `__atomic_compare_exchange_4` is a five-argument call on this core, with four values live across it; every arrangement without the extra frame had more code, which the size budget does not allow.

## P3.05e — integer float multiplication, division and square root

Decision D85, `docs/research/softfloat.md`. Before → after, release build. The `nofp` layer replaces libgcc's
`__aeabi_fmul` (504 bytes) and `__aeabi_fdiv` (512) with integer ones (340 and 296). Its `sqrtf` replaces the `sp`
layer's, which went through the 64-bit `__libc_isqrt`.

`libc.a` text: 157196 → 157678 (+482), the two helpers added (+636) and `sqrtf` 320 → 166 (−154). The images link
those helpers instead of libgcc's.

| Program | code | libc code |
|---|---|---|
| math-mix | 34116 → 33584 | +482 |
| float-exact | 3908 → 3744 | +340 |
| math-float | 9640 → 9036 | +482 |

Other programs are unchanged.

Grew, justified:
- `__aeabi_fmul` and `__aeabi_fdiv` are new members, and they replace larger libgcc objects in every image that uses them.
- Each has a 32-byte frame. Thumb-1 has eight low registers, and the operands, the partial products or the remainder, and both exponents are live together. So the stack depth of `sinf`, `expf` and `powf` grows by 24 bytes (272 → 296, 160 → 184, 208 → 232), and `main` of `math-float` 312 → 336.
- The other arrangements tried were larger or slower: two calls to the shared rounding, which then is not inlined, made a 12-byte frame of its own and cost 24 bytes more code.

## P3.05f — fixed-point exponentials and logarithms (speed flavour)

Decision D86, `docs/research/libm-fixed.md`. The release build is unchanged. In the speed build, `libc/math/nofp/speed`
replaces `expf`, `exp2f`, `logf` and `log2f`: each keeps its float-float code as the fallback and adds a fixed-point
fast path, with the cores `__libc_exp2f_fixed` (250 bytes) and `__libc_log2f_fixed` (786) and the shared product
`__libc_fixed_wide` (48). Speed build, before → after:

| Program | code | budget (Phase 2 + 10 %) |
|---|---|---|
| math-float | 9036 → 10520 | 10604 |
| math-mix | 33584 → 35120 | 37527 |

Other programs are unchanged. Grew, justified: the fast paths are the step's purpose, and the speed flavour's budget
holds. `logf`'s frame grows from 40 to 48 bytes.

`-Os` had kept a local copy of the 64-bit product in each object, and the log table had 16-byte entries. With those,
`math-float` was 10728 bytes, over the budget; one shared helper and two arrays fixed it.

## P3.05g — fixed-point sine, cosine and tangent (speed flavour)

Decision D87, `docs/research/libm-fixed.md`. The release build is unchanged. The speed build adds `__libc_sinf_fixed`
(782 bytes, with a 144-byte table) and fast paths in `sinf` (+76), `cosf` (+76) and `tanf` (+256). Speed build,
before → after:

| Program | code | budget (Phase 2 + 20 %, D87) |
|---|---|---|
| math-float | 10520 → 11460 | 11568 |
| math-mix | 35120 → 36060 | 40939 |

Other programs are unchanged. Grew, justified: the fast paths are the step's purpose. `math-float` would have been
over the former +10 % (10604); by the user's choice the speed flavour's math images may grow by +20 %
(`budget.json`, `program_margins_percent`). The frames of `sinf`, `cosf` and `tanf` grow by 16–24 bytes.

## P3.05h — fixed-point error functions (speed flavour)

Decision D88, `docs/research/libm-fixed.md`. The release build is unchanged. The speed build adds
`__libc_erff_fixed` (2328 bytes, about 1 KB of it tables) and fast paths in `erff` (+68) and `erfcf` (+64). No
reference program links the error functions, so every image is unchanged; both budgets hold.

## P3.07 — introsort qsort (speed flavour)

Decision D90, `docs/research/qsort-speed.md`. The release build is unchanged. In the speed build, `qsort.o` grows by
about 625 bytes (the introsort keeps the heapsort as its fallback), and `qsort`'s stack depth by about 440 bytes, most
of it the stack of waiting ranges (one per bit of `size_t`). No reference program links `qsort`; every image is
unchanged and both budgets hold.

## D102 — the conversions to integers no longer link libgcc's arithmetic

`libc.a` grows by 618 bytes: the `soft-int` layer's `__aeabi_d2lz` (164), `__aeabi_d2ulz` (116), `__aeabi_f2lz` (116)
and `__aeabi_f2ulz` (80), and the nofp layer's `__aeabi_d2uiz` (80) and `__aeabi_f2uiz` (62). Before, a program that
converted a double to a 64-bit integer linked libgcc's conversions, which subtract in floating point and brought in
libgcc's `subdf3.o` (its own `__aeabi_dadd`, about 1.4 KB) in place of D85's. No reference program converts to a
64-bit integer, so every image is unchanged and both budgets hold.


## D104 — `.preinit_array` moves to the port

`__libc_start_main` runs only `.init_array` (24 bytes smaller); the port's start-up code walks `.preinit_array`. Every
reference image changes by −4 bytes. The baseline also records the D102 members (the soft-int and nofp conversions, `docs/size/armv6-m.md` D102), which that step did not write.

## D116 — the speed flavour's own budget

The speed flavour's fixed-point `powf` (PLAN D115) adds about 1.1 KB to the
images that link it, beyond the size figures plus D87's 20 % margin. Since
PLAN D116 (the maintainer's decision), `docs/size/budget.json` holds this
core's speed images to their own figures (`flavour_images.speed`), with no
margin, as armv7-m's since D110. Speed-flavour code bytes:

| Program | code |
|---|---:|
| hello-puts | 2239 |
| hello-printf-int | 8197 |
| hello-printf-int-nofloat | 5303 |
| hello-printf-int-minimal | 4831 |
| hello-printf-float | 10802 |
| malloc-heavy | 2326 |
| malloc-heavy-tiny | 2246 |
| float-exact | 3740 |
| math-float | 12536 |
| math-mix | 37136 |

The size flavour is unchanged.

## D121 — the C++ ABI hooks (issue #98)

New members: `__cxa_atexit` (code and the first static block of 32 registrations), `__cxa_finalize`, `__cxa_thread_atexit`, `__libc_cxa_thread_run` and `__dso_handle`. `__libc_atexit_run` and `thrd_exit` each gain one weak call (12–16 bytes) that runs the C++ `thread_local` destructors when they are linked. A C program links none of the new members, and every reference image is unchanged.
