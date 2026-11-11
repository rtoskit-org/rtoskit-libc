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
