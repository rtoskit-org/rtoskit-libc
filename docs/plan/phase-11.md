# Phase 11 — The fast build type

Prerequisite: PLAN D162 (this phase), D73 and D77 (the speed flavour and its
`speed/` files), D45 (`MATH_ERREXCEPT`), D44 (which cores have `<fenv.h>`),
D47, D48, D57, D98, D125 (today's accuracy), D124 (the libm vectors), D72 and
D92 (bench, AOR comparison); D163 (the answers to Q49–Q51).
Research note: [`docs/research/fast-math.md`](../research/fast-math.md).
Targets: every architecture in `ARCHS`; the gate figures are the Arm cores'.

**Goal:** a fourth build type, `fast`, beside `debug`, `release` (size) and
`speed`, whose maths library trades accuracy (a looser error bound) and
rounding (round to nearest only) for speed, so that on every Arm core its maths
throughput reaches picolibc's and AOR's, and beats them where they lose
today. Everything outside libm is the speed flavour's.

**The fast contract** (D162, D163, `fast-math.md` §4):
- to nearest: below 1 ulp for exp, log, pow, trig and inverse trig; below
  1.5 ulp for `tan`, the hyperbolic functions and `erf`; `erfc`, `lgamma`,
  `tgamma` and everything not listed as in speed;
- rounding: round to nearest only. The functions compute as if rounding to
  nearest and never read or set the mode; in a directed mode (x86_64 and
  armv7e-m-dp) only the special values, exceptions, ranges and exact
  functions below are guaranteed, not an error bound;
- unchanged: the headers (`math_errhandling` is `MATH_ERREXCEPT`, `<fenv.h>`,
  `__STDC_IEC_559__`); every special value, signed zero, overflow and
  underflow result of C23 F.10 in every mode; gradual underflow; bounded
  ranges and symmetry; exact argument reduction; the exact functions exact;
- `errno` and floating-point exceptions exactly as the other build types
  (the maintainer's decision, Q48): `math_errhandling` stays
  `MATH_ERREXCEPT`, so libm leaves `errno` alone in `fast` as it does in
  debug, release and speed; invalid, divide-by-zero, overflow and deserved
  underflow are raised by the same `__libc_math_raise_*` calls on every core,
  including the soft-`<fenv.h>` ones, where they are what the other build
  types do there (D44).

**Out of scope:** changing `debug`, `release` or `speed` in any way (their
figures, tests and baselines stay byte-identical); `-ffast-math` for the
library as a whole; flush-to-zero; relaxing anything outside libm (a later
phase may, through the same `fast/` mechanism: `strtod`, `printf`'s `%f`);
the complex functions; long double beyond sharing the double functions (D7).

**Every step's definition of done:**
- `make` and `make check` on every architecture in all four build types,
  libgcc-check clean; the libm vectors pass at the fast bounds in `fast` and
  at today's bounds in the other three, unchanged;
- every touched function's special values, thresholds and symmetry cases
  (`fast-math.md` §8) in `test/unit`, running in every flavour; a
  `regression_*` case for every bug found;
- `make bench` for `fast` against `docs/perf/baseline-<arch>-fast.json`,
  re-locked with `BENCH_UPDATE=1` and the gains recorded in
  `docs/perf/<arch>.md`; `release` and `speed` baselines unchanged;
- `size-report diff` for release, and the fast flavour's budget; `fast/`
  members recorded in `docs/size/<arch>.md`;
- a research note per new algorithm (sources, error analysis, edge cases),
  clean-room; one function per file; no `#if`.

---

## P11.01 — Research and decisions  `[x]`
- [x] `docs/research/fast-math.md`: sources (AOR's `math_config.h` and error
  bounds, picolibc's build options, newlib, musl, glibc, GCC's `-f*-math`,
  CORE-MATH, C23 7.12 and F.10), the cost of every guarantee and where it is
  paid in `libc/math/`, the proposed contract per family, how the tests
  express it, the expected gain per core and the method.
- [x] PLAN D162 (the phase, the build type and its fallback chain, the
  proposed contract) and the phase table.
- [x] Open questions Q48 (`errno` and exceptions), Q49 (the bounds and the
  compiler flags), Q50 (release archives, or a flavour archive), Q51 (the
  per-flavour test expectations) recorded.
- [x] Q48 answered by the maintainer: `fast` keeps `errno` and the
  floating-point exceptions as the other build types do, on every core;
  `fast` relaxes only accuracy and rounding (nearest only). Recorded in D162.
- [x] Q49 answered (D163): the bounds above; the `fast/` files drop
  `-frounding-math` and keep `-fsignaling-nans` and trapping math.
- [x] Q50 answered (D163): `fast` is a build type, and ships in the release
  archives once P11.06 meets the exit criterion.
- [x] Q51 answered (D163): per-flavour expectation files, the strict ones
  unchanged.
- [x] The maintainer confirmed D162's reading of Q48: no build type sets
  `errno` from `<math.h>`, `fast` included (D163).

## P11.02 — The `fast` build type and its gates

Split (CLAUDE.md workflow) into P11.02a, the build type and every gate but the
audit of the strict-contract tests; P11.02b, that audit (eight tests of about
2,500 lines) with the fast expectations of the three uniform tests; and
P11.02c, the fast expectations of the three tests of inline checks.

### P11.02a — The build type, the fallback chain and the gates  `[x]`
- [x] `tools/makefiles/fast.mk` like `speed.mk` (`-Os -g3`,
  `LIBC_SPEED_CFLAGS := -O2`, `-DNDEBUG -DBUILD_TYPE_FAST`, `export
  LIBC_FLAVOUR := fast`); `BUILDS ?= debug release speed fast`; `make help`,
  README and `docs/configuration.md`; `build/<arch>/fast/`, `images/<arch>/fast/`.
- [x] The fallback chain: exported `LIBC_FLAVOURS` (`speed` → `speed`,
  `fast` → `speed fast`); every `$(filter speed,${LIBC_FLAVOUR})` in
  `libc/libc.mk`, `libc/math/subdir.mk`, `libc/stdio/subdir.mk`,
  `libc/stdlib/subdir.mk` and the layer makefiles filters `LIBC_FLAVOURS`;
  `LIBC_STRING_KERNELS_fast := ${LIBC_STRING_KERNELS_speed}` in every arch
  file.
- [x] `fast/` directories: `libc/math/fast/` and `libc/math/<layer>/fast/`
  replace same-named generic, layer and `speed/` files (precedence
  `<layer>/fast/` > `fast/` > `<layer>/speed/` > `<layer>/` > generic);
  archived from `libc/libc.mk` only when `fast` is in `LIBC_FLAVOURS`; one
  check that a `fast` build with no `fast/` file has the speed build's
  `libc.a` members, byte for byte (`size-report --build-type fast same --as
  speed`). A `libc.a` member name in two modules now stops the build
  (`libc.unique`, D164).
- [x] Tests: `test/unit/libm_vectors` bounds moved into `bounds.c` (today's,
  same checks) and `fast/bounds.c` (the contract), selected by the module's
  `subdir.mk`; a `flavours` case that the fast build's headers equal the
  speed build's (`flavours.run`, and `math_errhandling` in `flavours.c`).
- [x] Bench: `docs/perf/baseline-<arch>-fast.json` (equal to speed at this
  step); `bench-runner compare` and `aor` take `--build-type fast` as the
  reference column. The release and speed baselines predated P10.07's
  `threads` cost (Q52); re-locked by D166, the fast path is issue #144.
- [x] Size: the fast flavour in `docs/size/budget.json` under the speed
  flavour's rule (D73); `size-report --build-type fast budget --flavour fast`.
- [x] CI (D119): `make check` covers `fast` through `BUILDS`; the Arm size
  job adds the fast budget; note the 33 % longer `make check` against Q33.
- [x] Release (D158, D163): the workflow and `release-package` leave the
  `fast` tree out until P11.06; `docs/release.md` says so.

### P11.02b — The audit, and the uniform tests' fast expectations  `[x]`
Split again (CLAUDE.md workflow): the audit showed three of the eight tests
checking one comparison throughout, which a small expectation file covers,
and three mixing about 160 inline checks of every kind, each needing its own
reading; those are P11.02c.
- [x] The audit of every strict-contract test (`float_hard`,
  `float_accuracy`, `libm_dense`, `libm_exp`, `libm_trig`, `libm_special`,
  `fpu_math`, `float_exact`), its findings in `docs/research/fast-math.md`
  §10: `fpu_math` and `float_exact` test exact functions only and need no
  `fast/` file; `float_hard`, `float_accuracy` and `libm_dense` take one
  expectation file each; `libm_exp`, `libm_trig` and `libm_special` go to
  P11.02c.
- [x] `test/unit/include/fast_contract.h`: the relaxed families and their
  bounds, and a float result's error in ulps, shared by every `fast/` file
  (`libm_vectors/fast/bounds.c` included).
- [x] `float_hard`, `float_accuracy`, `libm_dense`: the strict expectation in
  `expect.c` (the same checks as before), the fast one in `fast/expect.c`,
  selected by each module's `subdir.mk`.
- [x] `make check` in all four build types; with no `fast/` libm file the
  fast expectations pass as the strict ones do.

### P11.02c — The inline-check tests' fast expectations  `[x]`
- [x] `libm_exp`, `libm_trig`, `libm_special`: every check of a relaxed
  function's last bit (the reference vectors' bound to nearest), of an exact
  result F.10 does not fix (`exp2(k)`, `log2(2^k)`, `log10(10^k)`, `cbrt(27)`,
  `rootn`, `pow`'s exact cases) or of a directed result goes through the
  build type's `libm_expect.h` (`test/unit/include/strict/` or `fast/`, by
  include path), with the exception set kept but for inexact where the
  result is no longer required exact (`EXPECT_EXACT`); every F.10 special
  value, signed zero, overflow, underflow, zero result and exception check
  stays in the shared source (`fast-math.md` §11). 104 checks routed: 58 in
  `libm_exp`, 43 in `libm_trig`, 3 in `libm_special` (only `erf` is relaxed).
- [x] `make check` in all four build types.

## P11.03 — Float exp, log and pow

Split (CLAUDE.md workflow): fourteen functions in three evaluations (double
on the double-FPU cores, float on the single-precision FPU cores, fixed
point on the cores without an FPU) are one sub-step per family.
Research note: [`docs/research/libm-fast-float-exp-log.md`](../research/libm-fast-float-exp-log.md),
extended by each sub-step. Every sub-step: the libm vectors at the fast
bounds, the special-value and threshold cases of `fast-math.md` §8, an
exhaustive host sweep of each new float evaluation (every float argument
against the double function), `make check` in all four build types, the
fast bench re-locked with the gains and the members recorded.

### P11.03a — `expf`, `exp2f`, `exp10f` on the FPU cores  `[x]`
- [x] Research: the note's design and error analysis for the exponentials
  (a 2^(j/32) table and a double polynomial on the double-FPU cores; a
  float-only reduction and polynomial on the single-precision FPU cores).
- [x] The precedence among layers (supersedes D162's): a layer's own files,
  `speed/` and `fast/`, beat the generic `fast/` file and every earlier
  layer's, so a `fast/` file never displaces a later layer's faster code
  (the fixed-point `nofp/speed` files on the cores without an FPU); and a
  layer's `fast/` applies only where the arch file lists it in
  `LIBC_MATH_FAST_LAYERS` (D168).
- [x] `libc/math/fast/{expf,exp2f,exp10f}.c` (x86_64, armv7e-m-dp) and
  `libc/math/sp-nearest/fast/{expf,exp2f,exp10f}.c` (armv7e-m,
  armv8-m.main). On armv6-m, armv6s-m and armv7-m the float-only
  `exp10f` measured 43 % and 25 % slower (software `fmaf`), so those cores
  keep the speed files (P11.03e).
- [x] The x86_64 `float_accuracy` links the `sp-nearest/fast/` files over
  the layer's in fast, so it measures the fast float code natively.
- [x] Tests (`test/unit/libm_thresholds`, every build type; the libm
  vectors at the fast bounds; an exhaustive host sweep: double path at most
  0.5096 ulp, float path at most 0.8527), gains (`expf` 160 → 58 on
  armv7e-m, 172 → 52 on armv7e-m-dp) and members recorded.

### P11.03b — `logf`, `log2f`, `log10f`  `[x]`
- [x] Research (the note, sections 7–9): 2 atanh in double on the
  double-FPU cores, no table; on the single-precision FPU cores a 32-entry
  inverse table with an exact high part, r = x − 1 within 2^-5 of 1 (the
  first build's neighbour entry cancelled to 2.72 ulp); implementation,
  tests (`libm_thresholds`: the logarithms, log(1) = +0 downward, the
  double path's first bug), gains (`logf` 165 → 74 on armv7e-m, 162 → 81
  on armv7e-m-dp, 267 → 69 on x86_64). Exhaustive: double 0.5000, float at
  most 0.8341 ulp (D169).

### P11.03c — `powf`  `[x]`
- [x] Research (the note, sections 10–11): the F.10.4.5 cases in one helper;
  2^(y log2 |x|) in double on the double-FPU cores; on the single-precision
  FPU cores log2 |x| as a float pair to 2^-39 absolute, the product's error
  by a fused multiply-add and the exp kernel (the first build's p_lo (1 − r)
  left 1.34 ulp near x = 0.996); implementation, tests (`libm_thresholds`:
  every F.10.4.5 case, the thresholds and exceptions;
  `regression_pow_minus_one`: the strict `pow` gave +1 for (−1)^odd beyond
  2^20), gains (`powf` 367 → 215 on armv7e-m, 398 → 159 on
  armv7e-m-dp, 633 → 181 on x86_64). Sampled: double 0.5096, float at most 0.8749 ulp
  (D170).

### P11.03d — `expm1f`, `exp2m1f`, `exp10m1f`, `log1pf`, `log2p1f`, `log10p1f`, `cbrtf`  `[x]`
- [x] Research (the note, sections 12–14), over P11.03a's and P11.03b's
  kernels, keeping the relative accuracy near 0: the m1 forms' leading
  terms by exact two-sums on the float path, the series near 0 on the
  double path; 1 + x with its error carried (float) or never formed near 0
  (double); `cbrtf` by a polynomial start and Newton or Halley steps;
  implementation, tests (`libm_thresholds`: the seven functions' special
  values, thresholds and near-0 results), gains (`expm1f` 178 → 78, `log1pf`
  197 → 96, `cbrtf` 149 → 81 on armv7e-m; `expm1f` 264 → 61 on x86_64). Every float:
  double at most 0.5473 ulp, float at most 0.8472 (D171).

### P11.03e — The cores without an FPU: the fixed-point files without their test  `[x]`
- [x] armv6-m, armv6s-m, armv7-m: `nofp/fast/` variants of `nofp/speed`'s
  `expf`, `exp2f`, `logf`, `log2f`, `log10f`, `powf` without the rounding
  test and its float-float fallback (the fast bounds allow the fixed
  point's 0.5 + 8/512 ulp; the fallback stays only for what the fixed
  point does not cover), and a fixed-point `exp10f` over the same exp2
  core; the float-only files of P11.03a–d measured against them on these
  cores (the m1 forms slower, the others at most a quarter faster: the
  layer stays off); tests (`libm_thresholds`: each side of the fixed
  paths' edges), gains (`exp10f` 11,103 → 367 on
  armv6-m, 5,461 → 117 on armv7-m; `logf` 2,210 → 670, 1,026 → 253). Every argument of the fixed paths on the
  host: at most 0.5062 ulp (D172).

Split from one step (P11.03e): on these cores the speed `exp10f`, the m1
and p1 forms and `cbrtf` are float-float on soft-float, 8,600–13,200
instructions on armv6-m against `expf`'s 375, so new fixed-point code is
where the gain is, one family per sub-step.

### P11.03f — The cores without an FPU: the m1 forms  `[x]`
- [x] Fixed-point `expm1f`, `exp2m1f`, `exp10m1f` over the exp2 core, with
  the relative accuracy near 0 (a series below the cancellation); tests
  (`libm_thresholds`: each side of the series' end, the core's 2^30 and the
  hand-off), gains (`expm1f` 12,310 → 461 on
  armv6-m, 6,065 → 151 on armv7-m). Every argument on the host: at most 0.7328
  ulp (D173).

### P11.03g — The cores without an FPU: the p1 forms and `cbrtf`  `[x]`
- [x] Fixed-point `log1pf`, `log2p1f`, `log10p1f` over the log2 core (1 + x
  exact as an integer, a series near 0) and an integer `cbrtf`; the m1 and
  p1 forms' tiny paths through the series too (no float constant); tests
  (`libm_thresholds`: each side of 2^-5 and 2^24, near −1, `cbrtf`'s
  powers), gains (`log1pf` 12,789 → 699, `cbrtf`
  8,590 → 630 on armv6-m; `log1pf` 6,386 → 254 on armv7-m). Every argument on the host: at most 0.5779
  ulp (D174).

## P11.04 — Float trigonometric, inverse and hyperbolic functions  `[x]`

Split (CLAUDE.md workflow), as P11.03: sixteen functions and their `pi`
forms in three evaluations (double, float, fixed point), one family per
sub-step, the cores without an FPU last. Research note:
`docs/research/libm-fast-float-trig.md`, extended by each sub-step (Cody–Waite
in float or double per core for small |x|, the D136 integer reduction above,
polynomial degrees for the fast bounds). Every sub-step: the libm vectors at
the fast bounds, the special-value and threshold cases (`libm_thresholds`),
an exhaustive host sweep of each new evaluation, `make check` in all four
build types, the fast bench re-locked with the gains and the members
recorded. Speed today (instructions per call): `sinf` 337 on x86_64, 187 on
armv7e-m-dp, 575 on armv7e-m, 2,933 on armv6-m; `asinf` 505, 238, 315,
23,011; `sinhf` 334, 223, 267, 15,310.

### P11.04a — `sinf`, `cosf`, `sincosf`, `tanf`, `sinpif`, `cospif`, `tanpif` on the FPU cores  `[x]`
- [x] Research (the note, sections 1–5): Cody–Waite in double below 2^20 on
  the double-FPU cores, the strict integer reduction and a float table
  kernel on the single-precision ones; implementation, tests
  (`libm_thresholds`: special values, the π forms' exact cases and poles,
  each side of the reductions' edges), gains (`sinf` 575 → 128 on armv7e-m,
  187 → 58 on armv7e-m-dp, 337 → 63 on x86_64). Every finite
  float: double at most 0.5002 ulp, float at most 0.75 (D175).

### P11.04b — `asinf`, `acosf`, `atanf`, `atan2f` and their `pi` forms on the FPU cores  `[x]`
- [x] Research (the note, sections 6–8): one atan table kernel (nine
  breakpoints j/8) in double on the double-FPU cores, asin and acos through
  it; float pairs with the half-angle asin polynomial on the
  single-precision ones; implementation, tests (`libm_thresholds`: the
  special values in every quadrant, each side of 1/2 and 1, and the
  regressions of the subnormal atan2 and π forms; `regression_atanf_sixteenth`
  for the strict core's breakpoint index, fixed too), gains (`asinf` 315 → 64
  on armv7e-m, 238 → 73 on armv7e-m-dp, 505 → 76 on x86_64). Every float:
  double 0.5000 ulp, float at most 0.7512 (D176).

### P11.04c — `sinhf`, `coshf`, `tanhf`, `asinhf`, `acoshf`, `atanhf` on the FPU cores  `[x]`
- [x] Over the fast `expf`, `expm1f`, `logf`, `log1pf` kernels (P11.03);
  research (the note, sections 9–11), implementation (in double over the
  exp2 and log kernels; float pairs over an exponential pair and the log1pf
  reduction with a pair argument), tests (`libm_thresholds`: the special
  values, the poles, overflow, each side of every edge), gains (`sinhf` 267 →
  90 on armv7e-m, 223 → 68 on armv7e-m-dp, 334 → 66 on x86_64). Every finite
  float: double at most 0.5383 ulp, float at most 0.8440 (D177).

### P11.04d — `erff`, `erfcf` on the FPU cores

Split (CLAUDE.md workflow): the fast contract keeps `erfcf` strict (fast-math.md
§4, D163), and the maintainer chose a fast `erfcf` that stays correctly rounded
(D178): the fast result where a rounding test proves it, the strict code
otherwise. In double that test is cheap; the float path first needs pairs good
to about 2^-32 for it to pay, a sub-step of its own.

### P11.04d1 — `erff` on the FPU cores, `erfcf` on the double-FPU cores  `[x]`
- [x] Research (the note, sections 12–14): R(x) = erfc(x) e^(x²) in fitted
  pieces with e^(-x²) from an exact square, the series below 1/2; `erfcf`
  behind a rounding test with the strict code as the fallback (0.8 % of
  arguments); implementation, tests (`libm_thresholds`: the special values,
  underflow, each side of every edge), gains (`erff` 1383 → 123 on armv7e-m,
  421 → 113 on armv7e-m-dp, 511 → 108 on x86_64; `erfcf` 1458 → 117 on
  x86_64, 796 → 123 on armv7e-m-dp). Every float: `erff` at most 0.7905
  ulp; `erfcf` correctly rounded in every direction (D178).

### P11.04d2 — `erfcf` on the single-precision FPU cores  `[x]`
- [x] A pair-accurate e^(-x^2) and R(x) for a rounding test with the
  float-float code as the fallback; research (the note, section 15):
  e^(-x^2) from a 2^(j/32) pair table, R in degree-9 pieces with five pair
  coefficients, every term and cross term exact (2^-32.3 over every
  argument), the test at 2^-31 (1.5 % fall back); implementation, tests
  (`libm_thresholds`: the regressions the sweeps found, exactly), gains
  (`erfcf` 1224 → 233 on armv7e-m and armv8-m.main). Every float correctly
  rounded (these cores round to nearest only, D57) (D179).

### P11.04e — The cores without an FPU: the trigonometric functions and `erff`

Split (CLAUDE.md workflow): the step's text took `nofp/speed`'s files
without their rounding test, as P11.03e did, but that test is not where the
time goes. `sinf` runs D136's double-precision kernel (2933 instructions on
armv6-m), the π forms have no fixed-point path at all (`sinpif` 17,579),
and the test fails about once in 2^35. A float-precision path needs its
own reduction, and `erff` its own kernel: two sub-steps. `erfcf` keeps
speed's code, rounding test included: D178 keeps it correctly rounded.

### P11.04e1 — The cores without an FPU: `sinf`, `cosf`, `sincosf`, `tanf` and the `pi` forms  `[x]`
- [x] D87's fixed-point kernel (`__libc_sinf_fixed`, 720 instructions on
  armv6-m, 270 on armv7-m, measured) fed by a fixed-point reduction (32/pi to
  128 bits for |x| < 2^20; the pi forms' exact 32x), rounded to nearest
  without the rounding test; the kernel's declines and |x| >= 2^20 to the
  strict float-float code; research (the note, section 16), implementation,
  tests (`libm_thresholds`: each side of the fixed paths' edges), gains
  (`sinf` 2933 → 1090 on armv6-m, 842 → 379 on armv7-m; `sinpif` 17,579 →
  920 and 8812 → 369). Every finite float at most 0.5094 ulp (D180).

### P11.04e2 — The cores without an FPU: `erff`  `[x]`
- [x] A fixed-point erf for the fast bound (R(x) pieces over the fixed exp2
  core, as P11.04d's float path); research, implementation, tests, gains.
  Done as erf itself in seven pieces with no exponential (D181): above 1/2
  an absolute error suffices. 0.5223 ulp; 2961 → 414 instructions on armv6-m.

### P11.04f1 — The cores without an FPU: `atanf`, `atan2f` and their π forms  `[x]`
- [x] Fixed-point `atanf`, `atan2f`, `atanpif` and `atan2pif` (float-float on
  soft-float today, 17,683–18,355 instructions on armv6-m): a fixed-point
  division and an atan kernel; tests, gains. Done (D182): at most 0.5746
  ulp; `atanf` 17,683 → 751 instructions on armv6-m.

### P11.04f2 — The cores without an FPU: `asinf`, `acosf` and their π forms  `[x]`
- [x] Fixed-point `asinf`, `acosf`, `asinpif` and `acospif` (22,944–23,828
  instructions on armv6-m): a fixed-point square root and the asin series;
  tests, gains. Done (D183): at most 0.5240 ulp; `asinf` 22,944 → 730
  instructions on armv6-m.

P11.04f was split into f1 and f2 (CLAUDE.md workflow): each needs its own
fixed-point building block, a division for the arctangent and a square root
for the half-angle forms.

### P11.04g1 — The cores without an FPU: `sinhf`, `coshf`, `tanhf`  `[x]`
- [x] Over the fixed-point exp2 core (15,181–15,977 instructions on armv6-m
  today), with the odd series near 0 and a fixed-point quotient for tanh;
  tests, gains. Done (D184): at most 0.5083 ulp; `sinhf` 15,310 → 627
  instructions on armv6-m.

### P11.04g2 — The cores without an FPU: `asinhf`, `acoshf`, `atanhf`  `[x]`
- [x] Over the fixed-point log2 core, square root and quotient (15,868–19,926
  instructions on armv6-m today); tests, gains. Done (D185): at most 0.5260
  ulp; `asinhf` 19,353 → 1152 instructions on armv6-m.

P11.04g was split into g1 and g2 (CLAUDE.md workflow): the first three rest
on the exponential, the last three on the logarithm.

## P11.05 — Double functions  `[x]`

Split (CLAUDE.md workflow), as P11.03 and P11.04: by family, each with the
double-FPU cores (x86_64, armv7e-m-dp: double only, no double-double finish)
and the soft-double cores (armv6-m, armv6s-m, armv7-m, armv7e-m,
armv8-m.main: 64-bit fixed-point kernels, one rounding, no test) apart.
AOR's functions come first, as they gate P11.06. Research note:
`docs/research/libm-fast-double.md` (D186), extended by each sub-step. Every
sub-step: the libm vectors at the fast bounds, `libm_thresholds`, a sampled
host sweep (10^9 random arguments plus each piece and edge, against long
double), `make check` in all four build types, the fast bench re-locked with
the gains and the members recorded.

### P11.05a — Research and decisions  `[x]`
- [x] `docs/research/libm-fast-double.md`: the costs today against AOR, the
  soft-double operations' costs, the sources, the choice per core, the error
  budget and the split (D186). Docs only.

### P11.05b — `exp`, `exp2`, `exp10`, `expm1` on the double-FPU cores  `[x]`
- [x] A 2^(j/N) table and a short polynomial in double, one rounding (AOR:
  `exp` 56, `exp2` 52 instructions on armv7e-m-dp); tests, gains. Done (D187):
  `exp` 169 → 53, `exp2` 183 → 51 on armv7e-m-dp; at most 0.553 ulp. The fast
  size figures may grow through P11.05 (the maintainer's decision, D187).

### P11.05c1 — `exp`, `exp2`, `exp10` on the soft-double cores  `[x]`
- [x] A 64-bit fixed-point exp2 kernel, one rounding (AOR: `exp` 1090 on
  armv7e-m, 3954 on armv6-m); tests, gains. Done (D188): `exp` 3813 → 317 on
  armv7e-m and 10,317 → 1410 on armv6-m; at most 0.5073 ulp.

### P11.05c2 — `expm1` on the soft-double cores  `[x]`
- [x] Over c1's kernel, with the relative accuracy near 0 and where its terms
  cancel; tests, gains. Done (D189): 4075 → 346 on armv7e-m and 10,938 → 1468
  on armv6-m; at most 0.5254 ulp.

P11.05c was split into c1 and c2 (CLAUDE.md workflow): expm1 needs more than
the kernel's absolute accuracy where 2^(k/64) − 1 and the polynomial cancel.

### P11.05d — `log`, `log2`, `log10`, `log1p` on the double-FPU cores  `[x]`
- [x] 1/c and log c from a table and a short polynomial in double (AOR: `log`
  68, `log2` 73); tests, gains. Done (D190): `log` 150 → 68, `log2` 200 → 72
  on armv7e-m-dp; at most 0.5249 ulp.

### P11.05d1 — The strict `log1p`'s undeserved underflow (found in P11.05d)  `[x]`
- [x] `log1p(DBL_MAX)` raised underflow in the strict build: the core's
  correction t/s = 1/x is subnormal there. D227: from 2^100 `log1p`,
  `log2p1` and `log10p1` are `log`, `log2` and `log10` (in place of their
  `isinf` test, so the common path keeps its length);
  `regression_log1p_large` (`log1p`, `log2p1`, `log10p1` of DBL_MAX, 2^1000
  and next to 2^1022 and 2^100); `libm_thresholds`' line back under QUIET.
  Gates: release members 12–35 bytes smaller (armv7e-m-dp: `log1p`'s frame
  +8), every budget held; release and speed 1–2 instructions per call faster
  on the FPU cores, about 30 on armv6-m; `docs/size/`, `docs/perf/`.

### P11.05e — `log`, `log2`, `log10`, `log1p` on the soft-double cores  `[x]`
- [x] A 64-bit fixed-point log2 kernel (AOR: `log` 1245 and 4335, `log2` 1514
  and 5211); tests, gains. Done (D192): `log` 6184 → 334 on armv7e-m and
  16,964 → 1454 on armv6-m, `log2` 7758 → 349 and 21,420 → 1535; at most
  0.5190 ulp.

### P11.05f — `pow`, `cbrt` on the double-FPU cores  `[x]`
- [x] The log with its tail, the exp of a pair (AOR: `pow` 135); tests, gains.
  Done (D193): `pow` 393 → 133 and `cbrt` 132 → 90 on armv7e-m-dp; at most
  0.5498 ulp.

### P11.05g1 — `pow` on the soft-double cores  `[x]`
- [x] Over P11.05c's and e's kernels with a 128-bit product (AOR: `pow` 3576
  and 12,178); tests, gains. Done (D194): 11,531 → 923 on armv7e-m and 31,522
  → 3938 on armv6-m; at most 0.5029 ulp.

### P11.05g2 — `cbrt` on the soft-double cores  `[x]`
- [x] In 64-bit fixed point, the final rounding honouring the caller's
  direction (`conformance`'s Q36 cases); tests, gains. These cores' `<fenv.h>`
  has `FE_TONEAREST` alone (D44), so the Q36 cases do not run there and to
  nearest is the caller's direction (D195). Every sampled result correctly
  rounded; 3765 → 296 instructions on armv7e-m, 14,702 → 1230 on armv6-m.

P11.05g was split into g1 and g2 (CLAUDE.md workflow): `pow` needs log2 x to
about 2^-72 relative (|y log2 x| up to 1075), a multi-word design of its own;
`cbrt` shares none of it.

### P11.05h — `sin`, `cos`, `sincos`, `tan` on the double-FPU cores  `[x]`
- [x] Cody–Waite in double and the polynomials without the double-double
  finish; the strict reduction beyond; tests, gains. A reduction by π/32 and
  a table of exact pairs (D196): at most 0.5063 ulp below 2^20; `sin` 165 →
  96 instructions on armv7e-m-dp.

### P11.05i — `sin`, `cos`, `sincos`, `tan` on the soft-double cores  `[x]`
- [x] D136's kernel without its rounding test, the strict path from 2^20;
  tests, gains. D197: at most 0.5049 ulp; `cos` 1067 → 755 instructions on
  armv7e-m, `sincos` 1390 → 1065.

### P11.05j — `asin`, `acos`, `atan`, `atan2` on the double-FPU cores  `[x]`
- [x] In double; tests, gains. D198: at most 0.5288 ulp; `atan2` 360 → 141
  instructions on armv7e-m-dp, `asin` 220 → 135.

### P11.05k1 — `atan`, `atan2` on the soft-double cores  `[x]`
- [x] 64-bit fixed point: one kernel for atan(y/x), y <= x, with a 64-bit
  quotient (P11.04f1's, widened); tests, gains. D199: at most 0.5093 ulp;
  `atan` 7741 → 523 instructions on armv7e-m, 21,175 → 2542 on armv6-m.

### P11.05k2 — `asin`, `acos` on the soft-double cores  `[x]`
- [x] 64-bit fixed point, with a 64-bit square root (P11.04f2's, widened);
  tests, gains. D200: at most 0.5049 ulp; `asin` 13,029 → 661 instructions
  on armv7e-m, 34,625 → 3293 on armv6-m.

P11.05k was split into k1 and k2 (CLAUDE.md workflow), as P11.04f was: each
half needs its own fixed-point building block, a 64-bit quotient for the
arctangent and a 64-bit square root for the half-angle forms.

### P11.05l — The hyperbolic functions on the double-FPU cores  `[x]`
- [x] Over P11.05b's and d's evaluations; tests, gains (D201; the note, §19):
  sinh, cosh and tanh over one kernel on P11.05b's table, asinh, acosh and
  atanh as the logarithm of a pair over P11.05d's; at most 0.5493 ulp
  (`sinh`, `cosh` above 22, exp's own error) and 0.52 for the rest.

### P11.05m — The hyperbolic functions on the soft-double cores
Split, as P11.04g was: the two halves rest on different kernels.

### P11.05m1 — `sinh`, `cosh`, `tanh` on the soft-double cores  `[x]`
- [x] Over P11.05c's exp kernel (e^|x| and e^-|x| on one integer scale, a 64-bit
  quotient for tanh); tests, gains (D202; the note, §20): at most 0.5083 ulp;
  `tanh` 7193 → 500 instructions on armv7e-m.

### P11.05m2 — `asinh`, `acosh`, `atanh` on the soft-double cores  `[x]`
- [x] Over P11.05e's log kernel and P11.05k2's square root; tests, gains
  (D203; the note, §21): at most 0.5229 ulp; `asinh` 15,215 → 535
  instructions on armv7e-m.

### P11.05n — `erf` on the double-FPU cores  `[x]`
- [x] (AOR: 69 instructions on armv7e-m-dp); tests, gains (D204; the note,
  §22): 418 → 68 instructions on armv7e-m-dp, at most 0.5264 ulp.

### P11.05o — `erf` on the soft-double cores  `[x]`
- [x] (AOR: 8263 on armv7e-m, 25,491 on armv6-m); tests, gains (D205; the
  note, §23): 12,171 → 518 instructions on armv7e-m and 40,655 → 2049 on
  armv6-m, at most 0.5044 ulp.

## P11.06 — Comparison and wrap-up
Split in four, as the size check needs the maintainer's decision (D191) before
the docs and the phase exit; then in eight (D207), then in twelve (D211).

### P11.06a — Comparison against AOR and picolibc  `[x]`
- [x] `bench-runner compare` and `bench-runner aor` with the fast build as
  reference on every Arm core; sampled accuracy of the fast build against
  AOR's (P4.05's method); `docs/perf/phase-11.md`. The comparison builds
  leave out the C23 functions picolibc and newlib-nano lack (D206).
- [x] Exit check: on armv7e-m the fast build's maths throughput at or below
  picolibc's; at or below AOR's on every case AOR implements, on every Arm
  core; any miss recorded as a Q*. armv7e-m: picolibc takes 1.75 times the
  fast build's instructions (geometric mean, 132 cases); 22 cases are slower
  (Q54). AOR: no case slower on armv6-m, armv6s-m, armv7-m, armv7e-m and
  armv8-m.main (AOR takes 4.3–6.1 times our instructions); on armv7e-m-dp 11
  float functions are slower, AOR 0.55–0.96 of ours (Q55, for P11.06b).

The maintainer's decisions on P11.06a's results (D207) add four sub-steps
before the size check: Q55's armv7e-m-dp gap to AOR (b, c), Q53's selective
inlining (d) and a reduction of armv7e-m-dp's `math-mix` (e), whose figures
go back to the maintainer before the re-lock (f). P11.06a's Q55 said the
double-FPU core used armv7e-m's float-float files; it uses the double path
of `libc/math/fast/`, shared with x86_64, so Q55 is closed by making that
path faster rather than by a new layer (D207).

### P11.06b — The double path's float exp, log and pow (Q55)  `[x]`
- [x] `libc/math/fast/` `expf`, `exp2f`, `exp10f`, `logf`, `log2f`,
  `log10f`, `powf` (the double-FPU cores, x86_64 and armv7e-m-dp): one
  integer test for every special argument, the logarithms table-driven
  without a division; at or below AOR's instruction counts on armv7e-m-dp,
  inside the fast contract (exhaustive sweeps; `powf` sampled), the
  research note updated (§18, D208). armv7e-m-dp: `expf` 52 → 42 (AOR 42),
  `exp2f` 50 → 39 (39), `logf` 81 → 45 (46), `log2f` 81 → 45 (48),
  `log10f` 83 → 47 (50), `powf` 159 → 86 (88); at most 0.5122 ulp.
  `math-float` and `math-mix` +280 and +272 bytes (the table), re-locked
  with the maintainer's approval.

### P11.06c — The double path's `erff`, `sinf`, `cosf`, `sincosf` (Q55)  `[x]`
- [x] The same for the rest of AOR's float functions on the double path;
  at or below AOR's counts on armv7e-m-dp, or the remaining gap recorded.
  D209: one integer test each, two parts of π/2 and shorter fitted
  polynomials for the trigonometric floats, `erff` in eight pieces without
  an exponential. armv7e-m-dp: `sinf` 58 → 43 (AOR 50.5), `cosf` 50 → 43
  (47.2), `sincosf` 84 → 67 (80.8), `erff` 110 → 65 (67.1); at most 0.5095
  ulp. With P11.06b, every function AOR provides is at or below its count
  on every Arm core (P11.06a's exit check).

### P11.06d — Selective inlining in `dp-fixed/fast` (Q53)  `[x]`
- [x] `[[gnu::always_inline]]` copies of the high product on the hot paths,
  kernel by kernel, measured against the fast budgets; keep only what fits
  (D207). D210: on the Thumb-2 cores, the atan series, the log core, `pow`'s
  logarithm and the exp2 fraction; armv7e-m `exp` 316 → 261, `log` 334 →
  295, `pow` 923 → 817, `atan2` 524 → 488; `math-mix` +1.1 KB, within the
  locks. Armv6-M keeps the calls.

### P11.06e — Reducing armv7e-m-dp's `math-mix`  `[x]`
- [x] Measure the bytes saved and the instructions lost for a shared
  out-of-line trig kernel and slow paths that do not pull the strict cores;
  the figures go to the maintainer, who decides the D191 line for that core
  (D207). `docs/research/libm-fast-double.md` §25: a shared `sin`/`cos`
  kernel saves 272 bytes for +19 instructions each (prototyped, reverted);
  the trigonometric, `atan2` and `exp`/`pow` slow paths without the strict
  cores about 450, 1,600 and 1,400 bytes at no cost on the common path; a
  64-entry log table 1,536 bytes for about +2 instructions in `log` and
  `pow`. All four would leave about 15,300 bytes (+26 %); none reaches +20 %.
  The maintainer chose all four and a line of the core's own (D211).

The maintainer's decision on P11.06e (D211) adds four sub-steps before the
size check, each with its sweeps and gates: the slow paths without the strict
cores (f, g, h) and the smaller log table (i). The size check, the docs and
the exit follow (j, k, l).

### P11.06f — The trigonometric slow path on the fast kernels (D211 B)  `[x]`
- [x] `__libc_fast_trig_slow`: from 2^20 and next to a zero, the strict
  reduction (`__libc_rem_pio2`, its r as a pair) and the fast kernels in
  place of `__libc_sin_core` and `__libc_cos_core`; inside the fast
  contract (sampled sweeps at large arguments and next to the zeros), bytes
  recorded. D212: polynomials in r with exact leading terms (the common
  path's table kernel saved less); `sin` 0.5918, `cos` 0.5830, `tan` 0.6489
  ulp on 10^8 samples; armv7e-m-dp `math-mix` −200 bytes, `math-float` −256.

### P11.06g — `atan2`'s slow path by scaling (D211 C)  `[x]`
- [x] `__libc_fast_atan2_slow` without the strict `atan2` core: subnormal
  coordinates and exponents more than 60 apart scaled into the fast path's
  domain or answered from the quotient; inside the fast contract, bytes
  recorded. D213: at most 0.5010 ulp on 4·10^8 slow-path pairs; `math-mix`
  armv7e-m-dp 20,112 → 18,758, the soft-double cores −2,560 to −2,720.

### P11.06h — `exp`'s and `pow`'s ends on the fast kernel (D211 D)  `[x]`
- [x] The results next to overflow and underflow (`__libc_fast_exp_special`,
  `pow`'s ends) by the fast exponential with a two-step scaling, without
  `__libc_exp_core`, its table and `__libc_exp_scale`; inside the fast
  contract, bytes recorded. D214: `exp`, `exp2` and `exp10`'s special paths
  (`pow`'s ends already were); `__libc_fast_exp_scale`'s subnormal results
  rounded once (`pow` 0.7683 → 0.5483 ulp there); `math-mix` armv7e-m-dp
  18,758 → 17,520, the soft-double cores −744 to −864.

### P11.06i — A 64-entry double log table (D211 E)  `[x]`
- [x] `__libc_fast_log_table` halved (1,536 bytes), the polynomial one term
  longer, r = z·invc − 1 still exact (a new grid for invc, or a correction
  term); `log`, `log10`, `log1p` and `pow` inside the fast contract, the
  instruction cost recorded. D215: both double log tables at 64 entries, 1
  half way into its entry and each invc chosen for the smallest
  |r|³/|log z| (`pow` 0.6045 ulp with the old placement, 0.5547 now; `log`
  0.5063); armv7e-m-dp `log` 66.69 → 65.69 instructions, `log2` and `pow`
  unchanged, the rest +2, all below AOR; `math-mix` 17,520 → 16,032.

### P11.06j — Size  `[x]`
- [x] Size: fast budget held, members recorded; `docs/size/`. After the final
  re-lock (D187), compare every core's fast `math-mix` and `math-float` with
  the fast flavour's original rule, +20 % over the speed figure (D191): a core
  still over it is the maintainer's decision before the phase exit, not
  re-locked silently. At P11.05d: armv7e-m-dp `math-mix` +54.5 %
  (12,174 → 18,806 bytes); armv6-m +11.3 %, armv7-m +12.3 %, armv7e-m and
  armv8-m.main +16.2 % and +16.1 %. At P11.05o: armv7e-m-dp `math-mix` +65.3 %
  (12,174 → 20,128); the soft-double cores +10.5 % to +13.5 %; `math-float`
  at most +3.3 %. The figures after P11.06b–i; armv7e-m-dp's `math-mix` gets
  a line of its own at its figure after P11.06f–i (D211).
  D216: every core re-locked at its final fast figures; `math-mix` +3.2 % to
  +5.8 % over speed on the soft-double cores, `math-float` at most +3.3 %;
  armv7e-m-dp's `math-mix` +31.7 % (16,032 bytes), locked as its own line
  (D211; estimated there at about 15,300). No core needs a new decision.

### P11.06k — Docs and the release archives  `[x]`
- [x] README (Performance table gains a fast column; the accuracy
  paragraph states the fast contract), `docs/configuration.md`,
  `docs/release.md`: `release-package` adds the `fast` tree to each archive
  (D163), once the exit check above passes.
  Done: the README's build types, accuracy table (the fast contract) and
  Cortex-M4F performance table (fast: maths 0.25 of speed's instructions,
  every other program 1.00); `docs/configuration.md`'s build types;
  `release-package` and the release workflow package `release`, `speed` and
  `fast` (tested on armv7e-m: three trees, fast's `libc.a` its own).

### P11.06l — Phase exit  `[x]`
- [x] Phase exit: PLAN's phase table, decisions, open questions. D217: the
  exit criterion met point by point (armv7e-m against picolibc on the
  aggregate, the 22 slower cases Q54; AOR at or below on every Arm core;
  the size budget at D216's figures); open for later: Q54, and two defects
  of the strict library seen on the way, Q56 (double `sin` and `tan` beyond
  2^20) and Q57 (`erf` below 2^-28). PLAN's phase table and index unchanged
  in wording: the criterion stands as written.

Phase 11's exit criterion was met at P11.06l (D217). The maintainer then
reopened the phase for Q54 (D218): P11.07.

---

## P11.07 — Q54: the fast cases still slower than picolibc (D218)  `[~]`

On armv7e-m, 22 fast maths cases take more instructions than picolibc's
(P11.06a). Two groups, one sub-step per family, armv7e-m and armv8-m.main
(the single-precision FPU cores) first:
- **the functions phase 11 left strict:** `sqrt`, `hypot`, `hypotf`,
  `erfc`, `erfcf`, `lgamma`, `lgammaf`, `tgamma`, `tgammaf` (1.4–5.7 times
  picolibc's instructions);
- **the float-float fast files on the single-precision FPU:** `tanf` (216
  against 96), `sinf` (128/101), `cosf` (123/100), `atanf` (92/65), `cbrtf`
  (81/53), `expm1f`, `log1pf`, `asinf`, `acosf`, `atan2f` and the inverse
  hyperbolic floats (4–21 %). They keep about 0.5 ulp where picolibc
  computes in float at about 1 ulp.

Phase 11's rules hold: `fast` changes alone, debug, release and speed
byte-identical, and every sub-step meets the definition of done at the top
of this file. The fast contract widens only as Q58 and Q59 decided (D219, D220). `sqrt`
stays correctly rounded (C23 F.3, IEEE 754), so it can only be made faster
in `fast` while staying exact.

**Exit:** on armv7e-m, no fast maths case slower than picolibc's but those
the maintainer excludes; every Arm core still at or below AOR's count for
every function AOR implements; the fast size budget held (D187's re-lock,
armv7e-m-dp's own line, D211).

### P11.07a — Research and decisions  `[x]`
- [x] The strict-only functions: what a fast `hypot`/`hypotf`, `erfc`/`erfcf`,
  `lgamma`/`lgammaf` and `tgamma`/`tgammaf` would cost and gain on every
  core (armv7e-m and armv8-m.main first), each against picolibc's count and
  bound; whether a correctly rounded `sqrt` can be faster in `fast` (the
  double one runs on the soft-double helpers there).
- [x] The single-precision FPU's float functions (`sinf`, `cosf`, `sincosf`,
  `tanf`, `atanf`, `asinf`, `acosf`, `atan2f`, `cbrtf`, `expm1f`, `log1pf`,
  `asinhf`, `acoshf`, `atanhf`): evaluation in float within the bound (1
  ulp; 1.5 for `tanf` and the hyperbolic functions), and the instruction
  counts each target (0.55, 0.75, the bound) reaches against picolibc's.
- [x] `docs/research/fast-math.md` (the contract as it may widen) and
  `libm-fast-float-trig.md`, `libm-fast-float-exp-log.md`; the maintainer's
  answers to Q58 and Q59, recorded as D*; the sub-steps below adjusted to
  them (split, dropped or added). Done: `fast-math.md` §12 (the 22
  functions measured; musl's and picolibc's methods; prototypes: `sinf` 0.86
  ulp in about 50 instructions, `hypotf` correctly rounded in about 30, a
  cheap `tanf` 2.7 ulp); D219 (all the strict-only functions, `sqrt` and
  `hypot`/`hypotf` correctly rounded, the rest at 1 ulp, `lgamma`'s zeros
  by an absolute error) and D220 (a 0.9-ulp target on the
  single-precision FPU); every sub-step below kept.

### P11.07b — `hypot`, `hypotf`  `[x]`
- [x] Fast files, correctly rounded (D219): `hypotf` in float with exact
  squares and one root correction (`sp-nearest/fast/`), `hypot` in fixed
  point on the soft-double cores (`dp-fixed/fast/`) and in double on the
  double-FPU cores where it beats the strict file; every test unchanged;
  tests, gains. (The steps below that relax a function add its entry to
  `test/unit/include/fast_contract.h` and `fast-math.md` §4, and fast
  expectation files where a strict test checks the last bit.)
  D222: correctly rounded in all three (float, double, fixed point), exact
  midpoint tests, ties to even; 0 misrounded in 10^10 and 2·10^9 samples
  and every Pythagorean tie; armv7e-m `hypot` 5835 → 346 instructions
  (picolibc 1443), `hypotf` 161 → 52.4 (53.9). Q61: the soft-float cores'
  `hypotf` and armv7e-m-dp's `hypot` stay above picolibc.

### P11.07c — `erfc`, `erfcf`  `[~]`
Split: the float function rests on an existing kernel, the double needs new
ones (fixed point on the soft-double cores, double on the double-FPU cores).

### P11.07c1 — `erfcf` at 1 ulp  `[x]`
- [x] `sp-nearest/fast/erfcf.c` without its rounding test and float-float
  fallback (D219: `erfcf` gives up D178's correct rounding in `fast`); the
  fast contract's `erfc` entry (1 ulp, `erfc` and `erfcf`); tests, gains.
  D223: `erff`'s pair kernel from 1/2, 1 − erf below; every float within
  0.8018 ulp; armv7e-m 233.3 → 131.3 instructions (picolibc 168.75); the
  accurate kernel and two tables (676 bytes) removed.

### P11.07c2 — `erfc` at 1 ulp  `[x]`
- [x] The double `erfc` in fixed point on the soft-double cores and in
  double on the double-FPU cores, at 1 ulp (D219); as P11.07b.

### P11.07m — The strict `sin`, `cos`, `tan` below 0.53 ulp (bug fix)  `[x]`
- [x] Report: `sin(0x1.bd0a14269acdfp+707)` 0.593 ulp, `tan(0x1.f3605e569c782p+525)`
  0.606 ulp, 0.3–0.4 % misrounded beyond 2^20, against README's 0.53 ulp. Cause:
  the cores, not Payne–Hanek, and below 2^20 too (D228,
  `docs/research/trig-cores.md`). New cores (cos: half angle in release, series in
  `libc/math/speed/`); `regression_trig_cores`; sampled
  sweep with none above 0.53 ulp (worst 0.5006); size and bench rebaselined (growth and slowdown accepted).

### P11.07d — `lgamma`, `lgammaf`  `[x]`
At 1 ulp, and an absolute error (2^-60 double, 2^-28 float) within 2^-6 of
a zero (D219); the poles and `signgam` as in speed; as P11.07b. Split, as
P11.07c was: one function and core family a step.

### P11.07d1 — the double `lgamma` on the double-FPU cores  `[x]`
- [x] `libc/math/fast/lgamma.c` in double (x86_64, armv7e-m-dp; the
  soft-double cores run it on their helpers until d2).

### P11.07d2 — the double `lgamma` in fixed point  `[x]`
- [x] `libc/math/dp-fixed/fast/lgamma.c` on the soft-double cores.

### P11.07d3 — `lgammaf`  `[x]`
- [x] On the single-precision FPU cores (`sp-nearest/fast/`), and in
  double on the double-FPU cores.

### P11.07e — `tgamma`, `tgammaf`  `[x]`
As P11.07b; overflow, underflow and the poles as in speed. Split as P11.07d.

### P11.07e1 — the double `tgamma` on the double-FPU cores  `[x]`
- [x] `libc/math/fast/tgamma.c` in double (x86_64, armv7e-m-dp; the
  soft-double cores run it on their helpers until e2).

### P11.07e2 — the double `tgamma` in fixed point  `[x]`
- [x] `libc/math/dp-fixed/fast/tgamma.c` on the soft-double cores.

### P11.07e3 — `tgammaf`  `[x]`
- [x] On the single-precision FPU cores, and from the fast `tgamma`
  elsewhere.

### P11.07f — `sqrt` in `fast`, still correctly rounded  `[x]`
- [x] Fixed point on the soft-double cores (`dp-fixed/fast/`): the rsqrt
  seed and Newton steps, the rounding by an exact integer test (D219);
  correctly rounded, every test unchanged; gains.

### P11.07g — `sinf`, `cosf`, `sincosf`, `tanf` on the single-precision FPU  `[x]`
- [x] `libc/math/sp-nearest/fast/` (the fast layer armv7e-m and
  armv8-m.main share) at D220's 0.9-ulp target, `tanf` at its 1.5 bound;
  every float checked; tests, gains. Done (D234): 0.6493, 0.6473 and
  0.9974 ulp; armv7e-m `sinf` 128.3 → 82.3, `cosf` 123.3 → 79.3,
  `sincosf` 212.3 → 133.3, `tanf` 216.3 → 90.3 (picolibc 101, 100, 214, 96).

### P11.07h — `atanf`, `asinf`, `acosf`, `atan2f` on the single-precision FPU  `[x]`
- [x] As P11.07g. Done (D235): 0.6501, 0.7684, 0.7430 and 0.6533 (sampled)
  ulp; armv7e-m `atanf` 92.3 → 60.4, `asinf` 64.3 → 53.8, `acosf` 63.3 →
  48.6, `atan2f` 118.4 → 91.9 (picolibc 65, 57, 50, 109).

### P11.07i — `cbrtf`, `expm1f`, `log1pf` on the single-precision FPU  `[x]`
- [x] As P11.07g. Done (D237): 0.7503, 0.8432 and 0.7548 ulp; armv7e-m
  `cbrtf` 81.3 → 50.3, `expm1f` 78.4 → 52.3, `log1pf` 96.3 → 77.5
  (picolibc 53, 61, 82).

### P11.07j — `asinhf`, `acoshf`, `atanhf` on the single-precision FPU  `[x]`
- [x] As P11.07g. Done (D238): 0.7536, 0.7421 and 0.7538 ulp (bound 1.5);
  armv7e-m `asinhf` 135.3 → 98.7, `acoshf` 125.3 → 101.3, `atanhf` 111.3 →
  102.3 (picolibc 112, 106, 107).

### P11.07k — Comparison, size and docs  `[ ]`
- [ ] P11.06a's comparison again (`bench-runner compare` and `aor`, every
  Arm core, the fast build as reference) and `aor-accuracy`;
  `docs/perf/phase-11.md`; the exit check above, any case still slower
  recorded.
- [ ] Size: the fast budget re-locked at the new figures (D187), D191's line
  checked as at P11.06j; `docs/size/`.
- [ ] README (the fast row and the Cortex-M4F table), `docs/configuration.md`,
  `fast-math.md` §4.

### P11.07l — Phase exit  `[ ]`
- [ ] Phase exit: PLAN's phase table, decisions, open questions; Q54
  closed or what is left of it recorded.
