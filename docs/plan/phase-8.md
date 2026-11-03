# Phase 8 — Faster sine and cosine

Prerequisite: PLAN D133; P7.03 (`sincos` and `sincosf` over `sin` and `cos`);
D77 (speed-flavour files); D86 and P3.05g (`docs/research/libm-fixed.md`, the
fixed-point float functions); D124 (the libm vectors).
Targets: every architecture in `ARCHS`.

**Goal:** two speedups measured in the P7.03 follow-up:

1. `sincos` and `sincosf` reduce the argument once and evaluate both
   polynomials, instead of calling `sin` and `cos`, which reduce it twice;
2. double `sin`, `cos` (and with them `sincos` and `tan`) in 64-bit fixed
   point on the cores without a double-precision FPU (armv6-m, armv6s-m,
   armv7-m, armv7e-m, armv8-m.main). Their soft-double path takes
   6,876 (armv7e-m) to 17,710 (armv6-m) instructions per call. This is the
   double-precision counterpart of P3.05g.

CORDIC was considered and rejected (D133): slower than the polynomials on the
double-FPU cores, slower than a fixed-point polynomial on the others, and
short of the 0.53-ulp bound without arithmetic wider than 64 bits.

**Every step's definition of done (DoD)** is the Phase 1 DoD plus:

- the libm vectors (D124) pass in every rounding direction at the existing
  bounds (double below 0.53 ulp to nearest), and `sincos` and `sincosf` stay
  bit-identical to `sin` and `cos` (`test/unit/posix`);
- `make bench` shows no case slower by more than 1 %, and the step's gains
  are recorded with `make bench BENCH_UPDATE=1` and in `docs/perf/<arch>.md`;
- the size flavour is unchanged (`size-report diff`) unless the step names
  the decision that allows it, and the speed flavour stays within its budget.

---

## P8.01 — Research  `[ ]`
- [ ] `docs/research/sincos.md`: how musl, glibc, FreeBSD msun, CORE-MATH, ARM optimized-routines and Bionic compute `sincos`/`sincosf` (one reduction, both polynomials, the quadrant swap and signs); where our reductions live per layer (`libc/math`, `sp`, `sp-nearest`, `fpv4-sp`, `fpv5-sp`, `fpv5-dp`, `soft-dp`, `nofp`, `x86_64`), and whether each can return its reduced argument and quadrant to a shared kernel.
- [ ] Fixed-point double sine and cosine: polynomial degree and coefficient precision for |r| ≤ π/4 in 64-bit (or 64 + 32-bit) fixed point, an error bound that keeps 0.53 ulp (including results near zero, where `sin` needs relative accuracy), the reduction's output format (from `__libc_rem_pio2`), and the multiply cost per core (`MULS` only on Armv6-M, `UMULL` on Armv7-M and Armv8-M). Record CORDIC's numbers next to it.
- [ ] Bench cases for `sincos` and `sincosf` in `test/bench` (both flavours), measured before any change.

## P8.02 — One reduction for `sincos` and `sincosf`  `[ ]`
- [ ] A kernel per layer that takes the reduced argument and quadrant and returns both results, shared by `sin`, `cos` and `sincos` where that costs no size; otherwise `sincos` alone in the layer's `speed/` directory (D77).
- [ ] Results bit-identical to `sin` and `cos`, so `sincos` keeps its P7.03 guarantee.
- [ ] Bench, size and the notes for every architecture.

## P8.03 — Fixed-point double `sin` and `cos` on the soft-double cores  `[ ]`
- [ ] Split if large: Armv6-M (`MULS` only) and Armv7-M/Armv8-M (`UMULL`) may need separate steps.
- [ ] `sin`, `cos`, `sincos` (and `tan`, if the same kernel serves it) in 64-bit fixed point, in the speed flavour's `speed/` files of the soft-double layers (`nofp`, `soft-dp`), keeping the soft-double path for the size flavour unless the fixed-point version is also smaller.
- [ ] The error bound measured on the host over at least 10⁸ random arguments plus the D124 vectors, hard cases in `libm_dense`, and a mutant check that the tests catch a lost guard bit.
- [ ] Bench (target: about 10× on armv7e-m, more on armv6-m), size and the notes.

## P8.04 — Wrap-up  `[ ]`
- [ ] `docs/perf/<arch>.md` summary, PLAN decisions, and the speed budgets re-locked where a decision allows it.
