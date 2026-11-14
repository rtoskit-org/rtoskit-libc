# Phase 11: the fast flavour against picolibc, newlib-nano and AOR

Plan: `docs/plan/phase-11.md` (P11.06a). Decisions: PLAN D162–D205, D206.
Research: `docs/research/fast-math.md`, `libm-fast-float-exp-log.md`,
`libm-fast-float-trig.md`, `libm-fast-double.md`.

Measured at the P11.05o library (`c2cb4a7`):

- AOR revision `f67e7daa5457646fad9f75580263dba654c988aa` (PLAN D92, D93);
- picolibc 1.8.6 and newlib-nano 4.4.0.20231231 (the Ubuntu packages of
  `size-report compare`);
- qemu 8.2.2 with the instruction-counting plugin;
- arm-none-eabi GCC 16.2.

As in phases 3 and 4 (`docs/perf/phase-3.md`, `phase-4.md`), the speed
figures count guest instructions per operation, not cycles.

## Summary

The fast build (`BUILD_TYPE=fast`, D162) replaces the strict libm with a
faster, less accurate one: round to nearest only, target 0.55 ulp, bound 1 ulp
(1.5 for the hyperbolic functions and `erf`, D164). Every other module is the
speed flavour's.

**Against picolibc and newlib-nano.** The table gives each library's
instructions as a ratio to the fast flavour, as a geometric mean over the 132
math cases all five builds share. The C23 functions neither library has are
left out (D206).

| Core | size | speed | picolibc | newlib-nano |
|---|---:|---:|---:|---:|
| armv6-m, armv6s-m | 8.28 | 4.55 | 2.52 | 2.65 |
| armv7-m | 11.86 | 5.94 | 3.69 | 3.85 |
| armv7e-m, armv8-m.main | 4.32 | 3.81 | 1.75 | 1.86 |
| armv7e-m-dp | 2.12 | 2.12 | 1.15 | 1.01 |

**Against AOR.** The table gives AOR's instructions as a ratio to the fast
flavour, over the 17 functions AOR provides (`cosf`, `erf`, `erff`, `exp`,
`exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`,
`logf`, `pow`, `powf`, `sincosf`, `sinf`), throughput.

| Core | AOR / fast, geometric mean | range | speed / fast |
|---|---:|---:|---:|
| armv6-m, armv6s-m | 4.32 | 2.73–25.2 | 3.88 |
| armv7-m | 4.30 | 2.15–23.1 | 4.52 |
| armv7e-m, armv8-m.main | 6.04, 6.08 | 3.34–16.0 | 6.28, 6.30 |
| armv7e-m-dp | 0.83 | 0.55–1.06 | 3.08 |
| x86_64 (not gated) | 1.29 | 0.49–44.9 | 4.57 |

**Accuracy** (P4.05's method: 16,384 sampled arguments per function, round to
nearest, measured on the host with MPFR; the full table is below).

- Every fast function stays inside its bound.
- On the soft-float and double-FPU cores, the fast functions stay at or below
  0.524 ulp.
- AOR misrounds more often than the fast build in `sinf`, `cosf`, `erff`
  (0.887 ulp) and `erf` (0.837 ulp).
- On armv7e-m (single-precision FPU), the fast float path of `expf`,
  `exp2f`, `powf` and `erff` reaches 0.72–0.80 ulp, in about 5 % of the
  samples. This is the float-only design these notes already measured
  exhaustively: `expf` 0.8527 ulp, `libm-fast-float-exp-log.md` §6. It is
  inside the 1-ulp bound, but above the 0.55 target.

## Exit check (P11.06a)

1. **armv7e-m, the fast build's maths throughput at or below picolibc's.** Met
   on the aggregate: picolibc takes 1.75 times the fast build's instructions
   over the 132 shared cases.

   Not met case by case: 22 throughput cases are slower than picolibc's
   (Q54). They fall into three groups:
   - **Functions phase 11 left strict.** `sqrt`, `hypot`, `hypotf`, `erfc`,
     `erfcf`, `lgamma`, `lgammaf`, `tgamma` and `tgammaf`, from 1.4 times
     (`erfcf`) to 5.7 times (`erfc`) picolibc's instructions.
   - **Float-float fast files that are still slower.** Throughput,
     instructions, fast against picolibc:

     | Function | Fast | picolibc |
     |---|---:|---:|
     | `sinf` | 128 | 101 |
     | `cosf` | 123 | 100 |
     | `tanf` | 216 | 96 |
     | `atanf` | 92 | 65 |
     | `asinf` | 64 | 57 |
     | `acosf` | 63 | 50 |
     | `atan2f` | 118 | 109 |
     | `cbrtf` | 81 | 53 |
     | `expm1f` | 78 | 61 |
     | `log1pf` | 96 | 82 |

   - **The inverse hyperbolic floats.** `asinhf`, `acoshf` and `atanhf` are
     4–21 % slower.

   picolibc computes these functions in plain float, at about 1 ulp. Our
   fast files keep 0.5 ulp, with float-float arithmetic.
2. **At or below AOR's on every case AOR implements, on every Arm core.** Met on
   armv6-m, armv6s-m, armv7-m, armv7e-m and armv8-m.main: no case is slower,
   in throughput or in latency.

   Not met on armv7e-m-dp (Q55). There AOR's float functions evaluate in
   double on the FPU, while ours use the single-precision files of
   armv7e-m (float-float with `VFMA`). The cases where AOR is faster:
   - `powf`: AOR takes 0.55 of our instructions.
   - `logf`, `log2f`, `log10f`: 0.57–0.60.
   - `erff`: 0.59.
   - `exp2f`, `expf`: 0.78–0.81.
   - `sinf`, `cosf`, `sincosf`: 0.87–0.96.
   - `log`: equal to the instruction count shown (67.6), 0.04 behind in the
     second decimal.

   The fast double functions are level with AOR or ahead: `exp`, `exp2`,
   `exp10`, `log2`, `pow`, and `erf` (68 against 69).

### Reproducing the phase

```sh
make && make aor                                       # every build type, AOR's archives
tools/scripts/bench-runner --build-type fast compare   # picolibc and newlib-nano (downloads both packages)
tools/scripts/bench-runner --build-type fast aor --record   # docs/perf/aor-<arch>-fast.json and the AOR tables
tools/scripts/aor-accuracy arm                         # accuracy samples (libmpfr-dev), size, speed, fast and AOR
tools/scripts/aor-accuracy markdown build/x86_64/aor/accuracy/arm-results.txt
```

`bench-runner --build-type fast aor --from-record` regenerates the AOR
tables from the committed records.

## Against picolibc and newlib-nano

Instructions per operation. The math program is built against each library
with `tools/bench-compare/math.c`, which declares the seven C23 function
pairs neither library has (`exp2m1`, `exp10m1`, `log2p1`, `log10p1`,
`sinpi`, `cospi`, `tanpi`, and their float forms). `c23-math.c` links
placeholders for them, and their cases are dropped (`COMPARE_ABSENT`, D206).

#### armv6-m

Instructions per operation; the geometric mean is over every case all 5 builds share, as a ratio to rtoskit's fast flavour.

| Program | Case | rtoskit (size) | rtoskit (speed) | rtoskit (fast) | picolibc | newlib-nano |
|---|---|---:|---:|---:|---:|---:|
| string | geometric mean (603 cases) | 1.13 | 1.00 | 1.00 | 1.31 | 1.15 |
| | `memcpy/16/0:0` | 54.0 | 40.0 | 40.0 | 119 | 65.0 |
| | `memcpy/1024/0:0` | 1314 | 292 | 292 | 6167 | 884 |
| | `memset/1024/0` | 1054 | 169 | 169 | 5141 | 503 |
| | `strlen/64/0` | 115 | 115 | 115 | 279 | 146 |
| | `strcmp/64/0:0` | 541 | 478 | 478 | 537 | 167 |
| stdio | geometric mean (23 cases) | 1.14 | 1.00 | 1.00 | 1.67 | 1.12 |
| | `snprintf/%d` | 1638 | 1622 | 1622 | 2151 | 1222 |
| | `snprintf/%s` | 451 | 428 | 428 | 650 | 669 |
| | `snprintf/%f` | 13287 | 13202 | 13202 | 15716 | 6331 |
| | `snprintf/%g` | 14849 | 14761 | 14761 | 13075 | 6444 |
| | `fprintf/mix` | 4134 | 4077 | 4077 | 5802 | 4335 |
| strto | geometric mean (21 cases) | 8.09 | 1.00 | 1.00 | 1.36 | 1.68 |
| | `strtol/int` | 1501 | 422 | 422 | 513 | 535 |
| | `strtod/short` | 2720 | 1827 | 1827 | 2625 | 2079 |
| | `strtod/17-digits` | 23430 | 3167 | 3167 | 3908 | 5320 |
| malloc | geometric mean (9 cases) | 1.04 | 1.00 | 1.00 | 3.87 | 0.36 |
| | `fixed/16` | 224 | 224 | 224 | 90.5 | 49.5 |
| | `batch/shuffled` | 247 | 247 | 247 | 412 | 75.0 |
| | `realloc/grow` | 225 | 225 | 225 | 36095 | 452 |
| qsort | geometric mean (8 cases) | 3.75 | 1.00 | 1.00 | 1.39 | 1.35 |
| | `qsort/random/1024` | 769 | 314 | 314 | 313 | 309 |
| math | geometric mean (132 cases) | 8.28 | 4.55 | 1.00 | 2.52 | 2.65 |
| | `sqrtf/throughput` | 461 | 461 | 461 | 397 | 718 |
| | `expf/throughput` | 11034 | 375 | 368 | 2659 | 2768 |
| | `logf/throughput` | 12367 | 2210 | 670 | 2912 | 2982 |
| | `sinf/throughput` | 18350 | 2933 | 1090 | 2697 | 2706 |
| | `powf/throughput` | 22158 | 1222 | 1216 | 9269 | 9376 |
| | `erff/throughput` | 141024 | 2967 | 420 | 6949 | 6967 |
| | `exp/throughput` | 10317 | 10317 | 1410 | 5013 | 5156 |
| | `log/throughput` | 16964 | 16964 | 1454 | 5631 | 5734 |
| | `sin/throughput` | 17669 | 2833 | 2775 | 5001 | 5003 |
| | `pow/throughput` | 31522 | 31522 | 3938 | 17110 | 17279 |

#### armv6s-m

Instructions per operation; the geometric mean is over every case all 5 builds share, as a ratio to rtoskit's fast flavour.

