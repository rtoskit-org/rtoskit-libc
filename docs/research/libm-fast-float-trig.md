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

## 9. The hyperbolic functions on the double-FPU cores: `libc/math/fast/` (P11.04c)

Sources (ideas only, read 2026-11-11 through summaries): §9.1. The fast
bound is 1.5 ulp for all six (fast-math.md §4); in double they are
evaluated much better than that, over the P11.03 kernels, and rounded once.

- **sinhf:** below 1/2 the odd Taylor series to x¹¹ (the next term below
  2^-44 of the result); above, (E − 1/E)/2 with E = e^|x| from
  `__libc_fast_exp2` (2^-30.5; E − 1/E loses at most a factor 2.2 from 1/2
  up). From 89.5, and for the infinities, the double `sinh` (overflow).
- **coshf:** (E + 1/E)/2, a sum of positive terms everywhere.
- **tanhf:** m/(m + 2) with m = e^(2|x|) − 1: the m1 series below 2|x| =
  1/8, E − 1 above (from e^(1/8) the subtraction loses three bits of
  2^-30.5); from 9.5, and the infinities, the double `tanh`.
- **asinhf:** below 1/4 log1p(|x| + x²/(1 + √(1 + x²))) by
  `__libc_fast_log1p_double` (the argument's relative accuracy is the double
  operations'), above log(|x| + √(x² + 1)) by the new
  `__libc_fast_log_positive` (e ln2 + log m of the split double; x² cannot
  overflow a double), whose absolute error 2^-52 is far below the result.
- **acoshf:** log(x + √((x − 1)(x + 1))), both factors exact in double: the
  sum's rounding is 2^-53 absolute against a result of at least 2^-11.5
  (x = 1 + 2^-23), so one form serves; x = 1 returns +0 directly.
- **atanhf:** below 1/8 log1p(2|x|/(1 − |x|))/2 (the relative accuracy
  holds near 0), above log((1 + |x|)/(1 − |x|))/2, both terms exact.
  |x| ≥ 1 and NaN go to the double `atanh` (divide-by-zero, invalid).
- The tiny arguments of `sinhf`, `tanhf`, `asinhf` and `atanhf` (|x| below
  2^-30) as the other build types (`__libc_math_tinyf`, with the sign of the
  neglected term).

### 9.1 Sources

- **FreeBSD `e_sinhf.c`, `e_coshf.c`, `s_tanhf.c`, `s_asinhf.c`,
  `e_acoshf.c`, `e_atanhf.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_sinhf.c>
  and the others in the same directory), float throughout over the float
  library: sinh by expm1-based forms below 9, half of e^|x| to ln FLT_MAX and
  an exponential scaled by 1/2 to the overflow threshold; cosh by
  1 + t²/(2(1 + t)) with t = expm1|x| below ln2/2, (E + 1/E)/2 to 9, then
  as sinh; tanh by −t/(t + 2) with t = expm1(−2|x|) below 1 and 1 − 2/(t + 2)
  to 9, then ±1; asinh by log1p(|x| + x²/(1 + √(1 + x²))) below 2, the log of
  2|x| + 1/(√(x² + 1) + |x|) to 2^28, log|x| + ln2 above; acosh by
  log1p(t + √(2t + t²)) with t = x − 1 below 2, log(2x − 1/(x + √(x² − 1))) to
  2^28, log x + ln2 above; atanh by log1p of 2x + 2x²/(1 − x) below 1/2 and of
  2x/(1 − x) above, halved. No ulp bounds stated.
- **Arm Optimized Routines**, the Advanced SIMD `sinhf`, `tanhf`, `asinhf`
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/sinhf.c>
  and the others in that directory): sinh as t + t/(t + 1) over t =
  expm1|x| (2.26 ulp stated), tanh as q/(q + 2) with q = e^(2x) − 1 from its
  own reduction (2.08 + 0.5 ulp), asinh as log1p(x + x²/(1 + √(x² + 1)))
  with log(2x) above 2^64 (2.59 ulp stated); their last steps are not
  compensated.

What we take: the log1p forms near 0 and log 2|x| far out, m/(m + 2) for
tanh, and the half-exponential beyond 9; the float path below carries the
pairs that let it stay near half an ulp where AOR's forms reach 2.

## 10. The hyperbolic functions on the single-precision FPU cores: `libc/math/sp-nearest/fast/` (P11.04c)

Float only, so as in §7 each result is a pair rounded once.

- **The exponential pair** (`__libc_fast_expf_pair`, 0 ≤ a ≤ 90): a = k ln2
  + u as `expf` reduces it, e^u = 1 + u_hi + (u_lo + u² q(u)) with 1 + u_hi
  by a two-sum, then normalised by a Fast2Sum. The first build left the pair
  raw (its tail reaches 0.06), and the reciprocal 1/hi − lo/hi² of `coshf`
  and `sinhf` was off by lo²/hi³: 5·10⁻⁴ at 0.3 (a spot check before the
  sweep).
- **sinhf:** below 1 the odd series x + x z S(z) to x¹¹ (x¹³/13! is 2^-32.5
  of the result at 1), one rounding; up to 9, H − 1/(4H) with H = 2^(k−1) P
  and 1/(4H) = 2^(−k−1)/P as a pair (the quotient's remainder by a fused
  multiply-add), the leading difference by a two-sum; beyond 9 H alone
  (e^-2|x| below 2^-26), scaled in two exact steps since k reaches 130, so
  the overflow from 89.42 is the product's.
- **coshf:** H + 1/(4H) the same way from 2^-12 up; below, 1 + x² (1,
  inexact but for 0).
- **tanhf:** below 1, m/(m + 2) with m = e^(2|x|) − 1 as a pair from the m1
  kernel's leading terms (`__libc_fast_expm1f_pair`, k ≤ 3; the kernel
  keeps its own copy, as sharing it cost the m1 forms 0.8 % on these cores)
  and the pairs divided
  (`__libc_fast_divide_pairs`); up to 9.1, 1 − 2/(E + 1) with E = e^(2|x|) as
  a scaled pair (k ≤ 27, so m1's two-sums against 2^k − 1 would not be
  exact), 2/(E + 1) ≤ 0.24 with its remainder; beyond, ±(1 − 2^-30/|x|),
  which rounds to ±1 and raises inexact at run time.
- **asinhf:** up to 2^12, log1p(|x| + t) with z = x² exactly as a pair,
  w = 1 + z, s = √w with its remainder (`__libc_fast_sqrtf_pair`) and
  t = s − 1 (exact up to s = 2, else a two-sum): the argument to about
  2^-48, and `__libc_fast_log1pf_pair` adds u_lo/(1 + u) beside the log1pf
  reduction's own correction. Beyond 2^12, log 2|x| by the logf reduction
  with k + 1 (`__libc_fast_logf_twice`; the next term 1/(4x²) is 2^-26).
- **acoshf:** up to 2^12, log1p(t + √(t (t + 2))) with t = x − 1 exact, t + 2
  by a two-sum and the product's error by a fused multiply-add; beyond,
  log 2x.
- **atanhf:** log1p(2|x|/(1 − |x|))/2 with 1 − |x| by a two-sum and the
  quotient's remainder; ±1 divides at run time (divide-by-zero).

## 11. Measured (P11.04c)

Every finite float of the domain (4,278,162,238 arguments; `acoshf` from 1,
`atanhf` in [−1, 1]) against glibc's long double functions:

| Function | double path | float path |
|---|---:|---:|
| `sinhf` | 0.5131 ulp | 0.8440 ulp |
| `coshf` | 0.5092 | 0.7525 |
| `tanhf` | 0.5383 | 0.5641 |
| `asinhf` | 0.5000 | 0.7550 |
| `acoshf` | 0.5000 | 0.7416 |
| `atanhf` | 0.5000 | 0.7538 |

All within the 1.5 ulp bound and, but for the float `sinhf` near 0.87
(the series' x z S tail rounded in float, up to a seventh of the result),
within the 0.76 the P11.03 float logarithms reach, which the float
log1p-based forms inherit. The double `tanhf` 0.54 is just above 2|x| = 1/8,
where E − 1 loses three bits of the exponential's 2^-30.5.

Instructions per call (speed → fast):

| Function | x86_64 | armv7e-m-dp | armv7e-m | armv8-m.main |
|---|---:|---:|---:|---:|
| `sinhf` | 334 → 66 | 223 → 68 | 267 → 90 | 267 → 90 |
| `coshf` | 291 → 52 | 196 → 55 | 241 → 84 | 241 → 84 |
| `tanhf` | 355 → 68 | 218 → 72 | 253 → 104 | 253 → 104 |
| `asinhf` | 578 → 90 | 308 → 92 | 288 → 135 | 287 → 136 |
| `acoshf` | 494 → 71 | 259 → 74 | 266 → 125 | 266 → 126 |
| `atanhf` | 407 → 87 | 234 → 86 | 205 → 111 | 205 → 110 |

The cores without an FPU follow in P11.04g.

## 12. The error functions on the double-FPU cores: `libc/math/fast/` (P11.04d)

Sources (ideas only, read 2026-11-11 through summaries): §12.1. The fast
bound is 1.5 ulp for `erff`. `erfcf` is not relaxed: the contract keeps it
strict, correctly rounded in every direction (fast-math.md §4), and the
maintainer chose a fast `erfcf` that stays so (D178). The first build
measured 0.80 ulp on the float path and failed `libm_dense`,
`float_accuracy`, `libm_vectors` and `libm_special` there.

- **One function of the two:** R(x) = erfc(x) e^(x²), smooth and slowly
  varying, gives erfc x = e^(−x²) R(x), erf x = 1 − e^(−x²) R(|x|) and
  erfc(−x) = 2 − e^(−x²) R(x). For a float x, x² is exact in double, so the
  exponential (`__libc_fast_exp_double`, 2^-30.5) keeps erfc's relative
  accuracy out to its underflow at 10.05.
- **R** (`__libc_fast_erfc_table`, `__libc_fast_erfc_double`): five
  pieces of width 1/2 on [1/2, 3) in t = x − c, and above 3 x R(x) as a
  function G of w = 1/x² on [1/10.1², 1/9] (R = v G(v²), v = 1/x, one
  division), all of degree 8, fitted here by Chebyshev interpolation at 60
  digits: 2^-34.9 to 2^-42.4 relative (432 bytes). The degree hardly falls
  with narrower pieces (6–8 at width 1/2, measured), so six rows of one
  degree keep the code a single loop.
- **Near 0:** erf x = x P(x²) with P of degree 5 on [0, 1/4] (2^-36.3); erfc
  x = 1 − x P(x²) there, the subtraction exact enough in double.
- **The exponential:** `__libc_fast_exp2` is good to 2^-30.5 only;
  `__libc_fast_exp2_accurate` takes 2^r's Taylor polynomial to degree 5
  (2^-48.7 for |r| ≤ 1/64, 2^-46 with t = −x² log2 e rounded), so the result
  is good to about 2^-34 relative, R's pieces dominating.
- **erfcf's rounding test:** the double result keeps 29 bits below the
  float's. Within 2^20 of a float (0 or 2^29) or of the halfway point (2^28)
  the 2^-34 error could cross a rounding boundary in some direction, and the
  other build types' double `erfc` decides (with D98's hard case); that is
  2^22 of 2^29 patterns, about 0.8 % of arguments. Beyond −3.9 (within a
  quarter ulp of 2) and 9.19 (subnormal results, fewer bits) the strict
  code decides as well, NaN compared quietly (`isgreaterequal`: the first
  build's ordered comparison raised invalid, `libm_vectors`).
- **The ends:** `erff` is ±1 from 3.92 (erfc below a quarter ulp of 1), as
  the strict files.

### 12.1 Sources

- **FreeBSD `s_erff.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_erff.c>),
  float throughout: the linear term below about 2^-14; up to 0.84 a rational
  in x²; to 1.25 a rational in |x| − 1 offset from a constant; beyond, an
  asymptotic form exp(−x²) times a rational in 1/x², with the exponential
  split as the product of two (x truncated to 12 bits so that its square is
  exact, and the remainder (z − x)(z + x) separately); ±1 from 6 (erf) and
  the underflowing product from 11 (erfc). Error bounds of the fits around
  2^-28 to 2^-31; no ulp bound stated.
