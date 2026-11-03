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

## P8.01 — Research  `[x]`
- [x] `docs/research/sincos.md`: how glibc, FreeBSD msun and ARM optimized-routines compute `sincos`/`sincosf` (one reduction, both kernels, the quadrant swapping outputs and signs; musl and CORE-MATH were outside the session's network allowlist and are described from their published structure); where our reductions live per path, and how each can feed a shared kernel (double: `__libc_rem_pio2` + both double-double cores, bit-identical by construction; `sp-nearest`: `__libc_sincosf_core` exists but its entry tests differ from `sinf`'s; the double path's float functions need their hard-case tables shared; `nofp/speed`: `__libc_sinf_fixed(k)` and `(k + 16)`).
- [x] Fixed-point double sine and cosine: Taylor remainders on |r| ≤ π/4 (9 terms in z reach 2^-63 for sin r / r; a minimax 8-term polynomial about 2^-69), the table alternative (jπ/32, 144 bytes, 5 terms, best for `sincos`), relative accuracy near zero through sin r / r, a 128-bit fixed-point Cody–Waite reduction with the Payne–Hanek words beyond it, a rounding test with about 0.8 % fallback, and multiply costs (about 60 instructions per Horner step on Armv6-M, 10 with `UMULL`). Estimated `sin` ≈ 1,000 instructions on armv6-m and ≈ 260 on armv7e-m. CORDIC: about 1,700–2,000 and 600 for both results, 0.56 ulp absolute error, unbounded relative error near zero: not pursued.
- [x] Bench cases `math/sincos` and `math/sincosf` (sin + cos of each argument) in both flavours, recorded with `make bench BENCH_UPDATE=1`; no other case changed. `sincos` = `sin` + `cos` on every architecture (armv6-m 35,340, armv7e-m 12,864, armv7e-m-dp 344, x86_64 635 instructions per call).

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
