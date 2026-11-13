# Benchmarks: armv7-m

Core: Armv7-M without an FPU or the DSP extension (Cortex-M3; PLAN D108,
`docs/research/armv7-m.md`).

Method: `docs/research/benchmarks.md` (PLAN D72). Regenerate the tables with
`tools/scripts/bench-runner [--build-type speed] markdown --arch armv7-m [--previous <rev> | --against <file>]`;
`docs/perf/baseline-armv7-m.json` (size flavour) and `docs/perf/baseline-armv7-m-speed.json` (speed flavour, PLAN D73)
hold every case, including the full string sweep.

## D108 baseline

The architecture was added after phase 4, so its history starts here.

Against `armv6s-m`, the build a Cortex-M3 had to link before: the geometric mean
of armv7-m's instructions per operation over armv6s-m's, per program (below 1 is
fewer; math counts the throughput cases only):

| Program | Cases | Size flavour | Speed flavour |
|---|---:|---:|---:|
| string | 603 | 0.99 | 0.80 |
| stdio | 23 | 0.73 | 0.69 |
| strto | 21 | 0.36 | 0.50 |
| malloc | 9 | 0.44 | 0.43 |
| qsort | 8 | 0.84 | 0.86 |
| math | 62 | 0.48 | 0.50 |
| softfloat | 20 | 0.40 | 0.40 |
| threads | 7 | 0.53 | 0.53 |

- The size flavour's string kernels are the same Thumb-1 code on both, hence 0.99.
- The speed flavour adds the Thumb-2 `memcpy`/`memmove`/`memset`/`strcmp`; `strchr` stays C, the DSP version being armv7e-m's.
- strto, malloc, math and softfloat gain most: hardware divide, `UMULL`/`UMLAL` and libgcc's Thumb-2 soft-float assembly (replaced by the library's own in D109, below).

### Size flavour

Instructions per operation (qemu 8.2.2, release, size flavour).

#### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 33.0 / 28.0 | 37.0 / 37.0 | 38.0 / 64.0 | 53.0 / 92.0 | 113 / 284 | 353 / 1052 | 1313 / 4124 |
| `memmove-down` | 39.0 / 34.0 | 43.0 / 43.0 | 44.0 / 70.0 | 59.0 / 98.0 | 119 / 290 | 359 / 1058 | 1319 / 4130 |
| `memmove-up` | 39.0 / 34.0 | 43.0 / 43.0 | 44.0 / 70.0 | 52.0 / 106 | 112 / 346 | 352 / 1306 | 1312 / 5146 |
| `memset` | 30.0 / 28.0 | 33.0 / 37.0 | 34.0 / 54.0 | 46.0 / 66.0 | 94.0 / 114 | 286 / 306 | 1054 / 1074 |
| `memcmp` | 27.0 / 27.0 | 35.0 / 35.0 | 59.0 / 59.0 | 155 / 155 | 539 / 539 | 2075 / 2075 | 8219 / 8219 |
| `memchr` | 29.0 / 29.0 | 36.0 / 28.0 | 65.0 / 66.0 | 95.0 / 98.0 | 215 / 218 | 695 / 698 | 2615 / 2618 |
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 52.0 / 38.0 | 48.0 / 35.0 | 69.0 / 75.0 | 102 / 108 | 234 / 240 | 762 / 768 | 2874 / 2880 |
| `strcmp` | 28.0 / 28.0 | 36.0 / 36.0 | 60.0 / 60.0 | 156 / 156 | 540 / 540 | 2076 / 2076 | 8220 / 8220 |
| `strcpy` | 65.0 / 60.0 | 76.0 / 74.0 | 75.0 / 124 | 105 / 167 | 225 / 419 | 705 / 1427 | 2625 / 5459 |
| `mix/memcpy` (per call) | 215 | | | | | | |
| `mix/memset` (per call) | 89.6 | | | | | | |
| `mix/strlen` (per call) | 113 | | | | | | |

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2326 |
| `fputs/line` | 362 |
| `snprintf/%-20s` | 615 |
| `snprintf/%.17g` | 14944 |
| `snprintf/%.3f` | 15005 |
| `snprintf/%08x` | 537 |
| `snprintf/%a` | 962 |
| `snprintf/%c` | 267 |
| `snprintf/%d` | 475 |
| `snprintf/%e` | 14034 |
| `snprintf/%f` | 14751 |
| `snprintf/%g` | 15561 |
| `snprintf/%lld` | 7643 |
| `snprintf/%p` | 457 |
| `snprintf/%s` | 424 |
| `snprintf/%u-small` | 364 |
| `snprintf/%x` | 518 |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1960 |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 3146 |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2090 |

#### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1081 |
| `strtod/17-digits` | 5248 |
| `strtod/exp-large` | 176522 |
| `strtod/exp-small` | 29424 |
| `strtod/fixed` | 1832 |
| `strtod/hex` | 974 |
| `strtod/integer` | 1348 |
| `strtod/long` | 8474 |
| `strtod/micro` | 1684 |
| `strtod/min-normal` | 104270 |
| `strtod/short` | 1562 |
| `strtod/subnormal` | 114004 |
| `strtof/9-digits` | 4259 |
| `strtof/exp` | 9189 |
| `strtof/short` | 1335 |
| `strtol/hex` | 1195 |
| `strtol/int` | 1213 |
| `strtol/short` | 949 |
| `strtoll/long` | 1496 |
| `strtoul/hex16` | 1179 |
| `strtoull/hex` | 1446 |

#### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 118 |
| `batch/fifo` | 96.0 |
| `batch/lifo` | 91.1 |
| `batch/shuffled` | 111 |
| `calloc/256` | 234 |
| `fixed/16` | 92.0 |
| `fixed/256` | 92.0 |
| `fixed/4096` | 92.0 |
| `realloc/grow` | 99.1 |

#### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 222 |
| `batch/fifo` | 206 |
| `batch/lifo` | 206 |
| `batch/shuffled` | 206 |
| `calloc/256` | 238 |
| `fixed/16` | 96.5 |
| `fixed/256` | 96.5 |
| `fixed/4096` | 96.5 |
| `realloc/grow` | 97.0 |

#### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 147 |
| `qsort/equal/1024` | 114 |
| `qsort/random/1024` | 640 |
| `qsort/random/16` | 224 |
| `qsort/random/256` | 497 |
| `qsort/reversed/1024` | 602 |
| `qsort/sorted/1024` | 677 |
| `qsort/strings/256` | 595 |

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 13210 | 14622 |
| `acosh` | 11077 | 14203 |
| `asin` | 12697 | 14027 |
| `asinh` | 10705 | 16349 |
| `atan` | 9690 | 8476 |
| `atan2` | 10178 | 9099 |
| `atanh` | 8530 | 10300 |
| `cbrt` | 4299 | 5098 |
| `cos` | 9875 | 6500 |
| `cosh` | 8122 | 6357 |
| `erf` | 75967 | 14711 |
| `erfc` | 63393 | 44018 |
| `exp` | 5950 | 3874 |
| `exp2` | 4927 | 5320 |
| `expm1` | 6584 | 4128 |
| `floor` | 84.2 | 102 |
| `fmod` | 234 | 291 |
| `hypot` | 3674 | 6065 |
| `lgamma` | 20158 | 18552 |
| `log` | 6754 | 6625 |
| `log10` | 6748 | 8101 |
| `log1p` | 6970 | 7192 |
| `log2` | 5826 | 8227 |
| `pow` | 12201 | 12053 |
| `round` | 78.2 | 99.5 |
| `sin` | 9968 | 6524 |
| `sinh` | 8248 | 6776 |
| `sqrt` | 478 | 1972 |
| `tan` | 19593 | 11752 |
| `tanh` | 8585 | 7663 |
| `tgamma` | 30317 | 25199 |

#### softfloat

| Case | insns/op |
|---|---:|
| `d2f` | 19.4 |
| `d2i` | 19.9 |
| `d2l` | 44.9 |
| `dadd` | 56.2 |
| `dcmp` | 29.9 |
| `ddiv` | 354 |
| `dmul` | 58.0 |
| `dsub` | 59.1 |
| `f2d` | 15.9 |
| `f2i` | 15.9 |
| `f2l` | 27.4 |
| `fadd` | 59.3 |
| `fcmp` | 42.9 |
| `fdiv` | 156 |
| `fmul` | 39.9 |
| `fsub` | 58.2 |
| `i2d` | 21.6 |
| `i2f` | 29.9 |
| `l2d` | 30.1 |
| `l2f` | 32.9 |

#### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 19.0 |
| `mtx/nested` | 147 |
| `mtx/plain` | 84.0 |
| `mtx/recursive` | 87.0 |
| `mtx/timed` | 84.0 |
| `mtx/trylock` | 78.0 |
| `tss_get` | 34.0 |


### Speed flavour

Instructions per operation (qemu 8.2.2, speed, speed flavour).

#### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 19.0 / 19.0 | 34.0 / 34.0 | 34.0 / 34.0 | 42.0 / 42.0 | 54.0 / 81.0 | 78.0 / 153 | 174 / 441 |
| `memmove-down` | 23.0 / 23.0 | 38.0 / 38.0 | 38.0 / 38.0 | 46.0 / 46.0 | 58.0 / 85.0 | 82.0 / 157 | 178 / 445 |
| `memmove-up` | 23.0 / 23.0 | 38.0 / 38.0 | 38.0 / 38.0 | 44.0 / 44.0 | 54.0 / 77.0 | 78.0 / 143 | 174 / 407 |
| `memset` | 18.0 / 18.0 | 35.0 / 35.0 | 35.0 / 35.0 | 39.0 / 39.0 | 57.0 / 59.0 | 81.0 / 83.0 | 177 / 179 |
| `memcmp` | 27.0 / 27.0 | 35.0 / 35.0 | 59.0 / 59.0 | 155 / 155 | 539 / 539 | 2075 / 2075 | 8219 / 8219 |
| `memchr` | 29.0 / 29.0 | 36.0 / 28.0 | 65.0 / 66.0 | 95.0 / 98.0 | 215 / 218 | 695 / 698 | 2615 / 2618 |
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 52.0 / 38.0 | 48.0 / 35.0 | 69.0 / 75.0 | 102 / 108 | 234 / 240 | 762 / 768 | 2874 / 2880 |
| `strcmp` | 27.0 / 27.0 | 33.0 / 33.0 | 53.0 / 53.0 | 144 / 133 | 240 / 430 | 624 / 1582 | 2160 / 6190 |
| `strcpy` | 62.0 / 62.0 | 69.0 / 69.0 | 69.0 / 92.0 | 92.0 / 115 | 164 / 212 | 428 / 524 | 1484 / 1772 |
| `mix/memcpy` (per call) | 54.1 | | | | | | |
| `mix/memset` (per call) | 41.7 | | | | | | |
| `mix/strlen` (per call) | 113 | | | | | | |

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2151 |
| `fputs/line` | 255 |
| `snprintf/%-20s` | 575 |
| `snprintf/%.17g` | 14846 |
| `snprintf/%.3f` | 14919 |
| `snprintf/%08x` | 469 |
| `snprintf/%a` | 862 |
| `snprintf/%c` | 261 |
| `snprintf/%d` | 455 |
| `snprintf/%e` | 13869 |
| `snprintf/%f` | 14669 |
| `snprintf/%g` | 15476 |
| `snprintf/%lld` | 1353 |
| `snprintf/%p` | 420 |
| `snprintf/%s` | 396 |
| `snprintf/%u-small` | 353 |
| `snprintf/%x` | 450 |
| `snprintf/literal` | 308 |
| `snprintf/mix` | 1837 |
| `sscanf/%d` | 364 |
| `sscanf/%lf` | 2131 |
| `sscanf/%x` | 423 |
| `sscanf/mix` | 1602 |

#### strto

| Case | insns/op |
|---|---:|
| `atoi` | 207 |
| `strtod/17-digits` | 1241 |
| `strtod/exp-large` | 1130 |
| `strtod/exp-small` | 1296 |
| `strtod/fixed` | 941 |
| `strtod/hex` | 953 |
| `strtod/integer` | 499 |
| `strtod/long` | 2258 |
| `strtod/micro` | 815 |
| `strtod/min-normal` | 1287 |
| `strtod/short` | 737 |
| `strtod/subnormal` | 1338 |
| `strtof/9-digits` | 936 |
| `strtof/exp` | 964 |
| `strtof/short` | 510 |
| `strtol/hex` | 322 |
| `strtol/int` | 305 |
| `strtol/short` | 135 |
| `strtoll/long` | 516 |
| `strtoul/hex16` | 309 |
| `strtoull/hex` | 508 |

#### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 118 |
| `batch/fifo` | 96.0 |
| `batch/lifo` | 91.1 |
| `batch/shuffled` | 111 |
| `calloc/256` | 131 |
| `fixed/16` | 92.0 |
| `fixed/256` | 92.0 |
| `fixed/4096` | 92.0 |
| `realloc/grow` | 99.1 |

#### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 222 |
| `batch/fifo` | 206 |
| `batch/lifo` | 206 |
| `batch/shuffled` | 206 |
| `calloc/256` | 136 |
| `fixed/16` | 96.5 |
| `fixed/256` | 96.5 |
| `fixed/4096` | 96.5 |
| `realloc/grow` | 97.0 |

#### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 147 |
| `qsort/equal/1024` | 27.1 |
| `qsort/random/1024` | 274 |
| `qsort/random/16` | 128 |
| `qsort/random/256` | 216 |
| `qsort/reversed/1024` | 35.6 |
| `qsort/sorted/1024` | 27.1 |
| `qsort/strings/256` | 303 |

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 13210 | 14622 |
| `acosh` | 11077 | 14203 |
| `asin` | 12697 | 14027 |
| `asinh` | 10705 | 16349 |
| `atan` | 9690 | 8476 |
| `atan2` | 10178 | 9099 |
| `atanh` | 8530 | 10300 |
| `cbrt` | 4299 | 5098 |
| `cos` | 2120 | 6500 |
| `cosh` | 8122 | 6357 |
| `erf` | 2126 | 14711 |
| `erfc` | 2065 | 44018 |
| `exp` | 284 | 3874 |
| `exp2` | 238 | 5320 |
| `expm1` | 6584 | 4128 |
| `floor` | 84.2 | 102 |
| `fmod` | 234 | 291 |
| `hypot` | 3674 | 6065 |
| `lgamma` | 20158 | 18552 |
| `log` | 1310 | 6625 |
| `log10` | 6748 | 8101 |
| `log1p` | 6970 | 7192 |
| `log2` | 401 | 8227 |
| `pow` | 12201 | 12053 |
| `round` | 78.2 | 99.5 |
| `sin` | 2682 | 6524 |
| `sinh` | 8248 | 6776 |
| `sqrt` | 478 | 1972 |
| `tan` | 2736 | 11752 |
| `tanh` | 8585 | 7663 |
| `tgamma` | 30317 | 25199 |

#### softfloat

| Case | insns/op |
|---|---:|
| `d2f` | 19.4 |
| `d2i` | 19.9 |
| `d2l` | 44.9 |
| `dadd` | 56.2 |
| `dcmp` | 29.9 |
| `ddiv` | 354 |
| `dmul` | 58.0 |
| `dsub` | 59.1 |
| `f2d` | 15.9 |
| `f2i` | 15.9 |
| `f2l` | 27.4 |
| `fadd` | 59.3 |
| `fcmp` | 42.9 |
| `fdiv` | 156 |
| `fmul` | 39.9 |
| `fsub` | 58.2 |
| `i2d` | 21.6 |
| `i2f` | 29.9 |
| `l2d` | 30.1 |
| `l2f` | 32.9 |

#### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 19.0 |
| `mtx/nested` | 147 |
| `mtx/plain` | 84.0 |
| `mtx/recursive` | 87.0 |
| `mtx/timed` | 84.0 |
| `mtx/trylock` | 78.0 |
| `tss_get` | 34.0 |


## D109 — `soft-sp`: the library's own float helpers

The float run-time ABI helpers (`fadd fsub frsub fmul fdiv`, the comparisons and
`i2f ui2f l2f ul2f`) are now the `soft-sp` layer's Thumb-2 assembly instead of
libgcc's (PLAN D109, `docs/research/softfloat-thumb2.md`). Instructions per
operation in the `softfloat` program, the same in both flavours, with Arm Optimized
Routines' `at32` helpers for scale (`bench-runner aor`):

| Operation | libgcc (D108) | `soft-sp` | change | AOR |
|---|---:|---:|---:|---:|
| `fadd` | 59.3 | 56.3 | -5 % | 38.1 |
| `fsub` | 58.2 | 52.1 | -11 % | 37.6 |
| `fmul` | 39.9 | 39.3 | -2 % | 36.9 |
| `fdiv` | 156 | 57.7 | -63 % | 58.9 |
| `fcmp` | 42.9 | 28.9 | -33 % | 21.3 |
| `i2f` | 29.9 | 20.9 | -30 % | 20.9 |
| `l2f` | 32.9 | 26.9 | -18 % | 27.3 |
| `ddiv` | 354 | 245 | -31 % | |

- `ddiv` gains because `soft-dp` computes its reciprocal with a float division.
- Geometric means against D108: math 0.938 (size flavour) and 0.947 (speed), softfloat 0.883, strto 0.987 and 0.969; `cbrt`/`cbrtf` gain most (0.82–0.83).
- One case grows: `strtof/exp`, +0.1 % (size) and +0.9 % (speed), inside the 1 % gate. libgcc's `fmul` has a shortcut for a power-of-two operand, which that case multiplies by; `soft-sp`'s does not, as it would cost every other product a test.

### Size flavour

Instructions per operation (qemu 8.2.2, release, size flavour), change against the D108 baseline (5e57f83).

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2326 |
| `fputs/line` | 362 |
| `snprintf/%-20s` | 615 |
| `snprintf/%.17g` | 14944 |
| `snprintf/%.3f` | 14898 (-1 %) |
| `snprintf/%08x` | 537 |
| `snprintf/%a` | 962 |
| `snprintf/%c` | 267 |
| `snprintf/%d` | 475 |
| `snprintf/%e` | 14034 |
| `snprintf/%f` | 14751 |
| `snprintf/%g` | 15561 |
| `snprintf/%lld` | 7643 |
| `snprintf/%p` | 457 |
| `snprintf/%s` | 424 |
| `snprintf/%u-small` | 364 |
| `snprintf/%x` | 518 |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1960 |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 3039 (-3 %) |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2090 |

#### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1081 |
| `strtod/17-digits` | 5248 |
| `strtod/exp-large` | 176522 |
| `strtod/exp-small` | 29424 |
| `strtod/fixed` | 1725 (-6 %) |
| `strtod/hex` | 974 |
| `strtod/integer` | 1348 |
| `strtod/long` | 8474 |
| `strtod/micro` | 1577 (-6 %) |
| `strtod/min-normal` | 104270 |
| `strtod/short` | 1455 (-7 %) |
| `strtod/subnormal` | 114004 |
| `strtof/9-digits` | 4257 (-0 %) |
| `strtof/exp` | 9198 (+0 %) |
| `strtof/short` | 1225 (-8 %) |
| `strtol/hex` | 1195 |
| `strtol/int` | 1213 |
| `strtol/short` | 949 |
| `strtoll/long` | 1496 |
| `strtoul/hex16` | 1179 |
| `strtoull/hex` | 1446 |

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 11832 (-10 %) | 14076 (-4 %) |
| `acosh` | 10152 (-8 %) | 13773 (-3 %) |
| `asin` | 11349 (-11 %) | 13481 (-4 %) |
| `asinh` | 9783 (-9 %) | 15719 (-4 %) |
| `atan` | 8648 (-11 %) | 8070 (-5 %) |
| `atan2` | 9033 (-11 %) | 8667 (-5 %) |
| `atanh` | 7721 (-9 %) | 9757 (-5 %) |
| `cbrt` | 3545 (-18 %) | 4257 (-17 %) |
| `cos` | 9093 (-8 %) | 6500 |
| `cosh` | 7302 (-10 %) | 6166 (-3 %) |
| `erf` | 66112 (-13 %) | 13408 (-9 %) |
| `erfc` | 54334 (-14 %) | 40682 (-8 %) |
| `exp` | 5431 (-9 %) | 3874 |
| `exp2` | 4511 (-8 %) | 5320 |
| `expm1` | 6065 (-8 %) | 4128 |
| `floor` | 75.2 (-11 %) | 102 |
| `fmod` | 225 (-4 %) | 291 |
| `hypot` | 3336 (-9 %) | 5956 (-2 %) |
| `lgamma` | 18357 (-9 %) | 17908 (-3 %) |
| `log` | 6185 (-8 %) | 6405 (-3 %) |
| `log10` | 6181 (-8 %) | 7881 (-3 %) |
| `log1p` | 6386 (-8 %) | 6961 (-3 %) |
| `log2` | 5322 (-9 %) | 8007 (-3 %) |
| `pow` | 11060 (-9 %) | 11840 (-2 %) |
| `round` | 69.2 (-11 %) | 99.5 |
| `sin` | 9154 (-8 %) | 6524 |
| `sinh` | 7396 (-10 %) | 6585 (-3 %) |
| `sqrt` | 468 (-2 %) | 1972 |
| `tan` | 17903 (-9 %) | 11535 (-2 %) |
| `tanh` | 7763 (-10 %) | 7444 (-3 %) |
| `tgamma` | 27683 (-9 %) | 24297 (-4 %) |

#### softfloat

| Case | insns/op |
|---|---:|
| `d2f` | 19.4 |
| `d2i` | 19.9 |
| `d2l` | 44.9 |
| `dadd` | 56.2 |
| `dcmp` | 29.9 |
| `ddiv` | 244 (-31 %) |
| `dmul` | 58.0 |
| `dsub` | 59.1 |
| `f2d` | 15.9 |
| `f2i` | 15.9 |
| `f2l` | 27.4 |
| `fadd` | 56.3 (-5 %) |
| `fcmp` | 28.9 (-33 %) |
| `fdiv` | 57.7 (-63 %) |
| `fmul` | 39.2 (-2 %) |
| `fsub` | 52.1 (-11 %) |
| `i2d` | 21.6 |
| `i2f` | 20.9 (-30 %) |
| `l2d` | 30.1 |
| `l2f` | 26.9 (-18 %) |

### Speed flavour

Instructions per operation (qemu 8.2.2, speed, speed flavour), change against the D108 baseline (5e57f83).

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2151 |
| `fputs/line` | 255 |
| `snprintf/%-20s` | 575 |
| `snprintf/%.17g` | 14846 |
| `snprintf/%.3f` | 14812 (-1 %) |
| `snprintf/%08x` | 469 |
| `snprintf/%a` | 862 |
| `snprintf/%c` | 261 |
| `snprintf/%d` | 455 |
| `snprintf/%e` | 13869 |
| `snprintf/%f` | 14669 |
| `snprintf/%g` | 15476 |
| `snprintf/%lld` | 1353 |
| `snprintf/%p` | 420 |
| `snprintf/%s` | 396 |
| `snprintf/%u-small` | 353 |
| `snprintf/%x` | 450 |
| `snprintf/literal` | 308 |
| `snprintf/mix` | 1837 |
| `sscanf/%d` | 364 |
| `sscanf/%lf` | 2024 (-5 %) |
| `sscanf/%x` | 423 |
| `sscanf/mix` | 1602 |

#### strto

| Case | insns/op |
|---|---:|
| `atoi` | 207 |
| `strtod/17-digits` | 1241 |
| `strtod/exp-large` | 1130 |
| `strtod/exp-small` | 1296 |
| `strtod/fixed` | 834 (-11 %) |
| `strtod/hex` | 953 |
| `strtod/integer` | 499 |
| `strtod/long` | 2258 |
| `strtod/micro` | 708 (-13 %) |
| `strtod/min-normal` | 1287 |
| `strtod/short` | 630 (-15 %) |
| `strtod/subnormal` | 1338 |
| `strtof/9-digits` | 934 (-0 %) |
| `strtof/exp` | 973 (+1 %) |
| `strtof/short` | 400 (-22 %) |
| `strtol/hex` | 322 |
| `strtol/int` | 305 |
| `strtol/short` | 135 |
| `strtoll/long` | 516 |
| `strtoul/hex16` | 309 |
| `strtoull/hex` | 508 |

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 11832 (-10 %) | 14076 (-4 %) |
| `acosh` | 10152 (-8 %) | 13773 (-3 %) |
| `asin` | 11349 (-11 %) | 13481 (-4 %) |
| `asinh` | 9783 (-9 %) | 15719 (-4 %) |
| `atan` | 8648 (-11 %) | 8070 (-5 %) |
| `atan2` | 9033 (-11 %) | 8667 (-5 %) |
| `atanh` | 7721 (-9 %) | 9757 (-5 %) |
| `cbrt` | 3545 (-18 %) | 4257 (-17 %) |
| `cos` | 1990 (-6 %) | 6500 |
| `cosh` | 7302 (-10 %) | 6166 (-3 %) |
| `erf` | 2079 (-2 %) | 13408 (-9 %) |
| `erfc` | 2018 (-2 %) | 40682 (-8 %) |
| `exp` | 280 (-1 %) | 3874 |
| `exp2` | 235 (-2 %) | 5320 |
| `expm1` | 6065 (-8 %) | 4128 |
| `floor` | 75.2 (-11 %) | 102 |
| `fmod` | 225 (-4 %) | 291 |
| `hypot` | 3336 (-9 %) | 5956 (-2 %) |
| `lgamma` | 18357 (-9 %) | 17908 (-3 %) |
| `log` | 1233 (-6 %) | 6405 (-3 %) |
| `log10` | 6181 (-8 %) | 7881 (-3 %) |
| `log1p` | 6386 (-8 %) | 6961 (-3 %) |
| `log2` | 391 (-2 %) | 8007 (-3 %) |
| `pow` | 11060 (-9 %) | 11840 (-2 %) |
| `round` | 69.2 (-11 %) | 99.5 |
| `sin` | 2499 (-7 %) | 6524 |
| `sinh` | 7396 (-10 %) | 6585 (-3 %) |
| `sqrt` | 468 (-2 %) | 1972 |
| `tan` | 2596 (-5 %) | 11535 (-2 %) |
| `tanh` | 7763 (-10 %) | 7444 (-3 %) |
| `tgamma` | 27683 (-9 %) | 24297 (-4 %) |