| Program | Case | rtoskit (size) | rtoskit (speed) | rtoskit (fast) | picolibc | newlib-nano |
|---|---|---:|---:|---:|---:|---:|
| string | geometric mean (603 cases) | 1.13 | 1.00 | 1.00 | 1.31 | 1.15 |
| | `memcpy/16/0:0` | 54.0 | 40.0 | 40.0 | 119 | 65.0 |
| | `memcpy/1024/0:0` | 1314 | 292 | 292 | 6167 | 884 |
| | `memset/1024/0` | 1054 | 169 | 169 | 5141 | 503 |
| | `strlen/64/0` | 115 | 115 | 115 | 279 | 146 |
| | `strcmp/64/0:0` | 541 | 478 | 478 | 537 | 167 |
| stdio | geometric mean (23 cases) | 1.14 | 1.00 | 1.00 | 1.67 | 1.12 |
| | `snprintf/%d` | 1638 | 1622 | 1622 | 2151 | 1222 |
| | `snprintf/%s` | 451 | 428 | 428 | 650 | 669 |
| | `snprintf/%f` | 13287 | 13202 | 13202 | 15716 | 6331 |
| | `snprintf/%g` | 14849 | 14761 | 14761 | 13075 | 6444 |
| | `fprintf/mix` | 4134 | 4077 | 4077 | 5802 | 4335 |
| strto | geometric mean (21 cases) | 8.09 | 1.00 | 1.00 | 1.36 | 1.68 |
| | `strtol/int` | 1501 | 422 | 422 | 513 | 535 |
| | `strtod/short` | 2720 | 1827 | 1827 | 2625 | 2079 |
| | `strtod/17-digits` | 23430 | 3167 | 3167 | 3908 | 5320 |
| malloc | geometric mean (9 cases) | 1.04 | 1.00 | 1.00 | 3.87 | 0.36 |
| | `fixed/16` | 224 | 224 | 224 | 90.5 | 49.5 |
| | `batch/shuffled` | 247 | 247 | 247 | 412 | 75.0 |
| | `realloc/grow` | 225 | 225 | 225 | 36095 | 452 |
| qsort | geometric mean (8 cases) | 3.75 | 1.00 | 1.00 | 1.39 | 1.35 |
| | `qsort/random/1024` | 769 | 314 | 314 | 313 | 309 |
| math | geometric mean (132 cases) | 8.28 | 4.55 | 1.00 | 2.52 | 2.65 |
| | `sqrtf/throughput` | 461 | 461 | 461 | 397 | 718 |
| | `expf/throughput` | 11034 | 375 | 368 | 2659 | 2768 |
| | `logf/throughput` | 12367 | 2210 | 670 | 2912 | 2982 |
| | `sinf/throughput` | 18350 | 2933 | 1090 | 2697 | 2706 |
| | `powf/throughput` | 22158 | 1222 | 1216 | 9269 | 9376 |
| | `erff/throughput` | 141024 | 2967 | 420 | 6949 | 6967 |
| | `exp/throughput` | 10317 | 10317 | 1410 | 5013 | 5156 |
| | `log/throughput` | 16964 | 16964 | 1454 | 5631 | 5734 |
| | `sin/throughput` | 17669 | 2833 | 2775 | 5001 | 5003 |
| | `pow/throughput` | 31522 | 31522 | 3938 | 17110 | 17279 |

#### armv7-m

Instructions per operation; the geometric mean is over every case all 5 builds share, as a ratio to rtoskit's fast flavour.

| Program | Case | rtoskit (size) | rtoskit (speed) | rtoskit (fast) | picolibc | newlib-nano |
|---|---|---:|---:|---:|---:|---:|
| string | geometric mean (603 cases) | 1.47 | 1.00 | 1.00 | 1.53 | 1.22 |
| | `memcpy/16/0:0` | 53.0 | 42.0 | 42.0 | 87.0 | 43.0 |
| | `memcpy/1024/0:0` | 1313 | 174 | 174 | 4119 | 607 |
| | `memset/1024/0` | 1054 | 177 | 177 | 4117 | 366 |
| | `strlen/64/0` | 115 | 107 | 107 | 215 | 154 |
| | `strcmp/64/0:0` | 540 | 240 | 240 | 410 | 176 |
| stdio | geometric mean (23 cases) | 1.22 | 1.00 | 1.00 | 1.16 | 0.96 |
| | `snprintf/%d` | 475 | 455 | 455 | 893 | 528 |
| | `snprintf/%s` | 424 | 396 | 396 | 543 | 543 |
| | `snprintf/%f` | 14751 | 14669 | 14669 | 3990 | 3301 |
| | `snprintf/%g` | 15561 | 15476 | 15476 | 3640 | 3416 |
| | `fprintf/mix` | 2326 | 2151 | 2151 | 3506 | 2943 |
| strto | geometric mean (21 cases) | 6.05 | 1.00 | 1.00 | 0.97 | 1.73 |
| | `strtol/int` | 1213 | 305 | 305 | 239 | 205 |
| | `strtod/short` | 1455 | 630 | 630 | 817 | 1042 |
| | `strtod/17-digits` | 5248 | 1241 | 1241 | 1237 | 3139 |
| malloc | geometric mean (9 cases) | 1.07 | 1.00 | 1.00 | 7.24 | 0.68 |
| | `fixed/16` | 92.0 | 92.0 | 92.0 | 76.0 | 40.5 |
| | `batch/shuffled` | 111 | 111 | 111 | 335 | 63.7 |
| | `realloc/grow` | 99.1 | 99.1 | 99.1 | 28882 | 325 |
| qsort | geometric mean (8 cases) | 3.67 | 1.00 | 1.00 | 1.38 | 1.48 |
| | `qsort/random/1024` | 640 | 274 | 274 | 278 | 277 |
| math | geometric mean (132 cases) | 11.86 | 5.94 | 1.00 | 3.69 | 3.85 |
| | `sqrtf/throughput` | 468 | 468 | 468 | 369 | 545 |
| | `expf/throughput` | 5431 | 125 | 119 | 1247 | 1337 |
| | `logf/throughput` | 6185 | 1026 | 253 | 1322 | 1376 |
| | `sinf/throughput` | 9154 | 842 | 379 | 1279 | 1276 |
| | `powf/throughput` | 11060 | 374 | 368 | 4514 | 4623 |
| | `erff/throughput` | 66112 | 1122 | 184 | 3151 | 3145 |
| | `exp/throughput` | 3874 | 3874 | 316 | 2090 | 2199 |
| | `log/throughput` | 6405 | 6405 | 336 | 2212 | 2290 |
| | `sin/throughput` | 6524 | 770 | 755 | 1766 | 1763 |
| | `pow/throughput` | 11833 | 11833 | 923 | 7040 | 7208 |

#### armv7e-m

Instructions per operation; the geometric mean is over every case all 5 builds share, as a ratio to rtoskit's fast flavour.

| Program | Case | rtoskit (size) | rtoskit (speed) | rtoskit (fast) | picolibc | newlib-nano |
|---|---|---:|---:|---:|---:|---:|
| string | geometric mean (603 cases) | 1.51 | 1.00 | 1.00 | 1.48 | 1.18 |
| | `memcpy/16/0:0` | 53.0 | 42.0 | 42.0 | 87.0 | 41.0 |
| | `memcpy/1024/0:0` | 1313 | 174 | 174 | 4119 | 575 |
| | `memset/1024/0` | 1054 | 177 | 177 | 4117 | 366 |
| | `strlen/64/0` | 115 | 107 | 107 | 215 | 104 |
| | `strcmp/64/0:0` | 540 | 240 | 240 | 410 | 127 |
| stdio | geometric mean (23 cases) | 1.22 | 1.00 | 1.00 | 1.16 | 0.94 |
| | `snprintf/%d` | 475 | 455 | 455 | 893 | 506 |
| | `snprintf/%s` | 423 | 395 | 395 | 543 | 534 |
| | `snprintf/%f` | 14734 | 14652 | 14652 | 3993 | 3266 |
| | `snprintf/%g` | 15538 | 15453 | 15453 | 3646 | 3382 |
| | `fprintf/mix` | 2326 | 2151 | 2151 | 3506 | 2938 |
| strto | geometric mean (21 cases) | 6.17 | 1.00 | 1.00 | 1.00 | 1.77 |
| | `strtol/int` | 1213 | 305 | 305 | 239 | 204 |
| | `strtod/short` | 1384 | 559 | 559 | 821 | 1044 |
| | `strtod/17-digits` | 5241 | 1242 | 1242 | 1241 | 3130 |
| malloc | geometric mean (9 cases) | 1.07 | 1.00 | 1.00 | 7.21 | 0.68 |
| | `fixed/16` | 92.5 | 92.5 | 92.5 | 76.0 | 40.5 |
| | `batch/shuffled` | 111 | 111 | 111 | 335 | 63.7 |
| | `realloc/grow` | 99.2 | 99.2 | 99.2 | 28882 | 315 |
| qsort | geometric mean (8 cases) | 3.67 | 1.00 | 1.00 | 1.38 | 1.49 |
| | `qsort/random/1024` | 640 | 274 | 274 | 278 | 277 |
| math | geometric mean (132 cases) | 4.32 | 3.81 | 1.00 | 1.75 | 1.86 |
| | `sqrtf/throughput` | 10.3 | 10.3 | 10.3 | 10.3 | 23.3 |
| | `expf/throughput` | 160 | 160 | 58.3 | 75.9 | 94.9 |
| | `logf/throughput` | 165 | 165 | 74.3 | 84.5 | 85.6 |
| | `sinf/throughput` | 575 | 575 | 128 | 101 | 103 |
| | `powf/throughput` | 367 | 367 | 215 | 234 | 269 |
| | `erff/throughput` | 1383 | 1383 | 123 | 141 | 141 |
| | `exp/throughput` | 3813 | 3813 | 316 | 2085 | 2200 |
| | `log/throughput` | 6184 | 6184 | 334 | 2207 | 2285 |
| | `sin/throughput` | 6435 | 765 | 751 | 1767 | 1763 |
| | `pow/throughput` | 11531 | 11531 | 923 | 7037 | 7205 |

#### armv7e-m-dp

Instructions per operation; the geometric mean is over every case all 5 builds share, as a ratio to rtoskit's fast flavour.

