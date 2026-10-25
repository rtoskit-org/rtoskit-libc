# Benchmarks: armv7e-m-dp

Core: Cortex-M7 class (Thumb-2, DSP, FPv5-D16): the Thumb-1 string kernels (D70), float and double in hardware.

Method: `docs/research/benchmarks.md` (PLAN D72). Regenerate the tables with
`tools/scripts/bench-runner [--build-type speed] markdown --board linux-armv7e-m-dp [--previous <rev> | --against <file>]`;
`docs/perf/baseline-armv7e-m-dp.json` (size flavour) and `docs/perf/baseline-armv7e-m-dp-speed.json` (speed flavour, PLAN D73)
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
| `snprintf/%.17g` | 14930 |
| `snprintf/%.3f` | 14654 |
| `snprintf/%08x` | 537 |
| `snprintf/%a` | 965 |
| `snprintf/%c` | 266 |
| `snprintf/%d` | 475 |
| `snprintf/%e` | 14035 |
| `snprintf/%f` | 14737 |
| `snprintf/%g` | 15541 |
| `snprintf/%lld` | 7643 |
| `snprintf/%p` | 457 |
| `snprintf/%s` | 423 |
| `snprintf/%u-small` | 364 |
| `snprintf/%x` | 518 |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1959 |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 2777 |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2090 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1081 |
| `strtod/17-digits` | 5179 |
| `strtod/exp-large` | 176132 |
| `strtod/exp-small` | 29352 |
| `strtod/fixed` | 1421 |
| `strtod/hex` | 915 |
| `strtod/integer` | 1234 |
| `strtod/long` | 8378 |
| `strtod/micro` | 1281 |
| `strtod/min-normal` | 104178 |
| `strtod/short` | 1151 |
| `strtod/subnormal` | 113845 |
| `strtof/9-digits` | 4213 |
| `strtof/exp` | 9153 |
| `strtof/short` | 1128 |
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
| `acos` | 347 | 343 |
| `acosh` | 332 | 328 |
| `asin` | 354 | 336 |
| `asinh` | 403 | 388 |
| `atan` | 283 | 267 |
| `atan2` | 470 | 432 |
| `atanh` | 284 | 267 |
| `cbrt` | 137 | 134 |
| `cos` | 224 | 220 |
| `cosh` | 219 | 216 |
| `erf` | 437 | 433 |
| `erfc` | 937 | 933 |
| `exp` | 186 | 183 |
| `exp2` | 212 | 209 |
| `expm1` | 201 | 186 |
| `floor` | 10.3 | 11.8 |
| `fmod` | 164 | 178 |
| `hypot` | 35.4 | 199 |
| `lgamma` | 619 | 615 |
| `log` | 192 | 189 |
| `log10` | 218 | 215 |
| `log1p` | 242 | 226 |
| `log2` | 254 | 251 |
| `pow` | 462 | 457 |
| `round` | 10.3 | 11.8 |
| `sin` | 234 | 218 |
| `sinh` | 247 | 230 |
| `sqrt` | 10.3 | 11.6 |
| `tan` | 324 | 307 |
| `tanh` | 264 | 248 |
| `tgamma` | 869 | 865 |

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