#### softfloat

| Case | insns/op |
|---|---:|
| `d2f` | 19.4 |
| `d2i` | 19.9 |
| `d2l` | 44.9 |
| `dadd` | 56.2 |
| `dcmp` | 29.9 |
| `ddiv` | 244 (-31 %) |
| `dmul` | 58.0 |
| `dsub` | 59.1 |
| `f2d` | 15.9 |
| `f2i` | 15.9 |
| `f2l` | 27.4 |
| `fadd` | 56.3 (-5 %) |
| `fcmp` | 28.9 (-33 %) |
| `fdiv` | 57.7 (-63 %) |
| `fmul` | 39.2 (-2 %) |
| `fsub` | 52.1 (-11 %) |
| `i2d` | 21.6 |
| `i2f` | 20.9 (-30 %) |
| `l2d` | 30.1 |
| `l2f` | 26.9 (-18 %) |


## P5.02 — Speed flavour: Thumb-2 `strlen` (D112)

`libc/string/thumb2/strlen.S` replaces the Thumb-1 kernel in the speed flavour:
two words per `LDRD` with the classic zero-byte test (`docs/research/strlen-thumb2.md`).
No case is slower; `strlen` of 1024 bytes 1315 → 1187, of 7 bytes 61 → 36, and
`strcpy`, which calls it, gains too. The size flavour is unchanged.

Instructions per operation (qemu 8.2.2, speed, speed flavour), change against the D111 baseline (3e25343).

### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 19.0 / 19.0 | 34.0 / 34.0 | 34.0 / 34.0 | 42.0 / 42.0 | 54.0 / 81.0 | 78.0 / 153 | 174 / 441 |
| `memmove-down` | 23.0 / 23.0 | 38.0 / 38.0 | 38.0 / 38.0 | 46.0 / 46.0 | 58.0 / 85.0 | 82.0 / 157 | 178 / 445 |
| `memmove-up` | 23.0 / 23.0 | 38.0 / 38.0 | 38.0 / 38.0 | 44.0 / 44.0 | 54.0 / 77.0 | 78.0 / 143 | 174 / 407 |
| `memset` | 18.0 / 18.0 | 35.0 / 35.0 | 35.0 / 35.0 | 39.0 / 39.0 | 57.0 / 59.0 | 81.0 / 83.0 | 177 / 179 |
| `memcmp` | 27.0 / 27.0 | 35.0 / 35.0 | 59.0 / 59.0 | 155 / 155 | 539 / 539 | 2075 / 2075 | 8219 / 8219 |
| `memchr` | 29.0 / 29.0 | 36.0 / 28.0 | 65.0 / 66.0 | 95.0 / 98.0 | 215 / 218 | 695 / 698 | 2615 / 2618 |
| `strlen` | 35.0 / 25.0 | 35.0 (-17 %) / 37.0 | 36.0 (-10 %) / 47.0 | 53.0 (-4 %) / 64.0 | 107 (-7 %) / 118 | 323 (-9 %) / 334 | 1187 (-10 %) / 1198 |
| `strchr` | 52.0 / 38.0 | 48.0 / 35.0 | 69.0 / 75.0 | 102 / 108 | 234 / 240 | 762 / 768 | 2874 / 2880 |
| `strcmp` | 27.0 / 27.0 | 33.0 / 33.0 | 53.0 / 53.0 | 144 / 133 | 240 / 430 | 624 / 1582 | 2160 / 6190 |
| `strcpy` | 62.0 / 62.0 | 62.0 (-10 %) / 64.0 | 65.0 (-6 %) / 76.0 | 90.0 (-2 %) / 101 | 156 (-5 %) / 185 | 396 (-7 %) / 470 | 1356 (-9 %) / 1622 |
| `mix/memcpy` (per call) | 54.1 | | | | | | |
| `mix/memset` (per call) | 41.7 | | | | | | |
| `mix/strlen` (per call) | 103 (-9 %) | | | | | | |


## P5.03 — Speed flavour: Thumb-2 `memchr` (D113)

`libc/string/thumb2/memchr.S` replaces the generic C `memchr` in the speed
flavour: two aligned words per iteration with the classic zero-byte test on the
word XOR the replicated character (`docs/research/memchr-thumb2.md`). 0.57–0.75
of the C's instructions from 4 bytes up (1024 bytes 2615 → 1821), no case
slower. The size flavour is unchanged.

Instructions per operation (qemu 8.2.2, speed, speed flavour), change against the P5.02 baseline (a08b3c0).

### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 19.0 / 19.0 | 34.0 / 34.0 | 34.0 / 34.0 | 42.0 / 42.0 | 54.0 / 81.0 | 78.0 / 153 | 174 / 441 |
| `memmove-down` | 23.0 / 23.0 | 38.0 / 38.0 | 38.0 / 38.0 | 46.0 / 46.0 | 58.0 / 85.0 | 82.0 / 157 | 178 / 445 |
| `memmove-up` | 23.0 / 23.0 | 38.0 / 38.0 | 38.0 / 38.0 | 44.0 / 44.0 | 54.0 / 77.0 | 78.0 / 143 | 174 / 407 |
| `memset` | 18.0 / 18.0 | 35.0 / 35.0 | 35.0 / 35.0 | 39.0 / 39.0 | 57.0 / 59.0 | 81.0 / 83.0 | 177 / 179 |
| `memcmp` | 27.0 / 27.0 | 35.0 / 35.0 | 59.0 / 59.0 | 155 / 155 | 539 / 539 | 2075 / 2075 | 8219 / 8219 |
| `memchr` | 24.0 (-17 %) / 24.0 | 35.0 (-3 %) / 28.0 | 40.0 (-38 %) / 65.0 | 57.0 (-40 %) / 86.0 | 141 (-34 %) / 170 | 477 (-31 %) / 506 | 1821 (-30 %) / 1850 |
| `strlen` | 35.0 / 25.0 | 35.0 / 37.0 | 36.0 / 47.0 | 53.0 / 64.0 | 107 / 118 | 323 / 334 | 1187 / 1198 |
| `strchr` | 52.0 / 38.0 | 48.0 / 35.0 | 69.0 / 75.0 | 102 / 108 | 234 / 240 | 762 / 768 | 2874 / 2880 |
| `strcmp` | 27.0 / 27.0 | 33.0 / 33.0 | 53.0 / 53.0 | 144 / 133 | 240 / 430 | 624 / 1582 | 2160 / 6190 |
| `strcpy` | 62.0 / 62.0 | 62.0 / 64.0 | 65.0 / 76.0 | 90.0 / 101 | 156 / 185 | 396 / 470 | 1356 / 1622 |
| `mix/memcpy` (per call) | 54.1 | | | | | | |
| `mix/memset` (per call) | 41.7 | | | | | | |
| `mix/strlen` (per call) | 103 | | | | | | |