| Program | Case | rtoskit (size) | rtoskit (speed) | rtoskit (fast) | picolibc | newlib-nano |
|---|---|---:|---:|---:|---:|---:|
| string | geometric mean (603 cases) | 1.51 | 1.00 | 1.00 | 1.48 | 1.18 |
| | `memcpy/16/0:0` | 53.0 | 42.0 | 42.0 | 87.0 | 41.0 |
| | `memcpy/1024/0:0` | 1313 | 174 | 174 | 4119 | 575 |
| | `memset/1024/0` | 1054 | 177 | 177 | 4117 | 366 |
| | `strlen/64/0` | 115 | 107 | 107 | 215 | 104 |
| | `strcmp/64/0:0` | 540 | 240 | 240 | 410 | 127 |
| stdio | geometric mean (23 cases) | 1.22 | 1.00 | 1.00 | 1.16 | 0.74 |
| | `snprintf/%d` | 475 | 455 | 455 | 893 | 506 |
| | `snprintf/%s` | 423 | 395 | 395 | 543 | 534 |
| | `snprintf/%f` | 14737 | 14655 | 14655 | 3993 | 1057 |
| | `snprintf/%g` | 15541 | 15456 | 15456 | 3646 | 1189 |
| | `fprintf/mix` | 2326 | 2151 | 2151 | 3506 | 2938 |
| strto | geometric mean (21 cases) | 6.55 | 1.00 | 1.00 | 0.96 | 1.24 |
| | `strtol/int` | 1213 | 305 | 305 | 239 | 204 |
| | `strtod/short` | 1151 | 326 | 326 | 618 | 233 |
| | `strtod/17-digits` | 5179 | 1180 | 1180 | 1038 | 2175 |
| malloc | geometric mean (9 cases) | 1.07 | 1.00 | 1.00 | 7.21 | 0.68 |
| | `fixed/16` | 92.5 | 92.5 | 92.5 | 76.0 | 40.5 |
| | `batch/shuffled` | 111 | 111 | 111 | 335 | 63.7 |
| | `realloc/grow` | 99.2 | 99.2 | 99.2 | 28882 | 315 |
| qsort | geometric mean (8 cases) | 3.67 | 1.00 | 1.00 | 1.38 | 1.49 |
| | `qsort/random/1024` | 640 | 274 | 274 | 278 | 277 |
| math | geometric mean (132 cases) | 2.12 | 2.12 | 1.00 | 1.15 | 1.01 |
| | `sqrtf/throughput` | 10.3 | 10.3 | 10.3 | 10.3 | 23.3 |
| | `expf/throughput` | 172 | 172 | 52.3 | 75.9 | 43.3 |
| | `logf/throughput` | 162 | 162 | 81.3 | 80.5 | 44.3 |
| | `sinf/throughput` | 187 | 187 | 57.8 | 101 | 49.5 |
| | `powf/throughput` | 398 | 398 | 159 | 232 | 89.4 |
| | `erff/throughput` | 421 | 421 | 113 | 141 | 121 |
| | `exp/throughput` | 169 | 169 | 52.6 | 69.8 | 56.6 |
| | `log/throughput` | 150 | 150 | 67.6 | 79.8 | 64.6 |
| | `sin/throughput` | 165 | 165 | 96.1 | 102 | 106 |
| | `pow/throughput` | 393 | 393 | 133 | 227 | 134 |

#### armv8-m.main

Instructions per operation; the geometric mean is over every case all 5 builds share, as a ratio to rtoskit's fast flavour.

| Program | Case | rtoskit (size) | rtoskit (speed) | rtoskit (fast) | picolibc | newlib-nano |
|---|---|---:|---:|---:|---:|---:|
| string | geometric mean (603 cases) | 1.51 | 1.00 | 1.00 | 1.57 | 1.25 |
| | `memcpy/16/0:0` | 53.0 | 42.0 | 42.0 | 87.0 | 43.0 |
| | `memcpy/1024/0:0` | 1313 | 174 | 174 | 4119 | 607 |
| | `memset/1024/0` | 1054 | 177 | 177 | 4117 | 366 |
| | `strlen/64/0` | 115 | 107 | 107 | 215 | 154 |
| | `strcmp/64/0:0` | 540 | 240 | 240 | 410 | 176 |
| stdio | geometric mean (23 cases) | 1.21 | 1.00 | 1.00 | 1.15 | 0.95 |
| | `snprintf/%d` | 475 | 455 | 455 | 893 | 520 |
| | `snprintf/%s` | 423 | 395 | 395 | 543 | 535 |
| | `snprintf/%f` | 14733 | 14651 | 14651 | 3994 | 3276 |
| | `snprintf/%g` | 15537 | 15452 | 15452 | 3643 | 3393 |
| | `fprintf/mix` | 2323 | 2148 | 2148 | 3506 | 2946 |
| strto | geometric mean (21 cases) | 6.18 | 1.00 | 1.00 | 1.00 | 1.78 |
| | `strtol/int` | 1213 | 305 | 305 | 239 | 211 |
| | `strtod/short` | 1384 | 559 | 559 | 821 | 1040 |
| | `strtod/17-digits` | 5261 | 1241 | 1241 | 1241 | 3134 |
| malloc | geometric mean (9 cases) | 1.07 | 1.00 | 1.00 | 7.35 | 0.68 |
| | `fixed/16` | 90.5 | 90.5 | 90.5 | 76.0 | 40.5 |
| | `batch/shuffled` | 109 | 109 | 109 | 335 | 60.7 |
| | `realloc/grow` | 97.4 | 97.4 | 97.4 | 28882 | 324 |
| qsort | geometric mean (8 cases) | 3.67 | 1.00 | 1.00 | 1.38 | 1.49 |
| | `qsort/random/1024` | 640 | 274 | 274 | 278 | 277 |
| math | geometric mean (132 cases) | 4.33 | 3.81 | 1.00 | 1.75 | 1.86 |
| | `sqrtf/throughput` | 10.3 | 10.3 | 10.3 | 10.3 | 23.3 |
| | `expf/throughput` | 160 | 160 | 58.3 | 75.9 | 93.9 |
| | `logf/throughput` | 165 | 165 | 74.3 | 83.5 | 87.6 |
| | `sinf/throughput` | 574 | 574 | 126 | 101 | 102 |
| | `powf/throughput` | 353 | 353 | 203 | 232 | 273 |
| | `erff/throughput` | 1383 | 1383 | 123 | 141 | 139 |
| | `exp/throughput` | 3813 | 3813 | 316 | 2085 | 2201 |
| | `log/throughput` | 6184 | 6184 | 334 | 2207 | 2285 |
| | `sin/throughput` | 6435 | 754 | 740 | 1767 | 1763 |
| | `pow/throughput` | 11532 | 11532 | 923 | 7037 | 7207 |

## Against Arm Optimized Routines

#### x86_64

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 0 of 603 | — | — |
| stdio | 0 of 23 | — | — |
| strto | 0 of 21 | — | — |
| malloc | 0 of 9 | — | — |
| qsort | 0 of 8 | — | — |
| math | 34 of 160 | 1.29 | `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `pow`, `powf`, `sincosf`, `sinf` |
| softfloat | 0 of 20 | — | — |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `expf` | 237 | 237 | 47.7 | 43.7 | 0.92 |
| `exp2f` | 302 | 302 | 46.7 | 42.7 | 0.91 |
| `logf` | 267 | 267 | 68.7 | 49.7 | 0.72 |
| `log2f` | 337 | 337 | 70.7 | 49.7 | 0.70 |
| `log10f` | 303 | 303 | 71.7 | 52.7 | 0.73 |
| `powf` | 633 | 633 | 181 | 88.6 | 0.49 |
| `sinf` | 337 | 337 | 61.8 | 52.6 | 0.85 |
| `cosf` | 328 | 328 | 57.1 | 52.7 | 0.92 |
| `sincosf` | 493 | 493 | 112 | 82.6 | 0.74 |
| `erff` | 511 | 511 | 108 | 2171 | 20.01 |
| `exp` | 230 | 230 | 50.7 | 55.7 | 1.10 |
| `exp2` | 287 | 287 | 46.7 | 51.7 | 1.11 |
| `exp10` | 277 | 277 | 51.7 | 56.7 | 1.10 |
| `log` | 252 | 252 | 76.7 | 69.7 | 0.91 |
| `log2` | 330 | 330 | 87.7 | 81.7 | 0.93 |
| `pow` | 625 | 625 | 204 | 165 | 0.81 |
| `erf` | 504 | 504 | 98.0 | 4400 | 44.90 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 7.6 | 7.6 | 7.6 | 7.6 | 1.00 |
| `fsub` | 7.6 | 7.6 | 7.6 | 7.6 | 1.00 |
| `fmul` | 7.6 | 7.6 | 7.6 | 7.6 | 1.00 |
| `fdiv` | 7.6 | 7.6 | 7.6 | 7.6 | 1.00 |
| `fcmp` | 8.6 | 8.6 | 8.6 | 8.6 | 1.00 |
| `dadd` | 10.6 | 10.6 | 10.6 | 10.6 | 1.00 |
| `dsub` | 10.6 | 10.6 | 10.6 | 10.6 | 1.00 |
| `dmul` | 10.6 | 10.6 | 10.6 | 10.6 | 1.00 |
| `ddiv` | 10.6 | 10.6 | 10.6 | 10.6 | 1.00 |
| `dcmp` | 8.6 | 8.6 | 8.6 | 8.6 | 1.00 |
| `f2d` | 9.6 | 9.6 | 9.6 | 9.6 | 1.00 |
| `d2f` | 6.6 | 6.6 | 6.6 | 6.6 | 1.00 |
| `f2i` | 9.6 | 9.6 | 9.6 | 9.6 | 1.00 |
| `i2f` | 6.6 | 6.6 | 6.6 | 6.6 | 1.00 |
| `d2i` | 9.6 | 9.6 | 9.6 | 9.6 | 1.00 |
| `i2d` | 9.6 | 9.6 | 9.6 | 9.6 | 1.00 |
| `f2l` | 8.6 | 8.6 | 8.6 | 8.6 | 1.00 |
| `l2f` | 6.6 | 6.6 | 6.6 | 6.6 | 1.00 |
| `d2l` | 8.6 | 8.6 | 8.6 | 8.6 | 1.00 |
| `l2d` | 9.6 | 9.6 | 9.6 | 9.6 | 1.00 |

#### armv6-m

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 60 of 603 | 0.96 | `strcmp` |
| stdio | 0 of 23 | — | `__aeabi_fmul`, `strcmp` |
| strto | 3 of 21 | 0.97 | `__aeabi_fmul`, `strcmp` |
| malloc | 0 of 9 | — | `strcmp` |
| qsort | 1 of 8 | 1.06 | `strcmp` |
| math | 120 of 160 | 1.41 | `__aeabi_cfcmpeq`, `__aeabi_cfcmple`, `__aeabi_cfrcmple`, `__aeabi_dcmpeq`, `__aeabi_dcmpge`, `__aeabi_dcmpgt`, `__aeabi_dcmple`, `__aeabi_dcmplt`, `__aeabi_dcmpun`, `__aeabi_fadd`, `__aeabi_fcmpeq`, `__aeabi_fcmpge`, `__aeabi_fcmpgt`, `__aeabi_fcmple`, `__aeabi_fcmplt`, `__aeabi_fcmpun`, `__aeabi_fmul`, `__aeabi_frsub`, `__aeabi_fsub`, `__gesf2`, `__lesf2`, `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `pow`, `powf`, `sincosf`, `sinf`, `strcmp` |
| softfloat | 5 of 20 | 0.51 | `__aeabi_dcmplt`, `__aeabi_fadd`, `__aeabi_fcmplt`, `__aeabi_fmul`, `__aeabi_frsub`, `__aeabi_fsub`, `strcmp` |

##### string

