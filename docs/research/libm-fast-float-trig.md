# Fast float trigonometric, inverse and hyperbolic functions (P11.04)

Research note for P11.04 ([`docs/plan/phase-11.md`](../plan/phase-11.md),
PLAN D162, D163, D168), extended by each sub-step: P11.04a the sine, cosine
and tangent and their π forms on the FPU cores, P11.04b the inverse
functions, P11.04c the hyperbolic functions, P11.04d the error functions,
P11.04e–g the cores without an FPU. Like `libm-fast-float-exp-log.md` it
records the sources, the design per core, the error analysis and the
measured bounds, and contains no code from any other implementation (D1,
D2): the ideas are described in our own words, the constants computed here.

## 1. The target

The fast contract (fast-math.md §4, `test/unit/include/fast_contract.h`):
below 1 ulp to nearest for `sin`, `cos`, `sinpi`, `cospi`, `tanpi` and the
inverse functions, 1.5 ulp for `tan` and the hyperbolic functions and
`erf`; round to nearest only; the special values, exceptions and errno as
in the other build types. Today's speed files (instructions per call):
`sinf` 337 on x86_64, 187 on armv7e-m-dp (the double `sin` rounded, with
D98's hard-case table), 575 on armv7e-m (float-float, D60).

## 2. Sources (ideas only)

Read 2026-11-11 through summaries:

- **FreeBSD `s_sinf.c`, `k_sinf.c`, `k_tanf.c`, `e_rem_pio2f.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_sinf.c>,
  `.../k_sinf.c`, `.../k_tanf.c`, `.../e_rem_pio2f.c>`): the float functions
  evaluated in double; up to 9π/4 a shift by a small multiple of π/2 stored
  as doubles, beyond a Cody–Waite reduction in double (k = round(x 2/π), x −
  k·(a 25-bit leading π/2) − k·(its tail)) up to about 2^28·π/2, and a
  multi-precision routine above; odd polynomial of degree 9 for sin
  (2^-37.5 relative), tan by a polynomial of degree about 11 for tan(x)/x and
  the negative reciprocal for odd quadrants; x itself below 2^-12.
- **Arm Optimized Routines `sinf.c`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/sinf.c>):
  in double; a direct polynomial below π/4, a table-driven reduction up to
  about 120 with the quadrant selecting sine or cosine tables and a sign,
  an integer reduction above; 0.5607 ulp stated.

What we take: double evaluation with a Cody–Waite reduction where double is
hardware; the strict layer's own integer reduction where it is not.

## 3. The double-FPU cores: `libc/math/fast/` (P11.04a)

- **Reduction** (`__libc_fast_rem_pio2`, |x| < 2^20): k = round(x·2/π) by
  the 1.5·2^52 shift; r = ((x − k P1) − k P2) − k P3 with P1 and P2 the first
  two 33-bit pieces of π/2 (computed here with 300-bit arithmetic), so k P1
  and k P2 are exact for |k| < 2^20, and x − k P1 is exact as well (both
  multiples of 2^-32 for |x| ≥ 2^-9, the difference below 2^21); r's error
  is about 2^-52 of r. Beyond 2^20, and for the infinities and NaNs, the
  double `sin`, `cos`, `tan`, `sincos` rounded once (not on any common
  path).
- **Polynomials:** the Taylor series of sin r to r¹¹ (relative error
  2^-36.7 at π/4) and of cos r to r¹² (2^-38), in double; the quadrant k mod 4
  picks sine or cosine and the sign; tan r = sin r / cos r, −cos r / sin r for
  an odd k. One rounding to float.
- **The π forms:** 2x = n + 2y exactly in double (n by the shift, y = x − n/2,
  |y| ≤ 1/4), r = π y rounded once, then the same polynomials with the
  quadrant n mod 4. The exact cases (multiples of 1/2, and every float from
  2^23 up) as the strict files: ±0 with the sign C23 F.10.1 gives, ±1, ±∞
  with divide-by-zero for `tanpi` (a division at run time).
- The first build compared `!(|x| < 2^20)`: a relational operator on a NaN
  raises invalid, and `libm_vectors` and `libm_trig` caught `cosf(NaN)`;
  `isless` compares quietly.

## 4. The single-precision FPU cores: `libc/math/sp-nearest/fast/` (P11.04a)

- **Reduction:** below 64, a four-piece Cody–Waite reduction in float
  (`__libc_fast_rem_pio32f`): k = round(32x/π); P1, P2, P3 the first three
  14-bit pieces of π/32 (computed here), so that k P1, k P2, k P3 are exact
  for |k| < 2^10; x − k P1 exact by a fused multiply-add (both multiples of
  2^-28, the difference below 0.052), the next two subtractions by exact
  two-sums, P4 the rounded fourth piece (k P4 below 2^-36): s to about 2^-59
  absolute. Near the multiples of π/2, where sin or cos is small, |s| is at
  least 2^-26.3 for these floats (measured over all of them, at
  0x1.2d97c8p+2), so that is 2^-33 relative. From 64 up, the strict layer's
  integer reduction (`__libc_rem_pio2f`, D60: k mod 64, s to about 2^-48);
  s = x below 2^-6. The first build took the integer reduction everywhere:
  `sinf` measured 575 → 428 instructions on armv7e-m, the reduction about
  350 of them; with Cody–Waite below 64, 128.
- **Kernel** (`__libc_fast_sinf_kernel`): k + 8 = 16q + (j + 8) gives the
  quadrant q and the table angle a = |j|·π/32 (`__libc_fast_sinf_table`:
  sin a and cos a as float and remainder, j ≤ 8, 144 bytes, computed here);
  with f and g the signed sin a and cos a (swapped for an odd quadrant),
  sin(kπ/32 + s) = f + g s_hi + (f_lo + f(cos s − 1) + g(sin s − s_hi) + g_lo
  s_hi): g s_hi exact by a fused multiply-add, f + g s_hi exact by a
  two-sum, the rest small, so only the last addition rounds much. sin s − s
  = s³(−1/6 + s²/120) and cos s − 1 = s²(−1/2 + s²/24) (|s| ≤ π/64, the next
  terms below 2^-31 of the result).
- **tan:** the kernel's sine and cosine as unrounded pairs, divided with
  the quotient's remainder by a fused multiply-add
  (`__libc_fast_divide_pairs`). The first build divided the remainder by the
  pair's high part only; the kernel's low part carries the polynomial
  terms, up to 2^-10 of the high part, and that cost 2^-22: 4.5 ulp of
  `tanf` (`libm_vectors`, `float_accuracy`, `libm_trig`). The pairs are now
  normalised first by a Fast2Sum.
- **The π forms:** 32x = K + d exactly (truncated, then |d| ≤ 1/2), s = d·π/32
  as float-float (the product's error by a fused multiply-add), the kernel
  at K; below 2^-90, πx with x scaled by 2^64 (as the m1 forms of
  P11.03d).

## 5. Measured (P11.04a)

Every finite float (4,278,162,238 arguments) against glibc's long double
`sinl`, `cosl`, `tanl` (the π forms with x first reduced modulo 2 exactly,
and their exact cases exact):

| Function | double path | float path |
|---|---:|---:|
| `sinf` | 0.5001 ulp | 0.5066 ulp |
| `cosf` | 0.5001 | 0.5067 |
| `tanf` | 0.5002 | 0.5059 |
| `sinpif` | 0.5001 | 0.7500 |
| `cospif` | 0.5001 | 0.5064 |
| `tanpif` | 0.5002 | 0.7500 |

The float path's 0.75 is below 2^-126 only: πx rounded to a normal float
and then scaled into the subnormals rounds twice; elsewhere it is below 0.51.
`sincosf` takes the same kernels as `sinf` and `cosf`.

Instructions per call (speed → fast):

| Function | x86_64 | armv7e-m-dp | armv7e-m | armv8-m.main |
|---|---:|---:|---:|---:|
| `sinf` | 337 → 63 | 187 → 58 | 575 → 128 | 574 → 126 |
| `cosf` | 328 → 57 | 180 → 51 | 567 → 123 | 566 → 121 |
| `sincosf` | 493 → 111 | 270 → 86 | 786 → 212 | 784 → 208 |
| `tanf` | 451 → 79 | 225 → 69 | 794 → 216 | 792 → 212 |
| `sinpif` | 364 → 65 | 223 → 56 | 275 → 113 | 261 → 112 |
| `cospif` | 361 → 62 | 219 → 58 | 268 → 110 | 254 → 109 |
| `tanpif` | 495 → 77 | 257 → 75 | 494 → 207 | 479 → 203 |

The cores without an FPU follow in P11.04e.

## 6. The inverse functions on the double-FPU cores: `libc/math/fast/` (P11.04b)

Sources (ideas only, read 2026-11-11 through summaries): §6.1.

- **One kernel** (`__libc_fast_atan_double`, a ≥ 0 or +∞): above 1,
  atan a = π/2 − atan(1/a); then the nearest breakpoint c = j/8 (j = 0 … 8,
  `__libc_fast_atan_table`, atan(j/8) correctly rounded, computed here with
  200-bit arithmetic), t = (a − c)/(1 + a c) with a − c exact (Sterbenz:
  a and c within a factor 2 of each other once j ≥ 1), |t| ≤ 1/16, and
  atan a = atan c + atan t with the Taylor series of atan t to t⁹ (the next
  term, t¹¹/11, is 2^-47.5 of t). For j = 0 the table entry is 0 and t is a
  itself, so the error stays relative near 0. The kernel's total relative
  error is about 2^-43.5, far below the 2^-25 a float rounding needs; the
  result is rounded once.
- **atanf, atanpif:** the kernel on |x| with the sign of x; the π form
  times 1/π in double before the rounding. ±∞ give ±π/2 through 1/a = 0.
  The tiny arguments of `atanf` (|x| < 2^-30) as the other build types
  (`__libc_math_tinyf`: x, with underflow for a subnormal one).
- **asinf, asinpif:** asin x = atan(|x|/√(1 − x²)), with 1 − x² formed as
  (1 − x)(1 + x) in double (both factors exact for a float x, their product
  correct to 2^-53), so the cancellation near ±1 costs nothing. At ±1 the
  square root is 0: the branch returns π/2 directly, because the division
  would raise divide-by-zero (the first build did, `libm_thresholds`).
- **acosf, acospif:** acos x = atan(√(1 − x²)/|x|), or π minus it for a
  negative x; at x = 0 π/2 directly, for the same reason.
- **atan2f, atan2pif:** the quotient |y|/|x| in double (no overflow or
  underflow: it lies in [2^-277, 2^277]), the kernel, π minus it for a
  negative x, the sign of y; the zeros, infinities and NaNs (C23 F.10.1.4,
  F.10.1.14's signed cases) as the double `atan2`, `atan2pi` rounded.
- |x| > 1 and NaN of `asinf`, `acosf` and their π forms go to the double
  function (invalid, the NaN) through `isless`, which compares quietly.
- **The exact cases in every direction** (`libm_vectors`, `libm_trig`): at
  t = 0 the kernel's t + t·z·P is 0 + (−0) = −0 when rounding downward, so
  `acosf(1)` and `acospif(1)` return +0 and `atanpif(±0)` returns x + x
  directly; and a tiny quotient (below 2^-30, x > 0) gives a double atan2
  that rounds to an exact float without underflow, so `atan2f` nudges it
  toward zero by 2^-40 relative, as the strict file does (atan2 is
  q − q³/3), and the conversion rounds in the current direction and raises
  underflow.

### 6.1 Sources

- **FreeBSD `e_asinf.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_asinf.c>):
  x below about 2^-12; up to 1/2, x + x·R(x²) with a small rational R
  (2^-30 relative stated); above, the half-angle identity
  π/2 − 2 asin √((1 − |x|)/2), the square root and π/2 in double.
