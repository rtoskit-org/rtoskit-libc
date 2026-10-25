# Benchmarks: armv7e-m

Core: Cortex-M4F class (Thumb-2, DSP, FPv4-SP): the Thumb-1 string kernels (D70), float in hardware and float-float, double in software.

Method: `docs/research/benchmarks.md` (PLAN D72). Regenerate the tables with
`tools/scripts/bench-runner [--build-type speed] markdown --board linux-armv7e-m [--previous <rev> | --against <file>]`;
`docs/perf/baseline-armv7e-m.json` (size flavour) and `docs/perf/baseline-armv7e-m-speed.json` (speed flavour, PLAN D73)
hold every case, including the full string sweep.

## P3.01 — Phase 2 baseline

The library as Phase 2 left it (`phase-2-complete`), size flavour, release.

Instructions per operation (qemu 8.2.2, release, size flavour).

### string

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

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2341 |
| `fputs/line` | 377 |
| `snprintf/%-20s` | 614 |
| `snprintf/%.17g` | 14927 |
| `snprintf/%.3f` | 15225 |
| `snprintf/%08x` | 537 |
| `snprintf/%a` | 962 |
| `snprintf/%c` | 266 |
| `snprintf/%d` | 475 |
| `snprintf/%e` | 14032 |
| `snprintf/%f` | 14734 |
| `snprintf/%g` | 15538 |
| `snprintf/%lld` | 7643 |
| `snprintf/%p` | 457 |
| `snprintf/%s` | 423 |
| `snprintf/%u-small` | 364 |
| `snprintf/%x` | 518 |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1959 |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 3403 |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2090 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1081 |
| `strtod/17-digits` | 5376 |
| `strtod/exp-large` | 176358 |
| `strtod/exp-small` | 29543 |
| `strtod/fixed` | 2232 |
| `strtod/hex` | 1112 |
| `strtod/integer` | 1497 |
| `strtod/long` | 8575 |
| `strtod/micro` | 2086 |
| `strtod/min-normal` | 104376 |
| `strtod/short` | 1962 |
| `strtod/subnormal` | 114654 |
| `strtof/9-digits` | 4210 |
| `strtof/exp` | 9150 |
| `strtof/short` | 1125 |
| `strtol/hex` | 1195 |
| `strtol/int` | 1213 |
| `strtol/short` | 949 |
| `strtoll/long` | 1496 |
| `strtoul/hex16` | 1179 |
| `strtoull/hex` | 1446 |

### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 133 |
| `batch/fifo` | 112 |
| `batch/lifo` | 107 |
| `batch/shuffled` | 126 |
| `calloc/256` | 249 |
| `fixed/16` | 108 |
| `fixed/256` | 108 |
| `fixed/4096` | 108 |
| `realloc/grow` | 113 |

### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 237 |
| `batch/fifo` | 221 |
| `batch/lifo` | 221 |
| `batch/shuffled` | 222 |
| `calloc/256` | 254 |
| `fixed/16` | 112 |
| `fixed/256` | 112 |
| `fixed/4096` | 112 |
| `realloc/grow` | 111 |

### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 143 |
| `qsort/equal/1024` | 114 |
| `qsort/random/1024` | 640 |
| `qsort/random/16` | 224 |
| `qsort/random/256` | 497 |
| `qsort/reversed/1024` | 602 |
| `qsort/sorted/1024` | 677 |
| `qsort/strings/256` | 595 |

### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 433 | 16391 |
| `acosh` | 387 | 15690 |
| `asin` | 424 | 15803 |
| `asinh` | 413 | 18322 |
| `atan` | 350 | 9740 |
| `atan2` | 478 | 10501 |
| `atanh` | 301 | 11882 |
| `cbrt` | 191 | 6937 |
| `cos` | 685 | 6825 |
| `cosh` | 342 | 7119 |
| `erf` | 2538 | 16960 |
| `erfc` | 1854 | 51736 |
| `exp` | 227 | 4185 |
| `exp2` | 198 | 5775 |
| `expm1` | 260 | 4481 |
| `floor` | 27.3 | 119 |
| `fmod` | 164 | 280 |
| `hypot` | 219 | 6520 |
| `lgamma` | 802 | 21092 |
| `log` | 233 | 7530 |
| `log10` | 233 | 9093 |
| `log1p` | 265 | 8079 |
| `log2` | 212 | 9231 |
| `pow` | 494 | 13390 |
| `round` | 23.3 | 117 |
| `sin` | 693 | 6876 |
| `sinh` | 370 | 7566 |
| `sqrt` | 10.3 | 2009 |
| `tan` | 1031 | 12927 |
| `tanh` | 332 | 8538 |
| `tgamma` | 1197 | 28717 |

### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 19.0 |
| `mtx/nested` | 147 |
| `mtx/plain` | 84.0 |
| `mtx/recursive` | 87.0 |
| `mtx/timed` | 84.0 |
| `mtx/trylock` | 78.0 |
| `tss_get` | 34.0 |

## P3.02a — Speed flavour: Thumb-2 memory kernels

The speed flavour (`BUILD_TYPE=speed`, PLAN D73) with the Thumb-2 `memcpy`/`memmove`/`memset`
(`libc/string/thumb2`); everything else is still the size flavour's code. Change against the phase 2
baseline (the size flavour's tables above).

Instructions per operation (qemu 8.2.2, speed, speed flavour), change against docs/perf/baseline-armv7e-m.json.

### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 19.0 (-42 %) / 19.0 | 34.0 (-8 %) / 34.0 | 34.0 (-11 %) / 34.0 | 42.0 (-21 %) / 42.0 | 54.0 (-52 %) / 81.0 | 78.0 (-78 %) / 153 | 174 (-87 %) / 441 |
| `memmove-down` | 23.0 (-41 %) / 23.0 | 38.0 (-12 %) / 38.0 | 38.0 (-14 %) / 38.0 | 46.0 (-22 %) / 46.0 | 58.0 (-51 %) / 85.0 | 82.0 (-77 %) / 157 | 178 (-87 %) / 445 |
| `memmove-up` | 23.0 (-41 %) / 23.0 | 38.0 (-12 %) / 38.0 | 38.0 (-14 %) / 38.0 | 44.0 (-15 %) / 44.0 | 54.0 (-52 %) / 77.0 | 78.0 (-78 %) / 143 | 174 (-87 %) / 407 |
| `memset` | 18.0 (-40 %) / 18.0 | 35.0 (+6 %) / 35.0 | 35.0 (+3 %) / 35.0 | 39.0 (-15 %) / 39.0 | 57.0 (-39 %) / 59.0 | 81.0 (-72 %) / 83.0 | 177 (-83 %) / 179 |
| `memcmp` | 27.0 / 27.0 | 35.0 / 35.0 | 59.0 / 59.0 | 155 / 155 | 539 / 539 | 2075 / 2075 | 8219 / 8219 |
| `memchr` | 29.0 / 29.0 | 36.0 / 28.0 | 65.0 / 66.0 | 95.0 / 98.0 | 215 / 218 | 695 / 698 | 2615 / 2618 |
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 52.0 / 38.0 | 48.0 / 35.0 | 69.0 / 75.0 | 102 / 108 | 234 / 240 | 762 / 768 | 2874 / 2880 |
| `strcmp` | 28.0 / 28.0 | 36.0 / 36.0 | 60.0 / 60.0 | 156 / 156 | 540 / 540 | 2076 / 2076 | 8220 / 8220 |
| `strcpy` | 62.0 (-5 %) / 62.0 | 69.0 (-9 %) / 69.0 | 69.0 (-8 %) / 92.0 | 92.0 (-12 %) / 115 | 164 (-27 %) / 212 | 428 (-39 %) / 524 | 1484 (-43 %) / 1772 |
| `mix/memcpy` (per call) | 54.1 (-75 %) | | | | | | |
| `mix/memset` (per call) | 41.7 (-53 %) | | | | | | |
| `mix/strlen` (per call) | 113 | | | | | | |

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2200 (-6 %) |
| `fputs/line` | 270 (-28 %) |
| `snprintf/%-20s` | 607 (-1 %) |
| `snprintf/%.17g` | 14852 (-1 %) |
| `snprintf/%.3f` | 15150 (-0 %) |
| `snprintf/%08x` | 530 (-1 %) |
| `snprintf/%a` | 887 (-8 %) |
| `snprintf/%c` | 259 (-3 %) |
| `snprintf/%d` | 468 (-1 %) |
| `snprintf/%e` | 13879 (-1 %) |
| `snprintf/%f` | 14659 (-1 %) |
| `snprintf/%g` | 15463 (-0 %) |
| `snprintf/%lld` | 7636 (-0 %) |
| `snprintf/%p` | 450 (-2 %) |
| `snprintf/%s` | 416 (-2 %) |
| `snprintf/%u-small` | 357 (-2 %) |
| `snprintf/%x` | 511 (-1 %) |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1931 (-1 %) |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 3403 |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2081 (-0 %) |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1081 |
| `strtod/17-digits` | 4472 (-17 %) |
| `strtod/exp-large` | 170760 (-3 %) |
| `strtod/exp-small` | 24686 (-16 %) |
| `strtod/fixed` | 2232 |
| `strtod/hex` | 1112 |
| `strtod/integer` | 1497 |
| `strtod/long` | 7433 (-13 %) |
| `strtod/micro` | 2086 |
| `strtod/min-normal` | 86186 (-17 %) |
| `strtod/short` | 1962 |
| `strtod/subnormal` | 94698 (-17 %) |
| `strtof/9-digits` | 3338 (-21 %) |
| `strtof/exp` | 7360 (-20 %) |
| `strtof/short` | 1125 |
| `strtol/hex` | 1195 |
| `strtol/int` | 1213 |
| `strtol/short` | 949 |
| `strtoll/long` | 1496 |
| `strtoul/hex16` | 1179 |
| `strtoull/hex` | 1446 |

### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 133 |
| `batch/fifo` | 112 |
| `batch/lifo` | 107 |
| `batch/shuffled` | 126 |
| `calloc/256` | 146 (-41 %) |
| `fixed/16` | 108 |
| `fixed/256` | 108 |
| `fixed/4096` | 108 |
| `realloc/grow` | 113 |

### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 237 |
| `batch/fifo` | 221 |
| `batch/lifo` | 221 |
| `batch/shuffled` | 222 |
| `calloc/256` | 151 (-40 %) |
| `fixed/16` | 112 |
| `fixed/256` | 112 |
| `fixed/4096` | 112 |
| `realloc/grow` | 111 |

### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 143 |
| `qsort/equal/1024` | 114 |
| `qsort/random/1024` | 640 |
| `qsort/random/16` | 224 |
| `qsort/random/256` | 497 |
| `qsort/reversed/1024` | 602 |
| `qsort/sorted/1024` | 677 |
| `qsort/strings/256` | 595 |

### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 433 | 16391 |
| `acosh` | 387 | 15690 |
| `asin` | 424 | 15803 |
| `asinh` | 413 | 18322 |
| `atan` | 350 | 9740 |
| `atan2` | 478 | 10501 |
| `atanh` | 301 | 11882 |
| `cbrt` | 191 | 6937 |
| `cos` | 685 | 6825 |
| `cosh` | 342 | 7119 |
| `erf` | 2538 | 16960 |
| `erfc` | 1854 | 51736 |
| `exp` | 227 | 4185 |
| `exp2` | 198 | 5775 |
| `expm1` | 260 | 4481 |
| `floor` | 27.3 | 119 |
| `fmod` | 164 | 280 |
| `hypot` | 219 | 6520 |
| `lgamma` | 802 | 21092 |
| `log` | 233 | 7530 |
| `log10` | 233 | 9093 |
| `log1p` | 265 | 8079 |
| `log2` | 212 | 9231 |
| `pow` | 494 | 13390 |
| `round` | 23.3 | 117 |
| `sin` | 693 | 6876 |
| `sinh` | 370 | 7566 |
| `sqrt` | 10.3 | 2009 |
| `tan` | 1031 | 12927 |
| `tanh` | 332 | 8538 |
| `tgamma` | 1197 | 28717 |

### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 19.0 |
| `mtx/nested` | 147 |
| `mtx/plain` | 84.0 |
| `mtx/recursive` | 87.0 |
| `mtx/timed` | 84.0 |
| `mtx/trylock` | 78.0 |
| `tss_get` | 34.0 |

## P3.02b — Speed flavour: Thumb-2 `strcmp`/`strchr`

Word-at-a-time `strcmp` and DSP `strchr` in `libc/string/thumb2` (PLAN D74); `strlen` stays the Thumb-1 one
(the speed budget, Q16). Change against the P3.02a speed baseline; every other program is unchanged.

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
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 43.0 (-17 %) / 28.0 | 43.0 (-10 %) / 24.0 | 43.0 (-38 %) / 67.0 | 67.0 (-34 %) / 91.0 | 163 (-30 %) / 187 | 547 (-28 %) / 571 | 2083 (-28 %) / 2107 |
| `strcmp` | 27.0 (-4 %) / 27.0 | 33.0 (-8 %) / 33.0 | 53.0 (-12 %) / 53.0 | 144 (-8 %) / 133 | 240 (-56 %) / 430 | 624 (-70 %) / 1582 | 2160 (-74 %) / 6190 |
| `strcpy` | 62.0 / 62.0 | 69.0 / 69.0 | 69.0 / 92.0 | 92.0 / 115 | 164 / 212 | 428 / 524 | 1484 / 1772 |
| `mix/memcpy` (per call) | 54.1 | | | | | | |
| `mix/memset` (per call) | 41.7 | | | | | | |
| `mix/strlen` (per call) | 113 | | | | | | |

### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 143 |
| `qsort/equal/1024` | 114 |
| `qsort/random/1024` | 640 |
| `qsort/random/16` | 224 |
| `qsort/random/256` | 497 |
| `qsort/reversed/1024` | 602 |
| `qsort/sorted/1024` | 677 |
| `qsort/strings/256` | 585 (-2 %) |

## P3.03a — Speed flavour: integer conversions

Division-free `strtol` and relatives, 64-bit decimals in 10^9 chunks, 32-bit shifts for hex and octal, and a
`memcpy` buffer sink, all in the speed flavour's `libc/stdlib/speed` and `libc/stdio/speed` (PLAN D77); the scanf
engine at `-O2`. Change against the P3.02 speed baseline; only the programs that changed are shown, and the release
build is unchanged.

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2166 (-2 %) |
| `fputs/line` | 270 |
| `snprintf/%-20s` | 574 (-5 %) |
| `snprintf/%.17g` | 14829 (-0 %) |
| `snprintf/%.3f` | 15139 (-0 %) |
| `snprintf/%08x` | 469 (-12 %) |
| `snprintf/%a` | 862 (-3 %) |
| `snprintf/%c` | 260 (+0 %) |
| `snprintf/%d` | 455 (-3 %) |
| `snprintf/%e` | 13867 (-0 %) |
| `snprintf/%f` | 14652 (-0 %) |
| `snprintf/%g` | 15453 (-0 %) |
| `snprintf/%lld` | 1353 (-82 %) |
| `snprintf/%p` | 420 (-7 %) |
| `snprintf/%s` | 395 (-5 %) |
| `snprintf/%u-small` | 353 (-1 %) |
| `snprintf/%x` | 450 (-12 %) |
| `snprintf/literal` | 308 (-40 %) |
| `snprintf/mix` | 1836 (-5 %) |
| `sscanf/%d` | 374 (-18 %) |
| `sscanf/%lf` | 3378 (-1 %) |
| `sscanf/%x` | 435 (-19 %) |
| `sscanf/mix` | 1624 (-22 %) |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 207 (-81 %) |
| `strtod/17-digits` | 4472 |
| `strtod/exp-large` | 170760 |
| `strtod/exp-small` | 24686 |
| `strtod/fixed` | 2232 |
| `strtod/hex` | 1112 |
| `strtod/integer` | 1497 |
| `strtod/long` | 7433 |
| `strtod/micro` | 2086 |
| `strtod/min-normal` | 86186 |
| `strtod/short` | 1962 |
| `strtod/subnormal` | 94698 |
| `strtof/9-digits` | 3338 |
| `strtof/exp` | 7360 |
| `strtof/short` | 1125 |
| `strtol/hex` | 322 (-73 %) |
| `strtol/int` | 305 (-75 %) |
| `strtol/short` | 135 (-86 %) |
| `strtoll/long` | 516 (-66 %) |
| `strtoul/hex16` | 309 (-74 %) |
| `strtoull/hex` | 508 (-65 %) |

## P3.03b — Speed flavour: `strtod` middle path

A compact-table Eisel–Lemire-style middle path between Clinger's fast path and Simple Decimal Conversion, in-line
digit tests and a division-free Clinger guard, in the speed flavour's `libc/stdlib/speed/__libc_strtofloat.c`
(PLAN D78). Change against the P3.03a speed baseline; only the programs that changed are shown, and the release
build is unchanged.

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2166 |
| `fputs/line` | 270 |
| `snprintf/%-20s` | 574 |
| `snprintf/%.17g` | 14829 |
| `snprintf/%.3f` | 15139 |
| `snprintf/%08x` | 469 |
| `snprintf/%a` | 862 |
| `snprintf/%c` | 260 |
| `snprintf/%d` | 455 |
| `snprintf/%e` | 13867 |
| `snprintf/%f` | 14652 |
| `snprintf/%g` | 15453 |
| `snprintf/%lld` | 1353 |
| `snprintf/%p` | 420 |
| `snprintf/%s` | 395 |
| `snprintf/%u-small` | 353 |
| `snprintf/%x` | 450 |
| `snprintf/literal` | 308 |
| `snprintf/mix` | 1836 |
| `sscanf/%d` | 374 |
| `sscanf/%lf` | 2390 (-29 %) |
| `sscanf/%x` | 435 |
| `sscanf/mix` | 1624 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 207 |
| `strtod/17-digits` | 1377 (-69 %) |
| `strtod/exp-large` | 1314 (-99 %) |
| `strtod/exp-small` | 1440 (-94 %) |
| `strtod/fixed` | 1341 (-40 %) |
| `strtod/hex` | 1092 (-2 %) |
| `strtod/integer` | 648 (-57 %) |
| `strtod/long` | 2399 (-68 %) |
| `strtod/micro` | 1217 (-42 %) |
| `strtod/min-normal` | 1439 (-98 %) |
| `strtod/short` | 1137 (-42 %) |
| `strtod/subnormal` | 2036 (-98 %) |
| `strtof/9-digits` | 895 (-73 %) |
| `strtof/exp` | 939 (-87 %) |
| `strtof/short` | 300 (-73 %) |
| `strtol/hex` | 322 |
| `strtol/int` | 305 |
| `strtol/short` | 135 |
| `strtoll/long` | 516 |
| `strtoul/hex16` | 309 |
| `strtoull/hex` | 508 |

## P3.04a — Lock word fast path in the port (both flavours)

The seam's lock word (PLAN D79): the Linux port's `__rtos_lock_acquire` and `__rtos_lock_release` start with one
compare-and-swap on the word, with the owner's thread-local token, and need neither `__rtos_thread_self` nor an owner
pointer. libc is unchanged, so both flavours gain alike. Change against the P3.03b baselines; only the programs that
changed are shown.

### Release (size flavour)

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2326 (-1 %) |
| `fputs/line` | 362 (-4 %) |
| `snprintf/%-20s` | 614 |
| `snprintf/%.17g` | 14927 |
| `snprintf/%.3f` | 15225 |
| `snprintf/%08x` | 537 |
| `snprintf/%a` | 962 |
| `snprintf/%c` | 266 |
| `snprintf/%d` | 475 |
| `snprintf/%e` | 14032 |
| `snprintf/%f` | 14734 |
| `snprintf/%g` | 15538 |
| `snprintf/%lld` | 7643 |
| `snprintf/%p` | 457 |
| `snprintf/%s` | 423 |
| `snprintf/%u-small` | 364 |
| `snprintf/%x` | 518 |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1959 |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 3403 |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2090 |

#### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 118 (-11 %) |
| `batch/fifo` | 96.5 (-13 %) |
| `batch/lifo` | 91.6 (-14 %) |
| `batch/shuffled` | 111 (-12 %) |
| `calloc/256` | 234 (-6 %) |
| `fixed/16` | 92.5 (-14 %) |
| `fixed/256` | 92.5 (-14 %) |
| `fixed/4096` | 92.5 (-14 %) |
| `realloc/grow` | 99.2 (-12 %) |

#### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 222 (-6 %) |
| `batch/fifo` | 206 (-7 %) |
| `batch/lifo` | 206 (-7 %) |
| `batch/shuffled` | 207 (-7 %) |
| `calloc/256` | 238 (-6 %) |
| `fixed/16` | 97.0 (-13 %) |
| `fixed/256` | 97.0 (-13 %) |
| `fixed/4096` | 97.0 (-13 %) |
| `realloc/grow` | 97.1 (-12 %) |

### Speed flavour

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 2151 (-1 %) |
| `fputs/line` | 255 (-6 %) |
| `snprintf/%-20s` | 574 |
| `snprintf/%.17g` | 14829 |
| `snprintf/%.3f` | 15139 |
| `snprintf/%08x` | 469 |
| `snprintf/%a` | 862 |
| `snprintf/%c` | 260 |
| `snprintf/%d` | 455 |
| `snprintf/%e` | 13867 |
| `snprintf/%f` | 14652 |
| `snprintf/%g` | 15453 |
| `snprintf/%lld` | 1353 |
| `snprintf/%p` | 420 |
| `snprintf/%s` | 395 |
| `snprintf/%u-small` | 353 |
| `snprintf/%x` | 450 |
| `snprintf/literal` | 308 |
| `snprintf/mix` | 1836 |
| `sscanf/%d` | 374 |
| `sscanf/%lf` | 2390 |
| `sscanf/%x` | 435 |
| `sscanf/mix` | 1624 |

#### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 118 (-11 %) |
| `batch/fifo` | 96.5 (-13 %) |
| `batch/lifo` | 91.6 (-14 %) |
| `batch/shuffled` | 111 (-12 %) |
| `calloc/256` | 132 (-10 %) |
| `fixed/16` | 92.5 (-14 %) |
| `fixed/256` | 92.5 (-14 %) |
| `fixed/4096` | 92.5 (-14 %) |
| `realloc/grow` | 99.2 (-12 %) |

#### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 222 (-6 %) |
| `batch/fifo` | 206 (-7 %) |
| `batch/lifo` | 206 (-7 %) |
| `batch/shuffled` | 207 (-7 %) |
| `calloc/256` | 136 (-10 %) |
| `fixed/16` | 97.0 (-13 %) |
| `fixed/256` | 97.0 (-13 %) |
| `fixed/4096` | 97.0 (-13 %) |
| `realloc/grow` | 97.1 (-12 %) |

## P3.05b — Float-float products by fma (both flavours)

`__libc_ff_product` takes the exact error of a float product from one `VFMA.F32` on FPv4-SP and FPv5-SP (PLAN D82),
which every function of the float-float layer uses; the results are unchanged bit for bit. Change against the P3.04a
baselines; only the programs that changed are shown. The other boards are unchanged.

### Release (size flavour)

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 325 (-25 %) | 16391 |
| `acosh` | 266 (-31 %) | 15690 |
| `asin` | 315 (-26 %) | 15803 |
| `asinh` | 288 (-30 %) | 18322 |
| `atan` | 284 (-19 %) | 9740 |
| `atan2` | 410 (-14 %) | 10501 |
| `atanh` | 205 (-32 %) | 11882 |
| `cbrt` | 149 (-22 %) | 6937 |
| `cos` | 567 (-17 %) | 6825 |
| `cosh` | 241 (-30 %) | 7119 |
| `erf` | 1383 (-46 %) | 16960 |
| `erfc` | 1224 (-34 %) | 51736 |
| `exp` | 160 (-29 %) | 4185 |
| `exp2` | 141 (-29 %) | 5775 |
| `expm1` | 178 (-32 %) | 4481 |
| `floor` | 27.3 | 119 |
| `fmod` | 164 | 280 |
| `hypot` | 161 (-26 %) | 6520 |
| `lgamma` | 572 (-29 %) | 21092 |
| `log` | 165 (-29 %) | 7530 |
| `log10` | 165 (-29 %) | 9093 |
| `log1p` | 197 (-26 %) | 8079 |
| `log2` | 154 (-27 %) | 9231 |
| `pow` | 367 (-26 %) | 13390 |
| `round` | 23.3 | 117 |
| `sin` | 575 (-17 %) | 6876 |
| `sinh` | 267 (-28 %) | 7566 |
| `sqrt` | 10.3 | 2009 |
| `tan` | 794 (-23 %) | 12927 |
| `tanh` | 253 (-24 %) | 8538 |
| `tgamma` | 846 (-29 %) | 28717 |

### Speed flavour

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 325 (-25 %) | 16391 |
| `acosh` | 266 (-31 %) | 15690 |
| `asin` | 315 (-26 %) | 15803 |
| `asinh` | 288 (-30 %) | 18322 |
| `atan` | 284 (-19 %) | 9740 |
| `atan2` | 410 (-14 %) | 10501 |
| `atanh` | 205 (-32 %) | 11882 |
| `cbrt` | 149 (-22 %) | 6937 |
| `cos` | 567 (-17 %) | 6825 |
| `cosh` | 241 (-30 %) | 7119 |
| `erf` | 1383 (-46 %) | 16960 |
| `erfc` | 1224 (-34 %) | 51736 |
| `exp` | 160 (-29 %) | 4185 |
| `exp2` | 141 (-29 %) | 5775 |
| `expm1` | 178 (-32 %) | 4481 |
| `floor` | 27.3 | 119 |
| `fmod` | 164 | 280 |
| `hypot` | 161 (-26 %) | 6520 |
| `lgamma` | 572 (-29 %) | 21092 |
| `log` | 165 (-29 %) | 7530 |
| `log10` | 165 (-29 %) | 9093 |
| `log1p` | 197 (-26 %) | 8079 |
| `log2` | 154 (-27 %) | 9231 |
| `pow` | 367 (-26 %) | 13390 |
| `round` | 23.3 | 117 |
| `sin` | 575 (-17 %) | 6876 |
| `sinh` | 267 (-28 %) | 7566 |
| `sqrt` | 10.3 | 2009 |
| `tan` | 794 (-23 %) | 12927 |
| `tanh` | 253 (-24 %) | 8538 |
| `tgamma` | 846 (-29 %) | 28717 |

## P3.07 — Introsort qsort (speed flavour)

The speed flavour's `qsort` is a run check and an introsort (PLAN D90). The release flavour is unchanged. Change
against the P3.05h baselines; only the program that changed is shown.

### Speed flavour

#### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 143 |
| `qsort/equal/1024` | 27.1 (-76 %) |
| `qsort/random/1024` | 274 (-57 %) |
| `qsort/random/16` | 128 (-43 %) |
| `qsort/random/256` | 216 (-57 %) |
| `qsort/reversed/1024` | 35.6 (-94 %) |
| `qsort/sorted/1024` | 27.1 (-96 %) |
| `qsort/strings/256` | 303 (-48 %) |

## P4.03 — New program: `softfloat`

The floating-point operations that become run-time library calls without an FPU (PLAN D92). The cases are new, so there is no change column; the other programs are unchanged.

### Release (size flavour)

| Case | insns/op |
|---|---:|
| `d2f` | 19.9 |
| `d2i` | 21.9 |
| `d2l` | 178 |
| `dadd` | 63.9 |
| `dcmp` | 48.9 |
| `ddiv` | 583 |
| `dmul` | 57.6 |
| `dsub` | 67.2 |
| `f2d` | 15.9 |
| `f2i` | 7.8 |
| `f2l` | 141 |
| `fadd` | 7.8 |
| `fcmp` | 10.8 |
| `fdiv` | 7.8 |
| `fmul` | 7.8 |
| `fsub` | 7.8 |
| `i2d` | 40.9 |
| `i2f` | 7.8 |
| `l2d` | 48.6 |
| `l2f` | 32.9 |

### Speed flavour

| Case | insns/op |
|---|---:|
| `d2f` | 19.9 |
| `d2i` | 21.9 |
| `d2l` | 178 |
| `dadd` | 63.9 |
| `dcmp` | 48.9 |
| `ddiv` | 583 |
| `dmul` | 57.6 |
| `dsub` | 67.2 |
| `f2d` | 15.9 |
| `f2i` | 7.8 |
| `f2l` | 141 |
| `fadd` | 7.8 |
| `fcmp` | 10.8 |
| `fdiv` | 7.8 |
| `fmul` | 7.8 |
| `fsub` | 7.8 |
| `i2d` | 40.9 |
| `i2f` | 7.8 |
| `l2d` | 48.6 |
| `l2f` | 32.9 |

## D99 — our own double-precision helpers (Q24)

The `soft-dp` layer (`docs/research/softdouble.md`) replaces libgcc's double helpers. Instructions per operation, before → after. Release and speed flavour are the same, except `strto` and `stdio`. Every case not listed is unchanged.

### softfloat

| Case | before | after |
|---|---:|---:|
| `d2f` | 19.9 | 19.4 (-2.5 %) |
| `d2i` | 21.9 | 19.9 (-9.1 %) |
| `d2l` | 177.9 | 44.9 (-74.7 %) |
| `dadd` | 63.9 | 55.2 (-13.7 %) |
| `dcmp` | 48.9 | 43.9 (-10.2 %) |
| `ddiv` | 583.2 | 173.8 (-70.2 %) |
| `dmul` | 57.6 | 54.0 (-6.3 %) |
| `dsub` | 67.2 | 58.1 (-13.7 %) |
| `f2l` | 140.9 | 27.4 (-80.5 %) |
| `i2d` | 40.9 | 21.6 (-47.3 %) |
| `l2d` | 48.6 | 30.1 (-38.1 %) |

### math (double functions, throughput; latency moves the same, but for `fmod`)

| Function | before | after |
|---|---:|---:|
| `acos/throughput` | 16391 | 13738 (-16.2 %) |
| `acosh/throughput` | 15690 | 13495 (-14.0 %) |
| `asin/throughput` | 15803 | 13139 (-16.9 %) |
| `asinh/throughput` | 18322 | 15332 (-16.3 %) |
| `atan/throughput` | 9740 | 7800 (-19.9 %) |
| `atan2/throughput` | 10501 | 8424 (-19.8 %) |
| `atanh/throughput` | 11882 | 9342 (-21.4 %) |
| `cbrt/throughput` | 6937 | 3670 (-47.1 %) |
| `cos/throughput` | 6825 | 6493 (-4.9 %) |
| `cosh/throughput` | 7119 | 6028 (-15.3 %) |
| `erf/throughput` | 16960 | 12246 (-27.8 %) |
| `erfc/throughput` | 51736 | 38068 (-26.4 %) |
| `exp/throughput` | 4185 | 3882 (-7.3 %) |
| `exp2/throughput` | 5775 | 5324 (-7.8 %) |
| `expm1/throughput` | 4481 | 4143 (-7.5 %) |
| `floor/throughput` | 119.2 | 96.6 (-19.0 %) |
| `fmod/latency` | 265.5 | 277.0 (+4.3 %) |
| `hypot/throughput` | 6520 | 5955 (-8.7 %) |
| `lgamma/throughput` | 21092 | 17492 (-17.1 %) |
| `log/throughput` | 7530 | 6252 (-17.0 %) |
| `log10/throughput` | 9093 | 7703 (-15.3 %) |
| `log1p/throughput` | 8079 | 6798 (-15.9 %) |
| `log2/throughput` | 9231 | 7842 (-15.0 %) |
| `pow/throughput` | 13390 | 11737 (-12.3 %) |
| `round/throughput` | 117.2 | 94.6 (-19.3 %) |
| `sin/throughput` | 6876 | 6500 (-5.5 %) |
| `sinh/throughput` | 7566 | 6440 (-14.9 %) |
| `sqrt/throughput` | 2009 | 2012 (+0.2 %) |
| `tan/throughput` | 12927 | 11252 (-13.0 %) |
| `tanh/throughput` | 8538 | 7258 (-15.0 %) |
| `tgamma/throughput` | 28717 | 23658 (-17.6 %) |

`fmod/latency` grows 4.3 %: the latency loop's `x + y * 0.0` dependency goes through the zero paths of `__aeabi_dmul` and `__aeabi_dadd`, which cost about one ordinary addition here and less in libgcc. `fmod` itself, which is integer code, is unchanged (accepted in D99).

### strto and stdio (changed cases)

| Case | release before | release after | speed before | speed after |
|---|---:|---:|---:|---:|
| `strto/strtod/17-digits` | 5376 | 5241 (-2.5 %) | 1377 | 1242 (-9.8 %) |
| `strto/strtod/exp-large` | 176358 | 176175 (-0.1 %) | 1314 | 1131 (-13.9 %) |
| `strto/strtod/exp-small` | 29543 | 29400 (-0.5 %) | 1440 | 1297 (-9.9 %) |
| `strto/strtod/fixed` | 2232 | 1654 (-25.9 %) | 1341 | 763.0 (-43.1 %) |
| `strto/strtod/hex` | 1112 | 972.0 (-12.6 %) | 1092 | 952.0 (-12.8 %) |
| `strto/strtod/integer` | 1497 | 1346 (-10.1 %) | 648.0 | 497.0 (-23.3 %) |
| `strto/strtod/long` | 8575 | 8435 (-1.6 %) | 2399 | 2259 (-5.8 %) |
| `strto/strtod/micro` | 2086 | 1506 (-27.8 %) | 1217 | 637.0 (-47.7 %) |
| `strto/strtod/min-normal` | 104376 | 104225 (-0.1 %) | 1439 | 1288 (-10.5 %) |
| `strto/strtod/short` | 1962 | 1384 (-29.5 %) | 1137 | 559.0 (-50.8 %) |
| `strto/strtod/subnormal` | 114654 | 113957 (-0.6 %) | 2036 | 1339 (-34.2 %) |
| `stdio/snprintf/%.3f` | 15225 | 14814 (-2.7 %) | 15139 | 14728 (-2.7 %) |
| `stdio/sscanf/%lf` | 3403 | 2968 (-12.8 %) | 2390 | 1955 (-18.2 %) |

## D100 — strong helpers, and the layer's complex helpers

Making the soft-dp helpers strong changes no instruction. A full `make bench` shows all 824 cases unchanged, in both flavours.

The compiler's complex and `powi` helpers are in no benchmark program. They were measured separately: 1,000 calls each on moderate random operands, with the bench's qemu plugin, on armv7e-m (release). Instructions per call, including the loop:

| Helper | all libgcc (before D99) | libgcc's helper on soft-dp arithmetic (D99) | soft-dp (D100) |
|---|---:|---:|---:|
| `__muldc3` | 402 | 369 | 371 (+0.5 %) |
| `__divdc3` | 2460 | 1153 | 1059 (−8 %) |
| `__divsc3` | 1812 | 922 | 882 (−4 %) |
| `__powidf2` | 321 | 300 | 300 |

The first version of `__divdc3` (1393) called `logb` and `scalbn` three times for every division. It now skips the scaling when every part is zero or within 2^-128 to 2^128. Its results are unchanged, bit for bit, over 50 million host cases that include values on both sides of those bounds. `__divsc3` multiplies by one reciprocal instead of dividing twice. Over the same 50 million cases that gave the same float results.

## D101 — the comparisons in Thumb-2 assembly

The shared double ordering of `__aeabi_dcmp{lt,le,eq,gt,ge}` is Thumb-2 assembly now; it sets the flags for the callers, which are a call and one conditional move. The figures are instructions per operation, and are the same in both flavours.

| Case | before | after |
|---|---:|---:|
| `softfloat/dcmp` | 43.9 | 28.9 (−34.1 %) |
| `math/sqrt` | 2012 | 1968 (−2.2 %) |
| `math/hypot` | 5955 | 5835 (−2.0 %) |
| `math/exp` | 3882 | 3813 (−1.8 %) |
| `math/pow` | 11737 | 11533 (−1.7 %) |
| `math/atan2` | 8424 | 8314 (−1.3 %) |
| `math/sin` | 6500 | 6435 (−1.0 %) |

57 cases are faster in all, the double functions by 0.8 to 2.2 %, and none is slower.


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
| `strchr` | 43.0 / 28.0 | 43.0 / 24.0 | 43.0 / 67.0 | 67.0 / 91.0 | 163 / 187 | 547 / 571 | 2083 / 2107 |
| `strcmp` | 27.0 / 27.0 | 33.0 / 33.0 | 53.0 / 53.0 | 144 / 133 | 240 / 430 | 624 / 1582 | 2160 / 6190 |
| `strcpy` | 62.0 / 62.0 | 62.0 (-10 %) / 64.0 | 65.0 (-6 %) / 76.0 | 90.0 (-2 %) / 101 | 156 (-5 %) / 185 | 396 (-7 %) / 470 | 1356 (-9 %) / 1622 |
| `mix/memcpy` (per call) | 54.1 | | | | | | |
| `mix/memset` (per call) | 41.7 | | | | | | |
| `mix/strlen` (per call) | 103 (-9 %) | | | | | | |