Aligned case per size: size flavour / speed flavour / fast flavour / AOR.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `strcmp` | 29 / 30 / 30 / 39 | 37 / 37 / 37 / 45 | 61 / 58 / 58 / 47 | 157 / 142 / 142 / 71 | 541 / 478 / 478 / 167 | 2077 / 1822 / 1822 / 551 | 8221 / 7198 / 7198 / 2087 |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `sqrtf` | 461 | 461 | 461 | 429 | 0.93 |
| `cbrtf` | 8590 | 8590 | 630 | 596 | 0.95 |
| `hypotf` | 6293 | 6293 | 6293 | 3747 | 0.60 |
| `expf` | 11034 | 375 | 368 | 2452 | 6.66 |
| `exp2f` | 9269 | 316 | 310 | 2212 | 7.13 |
| `exp10f` | 11103 | 11103 | 367 | 337 | 0.92 |
| `expm1f` | 12310 | 12310 | 461 | 427 | 0.93 |
| `exp2m1f` | 10068 | 10068 | 432 | 398 | 0.92 |
| `exp10m1f` | 12406 | 12406 | 466 | 436 | 0.94 |
| `logf` | 12367 | 2210 | 670 | 2426 | 3.62 |
| `log2f` | 10547 | 583 | 577 | 2447 | 4.24 |
| `log10f` | 12365 | 678 | 670 | 2686 | 4.01 |
| `log1pf` | 12789 | 12789 | 699 | 670 | 0.96 |
| `log2p1f` | 11003 | 11003 | 608 | 582 | 0.96 |
| `log10p1f` | 13202 | 13202 | 699 | 671 | 0.96 |
| `powf` | 22158 | 1222 | 1216 | 5462 | 4.49 |
| `sinf` | 18350 | 2933 | 1090 | 3059 | 2.81 |
| `cosf` | 18281 | 2887 | 1100 | 3077 | 2.80 |
| `sincosf` | 33645 | 4418 | 1696 | 5156 | 3.04 |
| `tanf` | 36223 | 5065 | 2178 | 2144 | 0.98 |
| `sinpif` | 17579 | 17579 | 920 | 848 | 0.92 |
| `cospif` | 17172 | 17172 | 900 | 834 | 0.93 |
| `tanpif` | 35613 | 35613 | 2015 | 1936 | 0.96 |
| `asinf` | 22944 | 22944 | 730 | 694 | 0.95 |
| `acosf` | 23761 | 23761 | 749 | 718 | 0.96 |
| `atanf` | 17683 | 17683 | 751 | 716 | 0.95 |
| `atan2f` | 18355 | 18355 | 812 | 777 | 0.96 |
| `sinhf` | 15310 | 15310 | 627 | 590 | 0.94 |
| `coshf` | 15181 | 15181 | 622 | 589 | 0.95 |
| `tanhf` | 15977 | 15977 | 742 | 708 | 0.95 |
| `asinhf` | 19353 | 19353 | 1152 | 1115 | 0.97 |
| `acoshf` | 19926 | 19926 | 1182 | 1150 | 0.97 |
| `atanhf` | 15868 | 15868 | 877 | 842 | 0.96 |
| `erff` | 141024 | 2967 | 420 | 10570 | 25.17 |
| `erfcf` | 119604 | 2874 | 2874 | 2770 | 0.96 |
| `lgammaf` | 36729 | 36729 | 36729 | 20889 | 0.57 |
| `tgammaf` | 55326 | 55326 | 55326 | 31220 | 0.56 |
| `fmodf` | 273 | 273 | 273 | 230 | 0.84 |
| `floorf` | 105 | 105 | 105 | 70.7 | 0.67 |
| `roundf` | 103 | 103 | 103 | 68.7 | 0.66 |
| `sqrt` | 4169 | 4169 | 4169 | 4043 | 0.97 |
| `hypot` | 14773 | 14773 | 14773 | 14375 | 0.97 |
| `exp` | 10317 | 10317 | 1410 | 3923 | 2.78 |
| `exp2` | 14133 | 14133 | 1210 | 3304 | 2.73 |
| `exp10` | 14233 | 14233 | 1412 | 3916 | 2.77 |
| `exp2m1` | 14606 | 14606 | 14606 | 14387 | 0.99 |
| `exp10m1` | 14813 | 14813 | 14813 | 14602 | 0.99 |
| `log` | 16964 | 16964 | 1454 | 4303 | 2.96 |
| `log2` | 21420 | 21420 | 1535 | 5180 | 3.37 |
| `log2p1` | 22150 | 22150 | 22150 | 21952 | 0.99 |
| `log10p1` | 22163 | 22163 | 22163 | 21966 | 0.99 |
| `pow` | 31522 | 31522 | 3938 | 12146 | 3.08 |
| `sinpi` | 16829 | 16829 | 16829 | 16622 | 0.99 |
| `cospi` | 16792 | 16792 | 16792 | 16609 | 0.99 |
| `tanpi` | 34136 | 34136 | 34136 | 33931 | 0.99 |
| `erf` | 40655 | 40655 | 2049 | 25459 | 12.43 |
| `erfc` | 112956 | 112956 | 112956 | 112647 | 1.00 |
| `lgamma` | 47122 | 47083 | 47083 | 46671 | 0.99 |
| `tgamma` | 64747 | 64700 | 64700 | 64193 | 0.99 |
| `fmod` | 446 | 446 | 446 | 432 | 0.97 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 90.3 | 90.3 | 90.3 | 50.6 | 0.56 |
| `fsub` | 84.8 | 84.8 | 84.8 | 51.6 | 0.61 |
| `fmul` | 109 | 109 | 109 | 56.3 | 0.52 |
| `fdiv` | 292 | 292 | 292 | 292 | 1.00 |
| `fcmp` | 44.4 | 44.4 | 44.4 | 24.2 | 0.54 |
| `dadd` | 107 | 107 | 107 | 107 | 1.00 |
| `dsub` | 116 | 116 | 116 | 116 | 1.00 |
| `dmul` | 264 | 264 | 264 | 264 | 1.00 |
| `ddiv` | 844 | 844 | 844 | 844 | 1.00 |
| `dcmp` | 66.9 | 66.9 | 66.9 | 24.2 | 0.36 |
| `f2d` | 32.4 | 32.4 | 32.4 | 32.4 | 1.00 |
| `d2f` | 51.5 | 51.5 | 51.5 | 51.5 | 1.00 |
| `f2i` | 24.9 | 24.9 | 24.9 | 24.9 | 1.00 |
| `i2f` | 76.6 | 76.6 | 76.6 | 76.6 | 1.00 |
| `d2i` | 38.4 | 38.4 | 38.4 | 38.4 | 1.00 |
| `i2d` | 61.6 | 61.6 | 61.6 | 61.6 | 1.00 |
| `f2l` | 45.4 | 45.4 | 45.4 | 45.4 | 1.00 |
| `l2f` | 525 | 525 | 525 | 525 | 1.00 |
| `d2l` | 62.5 | 62.5 | 62.5 | 62.5 | 1.00 |
| `l2d` | 473 | 473 | 473 | 473 | 1.00 |