- **FreeBSD `e_acosf.c`** (same directory): the same R; π/2 split in two
  for the small arguments; above 1/2 the half-angle form, with the square
  root split into a truncated high part and a correction for its rounding
  error, in float.
- **FreeBSD `s_atanf.c`** (same directory): four reference angles as high
  and low parts (near 1/2, 1, 3/2 and ∞, the last by −1/x), reached by the
  matching Möbius maps above 7/16, then an odd polynomial of degree about 11
  split into even and odd halves; float throughout.
- **FreeBSD `e_atan2f.c`** (same directory): the signed special cases;
  π/2 when the exponents differ by more than 26; else `atanf(|y/x|)` and
  π as high and low by quadrant.
- **Arm Optimized Routines, Advanced SIMD `asinf`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/asinf.c>):
  one polynomial of degree 4 in z for both ranges (|x| < 1/2: z = x²; above,
  z = (1 − |x|)/2 and its square root), fitted to (asin √z − √z)/z^{3/2},
  2^-29; observed maxima 0.83 ulp below 1/2 and 2.41 above, its last
  rounding not compensated.

What we take: the half-angle identity above 1/2 (§7), the reflection by
1/a above 1, and reference angles with a Möbius map, here eight evenly
spaced ones so that the polynomial stays short. Neither source compensates
the last addition; the float path below carries a pair to the one rounding
for the < 1 ulp contract (AOR's uncompensated form measured 2.41 ulp).

## 7. The inverse functions on the single-precision FPU cores: `libc/math/sp-nearest/fast/` (P11.04b)

Float only, so each result is an unrounded pair hi + lo with one final
rounding.

- **atan kernel** (`__libc_fast_atanf_kernel`, 0 ≤ a ≤ 1 given as a +
  a_lo): the same breakpoints j/8 as §6 (`__libc_fast_atanf_table`, atan(j/8)
  as float and remainder, 72 bytes), j = ⌊(⌊16a⌋ + 1)/2⌋ from the exact 16a.
  The first build took j = ⌊8a + 1/2⌋, and 8a + 1/2 rounds up to 1 for the
  float just below 1/16: c = 1/8 is then more than twice a, a − c is
  inexact, and `atanf` measured 1.27 ulp there. n = a − c is exact (Sterbenz,
  c/2 ≤ a ≤ 2c for j ≥ 1, or c = 0);
  the denominator d = 1 + a c by a fused multiply-add, its rounding error
  d_lo by a second one (plus a_lo c); the quotient q = n/d and its remainder
  q_lo = ((n − q d) − q d_lo + a_lo)/d, the first product's error exact by a
  fused multiply-add. atan t − t = t³(−1/3 + t²/5 − t⁴/7) (|t| ≤ 1/16, the next
  term 2^-35 of t). The leading atan c + q by a two-sum, so the remainder
  table entry, q_lo, the polynomial tail and the two-sum's error all add into
  the low part. Above 1 the reciprocal 1/a as a pair (its remainder by a
  fused multiply-add), then π/2 minus the pair (`__libc_fast_pair_from`, π/2
  as float and remainder).
- **asin** (`__libc_fast_asinf_small`): up to 1/2, asin x = x + x³ P(x²)
  with P of degree 6 (seven coefficients), fitted here by Chebyshev
  interpolation of (asin √z − √z)/z^{3/2} on [0, 1/4], 2^-29.9 relative on
  asin; the pair is x and x³ P(x²) (the second below x/6). Above 1/2 the
  half-angle identity asin a = π/2 − 2 asin s, s = √((1 − a)/2) ≤ 1/2: 1 − a
  exact (Sterbenz) and the halving exact, the square root's remainder by a
  fused multiply-add (`__libc_fast_half_angle`), then the same polynomial on
  the pair s + s_lo and π/2 minus twice it.
- **acos:** up to 1/2, π/2 − asin x; above, 2 asin s, or π minus it for a
  negative x.
- **The π forms:** the same pair times 1/π as float and remainder
  (`__libc_fast_pair_over_pi`: the leading product's error by a fused
  multiply-add, so the pair's 2^-48 survives), one rounding. Far below 1,
  `asinpif` scales x by 2^64 first so that the constant's low part does not
  underflow (as the m1 forms of P11.03d).
- **atan2f, atan2pif:** the smaller of |x|, |y| over the larger as a pair
  (the remainder by a fused multiply-add), the kernel, π/2 minus it when |y|
  is the larger, π minus it for a negative x, the sign of y; the zeros,
  infinities and NaNs as the strict layer's file (C23 F.10.1.4, F.10.1.14),
  and `atan2pif`'s |x| = |y| exactly ±1/4 or ±3/4.
- **The subnormals in atan2:** the quotient's remainder (q·|x| − |y| by a
  fused multiply-add) falls below the subnormals when the smaller argument
  is below about 2^-125, and the first build measured 1.43 ulp (`atan2f`) and
  1.65 (`atan2pif`) there; both arguments are now scaled by 2^64 when one is
  below 2^-100 and both below 2^60 (with the larger from 2^60 the quotient is
  below 2^-160 and needs no remainder). A quotient below 2^-60 for a positive
  x is its own arctangent: `atan2f` returns it, rounded once (adding back a
  remainder rounded on the subnormal grid, half an ulp of a quotient near
  2^-125, measured 0.67 ulp in `complex_float`'s `clogf`, whose bound on these
  cores is 0.52); `atan2pif` scales the quotient by 2^64 (|y| up, or |x| down
  when it is above 2^64) before the product by 1/π and scales back once.
  `atanpif` below 2^-64 scales x the same way (the subnormal product rounded
  twice measured 0.999 ulp).
- The tiny arguments: |x| < 2^-12 returns x for `atanf` and `asinf`
  (x³/3 is below half an ulp of x); `atanpif` and `asinpif` keep ±0.

## 8. Measured (P11.04b)

Every float of the domain (4,278,162,238 finite arguments for `atanf` and
`atanpif`, 2,130,706,434 in [−1, 1] for the others) against glibc's long
double functions (the π forms divided by π in long double); `atan2f` and
`atan2pif` sampled, 400 million pairs in each of five groups: random bit
patterns, |y/x| within 2^±8, a y below 2^-119, an x below 2^-119, and |y/x|
from 2^-174 to 2^-55:

| Function | double path | float path |
|---|---:|---:|
| `atanf` | 0.5000 ulp | 0.5073 ulp |
| `atanpif` | 0.5000 | 0.7500 |
| `asinf` | 0.5000 | 0.6818 |
| `asinpif` | 0.5000 | 0.7512 |
| `acosf` | 0.5000 | 0.6589 |
| `acospif` | 0.5000 | 0.6424 |
| `atan2f` | 0.5000 | 0.5097 |
| `atan2pif` | 0.5000 | 0.7500 |

The double path is correctly rounded on every argument tried (the kernel's
2^-43.5 leaves the double rounding to float the only rounding that
matters). On the float path, `atanpif`'s and `atan2pif`'s 0.75 is for
subnormal results only (the π form rounded to a normal float and then
scaled, as §5), elsewhere below 0.51; `asinf`, `acosf` and their π forms
reach 0.64–0.75 just above 1/2, where π/2 − 2 asin s cancels to about a
third of π/2 and the low part's own float roundings (x z and P, each 2^-24
of a term about a tenth of the result) count for a few tenths of an ulp
more.

The strict float-float core (`__libc_atan2f_core`, the float cores' release
and speed builds) took its breakpoint as k = ⌊8t + 1/2⌋ as well, and its
`atanf` measured 1.27 ulp at 0x1.fffffep-5 (`libm_thresholds`' new
regression, run in every build type); with the exact index it is correctly
rounded on every finite float (host sweep of the layer, 0.5000 ulp).

Instructions per call (speed → fast):

| Function | x86_64 | armv7e-m-dp | armv7e-m | armv8-m.main |
|---|---:|---:|---:|---:|
| `asinf` | 505 → 76 | 238 → 73 | 315 → 64 | 309 → 64 |
| `acosf` | 519 → 73 | 241 → 66 | 325 → 63 | 319 → 63 |
| `atanf` | 407 → 64 | 217 → 63 | 284 → 92 | 278 → 92 |
| `atan2f` | 599 → 82 | 398 → 78 | 410 → 118 | 402 → 118 |

The cores without an FPU follow in P11.04f.