Instructions per operation (qemu 8.2.2, speed, speed flavour), change against docs/perf/baseline-armv7e-m-dp.json.

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
| `snprintf/%.17g` | 14855 (-1 %) |
| `snprintf/%.3f` | 14579 (-1 %) |
| `snprintf/%08x` | 530 (-1 %) |
| `snprintf/%a` | 890 (-8 %) |
| `snprintf/%c` | 259 (-3 %) |
| `snprintf/%d` | 468 (-1 %) |
| `snprintf/%e` | 13882 (-1 %) |
| `snprintf/%f` | 14662 (-1 %) |
| `snprintf/%g` | 15466 (-0 %) |
| `snprintf/%lld` | 7636 (-0 %) |
| `snprintf/%p` | 450 (-2 %) |
| `snprintf/%s` | 416 (-2 %) |
| `snprintf/%u-small` | 357 (-2 %) |
| `snprintf/%x` | 511 (-1 %) |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1931 (-1 %) |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 2777 |
| `sscanf/%x` | 540 |
| `sscanf/mix` | 2081 (-0 %) |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1081 |
| `strtod/17-digits` | 4275 (-17 %) |
| `strtod/exp-large` | 170534 (-3 %) |
| `strtod/exp-small` | 24495 (-17 %) |
| `strtod/fixed` | 1421 |
| `strtod/hex` | 915 |
| `strtod/integer` | 1234 |
| `strtod/long` | 7236 (-14 %) |
| `strtod/micro` | 1281 |
| `strtod/min-normal` | 85988 (-17 %) |
| `strtod/short` | 1151 |
| `strtod/subnormal` | 93889 (-18 %) |
| `strtof/9-digits` | 3341 (-21 %) |
| `strtof/exp` | 7363 (-20 %) |
| `strtof/short` | 1128 |
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
| `acos` | 347 | 343 |
| `acosh` | 332 | 328 |
| `asin` | 354 | 336 |
| `asinh` | 403 | 388 |
| `atan` | 283 | 267 |
| `atan2` | 470 | 432 |
| `atanh` | 284 | 267 |
| `cbrt` | 137 | 134 |
| `cos` | 224 | 220 |
| `cosh` | 219 | 216 |
| `erf` | 437 | 433 |
| `erfc` | 937 | 933 |
| `exp` | 186 | 183 |
| `exp2` | 212 | 209 |
| `expm1` | 201 | 186 |
| `floor` | 10.3 | 11.8 |
| `fmod` | 164 | 178 |
| `hypot` | 35.4 | 199 |
| `lgamma` | 619 | 615 |
| `log` | 192 | 189 |
| `log10` | 218 | 215 |
| `log1p` | 242 | 226 |
| `log2` | 254 | 251 |
| `pow` | 462 | 457 |
| `round` | 10.3 | 11.8 |
| `sin` | 234 | 218 |
| `sinh` | 247 | 230 |
| `sqrt` | 10.3 | 11.6 |
| `tan` | 324 | 307 |
| `tanh` | 264 | 248 |
| `tgamma` | 869 | 865 |

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
| `snprintf/%.17g` | 14832 (-0 %) |
| `snprintf/%.3f` | 14568 (-0 %) |
| `snprintf/%08x` | 469 (-12 %) |
| `snprintf/%a` | 865 (-3 %) |
| `snprintf/%c` | 260 (+0 %) |
| `snprintf/%d` | 455 (-3 %) |
| `snprintf/%e` | 13870 (-0 %) |
| `snprintf/%f` | 14655 (-0 %) |
| `snprintf/%g` | 15456 (-0 %) |
| `snprintf/%lld` | 1353 (-82 %) |
| `snprintf/%p` | 420 (-7 %) |
| `snprintf/%s` | 395 (-5 %) |
| `snprintf/%u-small` | 353 (-1 %) |
| `snprintf/%x` | 450 (-12 %) |
| `snprintf/literal` | 308 (-40 %) |
| `snprintf/mix` | 1836 (-5 %) |
| `sscanf/%d` | 374 (-18 %) |
| `sscanf/%lf` | 2752 (-1 %) |
| `sscanf/%x` | 435 (-19 %) |
| `sscanf/mix` | 1624 (-22 %) |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 207 (-81 %) |
| `strtod/17-digits` | 4275 |
| `strtod/exp-large` | 170534 |
| `strtod/exp-small` | 24495 |
| `strtod/fixed` | 1421 |
| `strtod/hex` | 915 |
| `strtod/integer` | 1234 |
| `strtod/long` | 7236 |
| `strtod/micro` | 1281 |
| `strtod/min-normal` | 85988 |
| `strtod/short` | 1151 |
| `strtod/subnormal` | 93889 |
| `strtof/9-digits` | 3341 |
| `strtof/exp` | 7363 |
| `strtof/short` | 1128 |
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
| `snprintf/%.17g` | 14832 |
| `snprintf/%.3f` | 14568 |
| `snprintf/%08x` | 469 |
| `snprintf/%a` | 865 |
| `snprintf/%c` | 260 |
| `snprintf/%d` | 455 |
| `snprintf/%e` | 13870 |
| `snprintf/%f` | 14655 |
| `snprintf/%g` | 15456 |
| `snprintf/%lld` | 1353 |
| `snprintf/%p` | 420 |
| `snprintf/%s` | 395 |
| `snprintf/%u-small` | 353 |
| `snprintf/%x` | 450 |
| `snprintf/literal` | 308 |
| `snprintf/mix` | 1836 |
| `sscanf/%d` | 374 |
| `sscanf/%lf` | 1764 (-36 %) |
| `sscanf/%x` | 435 |
| `sscanf/mix` | 1624 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 207 |
| `strtod/17-digits` | 1180 (-72 %) |
| `strtod/exp-large` | 1088 (-99 %) |
| `strtod/exp-small` | 1249 (-95 %) |
| `strtod/fixed` | 530 (-63 %) |
| `strtod/hex` | 895 (-2 %) |
| `strtod/integer` | 385 (-69 %) |
| `strtod/long` | 2202 (-70 %) |
| `strtod/micro` | 412 (-68 %) |
| `strtod/min-normal` | 1241 (-99 %) |
| `strtod/short` | 326 (-72 %) |
| `strtod/subnormal` | 1227 (-99 %) |
| `strtof/9-digits` | 898 (-73 %) |
| `strtof/exp` | 942 (-87 %) |
| `strtof/short` | 303 (-73 %) |
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
| `snprintf/%.17g` | 14930 |
| `snprintf/%.3f` | 14654 |
| `snprintf/%08x` | 537 |
| `snprintf/%a` | 965 |
| `snprintf/%c` | 266 |
| `snprintf/%d` | 475 |
| `snprintf/%e` | 14035 |
| `snprintf/%f` | 14737 |
| `snprintf/%g` | 15541 |
| `snprintf/%lld` | 7643 |
| `snprintf/%p` | 457 |
| `snprintf/%s` | 423 |
| `snprintf/%u-small` | 364 |
| `snprintf/%x` | 518 |
| `snprintf/literal` | 512 |
| `snprintf/mix` | 1959 |
| `sscanf/%d` | 457 |
| `sscanf/%lf` | 2777 |
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
| `snprintf/%.17g` | 14832 |
| `snprintf/%.3f` | 14568 |
| `snprintf/%08x` | 469 |
| `snprintf/%a` | 865 |
| `snprintf/%c` | 260 |
| `snprintf/%d` | 455 |
| `snprintf/%e` | 13870 |
| `snprintf/%f` | 14655 |
| `snprintf/%g` | 15456 |
| `snprintf/%lld` | 1353 |
| `snprintf/%p` | 420 |
| `snprintf/%s` | 395 |
| `snprintf/%u-small` | 353 |
| `snprintf/%x` | 450 |
| `snprintf/literal` | 308 |
| `snprintf/mix` | 1836 |
| `sscanf/%d` | 374 |
| `sscanf/%lf` | 1764 |
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

