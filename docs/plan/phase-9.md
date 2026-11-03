# Phase 9 — Trigonometric follow-ups to phase 8

Prerequisite: PLAN D136–D138 (the D136 fixed-point kernel), Q41–Q43, D110 (armv7-m's exact speed figures), D86–D88 (the float fixed-point functions).
Targets: the five soft-double cores (armv6-m, armv6s-m, armv7-m, armv7e-m, armv8-m.main), speed flavour.

**Goal:** answer Q41–Q43.
- **Q43:** `UMULL` for armv7-m's float fixed-point functions.
- **Q41:** float `sinf`, `cosf` and `sincosf` through the D136 integer reduction and kernel, rounded to float with a rounding test.
- **Q42:** `tan` (and with it `tanf`) through the same kernel, with a fixed-point quotient.

Q44 (the fallback rate, or a table) is left for the user to decide when this phase ends.

Starting point, speed flavour, instructions per call (`docs/perf`):

| Arch | `sin` | `tan` | `sinf` | `cosf` | `tanf` | `sincosf` |
|---|---:|---:|---:|---:|---:|---:|
| armv6-m | 2,877 | 32,451 | 4,729 | 3,742 | 5,140 | 5,505 |
| armv7-m | 812 | 11,535 | 2,499 | 1,990 | 2,596 | 3,055 |
| armv7e-m | 803 | 11,198 | 575 | 567 | 794 | 786 |

**Every step's definition of done (DoD)** is phase 8's:
- the D124 vectors at their bounds;
- `libm_dense` and the `sincos`/`sincosf` identity tests;
- `make check` everywhere;
- `make bench` with no case slower by more than 1 %, and the gains recorded;
- the size flavour unchanged and the speed budgets holding. A re-lock of an exact speed figure (D110) needs its own decision.

---

## P9.01 — Q43: `UMULL` for armv7-m's float fixed-point functions  `[x]`
- [x] nofp/speed's 32 × 32 → 64 multiply: `fixed.h` includes `fixed-wide.h`. `speed.mk` picks `umull/` (inline `UMULL`) on armv7-m and `muls/` (the out-of-line 16-bit helper, which `subdir.mk` builds only there) on Armv6-M.
- [x] armv7-m speed flavour, 24 cases faster and none slower: `powf` 865 → 374, `expf` 280 → 125, `exp2f` 235 → 118, `erff` 2,079 → 1,122, `erfcf` 2,018 → 1,068, `log10f` 466 → 259, `log2f` 391 → 247, `logf` 1,233 → 1,026, `tanf` 2,596 → 2,077, `sincosf` 3,055 → 2,553, `cosf` 1,990 → 1,739, `sinf` 2,499 → 2,248. The speed figures are re-locked downwards under D140 (`math-float` −168, `math-mix` −172 bytes). `make check` passes on armv7-m and armv6-m (speed).

## P9.02 — Q41: `sinf`, `cosf`, `sincosf` through the D136 kernel  `[x]`
- [x] nofp/speed's `sinf`, `cosf` and `sincosf` (armv6-m, armv6s-m, armv7-m) first try the D136 kernel:
  - `__libc_trig_fixed_from_float` builds the double from the bits and admits [2⁻²⁷, 2²⁰);
  - `__libc_trig_fixed_roundf` rounds the 64-bit result to float, declining within 18 units of a halfway point at bit 40;
  - the D87 path stays as the fallback (|x| ≥ 2²⁰, `cosf` below 2⁻²⁷, and any declined result);
  - `sincosf` falls back per result, so it stays bit-identical to `sinf` and `cosf`;
  - armv7e-m and armv8-m.main keep their single-precision FPU path (575 instructions; the kernel takes about 800 there).
- [x] Every float in the kernel's range, checked on the host (`tools/scripts/trig-fixed-check --float`, 9 seconds):
  - 394,264,576 arguments per function, for both `sinf` and `cosf`;
  - none declined and none incorrectly rounded;
  - 1,077,017 results within 2³⁰ units of a halfway point checked against MPFR, with a largest error of 6.5 units.
- [x] Speed flavour, 6 cases faster and none slower:
  - armv6-m: `sinf` 4,729 → 2,968, `cosf` 3,742 → 2,924, `sincosf` 5,505 → 4,459;
  - armv7-m (after P9.01): `sinf` 2,248 → 883, `cosf` 1,739 → 852, `sincosf` 2,553 → 1,234.

  `make check` passes on armv6-m and armv7-m (speed).
- [ ] Size: `math-float` now links the kernel. armv6-m and armv6s-m grow by 2,040 bytes and armv7-m by 1,712, and `math-mix` by about 250. The exact speed figures (D110) are settled once, after P9.03, which changes the same images.

## P9.03 — Q42: `tan` and `tanf` through the D136 kernel  `[ ]`
- [ ] tan r = sin r / cos r (−cos r / sin r in odd quadrants): a 64-bit reciprocal of the divisor's significand by Newton's iteration from a 32-bit seed, then one product. A rounding test, with the error bound measured on the host, and the double-double path as the fallback.
- [ ] `tanf` from the same quotient where P9.02 placed `sinf`.
- [ ] Hard cases in `libm_dense`, a mutant check, and bench.

## P9.04 — Wrap-up  `[ ]`
- [ ] Notes, decisions, and the question of Q44 put to the user.
