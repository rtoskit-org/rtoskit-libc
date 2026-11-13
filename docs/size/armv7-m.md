# Size: armv7-m

Release build of `ARCH_CROSS=armv7-m` (Armv7-M without an FPU or the DSP
extension: Cortex-M3), measured with `tools/scripts/size-report` (PLAN D51,
`docs/research/size-measurement.md`). The figures are in `baseline-armv7-m.json`;
`tools/scripts/size-report diff` compares a build with them. The architecture
was added after phase 4 (PLAN D108, `docs/research/armv7-m.md`), so its history
starts here.

## D108 baseline

Against the two builds a Cortex-M3 or an FPU-less Cortex-M4 could use before:
`armv6s-m` (which runs there, as Thumb-1), and `armv7e-m` for scale (the same
instruction set plus DSP and a single-precision FPU, which these cores lack).
Whole-image code bytes:

| Program | armv6s-m | armv7-m | change | armv7e-m (FPU) |
|---|---:|---:|---:|---:|
| hello-puts | 2151 | 1867 | -13.2 % | 1883 |
| hello-printf-int | 7757 | 7193 | -7.3 % | 7165 |
| hello-printf-int-nofloat | 4975 | 4431 | -10.9 % | 4451 |
| hello-printf-int-minimal | 4707 | 4191 | -11.0 % | 4211 |
| hello-printf-float | 10362 | 9850 | -4.9 % | 8522 |
| malloc-heavy | 2202 | 1894 | -14.0 % | 1910 |
| malloc-heavy-tiny | 2122 | 1834 | -13.6 % | 1850 |
| float-exact | 3740 | 3164 | -15.4 % | 2044 |
| math-float | 9032 | 8300 | -8.1 % | 4814 |
| math-mix | 33580 | 30104 | -10.4 % | 26792 |

- The integer programs are within 30 bytes of armv7e-m's: the C code is the
  same Thumb-2.
- The floating-point programs are larger than armv7e-m's because every float
  operation is a call to libgcc's soft-float helpers (`libgcc code` below),
  where armv7e-m has the FPU.

`libc.a` (814 members), release build:

| text | rodata | unwind | data | bss | tdata | tbss |
|---:|---:|---:|---:|---:|---:|---:|
| 162406 | 9934 | 0 | 108 | 1184 | 8 | 358 |

Reference images (`test/size`): code is text+rodata+unwind, RAM is data+bss, TLS is tdata+tbss per thread; the libc and port shares come from the link map:

| Program | code | RAM | TLS | libc code | libc RAM | libc TLS | port code | libgcc code | main stack |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| hello-puts | 1867 | 340 | 8 | 1108 | 296 | 4 | 720 | 0 | 128* |
| hello-printf-int | 7193 | 340 | 8 | 6399 | 296 | 4 | 720 | 0 | 1252+ |
| hello-printf-float | 9850 | 340 | 8 | 7741 | 296 | 4 | 720 | 1276 | 1268+ |
| malloc-heavy | 1894 | 44 | 8 | 976 | 12 | 4 | 720 | 0 | 184* |
| math-mix | 30104 | 20 | 0 | 27710 | 0 | 0 | 272 | 1632 | 564* |
| float-exact | 3164 | 20 | 0 | 1378 | 0 | 0 | 272 | 1256 | 88* |
| math-float | 8300 | 20 | 0 | 6292 | 0 | 0 | 272 | 1568 | 312* |
| hello-printf-int-nofloat | 4431 | 340 | 8 | 3639 | 296 | 4 | 720 | 0 | 1252+ |
| hello-printf-int-minimal | 4191 | 340 | 8 | 3403 | 296 | 4 | 720 | 0 | 1252+ |
| malloc-heavy-tiny | 1834 | 40 | 8 | 916 | 8 | 4 | 720 | 0 | 184* |

Worst stack depth in bytes. `*`: without the frames of assembly or libgcc helpers (leaf routines that push a few registers); `+`: a lower bound because of an indirect call, recursion, a dynamic frame or an unknown callee:

| Function | depth | Function | depth | Function | depth |
|---|---:|---|---:|---|---:|
| `printf` | 1236+ | `fprintf` | 1232+ | `snprintf` | 1228+ |
| `sscanf` | 3168+ | `fscanf` | 3176+ | `strtod` | 1084+ |
| `strtof` | 1084+ | `strtol` | 120 | `malloc` | 72* |
| `calloc` | 88* | `realloc` | 88* | `free` | 72* |
| `qsort` | 68+ | `bsearch` | 32+ | `strftime` | 400+ |
| `mktime` | 408* | `localtime` | 300* | `gmtime` | 84 |
| `fopen` | 120* | `fclose` | 88* | `fputs` | 120* |
| `puts` | 120* | `fgets` | 120* | `fflush` | 112* |
| `exit` | 32+ | `abort` | 48+ | `thrd_create` | 104* |
| `mtx_lock` | 72* | `cnd_wait` | 96* | `exp` | 200* |
| `log` | 252* | `pow` | 356* | `sin` | 288* |
| `cos` | 288* | `tan` | 360* | `atan2` | 452* |
| `lgamma` | 820* | `tgamma` | 772* | `erfc` | 376* |
| `cpow` | 1172* | `sinf` | 272* | `expf` | 144* |
| `powf` | 208* |  |  |  |  |

Largest members (text+rodata+unwind+data+bss+tdata+tbss), top 20:

| Member | text | rodata | unwind | data | bss | tdata | tbss |
|---|---:|---:|---:|---:|---:|---:|---:|
| `__libc_lgamma_core.o` | 4060 | 360 | 0 | 0 | 0 | 0 | 0 |
| `__libc_lgammaf_core.o` | 2122 | 932 | 0 | 0 | 0 | 0 | 0 |
| `__libc_log_core.o` | 1360 | 1472 | 0 | 0 | 0 | 0 | 0 |
| `__libc_erfc_table.o` | 856 | 1824 | 0 | 0 | 0 | 0 | 0 |
| `cpow.o` | 2640 | 16 | 0 | 0 | 0 | 0 | 0 |
| `__libc_atan2_core.o` | 1876 | 528 | 0 | 0 | 0 | 0 | 0 |
| `strftime.o` | 1926 | 285 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format_float.o` | 1896 | 90 | 0 | 0 | 0 | 0 | 0 |
| `strerror.o` | 44 | 1924 | 0 | 0 | 0 | 0 | 0 |
| `__libc_format.o` | 1876 | 62 | 0 | 0 | 0 | 0 | 0 |
| `__libc_exp_core.o` | 900 | 1024 | 0 | 0 | 0 | 0 | 0 |
| `__libc_rem_pio2.o` | 1626 | 160 | 0 | 0 | 0 | 0 | 0 |
| `cacosf.o` | 1680 | 0 | 0 | 0 | 0 | 0 | 0 |
| `catanhf.o` | 1656 | 8 | 0 | 0 | 0 | 0 | 0 |
| `casinhf.o` | 1662 | 0 | 0 | 0 | 0 | 0 | 0 |
| `__libc_scan.o` | 1604 | 0 | 0 | 0 | 0 | 0 | 0 |
| `asinh.o` | 1536 | 0 | 0 | 0 | 0 | 0 | 0 |
| `atan2pi.o` | 1468 | 0 | 0 | 0 | 0 | 0 | 0 |
| `catanh.o` | 1420 | 0 | 0 | 0 | 0 | 0 | 0 |
| `__divdc3.o` | 1358 | 0 | 0 | 0 | 0 | 0 | 0 |

## D109 — `soft-sp`: the library's own float helpers

The float run-time ABI helpers are now the `soft-sp` layer's Thumb-2 assembly
instead of libgcc's (PLAN D109, `docs/research/softfloat-thumb2.md`). The layer
replaces libgcc's three grouped objects whole (`_arm_addsubsf3.o`,
`_arm_muldivsf3.o`, `_arm_cmpsf2.o`), so a program links only the helpers it
calls instead of a whole group. Whole-image code bytes:

| Program | D108 | D109 | change | libc share | libgcc share |
|---|---:|---:|---:|---:|---:|
| hello-printf-float | 9850 | 9026 | -824 | +388 | -1212 |
| float-exact | 3164 | 2980 | -184 | +952 | -1136 |
| math-float | 8300 | 8192 | -108 | +1340 | -1448 |
| math-mix | 30104 | 29992 | -112 | +1340 | -1448 (and 4 bytes of padding) |

The integer programs do not change. `libc.a` grows by 2252 bytes of text: the
twenty new members.

| Member | text | Member | text |
|---|---:|---|---:|
| `__mulsc3.o` | 636 | `__aeabi_fadd.o` | 402 |
| `__aeabi_fmul.o` | 354 | `__aeabi_fdiv.o` | 354 |
| `__aeabi_l2f.o` | 88 | `__powisf2.o` | 76 |
| `__aeabi_ul2f.o` | 76 | `__libc_softfloat_compare.o` | 62 |
| `__aeabi_i2f.o` | 44 | `__aeabi_ui2f.o` | 34 |
| the five `fcmp*` | 82 | `cfcmple`, `cfcmpeq`, `cfrcmple` | 28 |
| `fsub`, `frsub` | 16 | | |

- `__mulsc3` (64 bytes of stack) and `__powisf2` (24) are libgcc's callers of
  the replaced group, which the layer must define too (D100); no entry point's
  stack depth changes.
- The size budget holds (`size-report budget`); for the speed flavour see D110 below.

## D110 — the speed flavour's own budget

The speed flavour's images exceed the size flavour's figures by more than the
margins of D71 and D87 (`math-float` 10536 bytes against 9960), because they
carry the fixed-point maths paths (`nofp/speed`, D86–D88), which take 0.03–0.27
of the size flavour's instructions on this core. Since PLAN D110,
`docs/size/budget.json` holds armv7-m's speed images to their own figures
(`flavour_images.speed.armv7-m`), with no margin:

| Program | code | RAM |
|---|---:|---:|
| hello-puts | 1987 | 348 |
| hello-printf-int | 7721 | 348 |
| hello-printf-int-nofloat | 4797 | 348 |
| hello-printf-int-minimal | 4371 | 348 |
| hello-printf-float | 9554 | 348 |
| malloc-heavy | 2074 | 52 |
| malloc-heavy-tiny | 2014 | 48 |
| float-exact | 2980 | 20 |
| math-float | 10536 | 20 |
| math-mix | 32400 | 20 |

## D112 — Thumb-2 `strlen` in the speed flavour

The speed flavour's `strlen` is the Thumb-2 kernel (108 bytes, against the
Thumb-1 kernel's 44). The images that link it grow by 64 bytes, and armv7-m's
speed figures are re-locked with them (PLAN D112): `hello-puts` 2051,
`malloc-heavy` 2138, `malloc-heavy-tiny` 2078. The size flavour is unchanged.

## D113 — Thumb-2 `memchr` in the speed flavour

The speed flavour's `memchr` is the Thumb-2 kernel (154 bytes, against the C
version's 88). The images that link it (everything that prints or calls `puts`,
through line-buffered writes) grow by 66 bytes, and armv7-m's speed figures are
re-locked with them (PLAN D113): `hello-puts` 2119, `hello-printf-int` 7785,
`hello-printf-int-nofloat` 4865, `hello-printf-int-minimal` 4439,
`hello-printf-float` 9618. The size flavour is unchanged.

## D115, D116 — fixed-point `powf`

The speed flavour's fixed-point `powf` (PLAN D115) adds about 1.0 KB to
`math-float` and `math-mix`; armv7-m's speed figures are re-locked with it
under D116: `math-float` 11580, `math-mix` 33440. The size flavour is
unchanged.

## D121 — the C++ ABI hooks (issue #98)

New members: `__cxa_atexit` (code and the first static block of 32 registrations), `__cxa_finalize`, `__cxa_thread_atexit`, `__libc_cxa_thread_run` and `__dso_handle`. `__libc_atexit_run` and `thrd_exit` each gain one weak call (12–16 bytes) that runs the C++ `thread_local` destructors when they are linked. A C program links none of the new members, and every reference image is unchanged.

## D124 — `(float)` of a NaN keeps its sign (issue #95)

The libm vectors found that `__libc_softdouble_d2f` set a NaN's sign from the double's exponent bits, so every narrowed NaN came out negative. Masking the payload adds 4 bytes to `__libc_softdouble_d2f.o`; no reference image changes.

## D125 — the libm vectors' accuracy fixes (Q36)

Correctness fixes, so the growth is accepted: `cbrt.o` +84, `tgamma.o` +504 and `pow.o` +80 bytes. `cbrt` gains an exact-residual Halley step, `tgamma` a double-double reciprocal for tiny arguments, and `pow` the tiny-exponent branch. `math-mix` grows by 80 bytes from `pow`. The size budget holds; the speed flavour's `math-mix` figure is re-locked at 33520 bytes (from 33440).

## D131 — the POSIX subset's `errno` codes (P7.02)

`strerror.o` grows by 181 bytes of message text for `ENOTUNIQ`, `EBADFD`, `ESOCKTNOSUPPORT`, `ESHUTDOWN`, `EHOSTDOWN` and `EFTYPE`. No reference image links `strerror`, so every image and both budgets are unchanged.

## D132 — the POSIX subset's functions (P7.03)

The new members (`open`, `read`, `write`, `close`, `lseek`, `fileno`, `fdopen`, `strnlen`, `strcasecmp`, `strncasecmp`, `reallocarray`, `sincos`, `sincosf`, `clock_gettime`, `clock_getres`, `nanosleep`, `sched_yield`) are linked only by programs that call them. `__libc_file_new` no longer closes the handle when it fails, so that a failed `fdopen` leaves the descriptor to its caller: `__libc_file_new.o` shrinks by 12 bytes, and its callers close the handle themselves, `fopen.o` +12 and `tmpfile.o` +24 bytes. `fopen`'s stack depth grows by 8 bytes. No reference image links any of them, so every image and both budgets are unchanged.

## D134 — `fstat`, `fcntl` and `getpid` (P7.04)

Three new members (`fstat`, `fcntl`, `getpid`), linked only by programs that call them; no other member, no image and neither budget changes. The optional `LIBC_UNWIND_TABLES=1` build is not measured here: the baselines and budgets are without unwind tables (D134; its cost is in `docs/research/posix-subset.md`).

## P8.02 — One reduction for `sincos` and `sincosf`

`sincos.o` grows to about 520–545 bytes and `sincosf.o` (the `sp-nearest` file) to 184–200 bytes: each now holds the entry tests, one reduction and both cores' calls instead of two calls. `sincos.o` is still smaller than `sin.o` + `cos.o`, so a program that calls only `sincos` shrinks; one that also calls `sin` and `cos` grows by the difference. No reference image links them; every image and both budgets are unchanged.

## P8.03 — Fixed-point double `sin`, `cos` and `sincos` (speed flavour, D136)

Speed flavour only: `__libc_trig_fixed.o`, `__libc_trig_dd.o` and `__libc_trig_fixed_round.o` (1,372 + 516 + 92 bytes) are new, and the speed `sin.o`, `cos.o` and `sincos.o` shrink by 110–165 bytes each (their double-double code moves to `__libc_trig_dd.o`). A program that calls `sin` or `cos` links the kernel and keeps the double-double cores for the fallback, about 2 KB more. This core's speed figures are exact (D110), so `math-mix`'s is re-locked under D138. The size flavour and its baselines are unchanged.

## P9.01 — `UMULL` for the float fixed-point functions (speed flavour, D140)

`__libc_fixed_wide.o` (the 16-bit product helper) leaves armv7-m's speed library, and its callers multiply inline. `math-float` shrinks by 168 bytes and `math-mix` by 172, and the exact speed figures are re-locked downwards under D140. The size flavour is unchanged.

## P9.02, P9.03 — The float functions and `tan` through the D136 kernel (speed flavour, D141, D142)

`sinf` and `cosf` now link the double kernel ahead of their D87 path, and the fixed-point `tan` keeps the double-double path as its fallback. The exact speed figures are re-locked under D142: `math-float` 11,412 → 13,516, `math-mix` 35,024 → 36,376 bytes. The size flavour is unchanged.

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

Members that grew, justified: 106 members, nearly all new (`libc/pthread`, `sched_get_priority_max`/`min`): `<pthread.h>`, linked only by programs that use threads; the seam's futex time base (D150) adds an argument at each wait. Reduced: 15 members, -690 bytes, mostly the C11 functions that are now wrappers over `<pthread.h>` (D156) and the removed `__libc_mutex_lock`, `__libc_mutex_unlock`, `__libc_cond_wait` and `__libc_thread_start`. Images: the seven programs that link the Linux test port's lock grow by 16 bytes each, with the library's share unchanged: the port's `__rtos_futex_wait` rejects an unknown time base (D150) and `__rtos_lock_acquire` passes one. The exact speed figures (D110) move by the same bytes and are re-locked (D157).

## P11.02a — The `fast` build type (D162, D164)

No member, image or stack depth changed: the size and speed flavours build the same objects, and the fast
flavour's `libc.a` has the speed flavour's members, byte for byte but the debug sections
(`size-report --build-type fast same --as speed`). No `libc/math/fast/` member yet. The fast flavour is budgeted
as the speed flavour (`docs/size/budget.json`).

## P11.03a — Fast `expf`, `exp2f`, `exp10f` (D168)

Release and speed unchanged (`size-report diff`; the speed budget holds). Fast members: none: fast is the speed flavour here (`size-report --build-type fast same --as speed`). The fast flavour's
budget holds.

## P11.03b — Fast `logf`, `log2f`, `log10f` (D169)

Release and speed unchanged (`size-report diff`); the fast budget holds. Fast members (code and constants, speed →
fast, bytes): none: fast is the speed flavour here.

## P11.03c — Fast `powf` (D170)

Release and speed: no growth (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed → fast, bytes): none: fast is the speed flavour here.

## P11.03d — Fast m1 and p1 forms, `cbrtf` (D171)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants,
speed → fast, bytes; several speed members call a shared core or the double function, not counted): none: fast is the speed flavour here.

## P11.03e — The cores without an FPU: fixed point without the test (D172)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed
→ fast, bytes; the speed files' float-float fallback and its cores, which fast leaves out of the logarithms, not
counted): `expf.o` 448 → 432, `exp2f.o` 232 → 212, `exp10f.o` 312 → 408, `logf.o` 452 → 196, `log2f.o` 212 → 152, `log10f.o` 452 → 196, `powf.o` 978 → 954.

## P11.03f — The cores without an FPU: fixed-point m1 forms (D173)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed
→ fast, bytes; speed's float-float cores, which fast no longer links for these, not counted): `expm1f.o` 592 → 580, `exp2m1f.o` 688 → 592, `exp10m1f.o` 698 → 616.

## P11.03g — The cores without an FPU: fixed-point p1 forms, `cbrtf` (D174)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold. Fast members (code and constants, speed
→ fast, bytes; speed's float-float cores, which fast no longer links for these, not counted): `log1pf.o` 336 → 894, `log2p1f.o` 368 → 1,086, `log10p1f.o` 500 → 1,138, `cbrtf.o` 592 → 324, `exp2m1f.o` 688 → 844, `exp10m1f.o` 698 → 864.

## P11.04a — Fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D175)

Unchanged here (`size-report diff`; the budgets hold).

## P11.04b — Fast `asinf`, `acosf`, `atanf`, `atan2f` and the π forms (D176)

Unchanged here (`size-report diff`; the budgets hold): the strict atan core's index fix (D176) is the same size on these
cores. The cores without an FPU follow in P11.04f.

## P11.04c — Fast `sinhf`, `coshf`, `tanhf`, `asinhf`, `acoshf`, `atanhf` (D177)

Unchanged here (`size-report diff`; the budgets hold). The cores without an FPU follow in P11.04g.

## P11.04d1 — Fast `erff`, and `erfcf` on the double-FPU cores (D178)

Unchanged here (`size-report diff`; the budgets hold). The cores without an FPU follow in P11.04e.

## P11.04d2 — Fast `erfcf` on the single-precision FPU cores (D179)

Unchanged here (`size-report diff`; the budgets hold): the cores without an FPU follow in P11.04e.

## P11.04e1 — The cores without an FPU: fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D180)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold, armv6-m's, armv6s-m's and armv7-m's own fast
figures included once the reduction and the kernel's combination moved out of line (inline in each caller, `math-mix`'s
`sinf` and `cosf` took 604 bytes over the locked figure on armv6-m). Fast members (code and constants, speed → fast, bytes):
`sinf.o` 288 → 358, `cosf.o` 268 → 350, `sincosf.o` 412 → 488, `tanf.o` 652 → 726, `sinpif.o` 508 → 822, `cospif.o` 388 → 602, `tanpif.o` 716 → 1098, `__libc_fixed_trig_reduce.o` new → 284, `__libc_fixed_trig_combine.o` new → 520.

## P11.04e2 — The cores without an FPU: fast `erff` (D181)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold (no size program calls `erff`). Fast members (code and constants, speed → fast, bytes): `erff.o` 396 → 694; it no longer pulls `__libc_erff_fixed.o` (2238) or `__libc_erff_core.o` (732), which stay for `erfcf`, so a program calling `erff` alone takes 3366 → 694.

## P11.04f1 — The cores without an FPU: fast `atanf`, `atan2f` and their π forms (D182)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold (no size program calls them). Fast members (code and constants, speed → fast, bytes): `atanf.o` 112 → 236, `atan2f.o` 776 → 532, `atanpif.o` 512 → 504, `atan2pif.o` 884 → 700, `__libc_fixed_atan.o` new → 716; they no longer pull `__libc_atan2f_core.o` (1270) or the float-float helpers, which stay for `asinf` and `acosf` until P11.04f2.

## P11.04f2 — The cores without an FPU: fast `asinf`, `acosf` and their π forms (D183)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold (no size program calls them). Fast members (code and constants, speed → fast, bytes): `asinf.o` 594 → 216, `acosf.o` 596 → 308, `asinpif.o` 800 → 444, `acospif.o` 716 → 424, `__libc_fixed_asin.o` new → 476; no real inverse function pulls `__libc_atan2f_core.o` or the float-float square root any more (the complex functions still do).

## P11.04g1 — The cores without an FPU: fast `sinhf`, `coshf` and `tanhf` (D184)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold (no size program calls them). Fast members (code and constants, speed → fast, bytes): `sinhf.o` 768 → 600, `coshf.o` 212 → 372, `tanhf.o` 990 → 732; they no longer pull the float-float exponential cores (speed's `coshf` is smaller only because those cores sit in other members).

## P11.04g2 — The cores without an FPU: fast `asinhf`, `acoshf` and `atanhf` (D185)

Release and speed unchanged (`size-report diff`); the speed and fast budgets hold (no size program calls them). Fast members (code and constants, speed → fast, bytes): `asinhf.o` 100 → 304, `acoshf.o` 984 → 416, `atanhf.o` 976 → 1018, `__libc_fixed_log_root.o` new → 898, `__libc_fixed_rsqrt_seed.o` new → 48 (the seeds moved out of `__libc_fixed_asin.o`, 476 → 428); speed's `asinhf.o` is small only because its float-float core sits in other members, which these no longer pull.

## P11.05b — Fast `exp`, `exp2`, `exp10` and `expm1` in double (D187)

Release and speed unchanged (`size-report diff`). `math-mix` (exp and pow) grows from 36376 to 37552 bytes of code in fast: the fast `exp` and its 1 KiB table on top of the strict core that pow still uses. The fast figures are re-locked (`size-report --flavour fast --lock`), as the maintainer decided for P11.05 (D187); `math-float` is re-locked smaller with them (P11.04's float steps took it below its speed figure).

## P11.05c1 — Fast `exp`, `exp2` and `exp10` in 64-bit fixed point (D188)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (exp and pow) shrinks from 37552 to 37304 bytes of code (36376 in speed): the fixed-point `exp`, its kernel and its 512-byte table replace D187's double file and its 1 KiB table, and the slow path stays the strict core that pow shares. Smaller than the locked fast figure, so it is not re-locked (P11.06 re-locks every core, D187).

## P11.05c2 — Fast `expm1` in 64-bit fixed point (D189)

Release and speed unchanged (`size-report diff`); the fast budgets hold. No size program calls `expm1`, and `math-mix` loses 8 bytes in fast (the kernel's polynomial as an inline).

## P11.05d — Fast `log`, `log2`, `log10` and `log1p` in double (D190)

Release and speed unchanged (`size-report diff`). `math-mix` (log and pow) grows from 37552 to 40832 bytes of code in fast (+3280): the fast `log` and its 3 KiB table on top of the strict log core that pow still uses. The fast figures are re-locked (`size-report --flavour fast --lock`), as the maintainer decided for P11.05 (D187).

## P11.05e — Fast `log`, `log2`, `log10` and `log1p` in 64-bit fixed point (D192)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (log and pow) shrinks from 40832 to 40296 bytes of code: the 1440-byte table and the two kernels replace D190's file and its 3 KiB table. Smaller than the locked fast figure, so it is not re-locked (P11.06 does, D187, D191).

## P11.05f — Fast `pow` and `cbrt` in double (D193)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (log and pow) grows from 40296 to 41392 bytes of code (+1096): pow runs the double file on this core's helpers until P11.05g, with the double logarithms' 3 KiB table beside the fixed-point one. The fast figures are re-locked (`size-report --flavour fast --lock`), as the maintainer decided for P11.05 (D187).

## P11.05g1 — Fast `pow` in fixed point (D194)

Release and speed unchanged (`size-report diff`). In fast, `math-mix` (log and pow) shrinks from 41392 to 38980 bytes of code (+7.2 % over speed's 36376, inside D191's +20 %): the fixed-point pow shares the logarithms' table and the exponentials' kernel, and D193's double table and the exp pairs leave this core. Not re-locked (P11.06 does, D187, D191).

## P11.05g2 — Fast `cbrt` in fixed point (D195)

Release and speed unchanged (`size-report diff`). In fast, no size program calls `cbrt`, and every budget holds; nothing is re-locked.