## P5.04a — Speed flavour: fixed-point `log10f` (D114)

`libc/math/nofp/speed/log10f.c`: the D86 `log2f` core times log10 2, with the
float-float `log10f` as fallback (`docs/research/libm-fixed.md`, P5.04a). Only
the `log10f` cases change; change against the P5.03 baseline (956fdb6).
Throughput, instructions per call.

| Function | float | double |
|---|---:|---:|
| `log10` | 466 (-92 %) | 7881 |


## P5.04b — Speed flavour: fixed-point `powf` (D115)

`libc/math/nofp/speed/powf.c`: y log2 |x| from the wide log2 core, then the
D86 exp2 core, with the float-float `powf` as fallback
(`docs/research/libm-fixed.md`, P5.04b). Only the `powf` cases change;
change against the P5.04a baseline (c4abaa0). Throughput, instructions per call.

| Function | float | double |
|---|---:|---:|
| `pow` | 865 (-92 %) | 11840 |

## D125 — the libm vectors' accuracy fixes (Q36)

Correctness fixes, so the slowdowns are accepted and the baselines re-recorded (both flavours): `cbrt` 1.2 % slower (the residual's extra soft-double products); `pow` within 1 %. Every other case is within 1 %.

## P8.01 — `sincos` and `sincosf` bench rows

`math/sincos` and `math/sincosf` (the sum of both results over the usual 16 arguments in [−10, 10]) are new; they cost `sin` + `cos` (`sinf` + `cosf`), since P7.03 implements them as the two calls. No other case changed. They are the starting point for phase 8 (`docs/research/sincos.md`).

## P8.02 — One reduction for `sincos` and `sincosf`

`sincos` and `sincosf` reduce the argument once and evaluate both results, bit-identical to `sin`/`cos` and `sinf`/`cosf` (`docs/research/sincos.md`). Instructions per call (throughput, 16 arguments in [−10, 10]): `sincos` 13,034 → 10,702 (−18 %); `sincosf` 18,256 → 16,796 (−8 %), speed flavour 4,498 → 3,055 (−32 %). No other case changed.

## P8.03 — Fixed-point double `sin`, `cos` and `sincos` (speed flavour)

The `dp-fixed` layer (D136): an integer reduction and 64-bit fixed-point polynomials, correctly rounded when the rounding test passes, the double-double path otherwise (`docs/research/sincos.md`). Speed flavour, instructions per call (throughput, 16 arguments in [−10, 10]): `sin` 6,524 → 812 (8.0×), `cos` 6,500 → 1,117 (5.8×), `sincos` 10,702 → 1,476 (7.3×). `cos` includes one argument of the 16 (x = −5.7875) whose result lies 2 units from a halfway point and takes the double-double path. The size flavour is unchanged; no other case changed.

## Phase 8 — Summary

Instructions per call (throughput, 16 arguments in [−10, 10]; size / speed flavour where they differ).

| Case | Before phase 8 | After P8.02 (one reduction) | After P8.03 (fixed point, speed flavour) |
|---|---:|---:|---:|
| `sincos` | 13,034 | 10,702 (−18 %) | 1,476 (8.8× in all) |
| `sincosf` | 18,256 / 4,498 | 16,796 / 3,055 | — |
| `sin` / `cos` | 6,524 / 6,500 | — | 812 / 1,117 |

Phase 8's decisions are D133, D136 and D137; its research is `docs/research/sincos.md`.

## P9.01 — `UMULL` for the float fixed-point functions (speed flavour, D140)

nofp/speed's 32 × 32 → 64 multiply is an inline `UMULL` on armv7-m instead of four 16-bit products out of line. 24 cases are faster and none slower (instructions per call): `powf` 865 → 374, `expf` 280 → 125, `exp2f` 235 → 118, `erff` 2,079 → 1,122, `erfcf` 2,018 → 1,068, `log10f` 466 → 259, `log2f` 391 → 247, `logf` 1,233 → 1,026, `tanf` 2,596 → 2,077, `sincosf` 3,055 → 2,553, `cosf` 1,990 → 1,739, `sinf` 2,499 → 2,248.

## P9.02, P9.03 — Float sine and cosine, and `tan`, through the D136 kernel (speed flavour, D141, D142)

Instructions per call: `tan` 11,535 → 1,012, `tanf` 2,077 → 1,083, `sinf` 2,248 → 842, `cosf` 1,739 → 810, `sincosf` 2,553 → 1,160. `sin`, `cos` and `sincos` gain about 4 % from the inlined high product. No case is slower.

## P11.02a — The `fast` build type (D162, D164)

`docs/perf/baseline-<arch>-fast.json` holds the fast build's counts, equal case for case to the speed build's: no
`libc/math/fast/` file yet. The release and speed counts are unchanged by the step (the same `libc.a`); their
baselines still predate P10.07's `threads` cost (Q52).

## D166 — Phase 10's `threads` cost, re-locked (Q52)

P10.07 put `<threads.h>` over `<pthread.h>` (D156); the baselines, last recorded at P8.02, are re-locked for it in
the release and speed build types alike. Instructions per operation: `mtx/plain` and `mtx/timed` 84 → 107, `mtx/recursive` 87 → 107, `mtx/trylock` 78 → 103, `mtx/nested` 147 → 184, `call_once/done` 19 → 21, `tss_get` 34 → 35. No other case changed. A direct
uncontended path for plain mutexes is issue #144.

## P11.03a — Fast `expf`, `exp2f`, `exp10f` (D168)

Unchanged here: the cores without an FPU keep the speed files (the float-only `sp-nearest/fast/` files cost six software `fmaf` calls; P11.03e). Instructions per call (throughput), speed and fast; `test/bench/math` gains `exp10` and `exp10f`, recorded in
the release and speed baselines with no other case changed.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `expf` | 125 | 125 | 0 |
| `exp2f` | 118 | 118 | 0 |
| `exp10f` | 5,461 | 5,461 | 0 |

## P11.03b — Fast `logf`, `log2f`, `log10f` (D169)

Unchanged here: the cores without an FPU keep the speed files (D168, P11.03e). Instructions per call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `logf` | 1,026 | 1,026 | 0 |
| `log2f` | 247 | 247 | 0 |
| `log10f` | 259 | 259 | 0 |

## P11.03c — Fast `powf` (D170)

Unchanged here: the cores without an FPU keep the speed files (D168, P11.03e); the strict `pow` fix makes `pow` shorter in every build type (a power of two tested before the size of y). Instructions per call (throughput).

| Function | build | before | after | change |
|---|---|---:|---:|---:|
| `pow` | release | 11,864 | 11,833 | -0.3 % |
| `pow` | speed | 11,864 | 11,833 | -0.3 % |
| `pow` | fast | 11,864 | 11,833 | -0.3 % |

## P11.03d — Fast m1 and p1 forms, `cbrtf` (D171)

Unchanged here: the cores without an FPU keep the speed files (D168, P11.03e). Instructions per call (throughput), speed and fast. `test/bench/math` gains `exp2m1`, `exp10m1`, `log2p1` and
`log10p1` (float and double) in every build type; nothing else changed in release and speed.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `expm1f` | 6,065 | 6,065 | 0 |
| `exp2m1f` | 4,979 | 4,979 | 0 |
| `exp10m1f` | 6,135 | 6,135 | 0 |
| `log1pf` | 6,386 | 6,386 | 0 |
| `log2p1f` | 5,545 | 5,545 | 0 |
| `log10p1f` | 6,619 | 6,619 | 0 |
| `cbrtf` | 3,545 | 3,545 | 0 |

## P11.03e — The cores without an FPU: fixed point without the test (D172)

`nofp/fast/`: the fixed-point files without their rounding test, and a fixed-point `exp10f` (D172). Instructions per
call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `expf` | 125 | 119 | -5 % |
| `exp2f` | 118 | 112 | -5 % |
| `exp10f` | 5,461 | 117 | -98 % |
| `logf` | 1,026 | 253 | -75 % |
| `log2f` | 247 | 240 | -3 % |
| `log10f` | 259 | 253 | -2 % |
| `powf` | 374 | 368 | -2 % |

## P11.03f — The cores without an FPU: fixed-point m1 forms (D173)

`nofp/fast/`: fixed-point m1 forms over the exp2 core (D173). Instructions per call (throughput), speed and fast;
release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `expm1f` | 6,065 | 151 | -98 % |
| `exp2m1f` | 4,979 | 150 | -97 % |
| `exp10m1f` | 6,135 | 152 | -98 % |

## P11.03g — The cores without an FPU: fixed-point p1 forms, `cbrtf` (D174)

`nofp/fast/`: fixed-point p1 forms over the log2 core and an integer `cbrtf` (D174). Instructions per call
(throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `log1pf` | 6,386 | 254 | -96 % |
| `log2p1f` | 5,545 | 242 | -96 % |
| `log10p1f` | 6,619 | 255 | -96 % |
| `cbrtf` | 3,545 | 162 | -95 % |

The m1 forms' tiny path without a float constant costs `exp2m1f` 150 → 151, `exp10m1f` 152 → 153 (fast, P11.03f → P11.03g, within the 1 % gate).

## P11.04a — Fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D175)

Unchanged here: the cores without an FPU follow in P11.04e (D175). `test/bench/math` gains `sinpi`, `cospi`, `tanpi` in every
build type.

## P11.04b — Fast `asinf`, `acosf`, `atanf`, `atan2f` and the π forms (D176)

The fast files are for the FPU cores; the cores without an FPU follow in P11.04f. The strict float-float atan core's index fix (D176: k from 16t exact, one soft-float addition fewer) in every build type, instructions per call (throughput):

| Function | before | after | change |
|---|---:|---:|---:|
| `asinf` | 11349 | 11310 | -0.3 % |
| `acosf` | 11832 | 11794 | -0.3 % |
| `atanf` | 8648 | 8608 | -0.5 % |
| `atan2f` | 9033 | 8995 | -0.4 % |

## P11.04c — Fast `sinhf`, `coshf`, `tanhf`, `asinhf`, `acoshf`, `atanhf` (D177)

Unchanged here: the cores without an FPU follow in P11.04g (D177).

## P11.04d1 — Fast `erff`, and `erfcf` on the double-FPU cores (D178)

Unchanged here: the cores without an FPU follow in P11.04e (D178).

## P11.04d2 — Fast `erfcf` on the single-precision FPU cores (D179)

Unchanged here: the cores without an FPU follow in P11.04e (D179).

## P11.04e1 — The cores without an FPU: fast `sinf`, `cosf`, `sincosf`, `tanf` and the π forms (D180)

`nofp/fast/`: a fixed-point reduction to kπ/32 + s, D87's kernel on that s, rounded to nearest; the π forms from an exact split of 32x (D180). Instructions per call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `sinf` | 842 | 379 | -55 % |
| `cosf` | 810 | 379 | -53 % |
| `sincosf` | 1160 | 654 | -44 % |
| `tanf` | 1083 | 568 | -48 % |
| `sinpif` | 8812 | 369 | -96 % |
| `cospif` | 8578 | 347 | -96 % |
| `tanpif` | 17671 | 563 | -97 % |

## P11.04e2 — The cores without an FPU: fast `erff` (D181)

`nofp/fast/erff.c`: erf directly in seven pieces of degree 7 in Q31 from 1/2 to 3.92 (no exponential), x (2/√π + u q(u)) below; rounded to nearest once, no test (D181). Instructions per call (throughput), speed and fast; release and speed unchanged, `erfcf` too (D178).

| Function | speed | fast | change |
|---|---:|---:|---:|
| `erff` | 1122 | 184 | -84 % |

## P11.04f1 — The cores without an FPU: fast `atanf`, `atan2f` and their π forms (D182)

`nofp/fast/`: a fixed-point quotient without a divide instruction and one atan kernel (series below 1/8, seven pieces above), rounded to nearest once (D182). Instructions per call (throughput), speed and fast; release and speed unchanged. The π forms have no bench case.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `atanf` | 8608 | 245 | -97 % |
| `atan2f` | 8995 | 261 | -97 % |

## P11.04f2 — The cores without an FPU: fast `asinf`, `acosf` and their π forms (D183)

`nofp/fast/`: one arcsine kernel (the series in x² up to 1/2, the half-angle form with a fixed-point square root above), rounded to nearest once (D183). Instructions per call (throughput), speed and fast; release and speed unchanged. The π forms have no bench case.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `asinf` | 11311 | 239 | -98 % |
| `acosf` | 11794 | 244 | -98 % |

## P11.04g1 — The cores without an FPU: fast `sinhf`, `coshf` and `tanhf` (D184)

`nofp/fast/`: the odd series near 0, the exp2 core twice for sinh and cosh, and 1 − 2/(e^2x + 1) by the fixed-point quotient for tanh, rounded to nearest once (D184). Instructions per call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `sinhf` | 7396 | 171 | -98 % |
| `coshf` | 7302 | 162 | -98 % |
| `tanhf` | 7763 | 203 | -97 % |

## P11.04g2 — The cores without an FPU: fast `asinhf`, `acoshf` and `atanhf` (D185)

`nofp/fast/`: series near 0 (and near 1 for acosh), above the log2 core on arguments formed exactly in integers (x² ± 1 with the fixed-point square root, or the quotient (1 − x)/(1 + x)), rounded to nearest once (D185). Instructions per call (throughput), speed and fast; release and speed unchanged, `asinf` and `acosf` too (their square root is now shared).

| Function | speed | fast | change |
|---|---:|---:|---:|
| `asinhf` | 9783 | 352 | -96 % |
| `acoshf` | 10152 | 356 | -96 % |
| `atanhf` | 7721 | 266 | -97 % |

## P11.05b — Fast `exp`, `exp2`, `exp10` and `expm1` in double (D187)

`libc/math/fast/`: a 64-entry table of 2^(j/64) and its tail, degree-3 polynomials by Horner's rule, one rounding; the strict core for the slow path (D187). This core runs them on its soft-double helpers until P11.05c gives it fixed-point kernels. Instructions per call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `exp` | 3874 | 1198 | -69 % |
| `exp2` | 5320 | 1060 | -80 % |
| `exp10` | 5334 | 1236 | -77 % |
| `expm1` | 4128 | 3603 | -13 % |

## P11.05c1 — Fast `exp`, `exp2` and `exp10` in 64-bit fixed point (D188)

`libc/math/dp-fixed/fast/`: x·log2(e) from x's significand in 64-bit fixed point, a 64-entry Q63 table of 2^(j/64), a degree-7 polynomial in Q64 and one rounding into the result's bits; no soft-double operation on the common path. Instructions per call (throughput): speed, P11.05b's fast (D187's double file on the soft-double helpers) and now; release and speed unchanged. `expm1` is P11.05c2.

| Function | speed | P11.05b | fast | change from speed |
|---|---:|---:|---:|---:|
| `exp` | 3874 | 1198 | 318 | -92 % |
| `exp2` | 5320 | 1060 | 282 | -95 % |
| `exp10` | 5334 | 1236 | 317 | -94 % |

## P11.05c2 — Fast `expm1` in 64-bit fixed point (D189)

`libc/math/dp-fixed/fast/expm1.c`: a Q64 series below 1/16, relative however small x is; from 1/16, P11.05c1's kernel result in Q63 minus 1 exactly in integers. Instructions per call (throughput): speed, P11.05b's fast (D187's double file on the soft-double helpers) and now; release and speed unchanged. `exp`, `exp2` and `exp10` are one instruction faster each (the kernel's polynomial as an inline).

| Function | speed | P11.05b | fast | change from speed |
|---|---:|---:|---:|---:|
| `expm1` | 4128 | 3603 | 347 | -92 % |

## P11.05d — Fast `log`, `log2`, `log10` and `log1p` in double (D190)

`libc/math/fast/`: a 128-entry table of invc and −log(invc), r = z·invc − 1 exact, a degree-5 polynomial and one rounding (D190). This core runs them on its soft-double helpers until P11.05e gives it fixed-point kernels. Instructions per call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `log` | 6405 | 1683 | -74 % |
| `log2` | 8007 | 1884 | -76 % |
| `log10` | 7881 | 2743 | -65 % |
| `log1p` | 6961 | 2149 | -69 % |

## P11.05e — Fast `log`, `log2`, `log10` and `log1p` in 64-bit fixed point (D192)

`libc/math/dp-fixed/fast/`: a 90-entry table with invc in Q31 and −log(invc) in Q64, the series and the sum in 128 bits, a relative series within 2^-5 of 1, one rounding; no soft-double operation. Instructions per call (throughput): speed, P11.05d's fast (D190's double files on the soft-double helpers) and now; release and speed unchanged.