- **Arm Optimized Routines `erff.c`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/erff.c>),
  float with fused multiply-adds: below 0.875 x + x P(x²) of degree 5; up
  to 4 erf = 1 − e^(−r(x)) with r a polynomial of degree about 6 in |x|
  (one float `expf`); ±1 beyond; 1.09 ulp stated at its worst point. Its
  scalar `erfcf` paths were not found (404); CORE-MATH's `erff` and `erfcf`
  were behind an access block and were not read.

What we take: the factor e^(−x²) with an exact square (FreeBSD's split, here
free in double and an exact pair in float) and a slowly varying factor
fitted in pieces; the series near 0. Unlike AOR's 1 − e^(−r), erfc keeps
its own relative accuracy, which `erfcf` needs.

## 13. The error functions on the single-precision FPU cores: `libc/math/sp-nearest/fast/` (P11.04d)

Float only, pairs to one rounding as §7 and §10.

- **e^(−x²)** (`__libc_fast_expf_pair_lo`): x² as an exact pair (a fused
  multiply-add for its error) and the exponential pair of §10 with the
  argument's low part; −x² − k ln2_hi is exact (k ln2_hi has 29 bits, the
  difference 19), so the pair keeps 2^-30 down to k = −146.
- **R** (`__libc_fast_erfcf_table`, `__libc_fast_erfcf_kernel`): the same
  six pieces at degree 7 in float: the constant term as a pair, then each
  coefficient rounded in turn and the rest refitted to the residual by
  relative least squares (rounding a Chebyshev fit's coefficients at once
  measured 2^-27.1; refitted, 2^-29.6 to 2^-33.9). c0 + c1 t is formed
  exactly (a fused multiply-add and a two-sum): c1 t reaches a third of R,
  and rounding it with the rest measured 1.06 ulp of `erfcf` near 0.53.
  Above 3, v = 1/x with its remainder. The product e^(−x²) R by a fused
  multiply-add for the leading error.
