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

## P8.02 — One reduction for `sincos` and `sincosf`  `[x]`
- [x] A kernel per path that takes the reduced argument and quadrant and returns both results. The double cores and the float cores were already shared by `sin`/`cos` and `sinf`/`cosf`, so only `sincos` and `sincosf` change, in every flavour:
  - `libc/math/sincos.c`: `sin`'s and `cos`'s entry tests, one `__libc_rem_pio2`, both double-double cores in one round-to-nearest section, the quadrant choosing and negating (before the final sum, as `sin.c` does, so the directed modes round alike);
  - `libc/math/sincosf.c` (double path: armv7e-m-dp, x86_64): the new `sincos`, then each result through `__libc_float_round` with `sinf`'s and `cosf`'s hard cases, now shared (`__libc_sinf_cases`, `__libc_cosf_cases`);
  - `libc/math/sp-nearest/sincosf.c`: one `__libc_rem_pio2f`, `__libc_sinf_core(k)` and `(k + 16)`;
  - `libc/math/nofp/speed/sincosf.c`: the same with the fixed-point core and its rounding test (armv6-m, armv6s-m, armv7-m speed).
- [x] Results bit-identical to `sin` and `cos`, so `sincos` keeps its P7.03 guarantee: `test/unit/posix` compares bits and exception flags against separate calls in every rounding direction the core has, over specials, tiny arguments, every quadrant of each reduction regime, the doubles and floats nearest n π/2 and their neighbours, the float hard cases, and a sweep of 141 binades. A mutant that negates one quadrant's sum instead of its double-double fails it.
- [x] Bench, size and the notes for every architecture. `sincos`: armv6-m −16 %, armv7-m, armv7e-m, armv8-m.main −18 %, armv7e-m-dp −33 %, x86_64 −30 %. `sincosf`: −8 % (size flavour, soft cores) to −35 % (armv6-m speed). No other case changed. `sincos.o` grows to 420–629 bytes, still less than `sin.o` + `cos.o`; no image changes and both budgets hold.

## P8.03a — The fixed-point kernel and its error bound  `[x]`
- [x] `__libc_trig_fixed` (`libc/math/dp-fixed/speed/`): an integer reduction for |x| < 2²⁰ (x times 2/π to 192 bits, the fraction to 128 bits), r = f π/2 to 64 bits, sin r / r and cos r by Taylor polynomials in z = r² in 64-bit fixed point, the quadrant, and a 64-bit significand per result. Only the polynomials the quadrant needs are evaluated. Its only multiply is `__libc_fixed_wide` (32 × 32 → 64), so one source serves Armv6-M and the `UMULL` cores. Arguments ≥ 2²⁰, or whose reduction cancels more than 64 bits, decline. No architecture builds the layer yet (P8.03b).
- [x] The rounding test (`__libc_trig_fixed_round`): round the 64-bit significand to 53 bits when its 11 discarded bits are more than E = 18 units from a halfway point (the soft-double cores round to nearest only, D44); otherwise decline to the double-double path. The double is built from bits.
- [x] `tools/scripts/trig-fixed-check` (with `tools/libm/trig_fixed_check.c`), run on the host against MPFR at 256 bits:
  - 10⁸ arguments (log-uniform over [2⁻²⁷, 2²⁰) and uniform over [−10, 10]) plus about 9 × 10⁵ doubles near n π/2;
  - largest error: sin 7.03 units, cos 7.08 (analytic bound about 16.6, so E = 18);
  - fallback 1.8 %; no argument declined; no fast result incorrectly rounded.

## P8.03b — `sin`, `cos`, `sincos` in the speed flavour  `[x]`
- [x] The `dp-fixed` layer (D136) on the five soft-double cores (armv6-m, armv6s-m, armv7-m, armv7e-m, armv8-m.main), last in `LIBC_MATH_LAYERS`.
  - **Speed files:** `sin.c`, `cos.c` and `sincos.c` apply the entry tests on the bits, call the kernel, and fall back to the double-double path (`__libc_trig_dd`, one reduction, both cores only where needed) for a result the kernel declines or cannot round. In `sincos`, each result falls back alone, so both stay bit-identical to this flavour's `sin` and `cos`.
  - **The multiply:** `__libc_trig_wide` is `UMULL` inline on the Thumb-2 cores (`umull/trig-wide.h`) and nofp's 16-bit product helper on Armv6-M (`muls/trig-wide.h`). `speed.mk` picks the directory, so there is no `#if`.
  - **Build:** the layer's files build at `-O2` (`LIBC_SPEED_CFLAGS`). The generic `libc/math/subdir.mk` now drops a layer's `speed/` files in the speed flavour too.
  - `tan` keeps the double-double path. The kernel's polynomials are sin r / r and cos r, and a quotient of them would need a division and an error analysis of its own.
- [x] Tests:
  - **`libm_dense`:** `near_double.inc` holds 32 arguments (`tools/scripts/trig-fixed-check --near`) whose 64-bit kernel result is within 4 units of a halfway point, rounds wrongly without the test, and is rounded correctly by the double-double path. They are checked through `sin`, `cos` and `sincos` in every build.
  - **The D124 vectors and the `sincos` identity test** (`test/unit/posix`) run unchanged.
  - **Mutants on armv7e-m speed:**
    - with no rounding test, 32 of 32 dense cases fail;
    - with a zero bound, 10 of 32 fail (the 22 others sit exactly on the halfway point, which a zero bound still declines);
    - with the bound halved to 9, none fail, because the measured largest error is 7.08 units;
    - the D124 vectors pass every mutant, since their 0.53-ulp bound cannot see a misrounded tie.

## P8.03c — Bench, size, notes  `[x]`
- [x] Bench, speed flavour (instructions per call). No other case changed, and the size flavour, armv7e-m-dp and x86_64 are unchanged.

  | Arch | `sin` | `cos` | `sincos` |
  |---|---|---|---|
  | armv6-m, armv6s-m | 17,669 → 2,877 (6.1×) | 17,660 → 3,710 (4.8×) | 29,654 → 5,239 (5.7×) |
  | armv7-m | 6,524 → 812 (8.0×) | 6,500 → 1,117 (5.8×) | 10,702 → 1,476 (7.3×) |
  | armv7e-m | 6,435 → 803 (8.0×) | 6,413 → 1,107 (5.8×) | 10,528 → 1,457 (7.2×) |
  | armv8-m.main | 6,435 → 795 (8.1×) | 6,413 → 1,102 (5.8×) | 10,528 → 1,456 (7.2×) |

  The target of "about 10× on armv7e-m" is not reached for `cos` and `sincos`, because one of the bench's 16 arguments falls back. Without it, `cos` costs about 720 on armv7e-m (8.9×). Armv6-M reaches 6×, not "more than armv7e-m": each 64-bit product there is 16 `MULS`. A table-driven variant and a narrower fallback are left to P8.04 to weigh.
- [x] The speed budgets hold on every core. The new members (about 2 KB) are recorded in `docs/size/<arch>.md`, and the size flavour is unchanged.

## P8.04 — Wrap-up  `[ ]`
- [ ] `docs/perf/<arch>.md` summary, PLAN decisions, and the speed budgets re-locked where a decision allows it.
