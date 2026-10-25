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