- **erff:** below 1/2, (2/√π) x + x³ Q(x²) with 2/√π as a pair (Q of degree
  5 in z, fitted the same way, 2^-31.2), and below 2^-13 the linear term
  with x scaled by 2^64 so that neither the low product nor a subnormal
  result rounds twice; above, 1 − 2^k (hi + lo), k ≥ −23. The first build
  took the series up to 0.875 as AOR does, and its float tail (a quarter of
  the result there) measured 1.23 ulp; from 1/2 the kernel's pair makes the
  subtraction cost nothing. ±0 return early: the scaled product gives +0
  for −0 (`libm_thresholds`).
- **erfcf** (P11.04d2, §15): the same kernel gives every float within
  0.80 ulp, but its pair is good to about 2^-26 only, so a rounding test like
  §12's would send more than half of the arguments to the float-float code;
  §15's kernel is good to 2^-32.3.

## 14. Measured (P11.04d1)

Every finite float (4,278,162,238 arguments) against glibc's long double
`erfl` and `erfcl`; `erfcf` also in the three directed modes over every
float of [−4, 9.2] (2,173,907,765 arguments each, the reference bracketed
from `erfcl`, and below |x| = 2^-64, where `erfcl` rounds to 1, from the
sign of x):

| Function | double path | float path |
|---|---:|---:|
| `erff` | 0.5002 ulp | 0.7905 ulp |
| `erfcf` | correctly rounded (0.5000, D98's hard case) | correctly rounded (P11.04d2, §15) |

The double `erfcf` is correctly rounded on every argument to nearest and in
every direction, but for D98's hard case −0x1.d93ec4p-17 rounding upward,
where the strict code's table gives its to-nearest result in every mode, as
the other build types do. The float `erfcf` of §13 measured 0.8018 ulp.

Instructions per call (speed → fast):

| Function | x86_64 | armv7e-m-dp | armv7e-m | armv8-m.main |
|---|---:|---:|---:|---:|
| `erff` | 511 → 108 | 421 → 113 | 1383 → 123 | 1383 → 123 |
| `erfcf` | 1458 → 117 | 796 → 123 | P11.04d2 | P11.04d2 |

The cores without an FPU follow in P11.04e.

## 15. erfcf on the single-precision FPU cores (P11.04d2)

`erfcf` correctly rounded (D178), float only. These cores round to nearest
only (the layer's soft `<fenv.h>`, D57), so the rounding test has one
boundary to avoid, the halfway point. The idea is Ziv's (*Fast evaluation
of elementary mathematical functions with correctly rounded last bit*,
ACM TOMS 17(3), 1991): an approximation with a proven error bound, and a
slower exact path when the bound does not decide the rounding.

- **The kernel** (`__libc_fast_erfcf_accurate`), erfc a = 2^k (hi + lo) for
  0 ≤ a < 9.2 to 2^-32.3 relative (measured over every argument against
  `erfcl`, worst at 0x1.89181p-14):
  - e^(−a²): a² as an exact pair; n = round(−32a²/ln2), u = −a² − n ln2/32
    with n times ln2/32's float high part exact (its last bit at 2^-25, and
    |u| ≤ ln2/64 needs 19 bits); e^u = 1 + u + u²/2 + u³/6 + u⁴/24 (the next
    term 2^-39.6) with 1 + u by a two-sum; times 2^(j/32) from a 32-entry
    table of float pairs (`__libc_fast_exp2f_table`, 256 bytes); 2^-37 or
    better. The powers take u + u_lo, not u alone: u_lo (a²'s rounding and
    n ln2/32's low part) reaches 4·10⁻⁵ of u, and squaring u alone measured
    2^-29.6.
  - R(x) = erfc(x) e^(x²) in seven degree-9 pieces
    (`__libc_fast_erfcf_r_table`, 420 bytes): width 1/2 on [0, 3), so that
    R covers x near 0 too and erfc = e^(−x²) R needs no 1 − erf form, and
    above 3 x R as G(1/x²) on [1/9.2², 1/9]. The coefficients of t⁰ to t⁴ are
    float pairs and each later one is rounded in turn with the rest refitted
    (2^-37.0 to 2^-47.3 in exact arithmetic). c0 + c1 t + ... + c4 t⁴ is
    formed exactly, every product's error by a fused multiply-add and every
    sum by a two-sum, and only t⁵ Q(t), at most 2^-12.5 of R, in float: with
    four pair coefficients the float c4 t⁴ (2^-9.7 of R) measured 2^-30.7.
    t itself is a pair (a − 1/4 is inexact below 1/8, and 1/x² − c is formed
    from v² + 2v v_lo), its low part entering through R′(t); rounding t cost
    2^-27.
  - The product e^(−a²) R with every cross term: both pairs carry large low
    parts (2^-13 and 2^-11 of their high parts), and leaving out e_lo r_lo
    measured 1.76 ulp.
- **The test** (`erfcf`): f = hi + lo rounded, d its exact remainder; f is
  the correctly rounded result unless |d| lies within 2^-31 f of the halfway
  point (half an ulp of f, a quarter below a power of two): 2.4 times the
  measured error. 1.5 % of the arguments in [2^-26, 9.19] fall back to the
  layer's float-float `erfcf` (D63, kept as a static copy, since this file
  replaces that one), as do x beyond −3.9 and 9.19 (2 within a quarter ulp,
  and subnormal results), NaN (compared quietly) and the infinities; below
  2^-26, 1 − x as the strict file. For x < 0, 2 − 2^k (hi + lo) by a
  two-sum, at least 1, so erfc's error counts at most half.