## P3.05c — Double-double products by fma (both flavours)

`__libc_dd_product` takes the exact error of a double product from one `VFMA.F64` on FPv5-DP (PLAN D83); every double
function, and the float functions computed in double, use it; the results are unchanged bit for bit. Change against
the P3.05b baselines; only the programs that changed are shown.

### Release (size flavour)

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 232 (-33 %) | 228 (-33 %) |
| `acosh` | 250 (-25 %) | 246 (-25 %) |
| `asin` | 238 (-33 %) | 220 (-35 %) |
| `asinh` | 302 (-25 %) | 287 (-26 %) |
| `atan` | 211 (-25 %) | 195 (-27 %) |
| `atan2` | 398 (-15 %) | 360 (-17 %) |
| `atanh` | 234 (-18 %) | 217 (-19 %) |
| `cbrt` | 137 | 134 |
| `cos` | 171 (-24 %) | 167 (-24 %) |
| `cosh` | 196 (-11 %) | 192 (-11 %) |
| `erf` | 421 (-4 %) | 418 (-4 %) |
| `erfc` | 787 (-16 %) | 783 (-16 %) |
| `exp` | 172 (-8 %) | 169 (-8 %) |
| `exp2` | 186 (-12 %) | 183 (-12 %) |
| `expm1` | 187 (-7 %) | 172 (-8 %) |
| `floor` | 10.3 | 11.8 |
| `fmod` | 164 | 178 |
| `hypot` | 35.4 | 138 (-31 %) |
| `lgamma` | 503 (-19 %) | 499 (-19 %) |
| `log` | 153 (-20 %) | 150 (-21 %) |
| `log10` | 164 (-25 %) | 161 (-25 %) |
| `log1p` | 203 (-16 %) | 187 (-17 %) |
| `log2` | 203 (-20 %) | 200 (-20 %) |
| `pow` | 396 (-14 %) | 392 (-14 %) |
| `round` | 10.3 | 11.8 |
| `sin` | 181 (-23 %) | 165 (-24 %) |
| `sinh` | 218 (-12 %) | 200 (-13 %) |
| `sqrt` | 10.3 | 11.6 |
| `tan` | 225 (-31 %) | 207 (-32 %) |
| `tanh` | 218 (-17 %) | 202 (-19 %) |
| `tgamma` | 722 (-17 %) | 719 (-17 %) |

