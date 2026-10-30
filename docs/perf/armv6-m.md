# Benchmarks: armv6-m

Core: Cortex-M0+ class (Thumb-1, no FPU): the Thumb-1 string kernels (D69), soft float and double.

Method: `docs/research/benchmarks.md` (PLAN D72). Regenerate the tables with
`tools/scripts/bench-runner [--build-type speed] markdown --board linux-armv6-m [--previous <rev> | --against <file>]`;
`docs/perf/baseline-armv6-m.json` (size flavour) and `docs/perf/baseline-armv6-m-speed.json` (speed flavour, PLAN D73)
hold every case, including the full string sweep.

## P3.01 — Phase 2 baseline

The library as Phase 2 left it (`phase-2-complete`), size flavour, release.

Instructions per operation (qemu 8.2.2, release, size flavour).

### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 34.0 / 29.0 | 38.0 / 38.0 | 39.0 / 65.0 | 54.0 / 93.0 | 114 / 285 | 354 / 1053 | 1314 / 4125 |
| `memmove-down` | 40.0 / 35.0 | 44.0 / 44.0 | 45.0 / 71.0 | 60.0 / 99.0 | 120 / 291 | 360 / 1059 | 1320 / 4131 |
| `memmove-up` | 40.0 / 35.0 | 44.0 / 44.0 | 45.0 / 71.0 | 53.0 / 107 | 113 / 347 | 353 / 1307 | 1313 / 5147 |
| `memset` | 30.0 / 28.0 | 33.0 / 37.0 | 34.0 / 54.0 | 46.0 / 66.0 | 94.0 / 114 | 286 / 306 | 1054 / 1074 |
| `memcmp` | 28.0 / 28.0 | 35.0 / 35.0 | 56.0 / 56.0 | 140 / 140 | 476 / 476 | 1820 / 1820 | 7196 / 7196 |
| `memchr` | 29.0 / 29.0 | 35.0 / 29.0 | 64.0 / 65.0 | 97.0 / 102 | 229 / 234 | 757 / 762 | 2869 / 2874 |
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 54.0 / 39.0 | 52.0 / 37.0 | 70.0 / 79.0 | 109 / 118 | 265 / 274 | 889 / 898 | 3385 / 3394 |
| `strcmp` | 29.0 / 29.0 | 37.0 / 37.0 | 61.0 / 61.0 | 157 / 157 | 541 / 541 | 2077 / 2077 | 8221 / 8221 |
| `strcpy` | 66.0 / 61.0 | 77.0 / 75.0 | 76.0 / 125 | 106 / 168 | 226 / 420 | 706 / 1428 | 2626 / 5460 |
| `mix/memcpy` (per call) | 223 | | | | | | |
| `mix/memset` (per call) | 95.6 | | | | | | |
| `mix/strlen` (per call) | 120 | | | | | | |

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4143 |
| `fputs/line` | 503 |
| `snprintf/%-20s` | 1166 |
| `snprintf/%.17g` | 13988 |
| `snprintf/%.3f` | 13867 |
| `snprintf/%08x` | 918 |
| `snprintf/%a` | 1204 |
| `snprintf/%c` | 292 |
| `snprintf/%d` | 1638 |
| `snprintf/%e` | 16998 |
| `snprintf/%f` | 13287 |
| `snprintf/%g` | 14849 |
| `snprintf/%lld` | 11027 |
| `snprintf/%p` | 539 |
| `snprintf/%s` | 451 |
| `snprintf/%u-small` | 431 |
| `snprintf/%x` | 631 |
| `snprintf/literal` | 555 |
| `snprintf/mix` | 3564 |
| `sscanf/%d` | 784 |
| `sscanf/%lf` | 5107 |
| `sscanf/%x` | 966 |
| `sscanf/mix` | 3311 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1211 |
| `strtod/17-digits` | 24062 |
| `strtod/exp-large` | 692937 |
| `strtod/exp-small` | 416299 |
| `strtod/fixed` | 3863 |
| `strtod/hex` | 2101 |
| `strtod/integer` | 2909 |
| `strtod/long` | 47601 |
| `strtod/micro` | 3368 |
| `strtod/min-normal` | 1850827 |
| `strtod/short` | 3352 |
| `strtod/subnormal` | 2040498 |
| `strtof/9-digits` | 16890 |
| `strtof/exp` | 78890 |
| `strtof/short` | 1634 |
| `strtol/hex` | 1678 |
| `strtol/int` | 1501 |
| `strtol/short` | 914 |
| `strtoll/long` | 2350 |
| `strtoul/hex16` | 1671 |
| `strtoull/hex` | 2544 |

### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 265 |
| `batch/fifo` | 237 |
| `batch/lifo` | 234 |
| `batch/shuffled` | 256 |
| `calloc/256` | 378 |
| `fixed/16` | 233 |
| `fixed/256` | 233 |
| `fixed/4096` | 233 |
| `realloc/grow` | 233 |

### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 389 |
| `batch/fifo` | 368 |
| `batch/lifo` | 368 |
| `batch/shuffled` | 369 |
| `calloc/256` | 386 |
| `fixed/16` | 240 |
| `fixed/256` | 240 |
| `fixed/4096` | 240 |
| `realloc/grow` | 232 |

### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 167 |
| `qsort/equal/1024` | 139 |
| `qsort/random/1024` | 769 |
| `qsort/random/16` | 267 |
| `qsort/random/256` | 596 |
| `qsort/reversed/1024` | 723 |
| `qsort/sorted/1024` | 813 |
| `qsort/strings/256` | 707 |

### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 26692 | 35771 |
| `acosh` | 22250 | 34288 |
| `asin` | 25875 | 34679 |
| `asinh` | 21693 | 39824 |
| `atan` | 18770 | 21229 |
| `atan2` | 19460 | 22656 |
| `atanh` | 16542 | 25888 |
| `cbrt` | 9412 | 14545 |
| `cos` | 18928 | 17700 |
| `cosh` | 15893 | 16736 |
| `erf` | 151747 | 40695 |
| `erfc` | 128215 | 112996 |
| `exp` | 11458 | 10356 |
| `exp2` | 9609 | 14172 |
| `expm1` | 12742 | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 8371 | 14812 |
| `lgamma` | 38415 | 47161 |
| `log` | 12864 | 17003 |
| `log10` | 12862 | 21106 |
| `log1p` | 13273 | 18064 |
| `log2` | 10973 | 21459 |
| `pow` | 23058 | 31562 |
| `round` | 103 | 210 |
| `sin` | 18996 | 17710 |
| `sinh` | 16022 | 17881 |
| `sqrt` | 2036 | 4208 |
| `tan` | 37649 | 32504 |
| `tanh` | 16684 | 20206 |
| `tgamma` | 57787 | 64810 |

### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 22.0 |
| `mtx/nested` | 275 |
| `mtx/plain` | 205 |
| `mtx/recursive` | 208 |
| `mtx/timed` | 205 |
| `mtx/trylock` | 203 |
| `tss_get` | 37.0 |

## P3.02a — Speed flavour: Thumb-2 memory kernels

The speed flavour (`BUILD_TYPE=speed`, PLAN D73) had no speed kernels here yet: its figures equal the size
flavour's.

Instructions per operation (qemu 8.2.2, speed, speed flavour), change against docs/perf/baseline-armv6-m.json.

### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 34.0 / 29.0 | 38.0 / 38.0 | 39.0 / 65.0 | 54.0 / 93.0 | 114 / 285 | 354 / 1053 | 1314 / 4125 |
| `memmove-down` | 40.0 / 35.0 | 44.0 / 44.0 | 45.0 / 71.0 | 60.0 / 99.0 | 120 / 291 | 360 / 1059 | 1320 / 4131 |
| `memmove-up` | 40.0 / 35.0 | 44.0 / 44.0 | 45.0 / 71.0 | 53.0 / 107 | 113 / 347 | 353 / 1307 | 1313 / 5147 |
| `memset` | 30.0 / 28.0 | 33.0 / 37.0 | 34.0 / 54.0 | 46.0 / 66.0 | 94.0 / 114 | 286 / 306 | 1054 / 1074 |
| `memcmp` | 28.0 / 28.0 | 35.0 / 35.0 | 56.0 / 56.0 | 140 / 140 | 476 / 476 | 1820 / 1820 | 7196 / 7196 |
| `memchr` | 29.0 / 29.0 | 35.0 / 29.0 | 64.0 / 65.0 | 97.0 / 102 | 229 / 234 | 757 / 762 | 2869 / 2874 |
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 54.0 / 39.0 | 52.0 / 37.0 | 70.0 / 79.0 | 109 / 118 | 265 / 274 | 889 / 898 | 3385 / 3394 |
| `strcmp` | 29.0 / 29.0 | 37.0 / 37.0 | 61.0 / 61.0 | 157 / 157 | 541 / 541 | 2077 / 2077 | 8221 / 8221 |
| `strcpy` | 66.0 / 61.0 | 77.0 / 75.0 | 76.0 / 125 | 106 / 168 | 226 / 420 | 706 / 1428 | 2626 / 5460 |
| `mix/memcpy` (per call) | 223 | | | | | | |
| `mix/memset` (per call) | 95.6 | | | | | | |
| `mix/strlen` (per call) | 120 | | | | | | |

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4143 |
| `fputs/line` | 503 |
| `snprintf/%-20s` | 1166 |
| `snprintf/%.17g` | 13988 |
| `snprintf/%.3f` | 13867 |
| `snprintf/%08x` | 918 |
| `snprintf/%a` | 1204 |
| `snprintf/%c` | 292 |
| `snprintf/%d` | 1638 |
| `snprintf/%e` | 16998 |
| `snprintf/%f` | 13287 |
| `snprintf/%g` | 14849 |
| `snprintf/%lld` | 11027 |
| `snprintf/%p` | 539 |
| `snprintf/%s` | 451 |
| `snprintf/%u-small` | 431 |
| `snprintf/%x` | 631 |
| `snprintf/literal` | 555 |
| `snprintf/mix` | 3564 |
| `sscanf/%d` | 784 |
| `sscanf/%lf` | 5107 |
| `sscanf/%x` | 966 |
| `sscanf/mix` | 3311 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1211 |
| `strtod/17-digits` | 24062 |
| `strtod/exp-large` | 692937 |
| `strtod/exp-small` | 416299 |
| `strtod/fixed` | 3863 |
| `strtod/hex` | 2101 |
| `strtod/integer` | 2909 |
| `strtod/long` | 47601 |
| `strtod/micro` | 3368 |
| `strtod/min-normal` | 1850827 |
| `strtod/short` | 3352 |
| `strtod/subnormal` | 2040498 |
| `strtof/9-digits` | 16890 |
| `strtof/exp` | 78890 |
| `strtof/short` | 1634 |
| `strtol/hex` | 1678 |
| `strtol/int` | 1501 |
| `strtol/short` | 914 |
| `strtoll/long` | 2350 |
| `strtoul/hex16` | 1671 |
| `strtoull/hex` | 2544 |

### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 265 |
| `batch/fifo` | 237 |
| `batch/lifo` | 234 |
| `batch/shuffled` | 256 |
| `calloc/256` | 378 |
| `fixed/16` | 233 |
| `fixed/256` | 233 |
| `fixed/4096` | 233 |
| `realloc/grow` | 233 |

### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 389 |
| `batch/fifo` | 368 |
| `batch/lifo` | 368 |
| `batch/shuffled` | 369 |
| `calloc/256` | 386 |
| `fixed/16` | 240 |
| `fixed/256` | 240 |
| `fixed/4096` | 240 |
| `realloc/grow` | 232 |

### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 167 |
| `qsort/equal/1024` | 139 |
| `qsort/random/1024` | 769 |
| `qsort/random/16` | 267 |
| `qsort/random/256` | 596 |
| `qsort/reversed/1024` | 723 |
| `qsort/sorted/1024` | 813 |
| `qsort/strings/256` | 707 |

### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 26692 | 35771 |
| `acosh` | 22250 | 34288 |
| `asin` | 25875 | 34679 |
| `asinh` | 21693 | 39824 |
| `atan` | 18770 | 21229 |
| `atan2` | 19460 | 22656 |
| `atanh` | 16542 | 25888 |
| `cbrt` | 9412 | 14545 |
| `cos` | 18928 | 17700 |
| `cosh` | 15893 | 16736 |
| `erf` | 151747 | 40695 |
| `erfc` | 128215 | 112996 |
| `exp` | 11458 | 10356 |
| `exp2` | 9609 | 14172 |
| `expm1` | 12742 | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 8371 | 14812 |
| `lgamma` | 38415 | 47161 |
| `log` | 12864 | 17003 |
| `log10` | 12862 | 21106 |
| `log1p` | 13273 | 18064 |
| `log2` | 10973 | 21459 |
| `pow` | 23058 | 31562 |
| `round` | 103 | 210 |
| `sin` | 18996 | 17710 |
| `sinh` | 16022 | 17881 |
| `sqrt` | 2036 | 4208 |
| `tan` | 37649 | 32504 |
| `tanh` | 16684 | 20206 |
| `tgamma` | 57787 | 64810 |

### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 22.0 |
| `mtx/nested` | 275 |
| `mtx/plain` | 205 |
| `mtx/recursive` | 208 |
| `mtx/timed` | 205 |
| `mtx/trylock` | 203 |
| `tss_get` | 37.0 |

## P3.02c — Speed flavour: Armv6-M kernels

The Armv6-M speed kernels `memcpy`/`memmove`/`memset`/`strcmp` in `libc/string/thumb1-speed` (PLAN D75).
Change against the previous speed baseline (equal to the phase 2 size baseline here).

### string