Every finite float (4,278,162,238 arguments) correctly rounded (0.5001 ulp
at most, a near-halfway case).

Instructions per call: `erfcf` 1224 → 233 on armv7e-m and armv8-m.main (the float-float code
alone, for the 1.5 %, costs about as much as before).

## 16. The trigonometric functions on the cores without an FPU: `libc/math/nofp/fast/` (P11.04e1)

armv6-m, armv6s-m and armv7-m, in integers. The speed flavour's `sinf`,
`cosf`, `sincosf` and `tanf` run D136's double-precision kernel (`sinf`
2933 instructions on armv6-m, 842 on armv7-m) and their rounding test fails
about once in 2^35, so dropping the test, as P11.03e did, gains almost
nothing; the π forms have no fixed-point path at all and run the
float-float code on soft float (`sinpif` 17,579 and 8812). What costs is the
precision: a float result needs about 32 bits, and D87's kernel
(`__libc_sinf_fixed`, nofp/speed) has them (1.29 units of 2^-32, measured
over every float), at 720 instructions on armv6-m and 270 on armv7-m
(measured with the bench's plugin). D87 fed it from the float-float
reduction (`__libc_rem_pio2f`), which is what made it slow on soft float.

- **The reduction** (`__libc_fixed_trig_reduce`, 2^-6 ≤ |x| < 2^20): x =
  m 2^(e−23); x·32/π = m T 2^(e−147) with T = ⌊32/π 2^124⌋ (four words,
  computed here), a 152-bit product from four 32 × 32 → 64 products; the
  integer part k from bit 147 − e (modulo 64), the signed fraction to 2^-64
  below it (a fraction of 1/2 or more rounds k up and is taken negative), and
  s = f·π/32 to 64 bits, in the kernel's units of 2^-48 (error below one
  unit). Below 2^-6, s = x exactly; below 2^-12, x itself (sin, tan) or 1.