#### armv6s-m

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 60 of 603 | 0.96 | `strcmp` |
| stdio | 0 of 23 | — | `__aeabi_fmul`, `strcmp` |
| strto | 3 of 21 | 0.97 | `__aeabi_fmul`, `strcmp` |
| malloc | 0 of 9 | — | `strcmp` |
| qsort | 1 of 8 | 1.06 | `strcmp` |
| math | 120 of 160 | 1.41 | `__aeabi_cfcmpeq`, `__aeabi_cfcmple`, `__aeabi_cfrcmple`, `__aeabi_dcmpeq`, `__aeabi_dcmpge`, `__aeabi_dcmpgt`, `__aeabi_dcmple`, `__aeabi_dcmplt`, `__aeabi_dcmpun`, `__aeabi_fadd`, `__aeabi_fcmpeq`, `__aeabi_fcmpge`, `__aeabi_fcmpgt`, `__aeabi_fcmple`, `__aeabi_fcmplt`, `__aeabi_fcmpun`, `__aeabi_fmul`, `__aeabi_frsub`, `__aeabi_fsub`, `__gesf2`, `__lesf2`, `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `pow`, `powf`, `sincosf`, `sinf`, `strcmp` |
| softfloat | 5 of 20 | 0.51 | `__aeabi_dcmplt`, `__aeabi_fadd`, `__aeabi_fcmplt`, `__aeabi_fmul`, `__aeabi_frsub`, `__aeabi_fsub`, `strcmp` |

##### string

Aligned case per size: size flavour / speed flavour / fast flavour / AOR.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `strcmp` | 29 / 30 / 30 / 39 | 37 / 37 / 37 / 45 | 61 / 58 / 58 / 47 | 157 / 142 / 142 / 71 | 541 / 478 / 478 / 167 | 2077 / 1822 / 1822 / 551 | 8221 / 7198 / 7198 / 2087 |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `sqrtf` | 461 | 461 | 461 | 429 | 0.93 |
| `cbrtf` | 8590 | 8590 | 630 | 596 | 0.95 |
| `hypotf` | 6293 | 6293 | 6293 | 3747 | 0.60 |
| `expf` | 11034 | 375 | 368 | 2452 | 6.66 |
| `exp2f` | 9269 | 316 | 310 | 2212 | 7.13 |
| `exp10f` | 11103 | 11103 | 367 | 337 | 0.92 |
| `expm1f` | 12310 | 12310 | 461 | 427 | 0.93 |
| `exp2m1f` | 10068 | 10068 | 432 | 398 | 0.92 |
| `exp10m1f` | 12406 | 12406 | 466 | 436 | 0.94 |
| `logf` | 12367 | 2210 | 670 | 2426 | 3.62 |
| `log2f` | 10547 | 583 | 577 | 2447 | 4.24 |
| `log10f` | 12365 | 678 | 670 | 2686 | 4.01 |
| `log1pf` | 12789 | 12789 | 699 | 670 | 0.96 |
| `log2p1f` | 11003 | 11003 | 608 | 582 | 0.96 |
| `log10p1f` | 13202 | 13202 | 699 | 671 | 0.96 |
| `powf` | 22158 | 1222 | 1216 | 5462 | 4.49 |
| `sinf` | 18350 | 2933 | 1090 | 3059 | 2.81 |
| `cosf` | 18281 | 2887 | 1100 | 3077 | 2.80 |
| `sincosf` | 33645 | 4418 | 1696 | 5156 | 3.04 |
| `tanf` | 36223 | 5065 | 2178 | 2144 | 0.98 |
| `sinpif` | 17579 | 17579 | 920 | 848 | 0.92 |
| `cospif` | 17172 | 17172 | 900 | 834 | 0.93 |
| `tanpif` | 35613 | 35613 | 2015 | 1936 | 0.96 |
| `asinf` | 22944 | 22944 | 730 | 694 | 0.95 |
| `acosf` | 23761 | 23761 | 749 | 718 | 0.96 |
| `atanf` | 17683 | 17683 | 751 | 716 | 0.95 |
| `atan2f` | 18355 | 18355 | 812 | 777 | 0.96 |
| `sinhf` | 15310 | 15310 | 627 | 590 | 0.94 |
| `coshf` | 15181 | 15181 | 622 | 589 | 0.95 |
| `tanhf` | 15977 | 15977 | 742 | 708 | 0.95 |
| `asinhf` | 19353 | 19353 | 1152 | 1115 | 0.97 |
| `acoshf` | 19926 | 19926 | 1182 | 1150 | 0.97 |
| `atanhf` | 15868 | 15868 | 877 | 842 | 0.96 |
| `erff` | 141024 | 2967 | 420 | 10570 | 25.17 |
| `erfcf` | 119604 | 2874 | 2874 | 2770 | 0.96 |
| `lgammaf` | 36729 | 36729 | 36729 | 20889 | 0.57 |
| `tgammaf` | 55326 | 55326 | 55326 | 31220 | 0.56 |
| `fmodf` | 273 | 273 | 273 | 230 | 0.84 |
| `floorf` | 105 | 105 | 105 | 70.7 | 0.67 |
| `roundf` | 103 | 103 | 103 | 68.7 | 0.66 |
| `sqrt` | 4169 | 4169 | 4169 | 4043 | 0.97 |
| `hypot` | 14773 | 14773 | 14773 | 14375 | 0.97 |
| `exp` | 10317 | 10317 | 1410 | 3923 | 2.78 |
| `exp2` | 14133 | 14133 | 1210 | 3304 | 2.73 |
| `exp10` | 14233 | 14233 | 1412 | 3916 | 2.77 |
| `exp2m1` | 14606 | 14606 | 14606 | 14387 | 0.99 |
| `exp10m1` | 14813 | 14813 | 14813 | 14602 | 0.99 |
| `log` | 16964 | 16964 | 1454 | 4303 | 2.96 |
| `log2` | 21420 | 21420 | 1535 | 5180 | 3.37 |
| `log2p1` | 22150 | 22150 | 22150 | 21952 | 0.99 |
| `log10p1` | 22163 | 22163 | 22163 | 21966 | 0.99 |
| `pow` | 31522 | 31522 | 3938 | 12146 | 3.08 |
| `sinpi` | 16829 | 16829 | 16829 | 16622 | 0.99 |
| `cospi` | 16792 | 16792 | 16792 | 16609 | 0.99 |
| `tanpi` | 34136 | 34136 | 34136 | 33931 | 0.99 |
| `erf` | 40655 | 40655 | 2049 | 25459 | 12.43 |
| `erfc` | 112956 | 112956 | 112956 | 112647 | 1.00 |
| `lgamma` | 47122 | 47083 | 47083 | 46671 | 0.99 |
| `tgamma` | 64747 | 64700 | 64700 | 64193 | 0.99 |
| `fmod` | 446 | 446 | 446 | 432 | 0.97 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 90.3 | 90.3 | 90.3 | 50.6 | 0.56 |
| `fsub` | 84.8 | 84.8 | 84.8 | 51.6 | 0.61 |
| `fmul` | 109 | 109 | 109 | 56.3 | 0.52 |
| `fdiv` | 292 | 292 | 292 | 292 | 1.00 |
| `fcmp` | 44.4 | 44.4 | 44.4 | 24.2 | 0.54 |
| `dadd` | 107 | 107 | 107 | 107 | 1.00 |
| `dsub` | 116 | 116 | 116 | 116 | 1.00 |
| `dmul` | 264 | 264 | 264 | 264 | 1.00 |
| `ddiv` | 844 | 844 | 844 | 844 | 1.00 |
| `dcmp` | 66.9 | 66.9 | 66.9 | 24.2 | 0.36 |
| `f2d` | 32.4 | 32.4 | 32.4 | 32.4 | 1.00 |
| `d2f` | 51.5 | 51.5 | 51.5 | 51.5 | 1.00 |
| `f2i` | 24.9 | 24.9 | 24.9 | 24.9 | 1.00 |
| `i2f` | 76.6 | 76.6 | 76.6 | 76.6 | 1.00 |
| `d2i` | 38.4 | 38.4 | 38.4 | 38.4 | 1.00 |
| `i2d` | 61.6 | 61.6 | 61.6 | 61.6 | 1.00 |
| `f2l` | 45.4 | 45.4 | 45.4 | 45.4 | 1.00 |
| `l2f` | 525 | 525 | 525 | 525 | 1.00 |
| `d2l` | 62.5 | 62.5 | 62.5 | 62.5 | 1.00 |
| `l2d` | 473 | 473 | 473 | 473 | 1.00 |

#### armv7-m

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 0 of 603 | — | — |
| stdio | 2 of 23 | 0.95 | `__aeabi_d2uiz`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_fdiv`, `__aeabi_fmul`, `__aeabi_ui2f`, `__aeabi_ul2d` |
| strto | 14 of 21 | 0.94 | `__aeabi_d2lz`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_f2iz`, `__aeabi_f2uiz`, `__aeabi_fdiv`, `__aeabi_fmul`, `__aeabi_ui2f`, `__aeabi_ul2d` |
| malloc | 0 of 9 | — | — |
| qsort | 0 of 8 | — | — |
| math | 160 of 160 | 1.32 | `__aeabi_d2f`, `__aeabi_d2iz`, `__aeabi_d2lz`, `__aeabi_dadd`, `__aeabi_dcmpeq`, `__aeabi_dcmpge`, `__aeabi_dcmpgt`, `__aeabi_dcmple`, `__aeabi_dcmplt`, `__aeabi_dcmpun`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_drsub`, `__aeabi_dsub`, `__aeabi_f2d`, `__aeabi_f2iz`, `__aeabi_fadd`, `__aeabi_fcmpeq`, `__aeabi_fcmpge`, `__aeabi_fcmpgt`, `__aeabi_fcmple`, `__aeabi_fcmplt`, `__aeabi_fcmpun`, `__aeabi_fdiv`, `__aeabi_fmul`, `__aeabi_frsub`, `__aeabi_fsub`, `__aeabi_i2d`, `__aeabi_i2f`, `__aeabi_l2d`, `__aeabi_l2f`, `__aeabi_ui2d`, `__aeabi_ui2f`, `__aeabi_ul2d`, `__aeabi_ul2f`, `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `pow`, `powf`, `sincosf`, `sinf` |
| softfloat | 18 of 20 | 0.81 | `__aeabi_d2f`, `__aeabi_d2iz`, `__aeabi_d2lz`, `__aeabi_dadd`, `__aeabi_dcmplt`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_drsub`, `__aeabi_dsub`, `__aeabi_f2d`, `__aeabi_f2iz`, `__aeabi_f2lz`, `__aeabi_fadd`, `__aeabi_fcmplt`, `__aeabi_fdiv`, `__aeabi_fmul`, `__aeabi_frsub`, `__aeabi_fsub`, `__aeabi_i2d`, `__aeabi_i2f`, `__aeabi_l2d`, `__aeabi_l2f`, `__aeabi_ui2d`, `__aeabi_ul2f` |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `sqrtf` | 468 | 468 | 468 | 460 | 0.98 |
| `cbrtf` | 3545 | 3545 | 162 | 150 | 0.93 |
| `hypotf` | 3336 | 3336 | 3336 | 2942 | 0.88 |
| `expf` | 5431 | 125 | 119 | 748 | 6.28 |
| `exp2f` | 4511 | 118 | 112 | 693 | 6.21 |
| `exp10f` | 5461 | 5461 | 117 | 105 | 0.89 |
| `expm1f` | 6065 | 6065 | 151 | 141 | 0.93 |
| `exp2m1f` | 4979 | 4979 | 151 | 139 | 0.92 |
| `exp10m1f` | 6135 | 6135 | 153 | 142 | 0.93 |
| `logf` | 6185 | 1026 | 253 | 716 | 2.82 |
| `log2f` | 5322 | 247 | 240 | 721 | 3.00 |
| `log10f` | 6181 | 259 | 253 | 772 | 3.04 |
| `log1pf` | 6386 | 6386 | 254 | 244 | 0.96 |
| `log2p1f` | 5545 | 5545 | 242 | 234 | 0.97 |
| `log10p1f` | 6619 | 6619 | 255 | 246 | 0.96 |
| `powf` | 11060 | 374 | 368 | 1596 | 4.34 |
| `sinf` | 9154 | 842 | 379 | 841 | 2.22 |
| `cosf` | 9093 | 810 | 379 | 843 | 2.22 |
| `sincosf` | 16796 | 1160 | 654 | 1405 | 2.15 |
| `tanf` | 17903 | 1083 | 568 | 557 | 0.98 |
| `sinpif` | 8812 | 8812 | 369 | 333 | 0.90 |
| `cospif` | 8578 | 8578 | 347 | 319 | 0.92 |
| `tanpif` | 17671 | 17671 | 563 | 527 | 0.94 |
| `asinf` | 11310 | 11310 | 239 | 230 | 0.96 |
| `acosf` | 11794 | 11794 | 244 | 236 | 0.97 |
| `atanf` | 8608 | 8608 | 245 | 234 | 0.96 |
| `atan2f` | 8995 | 8995 | 261 | 250 | 0.96 |
| `sinhf` | 7396 | 7396 | 171 | 158 | 0.92 |
| `coshf` | 7302 | 7302 | 162 | 153 | 0.95 |
| `tanhf` | 7763 | 7763 | 203 | 191 | 0.94 |
| `asinhf` | 9783 | 9783 | 352 | 340 | 0.97 |
| `acoshf` | 10152 | 10152 | 356 | 347 | 0.98 |
| `atanhf` | 7721 | 7721 | 266 | 256 | 0.96 |
| `erff` | 66112 | 1122 | 184 | 4248 | 23.10 |
| `erfcf` | 54334 | 1068 | 1068 | 1032 | 0.97 |
| `lgammaf` | 18357 | 18357 | 18357 | 15060 | 0.82 |
| `tgammaf` | 27683 | 27683 | 27683 | 22480 | 0.81 |
| `fmodf` | 225 | 225 | 225 | 208 | 0.93 |
| `floorf` | 75.2 | 75.2 | 75.2 | 62.0 | 0.82 |
| `roundf` | 69.2 | 69.2 | 69.2 | 56.0 | 0.81 |
| `sqrt` | 1972 | 1972 | 1972 | 1891 | 0.96 |
| `cbrt` | 4309 | 4309 | 299 | 289 | 0.97 |
| `hypot` | 5956 | 5956 | 5956 | 5394 | 0.91 |
| `exp` | 3874 | 3874 | 316 | 1156 | 3.65 |
| `exp2` | 5320 | 5320 | 281 | 989 | 3.52 |
| `exp10` | 5334 | 5334 | 316 | 1142 | 3.61 |
| `expm1` | 4128 | 4128 | 346 | 336 | 0.97 |
| `exp2m1` | 5488 | 5488 | 5488 | 4887 | 0.89 |
| `exp10m1` | 5620 | 5620 | 5620 | 4993 | 0.89 |
| `log` | 6405 | 6405 | 336 | 1307 | 3.89 |
| `log2` | 8007 | 8007 | 351 | 1589 | 4.53 |
| `log10` | 7881 | 7881 | 356 | 346 | 0.97 |
| `log1p` | 6961 | 6961 | 371 | 361 | 0.97 |
| `log2p1` | 8463 | 8463 | 8463 | 7270 | 0.86 |
| `log10p1` | 8462 | 8462 | 8462 | 7273 | 0.86 |
| `pow` | 11833 | 11833 | 923 | 3762 | 4.07 |
| `sin` | 6524 | 770 | 755 | 744 | 0.99 |
| `cos` | 6500 | 1074 | 760 | 750 | 0.99 |
| `sincos` | 10702 | 1400 | 1072 | 1048 | 0.98 |
| `tan` | 11535 | 1012 | 996 | 985 | 0.99 |
| `sinpi` | 6158 | 6158 | 6158 | 5506 | 0.89 |
| `cospi` | 6136 | 6136 | 6136 | 5490 | 0.89 |
| `tanpi` | 12168 | 12168 | 12168 | 10624 | 0.87 |
| `asin` | 13481 | 13481 | 665 | 657 | 0.99 |
| `acos` | 14076 | 14076 | 669 | 659 | 0.99 |
| `atan` | 8070 | 8070 | 529 | 520 | 0.98 |
| `atan2` | 8667 | 8667 | 528 | 520 | 0.98 |
| `sinh` | 6585 | 6585 | 511 | 501 | 0.98 |
| `cosh` | 6166 | 6166 | 496 | 487 | 0.98 |
| `tanh` | 7444 | 7444 | 502 | 493 | 0.98 |
| `asinh` | 15719 | 15719 | 533 | 523 | 0.98 |
| `acosh` | 13773 | 13773 | 499 | 488 | 0.98 |
| `atanh` | 9757 | 9757 | 524 | 514 | 0.98 |
| `erf` | 13408 | 13408 | 518 | 8364 | 16.16 |
| `erfc` | 40682 | 40682 | 40682 | 32519 | 0.80 |
| `lgamma` | 17908 | 17908 | 17908 | 15309 | 0.85 |
| `tgamma` | 24298 | 24298 | 24298 | 20737 | 0.85 |
| `fmod` | 291 | 291 | 291 | 260 | 0.90 |
| `floor` | 102 | 102 | 102 | 91.1 | 0.90 |
| `round` | 99.5 | 99.5 | 99.5 | 89.1 | 0.90 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 56.3 | 56.3 | 56.3 | 38.1 | 0.68 |
| `fsub` | 52.1 | 52.1 | 52.1 | 37.6 | 0.72 |
| `fmul` | 39.2 | 39.2 | 39.2 | 36.9 | 0.94 |
| `fdiv` | 57.7 | 57.7 | 57.7 | 58.9 | 1.02 |
| `fcmp` | 28.9 | 28.9 | 28.9 | 21.3 | 0.74 |
| `dadd` | 56.2 | 56.2 | 56.2 | 47.2 | 0.84 |
| `dsub` | 59.1 | 59.1 | 59.1 | 47.3 | 0.80 |
| `dmul` | 58.0 | 58.0 | 58.0 | 61.9 | 1.07 |
| `ddiv` | 244 | 244 | 244 | 85.9 | 0.35 |
| `dcmp` | 29.9 | 29.9 | 29.9 | 21.2 | 0.71 |
| `f2d` | 15.9 | 15.9 | 15.9 | 15.9 | 1.00 |
| `d2f` | 19.4 | 19.4 | 19.4 | 17.9 | 0.92 |
| `f2i` | 15.9 | 15.9 | 15.9 | 16.9 | 1.06 |
| `i2f` | 20.9 | 20.9 | 20.9 | 20.9 | 1.00 |
| `d2i` | 19.9 | 19.9 | 19.9 | 20.9 | 1.05 |
| `i2d` | 21.6 | 21.6 | 21.6 | 18.9 | 0.88 |
| `f2l` | 27.4 | 27.4 | 27.4 | 20.9 | 0.76 |
| `l2f` | 26.9 | 26.9 | 26.9 | 27.3 | 1.01 |
| `d2l` | 44.9 | 44.9 | 44.9 | 25.9 | 0.58 |
| `l2d` | 30.1 | 30.1 | 30.1 | 29.6 | 0.98 |