Aligned / worst of the misaligned offsets.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memcpy` | 36.0 (+6 %) / 30.0 | 40.0 (+5 %) / 37.0 | 40.0 (+3 %) / 67.0 | 40.0 (-26 %) / 94.0 | 52.0 (-54 %) / 187 | 100 (-72 %) / 523 | 292 (-78 %) / 1867 |
| `memmove-down` | 42.0 (+5 %) / 36.0 | 46.0 (+5 %) / 43.0 | 46.0 (+2 %) / 73.0 | 46.0 (-23 %) / 100 | 58.0 (-52 %) / 193 | 106 (-71 %) / 529 | 298 (-77 %) / 1873 |
| `memmove-up` | 42.0 (+5 %) / 36.0 | 46.0 (+5 %) / 43.0 | 46.0 (+2 %) / 73.0 | 42.0 (-21 %) / 94.0 | 60.0 (-47 %) / 214 | 132 (-63 %) / 646 | 420 (-68 %) / 2374 |
| `memset` | 32.0 (+7 %) / 28.0 | 35.0 (+6 %) / 39.0 | 35.0 (+3 %) / 56.0 | 44.0 (-4 %) / 65.0 | 49.0 (-48 %) / 80.0 | 73.0 (-74 %) / 104 | 169 (-84 %) / 200 |
| `memcmp` | 28.0 / 28.0 | 35.0 / 35.0 | 56.0 / 56.0 | 140 / 140 | 476 / 476 | 1820 / 1820 | 7196 / 7196 |
| `memchr` | 29.0 / 29.0 | 35.0 / 29.0 | 64.0 / 65.0 | 97.0 / 102 | 229 / 234 | 757 / 762 | 2869 / 2874 |
| `strlen` | 35.0 / 28.0 | 42.0 / 42.0 | 40.0 / 63.0 | 55.0 / 78.0 | 115 / 138 | 355 / 378 | 1315 / 1338 |
| `strchr` | 54.0 / 39.0 | 52.0 / 37.0 | 70.0 / 79.0 | 109 / 118 | 265 / 274 | 889 / 898 | 3385 / 3394 |
| `strcmp` | 30.0 (+3 %) / 30.0 | 37.0 / 37.0 | 58.0 (-5 %) / 58.0 | 142 (-10 %) / 142 | 478 (-12 %) / 478 | 1822 (-12 %) / 1822 | 7198 (-12 %) / 7198 |
| `strcpy` | 68.0 (+3 %) / 62.0 | 79.0 (+3 %) / 74.0 | 77.0 (+1 %) / 127 | 92.0 (-13 %) / 169 | 164 (-27 %) / 318 | 452 (-36 %) / 894 | 1604 (-39 %) / 3198 |
| `mix/memcpy` (per call) | 136 (-39 %) | | | | | | |
| `mix/memset` (per call) | 60.4 (-37 %) | | | | | | |
| `mix/strlen` (per call) | 120 | | | | | | |

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4142 (-0 %) |
| `fputs/line` | 451 (-10 %) |
| `snprintf/%-20s` | 1164 (-0 %) |
| `snprintf/%.17g` | 13910 (-1 %) |
| `snprintf/%.3f` | 13789 (-1 %) |
| `snprintf/%08x` | 916 (-0 %) |
| `snprintf/%a` | 1126 (-6 %) |
| `snprintf/%c` | 290 (-1 %) |
| `snprintf/%d` | 1636 (-0 %) |
| `snprintf/%e` | 16913 (-1 %) |
| `snprintf/%f` | 13209 (-1 %) |
| `snprintf/%g` | 14771 (-1 %) |
| `snprintf/%lld` | 11025 (-0 %) |
| `snprintf/%p` | 537 (-0 %) |
| `snprintf/%s` | 449 (-0 %) |
| `snprintf/%u-small` | 429 (-0 %) |
| `snprintf/%x` | 629 (-0 %) |
| `snprintf/literal` | 555 |
| `snprintf/mix` | 3556 (-0 %) |
| `sscanf/%d` | 784 |
| `sscanf/%lf` | 5107 |
| `sscanf/%x` | 966 |
| `sscanf/mix` | 3294 (-1 %) |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1211 |
| `strtod/17-digits` | 23288 (-3 %) |
| `strtod/exp-large` | 689247 (-1 %) |
| `strtod/exp-small` | 413319 (-1 %) |
| `strtod/fixed` | 3863 |
| `strtod/hex` | 2101 |
| `strtod/integer` | 2909 |
| `strtod/long` | 46683 (-2 %) |
| `strtod/micro` | 3368 |
| `strtod/min-normal` | 1839969 (-1 %) |
| `strtod/short` | 3352 |
| `strtod/subnormal` | 2028570 (-1 %) |
| `strtof/9-digits` | 16152 (-4 %) |
| `strtof/exp` | 77753 (-1 %) |
| `strtof/short` | 1634 |
| `strtol/hex` | 1678 |
| `strtol/int` | 1501 |
| `strtol/short` | 914 |
| `strtoll/long` | 2350 |
| `strtoul/hex16` | 1671 |
| `strtoull/hex` | 2544 |

### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 265 |
| `batch/fifo` | 237 |
| `batch/lifo` | 234 |
| `batch/shuffled` | 256 |
| `calloc/256` | 272 (-28 %) |
| `fixed/16` | 233 |
| `fixed/256` | 233 |
| `fixed/4096` | 233 |
| `realloc/grow` | 233 |

### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 389 |
| `batch/fifo` | 368 |
| `batch/lifo` | 368 |
| `batch/shuffled` | 369 |
| `calloc/256` | 279 (-28 %) |
| `fixed/16` | 240 |
| `fixed/256` | 240 |
| `fixed/4096` | 240 |
| `realloc/grow` | 232 |

### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 167 |
| `qsort/equal/1024` | 139 |
| `qsort/random/1024` | 769 |
| `qsort/random/16` | 267 |
| `qsort/random/256` | 596 |
| `qsort/reversed/1024` | 723 |
| `qsort/sorted/1024` | 813 |
| `qsort/strings/256` | 708 (+0 %) |

### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 26692 | 35757 (-0 %) |
| `acosh` | 22250 | 34288 |
| `asin` | 25875 | 34665 (-0 %) |
| `asinh` | 21693 | 39824 |
| `atan` | 18770 | 21215 (-0 %) |
| `atan2` | 19460 | 22642 (-0 %) |
| `atanh` | 16542 | 25874 (-0 %) |
| `cbrt` | 9412 | 14545 |
| `cos` | 18928 | 17700 |
| `cosh` | 15893 | 16736 |
| `erf` | 151747 | 40695 |
| `erfc` | 128215 | 112996 |
| `exp` | 11458 | 10356 |
| `exp2` | 9609 | 14172 |
| `expm1` | 12742 | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 8371 | 14812 |
| `lgamma` | 38415 | 47123 (-0 %) |
| `log` | 12864 | 17003 |
| `log10` | 12862 | 21106 |
| `log1p` | 13273 | 18064 |
| `log2` | 10973 | 21459 |
| `pow` | 23058 | 31562 |
| `round` | 103 | 210 |
| `sin` | 18996 | 17710 |
| `sinh` | 16022 | 17881 |
| `sqrt` | 2036 | 4208 |
| `tan` | 37649 | 32490 (-0 %) |
| `tanh` | 16684 | 20192 (-0 %) |
| `tgamma` | 57787 | 64762 (-0 %) |

### threads

| Case | insns/op |
|---|---:|
| `call_once/done` | 22.0 |
| `mtx/nested` | 275 |
| `mtx/plain` | 205 |
| `mtx/recursive` | 208 |
| `mtx/timed` | 205 |
| `mtx/trylock` | 203 |
| `tss_get` | 37.0 |

## P3.03a — Speed flavour: integer conversions

Division-free `strtol` and relatives, 64-bit decimals in 10^9 chunks, 32-bit shifts for hex and octal, and a
`memcpy` buffer sink, all in the speed flavour's `libc/stdlib/speed` and `libc/stdio/speed` (PLAN D77); the scanf
engine at `-O2`. Change against the P3.02 speed baseline; only the programs that changed are shown, and the release
build is unchanged.

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4086 (-1 %) |
| `fputs/line` | 451 |
| `snprintf/%-20s` | 1131 (-3 %) |
| `snprintf/%.17g` | 13887 (-0 %) |
| `snprintf/%.3f` | 13778 (-0 %) |
| `snprintf/%08x` | 801 (-13 %) |
| `snprintf/%a` | 1101 (-2 %) |
| `snprintf/%c` | 291 (+0 %) |
| `snprintf/%d` | 1622 (-1 %) |
| `snprintf/%e` | 16901 (-0 %) |
| `snprintf/%f` | 13202 (-0 %) |
| `snprintf/%g` | 14761 (-0 %) |
| `snprintf/%lld` | 3702 (-66 %) |
| `snprintf/%p` | 495 (-8 %) |
| `snprintf/%s` | 428 (-5 %) |
| `snprintf/%u-small` | 429 |
| `snprintf/%x` | 514 (-18 %) |
| `snprintf/literal` | 354 (-36 %) |
| `snprintf/mix` | 3439 (-3 %) |
| `sscanf/%d` | 655 (-16 %) |
| `sscanf/%lf` | 5088 (-0 %) |
| `sscanf/%x` | 815 (-16 %) |
| `sscanf/mix` | 2756 (-16 %) |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 277 (-77 %) |
| `strtod/17-digits` | 23288 |
| `strtod/exp-large` | 689247 |
| `strtod/exp-small` | 413319 |
| `strtod/fixed` | 3863 |
| `strtod/hex` | 2101 |
| `strtod/integer` | 2909 |
| `strtod/long` | 46683 |
| `strtod/micro` | 3368 |
| `strtod/min-normal` | 1839969 |
| `strtod/short` | 3352 |
| `strtod/subnormal` | 2028570 |
| `strtof/9-digits` | 16152 |
| `strtof/exp` | 77753 |
| `strtof/short` | 1634 |
| `strtol/hex` | 458 (-73 %) |
| `strtol/int` | 422 (-72 %) |
| `strtol/short` | 170 (-81 %) |
| `strtoll/long` | 1060 (-55 %) |
| `strtoul/hex16` | 436 (-74 %) |
| `strtoull/hex` | 1027 (-60 %) |

## P3.03b — Speed flavour: `strtod` middle path

A compact-table Eisel–Lemire-style middle path between Clinger's fast path and Simple Decimal Conversion, in-line
digit tests and a division-free Clinger guard, in the speed flavour's `libc/stdlib/speed/__libc_strtofloat.c`
(PLAN D78). Change against the P3.03a speed baseline; only the programs that changed are shown, and the release
build is unchanged.

### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4086 |
| `fputs/line` | 451 |
| `snprintf/%-20s` | 1131 |
| `snprintf/%.17g` | 13887 |
| `snprintf/%.3f` | 13778 |
| `snprintf/%08x` | 801 |
| `snprintf/%a` | 1101 |
| `snprintf/%c` | 291 |
| `snprintf/%d` | 1622 |
| `snprintf/%e` | 16901 |
| `snprintf/%f` | 13202 |
| `snprintf/%g` | 14761 |
| `snprintf/%lld` | 3702 |
| `snprintf/%p` | 495 |
| `snprintf/%s` | 428 |
| `snprintf/%u-small` | 429 |
| `snprintf/%x` | 514 |
| `snprintf/literal` | 354 |
| `snprintf/mix` | 3439 |
| `sscanf/%d` | 655 |
| `sscanf/%lf` | 4076 (-20 %) |
| `sscanf/%x` | 815 |
| `sscanf/mix` | 2756 |

### strto

| Case | insns/op |
|---|---:|
| `atoi` | 277 |
| `strtod/17-digits` | 3799 (-84 %) |
| `strtod/exp-large` | 4087 (-99 %) |
| `strtod/exp-small` | 3838 (-99 %) |
| `strtod/fixed` | 2928 (-24 %) |
| `strtod/hex` | 2116 (+1 %) |
| `strtod/integer` | 2005 (-31 %) |
| `strtod/long` | 5026 (-89 %) |
| `strtod/micro` | 2435 (-28 %) |
| `strtod/min-normal` | 3853 (-100 %) |
| `strtod/short` | 2459 (-27 %) |
| `strtod/subnormal` | 3849 (-100 %) |
| `strtof/9-digits` | 2402 (-85 %) |
| `strtof/exp` | 2433 (-97 %) |
| `strtof/short` | 1008 (-38 %) |
| `strtol/hex` | 458 |
| `strtol/int` | 422 |
| `strtol/short` | 170 |
| `strtoll/long` | 1060 |
| `strtoul/hex16` | 436 |
| `strtoull/hex` | 1027 |

## P3.04a — Lock word fast path in the port (both flavours)

The seam's lock word (PLAN D79): the Linux port's `__rtos_lock_acquire` and `__rtos_lock_release` start with one
compare-and-swap on the word, with the owner's thread-local token, and need neither `__rtos_thread_self` nor an owner
pointer. libc is unchanged, so both flavours gain alike. Change against the P3.03b baselines; only the programs that
changed are shown.

### Release (size flavour)

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4134 (-0 %) |
| `fputs/line` | 494 (-2 %) |
| `snprintf/%-20s` | 1166 |
| `snprintf/%.17g` | 13988 |
| `snprintf/%.3f` | 13867 |
| `snprintf/%08x` | 918 |
| `snprintf/%a` | 1204 |
| `snprintf/%c` | 292 |
| `snprintf/%d` | 1638 |
| `snprintf/%e` | 16998 |
| `snprintf/%f` | 13287 |
| `snprintf/%g` | 14849 |
| `snprintf/%lld` | 11027 |
| `snprintf/%p` | 539 |
| `snprintf/%s` | 451 |
| `snprintf/%u-small` | 431 |
| `snprintf/%x` | 631 |
| `snprintf/literal` | 555 |
| `snprintf/mix` | 3564 |
| `sscanf/%d` | 784 |
| `sscanf/%lf` | 5107 |
| `sscanf/%x` | 966 |
| `sscanf/mix` | 3311 |

#### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 256 (-3 %) |
| `batch/fifo` | 228 (-4 %) |
| `batch/lifo` | 225 (-4 %) |
| `batch/shuffled` | 247 (-4 %) |
| `calloc/256` | 369 (-2 %) |
| `fixed/16` | 224 (-4 %) |
| `fixed/256` | 224 (-4 %) |
| `fixed/4096` | 224 (-4 %) |
| `realloc/grow` | 225 (-4 %) |

#### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 380 (-2 %) |
| `batch/fifo` | 359 (-2 %) |
| `batch/lifo` | 359 (-2 %) |
| `batch/shuffled` | 360 (-2 %) |
| `calloc/256` | 376 (-2 %) |
| `fixed/16` | 232 (-4 %) |
| `fixed/256` | 232 (-4 %) |
| `fixed/4096` | 232 (-4 %) |
| `realloc/grow` | 224 (-4 %) |

### Speed flavour

#### stdio

| Case | insns/op |
|---|---:|
| `fprintf/mix` | 4077 (-0 %) |
| `fputs/line` | 442 (-2 %) |
| `snprintf/%-20s` | 1131 |
| `snprintf/%.17g` | 13887 |
| `snprintf/%.3f` | 13778 |
| `snprintf/%08x` | 801 |
| `snprintf/%a` | 1101 |
| `snprintf/%c` | 291 |
| `snprintf/%d` | 1622 |
| `snprintf/%e` | 16901 |
| `snprintf/%f` | 13202 |
| `snprintf/%g` | 14761 |
| `snprintf/%lld` | 3702 |
| `snprintf/%p` | 495 |
| `snprintf/%s` | 428 |
| `snprintf/%u-small` | 429 |
| `snprintf/%x` | 514 |
| `snprintf/literal` | 354 |
| `snprintf/mix` | 3439 |
| `sscanf/%d` | 655 |
| `sscanf/%lf` | 4076 |
| `sscanf/%x` | 815 |
| `sscanf/mix` | 2756 |

#### malloc

| Case | insns/op |
|---|---:|
| `batch/churn` | 256 (-3 %) |
| `batch/fifo` | 228 (-4 %) |
| `batch/lifo` | 225 (-4 %) |
| `batch/shuffled` | 247 (-4 %) |
| `calloc/256` | 262 (-3 %) |
| `fixed/16` | 224 (-4 %) |
| `fixed/256` | 224 (-4 %) |
| `fixed/4096` | 224 (-4 %) |
| `realloc/grow` | 225 (-4 %) |

#### malloc-tiny

| Case | insns/op |
|---|---:|
| `batch/churn` | 380 (-2 %) |
| `batch/fifo` | 359 (-2 %) |
| `batch/lifo` | 359 (-2 %) |
| `batch/shuffled` | 360 (-2 %) |
| `calloc/256` | 270 (-3 %) |
| `fixed/16` | 232 (-4 %) |
| `fixed/256` | 232 (-4 %) |
| `fixed/4096` | 232 (-4 %) |
| `realloc/grow` | 224 (-4 %) |

## P3.05e — Integer float multiplication, division and square root (both flavours)

`__aeabi_fmul`, `__aeabi_fdiv` and `sqrtf` are computed in integers in the `nofp` layer (PLAN D85). Every float function
of the `sp-nearest` layer calls the helpers. Change against the P3.05c baselines; only the programs that changed are
shown.

### Release (size flavour)

#### strto

| Case | insns/op |
|---|---:|
| `atoi` | 1211 |
| `strtod/17-digits` | 24062 |
| `strtod/exp-large` | 692937 |
| `strtod/exp-small` | 416299 |
| `strtod/fixed` | 3863 |
| `strtod/hex` | 2101 |
| `strtod/integer` | 2909 |
| `strtod/long` | 47601 |
| `strtod/micro` | 3368 |
| `strtod/min-normal` | 1850827 |
| `strtod/short` | 3352 |
| `strtod/subnormal` | 2040498 |
| `strtof/9-digits` | 16877 (-0 %) |
| `strtof/exp` | 78884 (-0 %) |
| `strtof/short` | 1529 (-6 %) |
| `strtol/hex` | 1678 |
| `strtol/int` | 1501 |
| `strtol/short` | 914 |
| `strtoll/long` | 2350 |
| `strtoul/hex16` | 1671 |
| `strtoull/hex` | 2544 |

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 23828 (-11 %) | 35771 |
| `acosh` | 19926 (-10 %) | 34288 |
| `asin` | 23011 (-11 %) | 34679 |
| `asinh` | 19353 (-11 %) | 39824 |
| `atan` | 17750 (-5 %) | 21229 |
| `atan2` | 18422 (-5 %) | 22656 |
| `atanh` | 15868 (-4 %) | 25888 |
| `cbrt` | 8590 (-9 %) | 14545 |
| `cos` | 18281 (-3 %) | 17700 |
| `cosh` | 15181 (-4 %) | 16736 |
| `erf` | 141024 (-7 %) | 40695 |
| `erfc` | 119604 (-7 %) | 112996 |
| `exp` | 11034 (-4 %) | 10356 |
| `exp2` | 9269 (-4 %) | 14172 |
| `expm1` | 12310 (-3 %) | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 6293 (-25 %) | 14812 |
| `lgamma` | 36729 (-4 %) | 47161 |
| `log` | 12367 (-4 %) | 17003 |
| `log10` | 12365 (-4 %) | 21106 |
| `log1p` | 12789 (-4 %) | 18064 |
| `log2` | 10547 (-4 %) | 21459 |
| `pow` | 22158 (-4 %) | 31562 |
| `round` | 103 | 210 |
| `sin` | 18350 (-3 %) | 17710 |
| `sinh` | 15310 (-4 %) | 17881 |
| `sqrt` | 461 (-77 %) | 4208 |
| `tan` | 36223 (-4 %) | 32504 |
| `tanh` | 15977 (-4 %) | 20206 |
| `tgamma` | 55326 (-4 %) | 64810 |

### Speed flavour

#### strto

| Case | insns/op |
|---|---:|
| `atoi` | 277 |
| `strtod/17-digits` | 3799 |
| `strtod/exp-large` | 4087 |
| `strtod/exp-small` | 3838 |
| `strtod/fixed` | 2928 |
| `strtod/hex` | 2116 |
| `strtod/integer` | 2005 |
| `strtod/long` | 5026 |
| `strtod/micro` | 2435 |
| `strtod/min-normal` | 3853 |
| `strtod/short` | 2459 |
| `strtod/subnormal` | 3849 |
| `strtof/9-digits` | 2389 (-1 %) |
| `strtof/exp` | 2427 (-0 %) |
| `strtof/short` | 903 (-10 %) |
| `strtol/hex` | 458 |
| `strtol/int` | 422 |
| `strtol/short` | 170 |
| `strtoll/long` | 1060 |
| `strtoul/hex16` | 436 |
| `strtoull/hex` | 1027 |

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 23828 (-11 %) | 35757 |
| `acosh` | 19926 (-10 %) | 34288 |
| `asin` | 23011 (-11 %) | 34665 |
| `asinh` | 19353 (-11 %) | 39824 |
| `atan` | 17750 (-5 %) | 21215 |
| `atan2` | 18422 (-5 %) | 22642 |
| `atanh` | 15868 (-4 %) | 25874 |
| `cbrt` | 8590 (-9 %) | 14545 |
| `cos` | 18281 (-3 %) | 17700 |
| `cosh` | 15181 (-4 %) | 16736 |
| `erf` | 141024 (-7 %) | 40695 |
| `erfc` | 119604 (-7 %) | 112996 |
| `exp` | 11034 (-4 %) | 10356 |
| `exp2` | 9269 (-4 %) | 14172 |
| `expm1` | 12310 (-3 %) | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 6293 (-25 %) | 14812 |
| `lgamma` | 36729 (-4 %) | 47123 |
| `log` | 12367 (-4 %) | 17003 |
| `log10` | 12365 (-4 %) | 21106 |
| `log1p` | 12789 (-4 %) | 18064 |
| `log2` | 10547 (-4 %) | 21459 |
| `pow` | 22158 (-4 %) | 31562 |
| `round` | 103 | 210 |
| `sin` | 18350 (-3 %) | 17710 |
| `sinh` | 15310 (-4 %) | 17881 |
| `sqrt` | 461 (-77 %) | 4208 |
| `tan` | 36223 (-4 %) | 32490 |
| `tanh` | 15977 (-4 %) | 20192 |
| `tgamma` | 55326 (-4 %) | 64762 |

## P3.05f — Fixed-point exponentials and logarithms (speed flavour)

`expf`, `exp2f`, `logf` and `log2f` take a 32-bit fixed-point fast path with a rounding test before the float-float
code (PLAN D86). The release flavour is unchanged. Change against the P3.05e baselines; only the program that changed
is shown.

### Speed flavour

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 23828 | 35757 |
| `acosh` | 19926 | 34288 |
| `asin` | 23011 | 34665 |
| `asinh` | 19353 | 39824 |
| `atan` | 17750 | 21215 |
| `atan2` | 18422 | 22642 |
| `atanh` | 15868 | 25874 |
| `cbrt` | 8590 | 14545 |
| `cos` | 18281 | 17700 |
| `cosh` | 15181 | 16736 |
| `erf` | 141024 | 40695 |
| `erfc` | 119604 | 112996 |
| `exp` | 375 (-97 %) | 10356 |
| `exp2` | 316 (-97 %) | 14172 |
| `expm1` | 12310 | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 6293 | 14812 |
| `lgamma` | 36729 | 47123 |
| `log` | 2210 (-82 %) | 17003 |
| `log10` | 12365 | 21106 |
| `log1p` | 12789 | 18064 |
| `log2` | 583 (-94 %) | 21459 |
| `pow` | 22158 | 31562 |
| `round` | 103 | 210 |
| `sin` | 18350 | 17710 |
| `sinh` | 15310 | 17881 |
| `sqrt` | 461 | 4208 |
| `tan` | 36223 | 32490 |
| `tanh` | 15977 | 20192 |
| `tgamma` | 55326 | 64762 |

## P3.05g — Fixed-point sine, cosine and tangent (speed flavour)

`sinf`, `cosf` and `tanf` take a 32-bit fixed-point fast path after the integer reduction, with a rounding test
before the float-float code (PLAN D87). The release flavour is unchanged. Change against the P3.05f baselines; only
the program that changed is shown.

### Speed flavour

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 23828 | 35757 |
| `acosh` | 19926 | 34288 |
| `asin` | 23011 | 34665 |
| `asinh` | 19353 | 39824 |
| `atan` | 17750 | 21215 |
| `atan2` | 18422 | 22642 |
| `atanh` | 15868 | 25874 |
| `cbrt` | 8590 | 14545 |
| `cos` | 3742 (-80 %) | 17700 |
| `cosh` | 15181 | 16736 |
| `erf` | 141024 | 40695 |
| `erfc` | 119604 | 112996 |
| `exp` | 375 | 10356 |
| `exp2` | 316 | 14172 |
| `expm1` | 12310 | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 6293 | 14812 |
| `lgamma` | 36729 | 47123 |
| `log` | 2210 | 17003 |
| `log10` | 12365 | 21106 |
| `log1p` | 12789 | 18064 |
| `log2` | 583 | 21459 |
| `pow` | 22158 | 31562 |
| `round` | 103 | 210 |
| `sin` | 4729 (-74 %) | 17710 |
| `sinh` | 15310 | 17881 |
| `sqrt` | 461 | 4208 |
| `tan` | 5140 (-86 %) | 32490 |
| `tanh` | 15977 | 20192 |
| `tgamma` | 55326 | 64762 |

## P3.05h — Fixed-point error functions (speed flavour)

`erff` and `erfcf` take a fixed-point fast path (Taylor steps from ODE recurrences) with a rounding test before the
float-float code (PLAN D88). The release flavour is unchanged. Change against the P3.05g baselines; only the program
that changed is shown.

### Speed flavour

#### math

Throughput, per call (the latency counts are the same under qemu).

| Function | float | double |
|---|---:|---:|
| `acos` | 23828 | 35757 |
| `acosh` | 19926 | 34288 |
| `asin` | 23011 | 34665 |
| `asinh` | 19353 | 39824 |
| `atan` | 17750 | 21215 |
| `atan2` | 18422 | 22642 |
| `atanh` | 15868 | 25874 |
| `cbrt` | 8590 | 14545 |
| `cos` | 3742 | 17700 |
| `cosh` | 15181 | 16736 |
| `erf` | 2967 (-98 %) | 40695 |
| `erfc` | 2874 (-98 %) | 112996 |
| `exp` | 375 | 10356 |
| `exp2` | 316 | 14172 |
| `expm1` | 12310 | 10978 |
| `floor` | 105 | 212 |
| `fmod` | 273 | 486 |
| `hypot` | 6293 | 14812 |
| `lgamma` | 36729 | 47123 |
| `log` | 2210 | 17003 |
| `log10` | 12365 | 21106 |
| `log1p` | 12789 | 18064 |
| `log2` | 583 | 21459 |
| `pow` | 22158 | 31562 |
| `round` | 103 | 210 |
| `sin` | 4729 | 17710 |
| `sinh` | 15310 | 17881 |
| `sqrt` | 461 | 4208 |
| `tan` | 5140 | 32490 |
| `tanh` | 15977 | 20192 |
| `tgamma` | 55326 | 64762 |

## P3.07 — Introsort qsort (speed flavour)

The speed flavour's `qsort` is a run check and an introsort (PLAN D90). The release flavour is unchanged. Change
against the P3.05h baselines; only the program that changed is shown.

### Speed flavour

#### qsort

| Case | insns/op |
|---|---:|
| `bsearch/1024` | 167 |
| `qsort/equal/1024` | 32.1 (-77 %) |
| `qsort/random/1024` | 314 (-59 %) |
| `qsort/random/16` | 156 (-41 %) |
| `qsort/random/256` | 250 (-58 %) |
| `qsort/reversed/1024` | 41.6 (-94 %) |
| `qsort/sorted/1024` | 31.1 (-96 %) |
| `qsort/strings/256` | 356 (-50 %) |

## P4.03 — New program: `softfloat`

The floating-point operations that become run-time library calls without an FPU (PLAN D92). The cases are new, so there is no change column; the other programs are unchanged.

### Release (size flavour)

| Case | insns/op |
|---|---:|
| `d2f` | 51.5 |
| `d2i` | 38.4 |
| `d2l` | 696 |
| `dadd` | 107 |
| `dcmp` | 66.9 |
| `ddiv` | 844 |
| `dmul` | 264 |
| `dsub` | 116 |
| `f2d` | 32.4 |
| `f2i` | 24.9 |
| `f2l` | 689 |
| `fadd` | 90.3 |
| `fcmp` | 44.4 |
| `fdiv` | 292 |
| `fmul` | 109 |
| `fsub` | 84.8 |
| `i2d` | 61.6 |
| `i2f` | 76.6 |
| `l2d` | 473 |
| `l2f` | 525 |

### Speed flavour

| Case | insns/op |
|---|---:|
| `d2f` | 51.5 |
| `d2i` | 38.4 |
| `d2l` | 696 |
| `dadd` | 107 |
| `dcmp` | 66.9 |
| `ddiv` | 844 |
| `dmul` | 264 |
| `dsub` | 116 |
| `f2d` | 32.4 |
| `f2i` | 24.9 |
| `f2l` | 689 |
| `fadd` | 90.3 |
| `fcmp` | 44.4 |
| `fdiv` | 292 |
| `fmul` | 109 |
| `fsub` | 84.8 |
| `i2d` | 61.6 |
| `i2f` | 76.6 |
| `l2d` | 473 |
| `l2f` | 525 |

## D102 — the conversions to integers in the soft-int and nofp layers

`__aeabi_d2lz`, `__aeabi_d2ulz`, `__aeabi_f2lz`, `__aeabi_f2ulz`, `__aeabi_d2uiz` and `__aeabi_f2uiz` replace libgcc's. libgcc's versions subtracted in floating point and linked libgcc's `subdf3.o`. 76 cases are faster in each flavour and none is slower. Instructions per operation (release and speed alike unless shown):

| Case | before | after | change |
|---|---:|---:|---:|
| `softfloat/d2l` | 696 | 62.5 | −91.0 % |
| `softfloat/f2l` | 689 | 45.4 | −93.4 % |
| `strto/strtod/integer` | 2909 (speed 2005) | 2272 (1368) | −21.9 % (−31.8 %) |
| `strto/strtod/hex` | 2101 (2116) | 1469 (1484) | −30.1 % (−29.9 %) |
| `strto/strtod/short` | 3352 (2459) | 2720 (1827) | −18.9 % (−25.7 %) |
| `strto/strtod/17-digits` | 24062 (3799) | 23430 (3167) | −2.6 % (−16.6 %) |
| `stdio/sscanf/%lf` | 5107 (4076) | 5028 (3997) | −1.5 % (−1.9 %) |

Only `d2l` and `f2l` measure the library. The other changes are the benchmarks' own conversions: `run_strtod` folds each result into its checksum through `__aeabi_d2lz` (634 instructions fewer per operation), the double `math` loops do the same (39.6 fewer), and `run_sscanf_f` through `__aeabi_d2uiz` (79 fewer). `strtod`, `sscanf` and the libm functions are unchanged; `floor` and `round` show −16 to −19 % only because they are short.


## P5.04a — Speed flavour: fixed-point `log10f` (D114)

`libc/math/nofp/speed/log10f.c`: the D86 `log2f` core times log10 2, with the
float-float `log10f` as fallback (`docs/research/libm-fixed.md`, P5.04a). Only
the `log10f` cases change; change against the P5.03 baseline (956fdb6).
Throughput, instructions per call.

| Function | float | double |
|---|---:|---:|
| `log10` | 678 (-95 %) | 21066 |


## P5.04b — Speed flavour: fixed-point `powf` (D115)

`libc/math/nofp/speed/powf.c`: y log2 |x| from the wide log2 core, then the
D86 exp2 core, with the float-float `powf` as fallback
(`docs/research/libm-fixed.md`, P5.04b). Only the `powf` cases change;
change against the P5.04a baseline (c4abaa0). Throughput, instructions per call.

| Function | float | double |
|---|---:|---:|
| `pow` | 1222 (-94 %) | 31523 |

## D125 — the libm vectors' accuracy fixes (Q36)

Correctness fixes, so the slowdowns are accepted and the baselines re-recorded (both flavours): `cbrt` 1.4 % slower (the residual's extra soft-double products); `pow` within 1 %. Every other case is within 1 %.
