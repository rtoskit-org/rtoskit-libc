# Research: faster sine and cosine (phase 8)

Plan: PLAN D133, `docs/plan/phase-8.md`. This note is P8.01. It covers two things:

- one argument reduction for `sincos` and `sincosf`;
- a 64-bit fixed-point double `sin` and `cos` for the cores without a double FPU.

Clean-room: the implementations below were read for structure only. No code was copied or translated.

## Sources reviewed

- **glibc** `sysdeps/ieee754/dbl-64/s_sincos.c`
  (<https://github.com/bminor/glibc/blob/master/sysdeps/ieee754/dbl-64/s_sincos.c>):
  - forces round-to-nearest, then branches on the magnitude;
  - tiny arguments return x and 1;
  - below about 0.855, both kernels run on x unreduced;
  - below about 2.43, one subtraction of π/2 split in two parts gives a double-double argument;
  - larger arguments use one reduction (a short one, or `__branred` above about 10⁸) and a quadrant;
  - odd quadrants swap the two output pointers, and the signs come from the quadrant bits;
  - both kernels always run on the same reduced pair.
- **FreeBSD msun** `lib/msun/src/k_sincos.h`
  (<https://github.com/freebsd/freebsd-src/blob/main/lib/msun/src/k_sincos.h>):
  - merges the separate sine and cosine kernels;
  - z = x² and w = z² are computed once and feed both polynomials;
  - the reduced argument's tail y corrects both;
  - cos is assembled as (1 − z/2) plus a correction.
- **ARM optimized-routines** `math/sincosf.c`
  (<https://github.com/ARM-software/optimized-routines/blob/master/math/sincosf.c>):
  - works in double precision;
  - one reduction: none below π/4, `reduce_fast` below 120, `reduce_large` on the bit pattern above;
  - one x²;
  - one call to a shared polynomial helper that gives both results;
  - the quadrant selects a sign table and, by its bit 1, a second coefficient table;
  - quoted error 0.56 ulp. That is not correctly rounded, which our float functions must be (D98).
- **musl** `src/math/sincos.c` and **CORE-MATH** `src/binary32/sincos`: both hosts (`git.musl-libc.org`, `gitlab.inria.fr`) are outside this session's network allowlist and were not read. From the published descriptions, musl follows the msun structure (one `__rem_pio2`, then both `__sin`/`__cos` kernels with a quadrant switch), and CORE-MATH's `sincosf` shares its reduction between its correctly rounded `sinf` and `cosf` paths.
- **Our own float work** (`docs/research/libm-fixed.md`, P3.05g/D87):
  - the fixed-point `sinf`/`cosf` on armv6-m use a 9-entry table of sin and cos of jπ/32;
  - short polynomials in s, |s| ≤ π/64, in 32 and 64-bit fixed point;
  - a rounding test (`__libc_fixed_round`) that falls back to the float-float path when the fixed result is too close to a rounding boundary.

  The double design below scales this up.
- **J.-M. Muller**, *Elementary Functions* (3rd ed., 2016), ch. 6 (table-based reduction) and ch. 11 (rounding tests); **P. T. P. Tang**, table-driven methods (ACM TOMS, 1989–1992); **CORDIC**: J. E. Volder (1959) and Muller ch. 9.

All the surveyed libraries do the same thing for `sincos`: one reduction, both kernels on the same reduced argument, and the quadrant swaps the outputs and sets their signs. None of them uses CORDIC on a machine with a multiplier.

## Where our sine and cosine live

| Function | Path | Used on |
|---|---|---|
| `sin`, `cos`, `tan` (double) | `libc/math/sin.c`, `cos.c`, `tan.c`: `__libc_rem_pio2` (Cody–Waite with 4 parts of π/2 below 2²⁰, integer Payne–Hanek above), then the double-double cores `__libc_sin_core` / `__libc_cos_core`, round-to-nearest forced around the core | every architecture. On armv7e-m-dp and x86_64 the double-double arithmetic is hardware (FMA products on fpv5-dp, D90); elsewhere every operation is a soft-double call |
| `sinf`, `cosf` | `libc/math/sinf.c`, `cosf.c`: the double function, then `__libc_float_round` with a table of hard cases (D98) | armv7e-m-dp, x86_64 (no `sp-nearest` layer) |
| `sinf`, `cosf` | `libc/math/sp-nearest/`: `__libc_rem_pio2f` (float-float), then `__libc_sinf_core(k, s)`, where cos is `__libc_sinf_core(k + 16, s)` | armv7-m, armv7e-m, armv8-m.main (and armv6-m in the size flavour) |
| `sinf`, `cosf`, `tanf` | `libc/math/nofp/speed/`: `__libc_rem_pio2f`, then `__libc_sinf_fixed` with a rounding test, falling back to `__libc_sinf_core` | armv6-m, armv6-m speed flavour (D87) |
| `sincos`, `sincosf` | `libc/math/sincos.c`, `sincosf.c` (P7.03): `sin` + `cos`, `sinf` + `cosf` | every architecture |

Each path can return its reduced argument and quadrant to a shared kernel:

- **Double.** `__libc_rem_pio2` already returns (quadrant, double-double r), and `sin` and `cos` differ only in the quadrant switch. `sincos` = one reduction, one mode switch, `__libc_sin_core(r)` and `__libc_cos_core(r)`, with the quadrant choosing which result goes where and its sign. Because the cores and their inputs are identical, the results are bit-identical to `sin` and `cos` by construction. The two cores also share x², the double-double square and r.lo's correction: an msun-style merged core could save those, but would have to keep the same rounding to stay bit-identical. So P8.02 keeps the two cores and only shares the reduction.
- **Float, `sp-nearest`.** `__libc_sincosf_core` already exists (one reduction, `__libc_sinf_core(k)` and `(k + 16)`), and the complex functions use it. Its input is a float-float y, and it skips the reduction below 2⁻⁶ the same way `sinf` does. It does not apply `sinf`'s `< 2⁻¹²` shortcut, and its cos below 2⁻²⁰ is the pair (1, −y²/2), so `sincosf` must use the same entry tests as `sinf` and `cosf`, not the complex helper as is, to stay bit-identical.
- **Float, on the double path** (dp cores, x86_64). `sincosf` = double `sincos` (one reduction), then each result through `__libc_float_round` with that function's own hard-case table. The tables are `static` in `sinf.c` and `cosf.c` today, so they would move to two internal objects. Linking `sincosf` then pulls in both tables (2 entries each), which is negligible.
- **Float, `nofp/speed`.** `__libc_sinf_fixed(k, …)` and `(k + 16, …)` after one `__libc_rem_pio2f`, each with its rounding test and its own fallback.
- **x86_64** has no trigonometric layer of its own, so it uses the generic files.

### Expected gain of the shared reduction (P8.02)

- **In range.** For |x| ≤ π/4 the reduction is a comparison, so `sincos` saves only the second call's entry tests and mode switch. The bench's arguments span [−10, 10], where 92 % of them need the Cody–Waite step: four double-double operations, which are roughly a quarter to a third of one `sin` call on the soft-double cores. Expected `sincos` ≈ 1.6–1.75 × `sin`, against 2 × today.
- **Large arguments** (Payne–Hanek) save most, about the whole second reduction.
- The figures will be measured in P8.02 against the P8.01 bench rows below.

## Fixed-point double sine and cosine (P8.03 design)

### Target

- At most 0.53 ulp to nearest (the D124/D125 harness bound), in every rounding direction.
- `sincos` stays bit-identical to `sin` and `cos`.
- The size flavour keeps today's double-double code, so the fixed-point files go in the soft-double layers' `speed/` directories (D77), as P3.05g did.

### Error analysis of the polynomial

These are the Taylor remainders on |r| ≤ π/4. They are computed in this note (Python `math`, below), with z = r² and sin r / r taken as a polynomial in z:

| Terms in z | sin r / r remainder | cos r remainder |
|---:|---:|---:|
| 7 | 2^-45.1 | 2^-41.2 |
| 8 | 2^-53.9 | 2^-49.8 |
| 9 | 2^-63.0 | 2^-58.8 |
| 10 | 2^-72.4 | 2^-68.0 |

A minimax polynomial on [0, (π/4)²] gains roughly 2^(2k−1) over Taylor for its last term, so 8 terms reach about 2^-69 for sin r / r and 2^-65 for cos r. The alternative is a table, as for the float functions. With sin and cos of jπ/32 for |s| ≤ π/64 (j = 0 … 8 by symmetry, as the float table), 5 terms reach 2^-68.8 for sin s / s and 2^-65.3 for cos s (Taylor). That costs 9 × 2 × 8 = 144 bytes of table and needs an angle-sum step: two products and a sum.

| Design | Products for one function | For `sincos` | Table |
|---|---:|---:|---:|
| Polynomial on \|r\| ≤ π/4 | z, 8 Horner steps, × r: about 10 | about 19 | none |
| Table, \|s\| ≤ π/64 | z, 5 + 5 Horner steps, angle sum 2: about 13 | 15 (a second angle sum) | 144 bytes |

The polynomial is better for `sin` or `cos` alone. The table is better for `sincos`, and is what the float code does. P8.03 measures both on armv6-m before choosing.

### Relative accuracy near zero

`sin r` needs relative, not absolute, accuracy when r is small:

- Compute sin r / r as a fixed-point number in [0.9, 1], with z in units of 2⁻⁶⁴.
- Multiply by r's 64-bit significand.
- Carry r's exponent separately.

cos r is in [0.7, 1] and needs only absolute accuracy. As in P3.05g, arguments for which r does not fit 64 bits after the reduction's cancellation (|r| < 2⁻³⁰ or so) decline to the double-double path.

### Reduction

- **Today.** The soft-double Cody–Waite (`floor`, three double-double products) is a large part of the cost on Armv6-M.
- **Fixed-point reduction.** Multiply x's 53-bit significand by 2/π held to 128 bits (four 32-bit words) for |x| < 2²⁰. That gives the quadrant and r in 64-bit fixed point with about 64 − 53 = 11 guard bits beyond what cancellation can take for most arguments.
- **Near multiples of π/2,** where the cancellation is larger (worst case about 2⁻⁶¹ relative for doubles), the existing Payne–Hanek words (`two_over_pi` in `__libc_rem_pio2.c`) extend the product. Alternatively, the code declines to the double-double path when the leading zero count of r exceeds the guard bits; this is rare and cheap to detect.
- **Above 2²⁰,** keep the integer Payne–Hanek, which is already integer arithmetic.

### Rounding test

- The 64-bit result carries an error bound E in units of its last bit, measured on the host as in P3.05f/g.
- Round to 53 bits only when the 11 discarded bits are farther than E from a halfway point (to nearest), or from a representable value (the directed modes). Otherwise fall back.
- With E around 8 units of 2⁻⁶⁴, the fallback rate is about 2⁻¹¹ × 2 × 8 ≈ 0.8 % of arguments.
- The fast path then gives exactly the correctly rounded result. That is stricter than today's 0.53 ulp, so P8.03 checks fixed and fallback results against the double-double path, not only against the 0.53-ulp bound.
- `sincos` uses the same per-function tests, so it stays bit-identical.

### Multiply cost per core

| Core | 32×32→64 | 64×64 → high 64 | One Horner step |
|---|---|---|---|
| Armv6-M (`MULS` 32×32→32 only) | four 16×16 `MULS` + adds, about 18 instructions | three 32×32→64 (low×low dropped, with a carry estimate), about 55 | about 60 |
| Armv7-M, Armv7E-M, Armv8-M.main (`UMULL`, `UMLAL`) | 1 | about 4 `UMULL`/`UMLAL` + adds, about 8 | about 10 |

### Estimated instructions per call (to be measured in P8.03)

| Core | Today `sin` | Fixed `sin` (reduction + conversions + polynomial) | Fixed `sincos` |
|---|---:|---:|---:|
| armv6-m | 17,669 | about 250 + 150 + 600 ≈ 1,000 | about 1,600 (polynomial) / 1,300 (table) |
| armv7e-m, armv8-m.main | 6,435 | about 80 + 80 + 100 ≈ 260 | about 360 / 300 |

### CORDIC, for the record (D133)

- **Iterations.** Rotation mode needs about 56 iterations for 53 bits, plus a few guard iterations.
- **Per iteration:** x −= y >> i, y += x >> i, z −= atan 2⁻ⁱ. That is three 64-bit additions, two 64-bit variable shifts, a table load and a sign branch:
  - about 30 instructions on Armv6-M (a variable 64-bit shift is about 8);
  - about 10 on Armv7-M.
- **Per call,** with the gain constant folded into the start value: about 1,700–2,000 instructions on Armv6-M and about 600 on Armv7-M, for both results together.
- **Accuracy.** The error is absolute: about n × 2⁻⁶³ ≈ 2⁻⁵⁷, which is 0.56 ulp for results in [0.5, 1).
  - For small sin values the relative error grows without bound, so CORDIC would need a polynomial path anyway.
  - It would need 96-bit state to pass a rounding test with a useful fallback rate.
- **Table:** 56 × 8 bytes of arctangents.
- **On the double-FPU cores,** 56 dependent iterations are slower than today's ~170-instruction polynomial.

Every row is worse than a fixed-point polynomial or table, so CORDIC is not pursued.

## Edge cases

- **±0:**
  - `sin` keeps the sign;
  - `cos` is 1;
  - `sincos(−0)` gives (−0, 1).
- **Tiny |x|** (< 2⁻²⁷ double, < 2⁻¹² float):
  - `sin` returns x, with underflow raised for subnormals (`__libc_math_tiny`);
  - `cos` returns 1 − tiny (inexact, and the correct directed result).

  `sincos` must apply both functions' entry tests to its one argument.
- **±∞ and NaN:**
  - both results are NaN;
  - ∞ raises invalid once per function, so twice for `sincos`, matching two calls (the flags are sticky, so this is unobservable);
  - `errno` is `EDOM` if the function sets it.
- **Multiples of π/2 near the cancellation limit,** and arguments ≥ 2²⁰ (Payne–Hanek).
- **Directed rounding:**
  - the double path forces nearest inside the core and rounds once at the end;
  - the fixed path's rounding test must use the current mode.
- **The quadrant swap:** k mod 4 = 1 gives sin = cos r and cos = −sin r, and so on. A sign or swap mistake shows up only in some quadrants, so tests sweep all four.

## Test vectors

- **`test/unit/posix`:**
  - `sincos`/`sincosf` bit-identical to `sin`/`cos` at ±0, the smallest subnormal, 0.5, ±1, π, 10²², the largest power of two, ±∞ and NaN;
  - P8.02 adds one argument per quadrant at each reduction regime: |x| < π/4, Cody–Waite, and ≥ 2²⁰.
- **`test/unit/libm_vectors`:** the MPFR vectors for `sin` and `cos` in every rounding direction (D124). P8.03 adds `sincos` as a pair check over the same vectors.
- **`test/unit/libm_dense`:** P8.03 adds:
  - arguments whose results lie within 2⁻⁶⁰ of a rounding boundary, found on the host with MPFR;
  - the doubles closest to multiples of π/2 (the reduction's worst cancellation).
- **Mutant check (P8.03):** with one guard bit dropped from the rounding test's bound, the dense cases must fail.

## Bench rows (P8.01)

`test/bench/math` gains `sincos` and `sincosf`, as a unary case whose value is sin + cos over the same 16 arguments in [−10, 10]. Their cost before any change is recorded below and in `docs/perf/<arch>.md`.

Measured with qemu instruction counting (`make bench BENCH_UPDATE=1`, throughput rows, instructions per call; 16 arguments in [−10, 10]). The size and speed flavours differ only where the table shows two figures (size / speed):

| Arch | `sin` | `cos` | `sincos` | `sinf` | `cosf` | `sincosf` |
|---|---:|---:|---:|---:|---:|---:|
| armv6-m, armv6s-m | 17,669 | 17,660 | 35,340 | 18,350 / 4,729 | 18,281 / 3,742 | 36,637 / 8,477 |
| armv7-m | 6,524 | 6,500 | 13,034 | 9,154 / 2,499 | 9,093 / 1,990 | 18,256 / 4,498 |
| armv7e-m | 6,435 | 6,413 | 12,864 | 575 | 567 | 1,155 |
| armv8-m.main | 6,435 | 6,413 | 12,864 | 574 | 566 | 1,153 |
| armv7e-m-dp | 165 | 167 | 344 | 187 | 180 | 380 |
| x86_64 | 308 | 313 | 635 | 337 | 328 | 679 |

`sincos` costs the sum of `sin` and `cos` plus the wrapper's few instructions everywhere, as P7.03's implementation implies. These are the rows P8.02 and P8.03 improve against. (D133's 6,876 for armv7e-m was an older figure; the current one is 6,435.)

## P8.02: results

The shared reduction is implemented as designed above, one file per path. Measured (instructions per call, throughput rows):

| Arch | `sincos` before → after | `sincosf` before → after |
|---|---|---|
| armv6-m, armv6s-m | 35,340 → 29,654 (−16 %) | 36,637 → 33,644 (−8 %); speed 8,477 → 5,504 (−35 %) |
| armv7-m | 13,034 → 10,702 (−18 %) | 18,256 → 16,796 (−8 %); speed 4,498 → 3,055 (−32 %) |
| armv7e-m, armv8-m.main | 12,863 → 10,527 (−18 %) | 1,154 → 786 (−32 %) |
| armv7e-m-dp | 344 → 230 (−33 %) | 379 → 270 (−29 %) |
| x86_64 | 635 → 445 (−30 %) | 679 → 492 (−27 %) |

The soft-double cores gain less than the estimate of 1.6–1.75 × `sin`: `sincos` is 1.68 × `sin` on armv6-m and 1.64 × on armv7e-m, because the double-double cores, not the Cody–Waite reduction, dominate there. That is what P8.03's fixed-point cores address. Where the arithmetic is hardware (armv7e-m-dp, x86_64) `sincos` costs 1.4 × `sin`.

**A subtlety found by the test.** `sin` negates the double-double result and then sums it in the caller's rounding mode. Summing first and negating the sum gives a different result in the directed modes (−(a + b) is not (−a) + (−b) when rounding up or down). The identity test catches it in every quadrant that negates.