#### armv7e-m

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 237 of 603 | 1.08 | `memchr`, `memset`, `strlen` |
| stdio | 20 of 23 | 1.00 | `__aeabi_d2uiz`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_ul2d`, `memchr`, `memset`, `strlen` |
| strto | 11 of 21 | 0.95 | `__aeabi_d2lz`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_ul2d`, `memchr`, `memset` |
| malloc | 1 of 9 | 1.02 | `memchr`, `memset` |
| qsort | 0 of 8 | — | `memchr`, `memset` |
| math | 100 of 160 | 1.78 | `__aeabi_d2f`, `__aeabi_d2iz`, `__aeabi_d2lz`, `__aeabi_dadd`, `__aeabi_dcmpeq`, `__aeabi_dcmpge`, `__aeabi_dcmpgt`, `__aeabi_dcmple`, `__aeabi_dcmplt`, `__aeabi_dcmpun`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_drsub`, `__aeabi_dsub`, `__aeabi_f2d`, `__aeabi_i2d`, `__aeabi_l2d`, `__aeabi_ui2d`, `__aeabi_ul2d`, `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `memchr`, `memset`, `pow`, `powf`, `sincosf`, `sinf` |
| softfloat | 12 of 20 | 0.80 | `__aeabi_d2f`, `__aeabi_d2iz`, `__aeabi_d2lz`, `__aeabi_dadd`, `__aeabi_dcmplt`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_drsub`, `__aeabi_dsub`, `__aeabi_f2d`, `__aeabi_f2lz`, `__aeabi_i2d`, `__aeabi_l2d`, `__aeabi_l2f`, `__aeabi_ui2d`, `__aeabi_ul2f`, `memchr`, `memset` |

##### string

Aligned case per size: size flavour / speed flavour / fast flavour / AOR.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memset` | 30 / 18 / 18 / 19 | 33 / 35 / 35 / 29 | 34 / 35 / 35 / 38 | 46 / 39 / 39 / 40 | 94 / 57 / 57 / 49 | 286 / 81 / 81 / 85 | 1054 / 177 / 177 / 229 |
| `memchr` | 29 / 24 / 24 / 23 | 36 / 35 / 35 / 27 | 65 / 40 / 40 / 42 | 95 / 57 / 57 / 63 | 215 / 141 / 141 / 123 | 695 / 477 / 477 / 363 | 2615 / 1821 / 1821 / 1323 |
| `strlen` | 35 / 35 / 35 / 42 | 42 / 35 / 35 / 42 | 40 / 36 / 36 / 42 | 55 / 53 / 53 / 56 | 115 / 107 / 107 / 104 | 355 / 323 / 323 / 290 | 1315 / 1187 / 1187 / 1034 |
| `strcpy` | 65 / 62 / 62 / 69 | 76 / 62 / 62 / 69 | 75 / 65 / 65 / 71 | 105 / 90 / 90 / 93 | 225 / 156 / 156 / 153 | 705 / 396 / 396 / 363 | 2625 / 1356 / 1356 / 1203 |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `expf` | 160 | 160 | 58.3 | 681 | 11.68 |
| `exp2f` | 141 | 141 | 59.3 | 632 | 10.65 |
| `logf` | 165 | 165 | 74.3 | 649 | 8.73 |
| `log2f` | 154 | 154 | 80.3 | 654 | 8.14 |
| `log10f` | 165 | 165 | 83.3 | 698 | 8.38 |
| `powf` | 367 | 367 | 215 | 1478 | 6.86 |
| `sinf` | 575 | 575 | 128 | 756 | 5.89 |
| `cosf` | 567 | 567 | 123 | 757 | 6.14 |
| `sincosf` | 786 | 786 | 212 | 1248 | 5.88 |
| `erff` | 1383 | 1383 | 123 | 550 | 4.48 |
| `sqrt` | 1968 | 1968 | 1968 | 1884 | 0.96 |
| `cbrt` | 3765 | 3765 | 296 | 286 | 0.97 |
| `hypot` | 5835 | 5835 | 5835 | 5289 | 0.91 |
| `exp` | 3813 | 3813 | 316 | 1090 | 3.45 |
| `exp2` | 5235 | 5235 | 280 | 934 | 3.34 |
| `exp10` | 5252 | 5252 | 315 | 1075 | 3.41 |
| `expm1` | 4075 | 4075 | 346 | 335 | 0.97 |
| `exp2m1` | 5408 | 5408 | 5408 | 4720 | 0.87 |
| `exp10m1` | 5541 | 5541 | 5541 | 4825 | 0.87 |
| `log` | 6184 | 6184 | 334 | 1245 | 3.73 |
| `log2` | 7758 | 7758 | 349 | 1514 | 4.34 |
| `log10` | 7635 | 7635 | 354 | 343 | 0.97 |
| `log1p` | 6727 | 6727 | 367 | 357 | 0.97 |
| `log2p1` | 8205 | 8205 | 8205 | 7042 | 0.86 |
| `log10p1` | 8204 | 8204 | 8204 | 7045 | 0.86 |
| `pow` | 11531 | 11531 | 923 | 3576 | 3.87 |
| `sin` | 6435 | 765 | 751 | 748 | 1.00 |
| `cos` | 6413 | 1067 | 755 | 753 | 1.00 |
| `sincos` | 10528 | 1390 | 1065 | 1050 | 0.99 |
| `tan` | 11198 | 1005 | 987 | 981 | 0.99 |
| `sinpi` | 6057 | 6057 | 6057 | 5298 | 0.87 |
| `cospi` | 6022 | 6022 | 6022 | 5269 | 0.87 |
| `tanpi` | 11807 | 11807 | 11807 | 10190 | 0.86 |
| `asin` | 13029 | 13029 | 662 | 654 | 0.99 |
| `acos` | 13628 | 13628 | 668 | 658 | 0.99 |
| `atan` | 7741 | 7741 | 523 | 514 | 0.98 |
| `atan2` | 8314 | 8314 | 524 | 516 | 0.98 |
| `sinh` | 6367 | 6367 | 508 | 498 | 0.98 |
| `cosh` | 5954 | 5954 | 494 | 485 | 0.98 |
| `tanh` | 7193 | 7193 | 500 | 491 | 0.98 |
| `asinh` | 15215 | 15215 | 535 | 526 | 0.98 |
| `acosh` | 13378 | 13378 | 499 | 488 | 0.98 |
| `atanh` | 9286 | 9286 | 524 | 514 | 0.98 |
| `erf` | 12171 | 12171 | 518 | 8263 | 15.96 |
| `erfc` | 37968 | 37968 | 37968 | 31472 | 0.83 |
| `lgamma` | 17366 | 17366 | 17366 | 14969 | 0.86 |
| `tgamma` | 23498 | 23498 | 23498 | 20222 | 0.86 |
| `fmod` | 280 | 280 | 280 | 250 | 0.89 |
| `floor` | 96.6 | 96.6 | 96.6 | 86.1 | 0.89 |
| `round` | 94.6 | 94.6 | 94.6 | 84.1 | 0.89 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fsub` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fmul` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fdiv` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fcmp` | 10.8 | 10.8 | 10.8 | 10.8 | 1.00 |
| `dadd` | 55.2 | 55.2 | 55.2 | 46.2 | 0.84 |
| `dsub` | 58.1 | 58.1 | 58.1 | 46.3 | 0.80 |
| `dmul` | 54.0 | 54.0 | 54.0 | 54.9 | 1.02 |
| `ddiv` | 174 | 174 | 174 | 81.9 | 0.47 |
| `dcmp` | 28.9 | 28.9 | 28.9 | 20.2 | 0.70 |
| `f2d` | 15.9 | 15.9 | 15.9 | 15.9 | 1.00 |
| `d2f` | 19.4 | 19.4 | 19.4 | 17.9 | 0.92 |
| `f2i` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `i2f` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `d2i` | 19.9 | 19.9 | 19.9 | 20.9 | 1.05 |
| `i2d` | 21.6 | 21.6 | 21.6 | 18.9 | 0.88 |
| `f2l` | 27.4 | 27.4 | 27.4 | 20.9 | 0.76 |
| `l2f` | 32.9 | 32.9 | 32.9 | 27.3 | 0.83 |
| `d2l` | 44.9 | 44.9 | 44.9 | 25.9 | 0.58 |
| `l2d` | 30.1 | 30.1 | 30.1 | 29.6 | 0.98 |