- **The kernel** (`__libc_fixed_trig_terms`, `__libc_fixed_trig_combine`):
  D87's, copied into nofp/fast/fixed-fast.h with s as an integer rather than
  a float pair (the speed file keeps its own, so the locked speed objects do
  not change): cos s − 1 and sin s/s − 1 in Q-format from u = s², and the
  9-entry table of sin and cos of jπ/32 in units of 2^-62. The terms depend
  on s only, so `sincosf`, `tanf` and `tanpif` form them once for both
  results. It declines (0) in a sine quadrant with a = 0 and |s| < 2^-14,
  where s would need more than 48 bits.
- **Rounding:** the top 32 bits below the leading one rounded to nearest
  (`__libc_fixed_nearest`, as P11.03e), with no test.
- **The fallbacks:** where the kernel declines, within 2^-14 of a multiple of
  π for sin (of π/2 for cos), and from 2^20, the strict layer's exact
  reduction and float-float core (`__libc_rem_pio2f`, `__libc_sinf_core`),
  not speed's file: both are correct for every argument, and the fallback is
  rare enough that its cost does not matter.
- **tanf:** sine over cosine as nofp/speed's `tanf` divides them (the
  quotient by the cosine's top 32 bits and a correction for its low half);
  either declining sends it to the float-float quotient.