| Function | speed | P11.05d | fast | change from speed |
|---|---:|---:|---:|---:|
| `log` | 6405 | 1683 | 336 | -95 % |
| `log2` | 8007 | 1884 | 351 | -96 % |
| `log10` | 7881 | 2743 | 357 | -95 % |
| `log1p` | 6961 | 2149 | 371 | -95 % |

## P11.05f — Fast `pow` and `cbrt` in double (D193)

`libc/math/fast/`: pow as the exp of y times a log pair from the logarithms' table, one rounding; cbrt by a polynomial, a Newton step and an exact-residual step (D193). This core runs them on its soft-double helpers until P11.05g. Instructions per call (throughput), speed and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `pow` | 11833 | 5717 | -52 % |
| `cbrt` | 4309 | 2106 | -51 % |

## P11.05g1 — Fast `pow` in fixed point (D194)

`libc/math/dp-fixed/fast/pow.c`: log2 x to about 2^-72 from the logarithms' table (Q126 sums, normalized 96-bit products), then P11.05c1's exp2 kernel; no soft-double operation. Instructions per call (throughput): speed, P11.05f's fast (D193's double file on the soft-double helpers) and now; release and speed unchanged.

| Function | speed | P11.05f | fast | change from speed |
|---|---:|---:|---:|---:|
| `pow` | 11833 | 5717 | 923 | -92 % |