#### armv7e-m-dp

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 237 of 603 | 1.08 | `memchr`, `memset`, `strlen` |
| stdio | 19 of 23 | 1.00 | `memchr`, `memset`, `strlen` |
| strto | 11 of 21 | 1.00 | `__aeabi_d2lz`, `memchr`, `memset` |
| malloc | 1 of 9 | 1.02 | `memchr`, `memset` |
| qsort | 0 of 8 | — | `memchr`, `memset` |
| math | 100 of 160 | 0.94 | `__aeabi_d2lz`, `__aeabi_l2d`, `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `memchr`, `memset`, `pow`, `powf`, `sincosf`, `sinf` |
| softfloat | 4 of 20 | 0.79 | `__aeabi_d2lz`, `__aeabi_f2lz`, `__aeabi_l2d`, `__aeabi_l2f`, `__aeabi_ul2f`, `memchr`, `memset` |

##### string

Aligned case per size: size flavour / speed flavour / fast flavour / AOR.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memset` | 30 / 18 / 18 / 19 | 33 / 35 / 35 / 29 | 34 / 35 / 35 / 38 | 46 / 39 / 39 / 40 | 94 / 57 / 57 / 49 | 286 / 81 / 81 / 85 | 1054 / 177 / 177 / 229 |
| `memchr` | 29 / 24 / 24 / 23 | 36 / 35 / 35 / 27 | 65 / 40 / 40 / 42 | 95 / 57 / 57 / 63 | 215 / 141 / 141 / 123 | 695 / 477 / 477 / 363 | 2615 / 1821 / 1821 / 1323 |
| `strlen` | 35 / 35 / 35 / 42 | 42 / 35 / 35 / 42 | 40 / 36 / 36 / 42 | 55 / 53 / 53 / 56 | 115 / 107 / 107 / 104 | 355 / 323 / 323 / 290 | 1315 / 1187 / 1187 / 1034 |
| `strcpy` | 65 / 62 / 62 / 69 | 76 / 62 / 62 / 69 | 75 / 65 / 65 / 71 | 105 / 90 / 90 / 93 | 225 / 156 / 156 / 153 | 705 / 396 / 396 / 363 | 2625 / 1356 / 1356 / 1203 |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `expf` | 172 | 172 | 52.3 | 42.3 | 0.81 |
| `exp2f` | 195 | 195 | 50.3 | 39.3 | 0.78 |
| `logf` | 162 | 162 | 81.3 | 46.3 | 0.57 |
| `log2f` | 203 | 203 | 81.3 | 48.3 | 0.59 |
| `log10f` | 173 | 173 | 83.3 | 50.3 | 0.60 |
| `powf` | 398 | 398 | 159 | 88.4 | 0.55 |
| `sinf` | 187 | 187 | 57.8 | 50.5 | 0.87 |
| `cosf` | 180 | 180 | 49.8 | 47.2 | 0.95 |
| `sincosf` | 270 | 270 | 84.2 | 80.8 | 0.96 |
| `erff` | 421 | 421 | 113 | 67.1 | 0.59 |
| `sqrt` | 11.6 | 11.6 | 11.6 | 11.6 | 0.99 |
| `cbrt` | 132 | 132 | 89.8 | 89.8 | 1.00 |
| `hypot` | 138 | 138 | 138 | 138 | 1.00 |
| `exp` | 169 | 169 | 52.6 | 55.6 | 1.06 |
| `exp2` | 183 | 183 | 50.6 | 51.6 | 1.02 |
| `exp10` | 185 | 185 | 54.6 | 55.6 | 1.02 |
| `expm1` | 172 | 172 | 106 | 106 | 1.00 |
| `exp2m1` | 173 | 173 | 173 | 173 | 1.00 |
| `exp10m1` | 182 | 182 | 182 | 182 | 1.00 |
| `log` | 150 | 150 | 67.6 | 67.6 | 1.00 |
| `log2` | 200 | 200 | 71.6 | 72.6 | 1.01 |
| `log10` | 161 | 161 | 76.6 | 76.6 | 1.00 |
| `log1p` | 187 | 187 | 85.8 | 85.7 | 1.00 |
| `log2p1` | 198 | 198 | 198 | 198 | 1.00 |
| `log10p1` | 198 | 198 | 198 | 198 | 1.00 |
| `pow` | 393 | 393 | 133 | 135 | 1.01 |
| `sin` | 165 | 165 | 96.1 | 96.1 | 1.00 |
| `cos` | 167 | 167 | 93.3 | 93.2 | 1.00 |
| `sincos` | 231 | 231 | 171 | 171 | 1.00 |
| `tan` | 207 | 207 | 133 | 133 | 1.00 |
| `sinpi` | 219 | 219 | 219 | 219 | 1.00 |
| `cospi` | 215 | 215 | 215 | 215 | 1.00 |
| `tanpi` | 254 | 254 | 254 | 254 | 1.00 |
| `asin` | 220 | 220 | 135 | 135 | 1.00 |
| `acos` | 228 | 228 | 127 | 127 | 1.00 |
| `atan` | 195 | 195 | 108 | 108 | 1.00 |
| `atan2` | 360 | 360 | 141 | 141 | 1.00 |
| `sinh` | 200 | 200 | 107 | 106 | 1.00 |
| `cosh` | 192 | 192 | 78.6 | 78.6 | 1.00 |
| `tanh` | 202 | 202 | 137 | 137 | 1.00 |
| `asinh` | 287 | 287 | 107 | 107 | 1.00 |
| `acosh` | 246 | 246 | 101 | 101 | 1.00 |
| `atanh` | 217 | 217 | 89.8 | 89.8 | 1.00 |
| `erf` | 418 | 418 | 67.9 | 69.2 | 1.02 |
| `erfc` | 783 | 783 | 783 | 783 | 1.00 |
| `lgamma` | 499 | 499 | 499 | 499 | 1.00 |
| `tgamma` | 719 | 719 | 719 | 718 | 1.00 |
| `fmod` | 178 | 178 | 178 | 178 | 1.00 |
| `floor` | 11.8 | 11.8 | 11.8 | 11.8 | 0.99 |
| `round` | 11.8 | 11.8 | 11.8 | 11.8 | 0.99 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fsub` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fmul` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fdiv` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fcmp` | 10.8 | 10.8 | 10.8 | 10.8 | 1.00 |
| `dadd` | 9.9 | 9.9 | 9.9 | 9.9 | 1.00 |
| `dsub` | 9.9 | 9.9 | 9.9 | 9.9 | 1.00 |
| `dmul` | 9.9 | 9.9 | 9.9 | 9.9 | 1.00 |
| `ddiv` | 9.9 | 9.9 | 9.9 | 9.9 | 1.00 |
| `dcmp` | 10.8 | 10.8 | 10.8 | 10.8 | 1.00 |
| `f2d` | 8.9 | 8.9 | 8.9 | 8.9 | 1.00 |
| `d2f` | 6.8 | 6.8 | 6.8 | 6.8 | 1.00 |
| `f2i` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `i2f` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `d2i` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `i2d` | 9.9 | 9.9 | 9.9 | 9.9 | 1.00 |
| `f2l` | 25.9 | 25.9 | 25.9 | 20.9 | 0.81 |
| `l2f` | 32.9 | 32.9 | 32.9 | 27.3 | 0.83 |
| `d2l` | 26.9 | 26.9 | 26.9 | 25.9 | 0.96 |
| `l2d` | 48.6 | 48.6 | 48.6 | 29.6 | 0.61 |

#### armv8-m.main

Instructions per operation: rtoskit-libc's size, speed and fast flavours, and the fast flavour with AOR's routines linked in. The ratio is AOR against the fast flavour, as a geometric mean over the cases whose count changed.