### Speed flavour

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 232 (-33 %) | 228 (-33 %) |
| `acosh` | 250 (-25 %) | 246 (-25 %) |
| `asin` | 238 (-33 %) | 220 (-35 %) |
| `asinh` | 302 (-25 %) | 287 (-26 %) |
| `atan` | 211 (-25 %) | 195 (-27 %) |
| `atan2` | 398 (-15 %) | 360 (-17 %) |
| `atanh` | 234 (-18 %) | 217 (-19 %) |
| `cbrt` | 137 | 134 |
| `cos` | 171 (-24 %) | 167 (-24 %) |
| `cosh` | 196 (-11 %) | 192 (-11 %) |
| `erf` | 421 (-4 %) | 418 (-4 %) |
| `erfc` | 787 (-16 %) | 783 (-16 %) |
| `exp` | 172 (-8 %) | 169 (-8 %) |
| `exp2` | 186 (-12 %) | 183 (-12 %) |
| `expm1` | 187 (-7 %) | 172 (-8 %) |
| `floor` | 10.3 | 11.8 |
| `fmod` | 164 | 178 |
| `hypot` | 35.4 | 138 (-31 %) |
| `lgamma` | 503 (-19 %) | 499 (-19 %) |
| `log` | 153 (-20 %) | 150 (-21 %) |
| `log10` | 164 (-25 %) | 161 (-25 %) |
| `log1p` | 203 (-16 %) | 187 (-17 %) |
| `log2` | 203 (-20 %) | 200 (-20 %) |
| `pow` | 396 (-14 %) | 392 (-14 %) |
| `round` | 10.3 | 11.8 |
| `sin` | 181 (-23 %) | 165 (-24 %) |
| `sinh` | 218 (-12 %) | 200 (-13 %) |
| `sqrt` | 10.3 | 11.6 |
| `tan` | 225 (-31 %) | 207 (-32 %) |
| `tanh` | 218 (-17 %) | 202 (-19 %) |
| `tgamma` | 722 (-17 %) | 719 (-17 %) |

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
| `d2f` | 6.8 |
| `d2i` | 7.8 |
| `d2l` | 26.9 |
| `dadd` | 9.9 |
| `dcmp` | 10.8 |
| `ddiv` | 9.9 |
| `dmul` | 9.9 |
| `dsub` | 9.9 |
| `f2d` | 8.9 |
| `f2i` | 7.8 |
| `f2l` | 25.9 |
| `fadd` | 7.8 |
| `fcmp` | 10.8 |
| `fdiv` | 7.8 |
| `fmul` | 7.8 |
| `fsub` | 7.8 |
| `i2d` | 9.9 |
| `i2f` | 7.8 |
| `l2d` | 48.6 |
| `l2f` | 32.9 |

### Speed flavour

| Case | insns/op |
|---|---:|
| `d2f` | 6.8 |
| `d2i` | 7.8 |
| `d2l` | 26.9 |
| `dadd` | 9.9 |
| `dcmp` | 10.8 |
| `ddiv` | 9.9 |
| `dmul` | 9.9 |
| `dsub` | 9.9 |
| `f2d` | 8.9 |
| `f2i` | 7.8 |
| `f2l` | 25.9 |
| `fadd` | 7.8 |
| `fcmp` | 10.8 |
| `fdiv` | 7.8 |
| `fmul` | 7.8 |
| `fsub` | 7.8 |
| `i2d` | 9.9 |
| `i2f` | 7.8 |
| `l2d` | 48.6 |
| `l2f` | 32.9 |

## D98 — Correctly rounded float functions on the double path (Q19)

The 18 float functions with hard cases (`docs/research/float-hard-cases.md`) test their double result for a float halfway point before rounding it, and look the argument up only when it is on one. Correctness, not speed: every call pays the test. Instructions per call, throughput (latency moves the same):

| Function | release before | release after | speed before | speed after |
|---|---:|---:|---:|---:|
| `acosf` | 232.1 | 241.1 (+3.9 %) | 232.1 | 241.1 (+3.9 %) |
| `acoshf` | 250.2 | 259.2 (+3.6 %) | 250.2 | 259.2 (+3.6 %) |
| `asinhf` | 302.3 | 308.3 (+2.0 %) | 302.3 | 308.3 (+2.0 %) |
| `atanf` | 210.7 | 216.7 (+2.8 %) | 210.7 | 216.7 (+2.8 %) |
| `cosf` | 170.9 | 179.9 (+5.3 %) | 170.9 | 179.9 (+5.3 %) |
| `erfcf` | 786.9 | 795.9 (+1.1 %) | 786.9 | 795.9 (+1.1 %) |
| `exp2f` | 186.3 | 195.3 (+4.8 %) | 186.3 | 195.3 (+4.8 %) |
| `lgammaf` | 502.9 | 511.9 (+1.8 %) | 502.9 | 511.9 (+1.8 %) |
| `log10f` | 164.3 | 173.3 (+5.5 %) | 164.3 | 173.3 (+5.5 %) |
| `log1pf` | 203.1 | 209.1 (+3.0 %) | 203.1 | 209.1 (+3.0 %) |
| `logf` | 153.3 | 162.3 (+5.9 %) | 153.3 | 162.3 (+5.9 %) |
| `sinf` | 180.9 | 186.9 (+3.3 %) | 180.9 | 186.9 (+3.3 %) |
| `sinhf` | 217.6 | 222.6 (+2.3 %) | 217.6 | 222.6 (+2.3 %) |
| `tgammaf` | 722.2 | 731.2 (+1.2 %) | 722.2 | 731.2 (+1.2 %) |


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