- **The π forms** (`sinpif`, `cospif`, `tanpif`): 32x = K + r 2^-shift
  exactly from the bits (`__libc_fixed_pi_split`; K the nearest integer,
  |r| 2^-shift ≤ 1/2), s = r·π/32 2^-shift from one 32 × 64-bit product
  (`__libc_fixed_pi_s`), the kernel at K (K + 16 for cos); where it
  declines, at sinpi's zeros, ±s itself from that product, relative however
  small (`__libc_fixed_pi_small`; s²/6 is below 2^-30.6 there). So the π
  forms never fall back. The exact cases as the strict files (the multiples
  of 1/2, every float from 2^23 up, divide-by-zero at tanpi's poles), and
  below 2^-90 πx rounded once, as they do.
- **Size:** the reduction and the kernel's combination are out of line
  (`__libc_fixed_trig_reduce.c`, `__libc_fixed_trig_combine.c`): inlined in
  each caller, `math-mix`'s `sinf` and `cosf` took 604 bytes over armv6-m's
  locked fast figure (432 on armv7-m); the two calls cost 23 instructions on
  armv7-m and 38 on armv6-m.

Measured on the host over every finite float (the files built with the
`umull` product, a plain 64-bit multiplication): `sinf` 0.5024 ulp, `cosf`
0.5024, `tanf` 0.5022, `sinpif` 0.5084, `cospif` 0.5051, `tanpif` 0.5094;
`sincosf` equal to `sinf` and `cosf`, bit for bit, on every finite float.

Instructions per call (speed → fast):

| Function | armv6-m, armv6s-m | armv7-m |
|---|---:|---:|
| `sinf` | 2933 → 1090 | 842 → 379 |
| `cosf` | 2887 → 1100 | 810 → 379 |
| `sincosf` | 4418 → 1696 | 1160 → 654 |
| `tanf` | 5065 → 2178 | 1083 → 568 |
| `sinpif` | 17579 → 920 | 8812 → 369 |
| `cospif` | 17172 → 900 | 8578 → 347 |
| `tanpif` | 35613 → 2015 | 17671 → 563 |

## 17. erff on the cores without an FPU: `libc/math/nofp/fast/` (P11.04e2)

armv6-m, armv6s-m and armv7-m, in integers; `erfcf` keeps speed's code, as
D178 requires. The speed flavour's `erff` (D88) already runs in fixed point,
but as Taylor steps from the ODE recurrences: twelve terms of three products
each about the nearest node i/8, then a rounding test and the float-float
series behind it (2961 instructions on armv6-m, 1114 on armv7-m). Dropping
the test alone would leave the steps.

### 17.1 Sources

- **CORE-MATH `erff.c`**
  (<https://gitlab.inria.fr/core-math/core-math/-/raw/master/src/binary32/erf/erff.c>,
  read this time; §12.1 could not reach it), correctly rounded in double:
  below 0.4375 x times an even polynomial of degree 14 (eight terms in x²);
  up to 0x1.f5a888p+1, where erf rounds to 1, erf itself from 56 pieces of
  width 1/16, a degree-7 polynomial each in the offset from the piece's
  middle; ±1 beyond. No exponential: above about 1/2 erf is far from 0, so
  an absolute error suffices, and a direct fit of erf is as good as
  1 − e^(−x²) R(x).
- **picolibc `sf_erf.c`**
  (<https://raw.githubusercontent.com/picolibc/picolibc/main/libm/math/sf_erf.c>):
  the Sun float translation, FreeBSD's intervals (§12.1).
- §12.1's FreeBSD and Arm Optimized Routines files.

What we take is CORE-MATH's observation, scaled to the fast bound and to
32-bit integers: an absolute error of 2^-30 is 1/64 of erf's ulp above 1/2,
so wide pieces of low degree suffice.

### 17.2 The method

- **From 1/2 to 3.92**: seven pieces of width 1/2 (from 1/2, 1, ... 7/2);
  in each erf(c + s/2) = a0 + a1 s + ... + a7 s^7 with c the middle and
  |s| ≤ 1/2, fitted here (Chebyshev, 256-bit arithmetic) and rounded to
  signed Q31. |x| is exact in units of 2^-28 (below 2^30); t = 4(|x| − c)
  in Q31 is exact, and the high half of t·p is p·s, so Horner's rule takes
  seven 32 × 32 → 64 products. Fit degrees measured (worst absolute error
  over the seven pieces, rounded coefficients): degree 7, 2^-29.9; degree 8,
  2^-31.7; width 1/4 at degree 6, 2^-32.7 but twice the table. Degree 7
  (224 bytes) is the cheaper and is well inside the bound. The result p is
  in [0.52, 1) in Q31: its 32 bits below the leading one are rounded to
  nearest by `__libc_fixed_nearest` (a carry gives 1).
- **Below 1/2**: erf x = x (2/√π + u q(u)), u = x² in Q32 (0 below 2^-16,
  where u·q is below 2^-33 relative), q of degree 4 fitted here (2^-33.5
  relative) in signed Q31, 2/√π in unsigned Q31 (0x906eba82, as D88); the
  sum times x's significand normalised (`__libc_fixed_significand`, so
  subnormal arguments too) is one 64-bit product, rounded once by
  `__libc_fixed_product_float` (subnormal results included). Relative
  error about 2^-31 however small x.
- **Beyond 3.92**, ±1, as speed (erf rounds to 1 from 0x1.f5a888p+1); NaN
  through x + x, ±0 itself.

### 17.3 Measured (P11.04e2)

Every float on the host (the file compiled with the library's headers and
nofp/speed's `umull` helpers; the products are exact, so armv6-m's `muls`
helpers give the same bits), against `erfl`: at most **0.5223 ulp**
(0x1.7acf48p+0), 1,761,288 of 2^32 not correctly rounded; the bound is
1.5. Instructions per call (speed → fast, the bench's plugin):

| | armv6-m | armv7-m |
| --- | ---: | ---: |
| `erff` | 2961 → 414 | 1114 → 177 |

Test vectors (`libm_thresholds`): each side of 1/2, 1, 3/2, 2, 5/2, 3,
7/2 and 3.92, of 2^-16 and 2^-13, subnormal arguments (2^-149 and
−0x1.3p-140) and a normal result from one (0x1.fffffcp-127), and the
sweep's worst point, within 1.5 ulp.

## 18. atanf, atan2f and their π forms on the cores without an FPU: `libc/math/nofp/fast/` (P11.04f1)

armv6-m, armv6s-m and armv7-m, in integers. These functions had no
fixed-point path: speed runs the strict layer's float-float core
(`__libc_atan2f_core`) on soft float, 17,683 (`atanf`) and 18,355
(`atan2f`) instructions on armv6-m. P11.04f was split into f1 (here) and f2
(`asinf`, `acosf` and their π forms), as each needs its own fixed-point
building block: a division here, a square root there.

### 18.1 Sources

- **CMSIS-DSP `arm_atan2_q31.c`**
  (<https://raw.githubusercontent.com/ARM-software/CMSIS-DSP/main/Source/FastMathFunctions/arm_atan2_q31.c>):
  Q31 in and out; the smaller magnitude over the larger by `arm_divide_q31`,
  then one polynomial of degree 12 on [0, 1] in Q31 (absolute), π/2 − atan
  when |y| > |x|, ±π by the quadrant. Absolute accuracy only: a float result
  needs relative accuracy as the ratio goes to 0.
- **libfixmath `fix16_trig.c`**
  (<https://raw.githubusercontent.com/PetteriAimonen/libfixmath/master/libfixmath/fix16_trig.c>):
  Q16.16, r = (x − |y|)/(x + |y|) and a cubic in r about π/4 (or 3π/4),
  with a result cache; far below float accuracy.
- **Qfplib** (<https://www.quinapalus.com/qfplib.html>): soft float for
  Cortex-M0 and M3 with an arctangent of high accuracy (CORDIC, by its
  documentation; the sources were not read).
- **Arm Optimized Routines, vector `atan2f`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/atan2f.c>):
  the quotient by a hardware division, then atan(z) = z + z P(z²) on
  [0, 1]; §6's notes for the scalar files.

CORDIC needs no division but gives an absolute angle; near 0 a float needs
the quotient anyway. What we take: the reduction to a ratio in [0, 1] and
the quadrant offsets (all four), a polynomial in pieces where atan is far
from 0 (as §17 for erf), and the series in q² where it is not.

### 18.2 The method

- **The quotient** (`__libc_fixed_quotient`, fixed-fast.h): n/d for 24-bit
  significands normalised to bit 31, n < d (n is halved first when its
  significand is the larger: exact, as it has 24 significant bits), so the
  quotient is in [1/2, 1). 1/d from the linear seed 24/17 − 8/17·d
  (2^-4.1 for d in [1, 2)) and two Newton steps r + r(1 − dr) (2^-16), then
  q = n·r corrected once by its exact remainder, q + (n − q·d)·r, rounded
  (2^-31; eight 32 × 32 → 64 products). Armv6-M has no divide instruction,
  and the 64-bit division by the library call cost `tanf` about 900
  instructions there (P11.04e1). d = 1 is a shift. The quotient is clamped
  to 1/2: rounded down, an exact 1/2 (n = d/2) came out a unit below and
  took the piece index below its range (found by the `atan2f` sweep: 5411
  ulp).
- **The kernel** (`__libc_fixed_atan(a, b)`, a ≤ b, out of line, shared by
  the four files): a/b = q·2^k. Below 1/8, atan(a/b) = (a/b)(1 + u p(u)),
  u = (a/b)² in Q32 (0 below 2^-16), p of degree 2 fitted here (2^-32.2),
  so relative however small the ratio, subnormal ratios included. From 1/8
  to 1, a/b in Q32 and seven pieces of width 1/8, atan(c + s/8) of degree 5
  in s (Chebyshev fits in 256-bit arithmetic: a0 in unsigned Q32, the rest
  in signed Q31; within 2^-31.3, measured with the rounded coefficients;
  degree 6 gave only 2^-31.6, Q31's floor). a = b gives π/4. The result is a
  64-bit significand and an exponent.
- **The functions**: `atanf` is atan(|x|/1) up to 1 (the quotient is then
  a shift) and π/2 − atan(1/|x|) above; x itself below 2^-12, π/2 from 2^26.
  `atan2f` takes the smaller magnitude over the larger: θ, π − θ,
  π/2 − θ or π/2 + θ by the quadrant, with the strict file's special cases.
  The offsets are in units of 2^-62, so θ below 2^-62 counts for nothing.
  The π forms multiply θ by 2^33/π and use the offsets 1 and 1/2. One
  rounding to nearest; subnormal results through
  `__libc_fixed_product_float`.

### 18.3 Measured (P11.04f1)

On the host (the files compiled with the library's headers and nofp/speed's
`umull` helpers; the products are exact, so armv6-m's `muls` gives the same
bits): `atanf` over every float at most **0.5746 ulp** (0x1.00418cp-3, just
above the series' end), `atanpif` **0.5396**; `atan2f` and `atan2pif` on 2
billion pairs (half of them with nearby exponents) **0.5687** and
**0.5499**, and on 40 million edge pairs (exact power-of-two ratios, their
neighbours, subnormal y, exponent gaps to ±150) 0.5395 and 0.5362. The
bound is 1. Instructions per call (speed → fast, the bench's plugin; the π
forms have no bench case):

| | armv6-m, armv6s-m | armv7-m |
| --- | ---: | ---: |
| `atanf` | 17,683 → 751 | 8608 → 245 |
| `atan2f` | 18,355 → 812 | 8995 → 261 |

Test vectors (`libm_thresholds`): each side of 1/8 and of the 2^26 and
2^27 ends, the sweeps' worst points, tiny ratios to a subnormal and to zero,
and the exact-1/8 regression, within 1 ulp.

## 19. asinf, acosf and their π forms on the cores without an FPU: `libc/math/nofp/fast/` (P11.04f2)

armv6-m, armv6s-m and armv7-m, in integers: the second half of P11.04f. As
§18's functions, these had no fixed-point path (`asinf` 22,944 and `acosf`
23,761 instructions on armv6-m, the float-float core over a float-float
square root).

### 19.1 Sources

- **Arm Optimized Routines, vector `asinf`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/asinf.c>):
  below 1/2, asin x = x + x³ P(x²); above, the half-angle form
  asin |x| = π/2 − 2(y + y z P(z)) with z = (1 − |x|)/2 and y = √z, one
  polynomial P for both intervals (z is at most 1/4 as x² is).
- **FreeBSD `e_asinf.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_asinf.c>):
  a rational approximation in x² from 2^-12 to 1/2 (2^-30.1), x itself
  below 2^-12, and the same half-angle form above with a float `sqrtf`.
- **CMSIS-DSP `arm_sqrt_q31.c`**
  (<https://raw.githubusercontent.com/ARM-software/CMSIS-DSP/main/Source/FastMathFunctions/arm_sqrt_q31.c>):
  a fixed-point square root by Newton steps on 1/√x,
  r(3 − x r²)/2, from a start value (three steps), then x·r.

What we take: AOR's one polynomial for both intervals, and the square root
by Newton steps on its reciprocal, as CMSIS-DSP, with a seed table and a
final correction by the exact remainder.

### 19.2 The method

- **The series** K(u) = 1 + u p(u) = asin(√u)/√u for u ≤ 1/4: p of degree
  6 in signed Q31 (u p within 2^-33.7, fitted here); degree 5 gave only
  2^-29.8, as the series converges slowly at u = 1/4.
- **Up to 1/2**: asin |x| = |x| K(x²), x² in Q32 (0 below 2^-16), times
  x's normalised significand: relative however small x (the π forms reach
  subnormal results).
- **Above 1/2**: z = (1 − |x|)/2 is exact in integers: |x| = m 2^-24 makes
  z = w 2^-25 with w = 2^24 − m, so u = z in Q32 is exact as well. √z =
  √(2w)·2^-13: 2w shifted by an even count into [2^30, 2^32) as t in Q32,
  then √t (`square_root`): 1/√t from a 24-entry table of 16-bit seeds over
  [1/4, 1) (2^-5.1, computed here), two Newton steps r + r(1 − t r²)/2
  (2^-18.6), s = t r, and s corrected once by its exact remainder,
  s + (t − s²) r/2, within a unit of 2^-32 (measured over every 997th t).
  The remainder may be negative: the last step can round r up by a unit,
  which the first build missed (taken as positive, 216–322 ulp in the
  sweeps, near x = 0.8). 2√z·K(z) then gives π/2 minus it (asin), itself
  (acos, x > 1/2) or π minus it (acos, x < −1/2).
- **The kernel** `__libc_fixed_asin` (out of line, shared by the four files)
  returns the product as a normalised 64-bit significand and an exponent,
  and whether it is the half-angle form; the callers add §18's offsets
  (π/2, π; 1/2, 1 for the π forms, which first scale by 2^33/π). x itself
  below 2^-12 (asin), π/2 and 1/2 below 2^-26 (acos, acospi), +0 at 1; the
  domain errors as the strict files.

### 19.3 Measured (P11.04f2)

Every float on the host (as §18): `asinf` at most **0.5083 ulp**, `acosf`
**0.5087**, `asinpif` **0.5240**, `acospif` **0.5149**; the bound is 1.
Instructions per call (speed → fast, the bench's plugin; the π forms have
no bench case):

| | armv6-m, armv6s-m | armv7-m |
| --- | ---: | ---: |
| `asinf` | 22,944 → 730 | 11,311 → 239 |
| `acosf` | 23,761 → 749 | 11,794 → 244 |

Test vectors (`libm_thresholds`): each side of 2^-12, 2^-16 and 2^-26, a
subnormal `asinpif`, the sweeps' worst points, and the square root's
regression, within 1 ulp.
