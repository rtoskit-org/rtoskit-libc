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
- strto, malloc, math and softfloat gain most: hardware divide, `UMULL`/`UMLAL` and libgcc's Thumb-2 soft-float assembly.

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

