# Phase 12 — The strict library's two accuracy defects: Q57 and Q62

Prerequisite: PLAN D221 (this phase), D241 (which moved Q57 and Q62 here;
Q56, the plan's first subject, was resolved by D228 in P11.07m), D251 (phase
11's exit), D47, D48 and D125 (the strict double accuracy, below 0.53 ulp),
D204 (the fast `erf`, where Q57 was seen), D225 and D226 (the fast `lgamma`'s
recurrence near the negative zeros, where Q62 was seen), D72 (bench).
Research notes: `docs/research/libm-special.md` (the strict files),
extended by each step.
Targets: every architecture in `ARCHS`.

**Goal:** bring the strict double `erf` and `lgamma` back to the README's
figures everywhere.
- **Q57:** the strict double `erf` takes x·2/√π alone below 2^-28 and drops
  0.376·x² of the result (about 0.53 ulp, P11.05n). Keep the next term.
- **Q62:** the strict `lgamma` (`__libc_lgamma_core.c`) is about 2^-57 off
  absolutely near the negative zeros (thousands of ulps of the tiny results
  there) and misrounds lgamma(−2.5). The fast files' recurrence (D225) fixes
  it in `fast` only. Fix it in the strict core.

**Unlike phase 11, the strict files change**, so debug, release, speed and
(where it shares them) fast change too:
- release and speed figures may move. Each step shows `size-report diff`
  and the bench diff against the release and speed baselines, and any growth
  is re-locked only as Q60 decides, recorded as a D*;
- accuracy may only get better: every strict test keeps its bound.

**Out of scope:** the complex functions; long double beyond sharing the
double functions (D7); the fast files, except where they share a strict file.

**Every step's definition of done:**
- `make` and `make check` on every architecture in all four build types,
  libgcc-check clean; the libm vectors at their bounds in every build type;
- the step's cases in `test/unit` (`libm_thresholds`, `libm_special` or
  `libm_vectors`), and a `regression_*` case at Q57's and Q62's worst points;
- a sampled host sweep against quad precision (10^8 arguments plus each
  piece's edges, SSE2 and fma paths), its figures in the research note;
- `make bench` for every build type the step touches, release and speed
  re-locked only as Q60 decides, recorded in `docs/perf/<arch>.md`;
- `size-report diff` and the budgets; `docs/size/<arch>.md`;
- clean-room; one function per file; no `#if`.

---

## P12.01 — Research and decisions  `[ ]`
- [ ] Q57: which build types and cores run the strict `libc/math/erf.c`
  (and `erfl`), its error below 2^-28 measured, the term to keep (x³·p0, as
  `libc/math/fast/erf.c` does), how it is computed without an fma, its cost.
- [ ] Q62: which build types and cores run `__libc_lgamma_core.c` (`lgamma`,
  `lgammal`, and `lgammaf` where it calls the double core); the error near
  the negative zeros and at lgamma(−2.5), measured on samples and at each
  zero to −18 (as D226 checked the fast file) and sampled below; its source (the Stirling series in plain double, the
  reflection's log of sin πx); the fix (D225's recurrence with an exact pair
  product, or the reflection with the error kept as a pair) and its cost in
  bytes and instructions, strict and on the soft-double cores.
- [ ] The maintainer's answer to Q60, recorded as a D*; the later steps
  adjusted to the findings.

## P12.02 — Q57: the strict `erf` below 2^-28  `[ ]`
- [ ] The tiny path keeps x³·p0 in every strict `erf` file P12.01 names;
  below 0.53 ulp there; `regression_erf_tiny` at the worst point; size and
  bench diffs for release and speed, re-locked as Q60 decides.

## P12.03 — Q62: the strict `lgamma` near its negative zeros  `[ ]`
Split at P12.01 by the files the fix lives in.
- [ ] The strict core accurate near every negative zero (the bound P12.01
  sets, at least the fast file's 2^-60 absolutely) and lgamma(−2.5)
  correctly rounded; `regression_lgamma_negative_zero` and
  `regression_lgamma_minus_2_5`; the same for `lgammaf` where it shares the
  core.

## P12.04 — Wrap-up  `[ ]`
- [ ] Size and bench: every re-lock recorded (Q60's D*); `docs/size/`,
  `docs/perf/`.
- [ ] README (the double functions' accuracy as measured),
  `libm-special.md`.
- [ ] Phase exit: PLAN's phase table, decisions, open questions.