| Program | Cases changed | AOR / fast (geometric mean) | AOR symbols linked |
|---|---:|---:|---|
| string | 237 of 603 | 1.08 | `memchr`, `memset`, `strlen` |
| stdio | 20 of 23 | 1.00 | `__aeabi_d2uiz`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_ul2d`, `memchr`, `memset`, `strlen` |
| strto | 11 of 21 | 0.95 | `__aeabi_d2lz`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_ul2d`, `memchr`, `memset` |
| malloc | 1 of 9 | 1.02 | `memchr`, `memset` |
| qsort | 0 of 8 | — | `memchr`, `memset` |
| math | 100 of 160 | 1.78 | `__aeabi_d2f`, `__aeabi_d2iz`, `__aeabi_d2lz`, `__aeabi_dadd`, `__aeabi_dcmpeq`, `__aeabi_dcmpge`, `__aeabi_dcmpgt`, `__aeabi_dcmple`, `__aeabi_dcmplt`, `__aeabi_dcmpun`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_drsub`, `__aeabi_dsub`, `__aeabi_f2d`, `__aeabi_i2d`, `__aeabi_l2d`, `__aeabi_ui2d`, `__aeabi_ul2d`, `cosf`, `erf`, `erff`, `exp`, `exp10`, `exp2`, `exp2f`, `expf`, `log`, `log10f`, `log2`, `log2f`, `logf`, `memchr`, `memset`, `pow`, `powf`, `sincosf`, `sinf` |
| softfloat | 12 of 20 | 0.80 | `__aeabi_d2f`, `__aeabi_d2iz`, `__aeabi_d2lz`, `__aeabi_dadd`, `__aeabi_dcmplt`, `__aeabi_ddiv`, `__aeabi_dmul`, `__aeabi_drsub`, `__aeabi_dsub`, `__aeabi_f2d`, `__aeabi_f2lz`, `__aeabi_i2d`, `__aeabi_l2d`, `__aeabi_l2f`, `__aeabi_ui2d`, `__aeabi_ul2f`, `memchr`, `memset` |

##### string

Aligned case per size: size flavour / speed flavour / fast flavour / AOR.

| Function | 0 | 1 | 4 | 16 | 64 | 256 | 1024 |
|---|---:|---:|---:|---:|---:|---:|---:|
| `memset` | 30 / 18 / 18 / 19 | 33 / 35 / 35 / 29 | 34 / 35 / 35 / 38 | 46 / 39 / 39 / 40 | 94 / 57 / 57 / 49 | 286 / 81 / 81 / 85 | 1054 / 177 / 177 / 229 |
| `memchr` | 29 / 24 / 24 / 23 | 36 / 35 / 35 / 27 | 65 / 40 / 40 / 42 | 95 / 57 / 57 / 63 | 215 / 141 / 141 / 123 | 695 / 477 / 477 / 363 | 2615 / 1821 / 1821 / 1323 |
| `strlen` | 35 / 35 / 35 / 42 | 42 / 35 / 35 / 42 | 40 / 36 / 36 / 42 | 55 / 53 / 53 / 56 | 115 / 107 / 107 / 104 | 355 / 323 / 323 / 290 | 1315 / 1187 / 1187 / 1034 |
| `strcpy` | 65 / 62 / 62 / 69 | 76 / 62 / 62 / 69 | 75 / 65 / 65 / 71 | 105 / 90 / 90 / 93 | 225 / 156 / 156 / 153 | 705 / 396 / 396 / 363 | 2625 / 1356 / 1356 / 1203 |

##### math

Throughput per call, for the functions whose count changed (AOR's own, or ours through AOR's soft-float).

| Function | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `expf` | 160 | 160 | 58.3 | 681 | 11.68 |
| `exp2f` | 141 | 141 | 59.3 | 632 | 10.65 |
| `logf` | 165 | 165 | 74.3 | 649 | 8.73 |
| `log2f` | 154 | 154 | 80.3 | 654 | 8.14 |
| `log10f` | 165 | 165 | 83.3 | 698 | 8.38 |
| `powf` | 353 | 353 | 203 | 1478 | 7.27 |
| `sinf` | 574 | 574 | 126 | 756 | 5.98 |
| `cosf` | 566 | 566 | 121 | 757 | 6.24 |
| `sincosf` | 784 | 784 | 208 | 1247 | 5.99 |
| `erff` | 1383 | 1383 | 123 | 550 | 4.48 |
| `sqrt` | 1968 | 1968 | 1968 | 1884 | 0.96 |
| `cbrt` | 3765 | 3765 | 296 | 286 | 0.97 |
| `hypot` | 5835 | 5835 | 5835 | 5289 | 0.91 |
| `exp` | 3813 | 3813 | 316 | 1090 | 3.45 |
| `exp2` | 5236 | 5236 | 280 | 934 | 3.34 |
| `exp10` | 5252 | 5252 | 315 | 1075 | 3.41 |
| `expm1` | 4075 | 4075 | 346 | 335 | 0.97 |
| `exp2m1` | 5408 | 5408 | 5408 | 4720 | 0.87 |
| `exp10m1` | 5541 | 5541 | 5541 | 4825 | 0.87 |
| `log` | 6184 | 6184 | 334 | 1245 | 3.73 |
| `log2` | 7758 | 7758 | 349 | 1514 | 4.34 |
| `log10` | 7635 | 7635 | 354 | 343 | 0.97 |
| `log1p` | 6727 | 6727 | 367 | 357 | 0.97 |
| `log2p1` | 8205 | 8205 | 8205 | 7042 | 0.86 |
| `log10p1` | 8204 | 8204 | 8204 | 7045 | 0.86 |
| `pow` | 11532 | 11532 | 923 | 3576 | 3.87 |
| `sin` | 6435 | 754 | 740 | 737 | 1.00 |
| `cos` | 6413 | 1057 | 743 | 741 | 1.00 |
| `sincos` | 10528 | 1379 | 1054 | 1039 | 0.99 |
| `tan` | 11198 | 1000 | 982 | 976 | 0.99 |
| `sinpi` | 6057 | 6057 | 6057 | 5299 | 0.87 |
| `cospi` | 6023 | 6023 | 6023 | 5270 | 0.87 |
| `tanpi` | 11807 | 11807 | 11807 | 10190 | 0.86 |
| `asin` | 13029 | 13029 | 662 | 654 | 0.99 |
| `acos` | 13628 | 13628 | 668 | 658 | 0.99 |
| `atan` | 7741 | 7741 | 523 | 514 | 0.98 |
| `atan2` | 8314 | 8314 | 524 | 516 | 0.98 |
| `sinh` | 6367 | 6367 | 508 | 498 | 0.98 |
| `cosh` | 5954 | 5954 | 494 | 485 | 0.98 |
| `tanh` | 7193 | 7193 | 500 | 491 | 0.98 |
| `asinh` | 15215 | 15215 | 536 | 526 | 0.98 |
| `acosh` | 13378 | 13378 | 499 | 488 | 0.98 |
| `atanh` | 9286 | 9286 | 524 | 514 | 0.98 |
| `erf` | 12173 | 12173 | 518 | 8251 | 15.94 |
| `erfc` | 37969 | 37969 | 37969 | 31472 | 0.83 |
| `lgamma` | 17366 | 17366 | 17366 | 14969 | 0.86 |
| `tgamma` | 23498 | 23498 | 23498 | 20222 | 0.86 |
| `fmod` | 281 | 281 | 281 | 251 | 0.89 |
| `floor` | 96.6 | 96.6 | 96.6 | 86.1 | 0.89 |
| `round` | 94.6 | 94.6 | 94.6 | 84.1 | 0.89 |

##### softfloat

Per operation.

| Operation | size | speed | fast | AOR | AOR / fast |
|---|---:|---:|---:|---:|---:|
| `fadd` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fsub` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fmul` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fdiv` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `fcmp` | 10.8 | 10.8 | 10.8 | 10.8 | 1.00 |
| `dadd` | 55.2 | 55.2 | 55.2 | 46.2 | 0.84 |
| `dsub` | 58.1 | 58.1 | 58.1 | 46.3 | 0.80 |
| `dmul` | 54.0 | 54.0 | 54.0 | 54.9 | 1.02 |
| `ddiv` | 174 | 174 | 174 | 81.9 | 0.47 |
| `dcmp` | 28.9 | 28.9 | 28.9 | 20.2 | 0.70 |
| `f2d` | 15.9 | 15.9 | 15.9 | 15.9 | 1.00 |
| `d2f` | 19.4 | 19.4 | 19.4 | 17.9 | 0.92 |
| `f2i` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `i2f` | 7.8 | 7.8 | 7.8 | 7.8 | 1.00 |
| `d2i` | 19.9 | 19.9 | 19.9 | 20.9 | 1.05 |
| `i2d` | 21.6 | 21.6 | 21.6 | 18.9 | 0.88 |
| `f2l` | 27.4 | 27.4 | 27.4 | 20.9 | 0.76 |
| `l2f` | 32.9 | 32.9 | 32.9 | 27.3 | 0.83 |
| `d2l` | 44.9 | 44.9 | 44.9 | 25.9 | 0.58 |
| `l2d` | 30.1 | 30.1 | 30.1 | 29.6 | 0.98 |

## Accuracy against AOR

16,384 sampled arguments per function, round to nearest. Each cell is the
maximum ulp error / the results not correctly rounded.

16,384 sampled arguments per function, round to nearest: maximum ulp error / results not correctly rounded.

| Function | armv6-m/size | armv6-m/speed | armv6-m/fast | armv6-m/aor | armv7e-m/size | armv7e-m/speed | armv7e-m/fast | armv7e-m/aor | armv7e-m-dp/size | armv7e-m-dp/speed | armv7e-m-dp/fast | armv7e-m-dp/aor |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `expf` | 0.500 / 0 | 0.500 / 0 | 0.504 / 29 | 0.501 / 4 | 0.500 / 0 | 0.500 / 0 | 0.776 / 777 | 0.501 / 4 | 0.500 / 0 | 0.500 / 0 | 0.506 / 17 | 0.501 / 4 |
| `exp2f` | 0.500 / 0 | 0.500 / 0 | 0.503 / 25 | 0.501 / 5 | 0.500 / 0 | 0.500 / 0 | 0.755 / 778 | 0.501 / 5 | 0.500 / 0 | 0.500 / 0 | 0.506 / 9 | 0.501 / 5 |
| `logf` | 0.500 / 0 | 0.500 / 0 | 0.502 / 23 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.514 / 2 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 |
| `log2f` | 0.500 / 0 | 0.500 / 0 | 0.502 / 29 | 0.541 / 1 | 0.500 / 0 | 0.500 / 0 | 0.505 / 3 | 0.541 / 1 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.541 / 1 |
| `log10f` | 0.500 / 0 | 0.500 / 0 | 0.502 / 30 | 0.510 / 6 | 0.500 / 0 | 0.500 / 0 | 0.501 / 3 | 0.510 / 6 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.510 / 6 |
| `sinf` | 0.500 / 0 | 0.500 / 0 | 0.502 / 22 | 0.559 / 110 | 0.500 / 0 | 0.500 / 0 | 0.501 / 2 | 0.559 / 110 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.559 / 110 |
| `cosf` | 0.500 / 0 | 0.500 / 0 | 0.502 / 14 | 0.558 / 108 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.558 / 108 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.558 / 108 |
| `erff` | 0.500 / 0 | 0.500 / 0 | 0.511 / 48 | 0.887 / 1298 | 0.500 / 0 | 0.500 / 0 | 0.720 / 69 | 0.887 / 1063 | 0.500 / 0 | 0.500 / 0 | 0.500 / 1 | 0.887 / 1063 |
| `powf` | 0.500 / 0 | 0.500 / 0 | 0.503 / 17 | 0.504 / 6 | 0.500 / 0 | 0.500 / 0 | 0.801 / 501 | 0.504 / 6 | 0.500 / 0 | 0.500 / 0 | 0.502 / 5 | 0.504 / 6 |
| `exp` | 0.500 / 0 | 0.500 / 0 | 0.501 / 3 | 0.502 / 9 | 0.500 / 0 | 0.500 / 0 | 0.501 / 3 | 0.502 / 9 | 0.500 / 0 | 0.500 / 0 | 0.523 / 30 | 0.502 / 6 |
| `exp2` | 0.501 / 4 | 0.501 / 4 | 0.500 / 4 | 0.502 / 8 | 0.501 / 4 | 0.501 / 4 | 0.500 / 4 | 0.502 / 8 | 0.501 / 4 | 0.501 / 4 | 0.512 / 25 | 0.501 / 9 |
| `log` | 0.500 / 0 | 0.500 / 0 | 0.500 / 5 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 5 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 |
| `log2` | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 | 0.500 / 0 |
| `erf` | 0.500 / 0 | 0.500 / 0 | 0.502 / 4 | 0.837 / 1560 | 0.500 / 0 | 0.500 / 0 | 0.502 / 4 | 0.837 / 1560 | 0.500 / 0 | 0.500 / 0 | 0.511 / 13 | 0.837 / 1541 |
| `pow` | 0.500 / 0 | 0.500 / 0 | 0.501 / 5 | 0.502 / 6 | 0.500 / 0 | 0.500 / 0 | 0.501 / 5 | 0.502 / 6 | 0.500 / 0 | 0.500 / 0 | 0.524 / 21 | 0.502 / 6 |