## P11.05g2 — Fast `cbrt` in fixed point (D195)

`libc/math/dp-fixed/fast/cbrt.c`: u ≈ f^(-1/3) by two Newton steps in Q31, T = f·u²·cbrt(2^j), then two corrections from exact integer residuals, one rounding; no soft-double operation. Instructions per call (throughput): speed, P11.05f's fast (D193's double file on the soft-double helpers) and now; release and speed unchanged.

| Function | speed | P11.05f | fast | change from speed |
|---|---:|---:|---:|---:|
| `cbrt` | 4309 | 2106 | 299 | -93 % |

AOR has no scalar double `cbrt`.

## P11.05i — Fast `sin`, `cos`, `sincos` and `tan` without the rounding test (D197)

`libc/math/dp-fixed/fast/`: D136's fixed-point kernel (D142's for `tan`) rounded to nearest with no test; the double-double path only where the kernel declines. Instructions per call (throughput): speed (the same kernel with its test, the fast flavour until now) and fast; release and speed unchanged.

| Function | speed | fast | change |
|---|---:|---:|---:|
| `sin` | 770 | 755 | -2 % |
| `cos` | 1074 | 760 | -29 % |
| `sincos` | 1400 | 1072 | -23 % |
| `tan` | 1012 | 996 | -2 % |

AOR has no scalar double `sin`, `cos` or `tan`.
