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
audit of the strict-contract tests, and P11.02b, that audit: eight tests of
about 2,500 lines whose fast expectations each need their own reading.

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
  reference column. The release and speed baselines predate P10.07's
  `threads` cost, so `make bench` fails there as before (Q52, not blocking).
- [x] Size: the fast flavour in `docs/size/budget.json` under the speed
  flavour's rule (D73); `size-report --build-type fast budget --flavour fast`.
- [x] CI (D119): `make check` covers `fast` through `BUILDS`; the Arm size
  job adds the fast budget; note the 33 % longer `make check` against Q33.
- [x] Release (D158, D163): the workflow and `release-package` leave the
  `fast` tree out until P11.06; `docs/release.md` says so.

### P11.02b — The strict-contract tests' fast expectations  `[ ]`
- [ ] The audit of every strict-contract test (`float_hard`,
  `float_accuracy`, `libm_dense`, `libm_exp`, `libm_trig`, `libm_special`,
  `fpu_math`, `float_exact`) with a `fast/` expectation file where one is
  needed (D163): every check about the last bit or a directed result of a
  relaxed function moves into the file, every other check (special values,
  exceptions, ranges, symmetry) stays in the shared source; the strict
  files unchanged in debug, release and speed. The audit's findings go into
  `docs/research/fast-math.md`.
- [ ] `make check` in all four build types; with no `fast/` libm file the
  fast expectations pass as the strict ones do.

## P11.03 — Float exp, log and pow  `[ ]`
- [ ] Research: `docs/research/libm-fast-float-exp-log.md` (a 2^(j/N) table
  and polynomial evaluated in float on the SP FPU cores and in double on the
  DP ones, fixed point on armv6-m/armv6s-m/armv7-m where it already wins;
  `logf` by an inverse table; `powf` with log x to about 2^-34; error
  bounds proved per path).
- [ ] `expf`, `exp2f`, `exp10f`, `expm1f`, `exp2m1f`, `exp10m1f`, `logf`,
  `log2f`, `log10f`, `log1pf`, `log2p1f`, `log10p1f`, `powf`, `cbrtf` in
  `fast/` files per layer; no hard-case tables, no float-float, no mode
  switching.
- [ ] Tests at the fast bounds and the special-value and threshold cases of
  `fast-math.md` §8; gains and members recorded.

## P11.04 — Float trigonometric, inverse and hyperbolic functions  `[ ]`
- [ ] Research: `docs/research/libm-fast-float-trig.md` (Cody–Waite in float
  or double per core for small |x|, the D136 integer reduction above,
  polynomial degrees for 1 ulp; `atanf`/`atan2f`/`asinf`/`acosf`;
  hyperbolic functions over the fast `expf`/`log1pf`; `erff`).
- [ ] `sinf`, `cosf`, `sincosf`, `tanf`, the `pi` forms, `asinf`, `acosf`,
  `atanf`, `atan2f`, `sinhf`, `coshf`, `tanhf`, `asinhf`, `acoshf`,
  `atanhf`, `erff`.
- [ ] Tests, gains and members recorded.

## P11.05 — Double functions  `[ ]`
- [ ] Research: `docs/research/libm-fast-double.md`: on the soft-double cores
  (armv6-m, armv6s-m, armv7-m, armv7e-m, armv8-m.main) whether each
  function is cheaper as an AOR-style double evaluation on the soft helpers
  or as a 64-bit fixed-point kernel (D136's style) with one final rounding
  and no fallback; on the FPU cores (armv7e-m-dp, x86_64) a double-only
  evaluation without the dd finish.
- [ ] `exp`, `exp2`, `exp10`, `expm1`, `log`, `log2`, `log10`, `log1p`,
  `pow`, `cbrt`, `sin`, `cos`, `sincos`, `tan`, `asin`, `acos`, `atan`,
  `atan2`, the hyperbolic functions and `erf`, per core; split this step per
  family if it is too big (CLAUDE.md workflow).
- [ ] Tests, gains and members recorded.

## P11.06 — Comparison and wrap-up  `[ ]`
- [ ] `bench-runner compare` and `bench-runner aor` with the fast build as
  reference on every Arm core; sampled accuracy of the fast build against
  AOR's (P4.05's method); `docs/perf/phase-11.md`.
- [ ] Exit check: on armv7e-m the fast build's maths throughput at or below
  picolibc's; at or below AOR's on every case AOR implements, on every Arm
  core; any miss recorded as a Q*.
- [ ] Size: fast budget held, members recorded; `docs/size/`.
- [ ] README (Performance table gains a fast column; the accuracy
  paragraph states the fast contract), `docs/configuration.md`,
  `docs/release.md`: `release-package` adds the `fast` tree to each archive
  (D163), once the exit check above passes.
- [ ] Phase exit: PLAN's phase table, decisions, open questions.
