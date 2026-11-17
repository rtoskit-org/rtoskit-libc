# Research: libm part 3, trigonometric, pi-scaled and hyperbolic functions

Step: P1.15. Decisions: PLAN D7, D45, D47, D48.

## Sources reviewed

- M. Payne and R. Hanek, *Radian reduction for trigonometric functions*,
  SIGNUM Newsletter 18(1), 1983: reduction by multiplying the significand
  with a window of the bits of 2/π chosen from the exponent. Bits earlier in
  the window only add multiples of 4 (whole turns). This is exact for every
  double.
- K. C. Ng, *Argument reduction for huge arguments: good to the last bit*
  (Sun, 1992), and the fdlibm/musl `__rem_pio2` (structure only):
  - Cody–Waite with π/2 split into 33-bit parts for medium arguments;
  - the worst case for doubles, x = 6381956970095103 · 2^797, whose
    distance to a multiple of π/2 is about 2^-61 relative, which sets the
    number of bits the reduction must keep;
  - the 2/π table layout (`a2f9836e 4e441529 fc2757d1 …`).
- J.-M. Muller, *Elementary Functions: Algorithms and Implementation*,
  3rd ed. (2016), chapter 11 (range reduction, hard cases) and chapter 6
  (table-driven methods).
- P. T. P. Tang, *Table-driven implementation of the expm1 function* and the
  arctan method of the same series: atan(t) = atan(c) + atan((t - c)/(1 + tc))
  over a grid of c.
- ARM optimized-routines `math/sin.c`, `math/cos.c` and the `sincosf`
  tables (structure only). They use polynomial kernels on |r| ≤ π/4 and
  Payne–Hanek beyond 2^20 for float.
- CORE-MATH (<https://core-math.gitlabpages.inria.fr/>): correctly rounded
  `sinpi`, `cospi`, `tanpi`, `asinpi`, `acospi`, `atanpi`, `atan2pi` (float),
  plus the exactness rules at integers and half-integers.
- IEEE 754-2019 9.2 and 9.2.1: the special values of sinPi, cosPi, tanPi,
  atanPi and atan2Pi.
  - sinPi(+n) is +0 and sinPi(−n) is −0.
  - cosPi(n + ½) is +0.
  - tanPi(+n) is +0 for even n and −0 for odd n, mirrored for −n.
  - tanPi(n + ½) is ±∞ (+ for even n), with divide-by-zero.
- The standard: C23 7.12.4 and 7.12.5, Annex F.10.1 and F.10.2.
- libc-test `src/math/{crlibm,ucb,sanity,special}` for sin, cos, tan, asin,
  acos, atan, atan2, sinh, cosh, tanh, asinh, acosh, atanh (f/l).

## Trade-offs

| Option | For | Against |
|---|---|---|
| fdlibm kernels (`__sin`, `__cos`, `__tan`, rational atan) | Small and fast | About 1 ulp, every function designed separately, and tan near π/2 is delicate |
| Sine/cosine tables (sin(k/64), cos(k/64)) | Shorter polynomials | 1.6 KiB of tables, plus a second addition formula |
| Taylor kernels with a double-double head, sharing the D47 machinery | One error analysis, about 2^-60 relative before the final rounding, and tan as a double-double quotient | Longer polynomials (r^17 and r^18) |
| Payne–Hanek for every argument | One reduction path | Slower for the common small arguments |

Chosen:
- Cody–Waite below 2^20 and Payne–Hanek above.
- Taylor kernels with the r³/6 (sin) and r²/2 (cos) terms exact as
  double-doubles.
- Tang's table method for atan (33 entries of atan(k/32)).
- Every other inverse function expressed through one atan2 core.
- Hyperbolic functions from Taylor series below 1 and from the D47 exp core
  above.

The cores run rounding to nearest. The result is rounded once, in the
caller's direction.

## Algorithms

- **`__libc_rem_pio2(x, &r)`** returns n mod 4 and r = x - nπ/2 as a
  double-double.
  - Below 2^20 it uses Cody–Waite with π/2 in four parts: 33, 33, 53 and
    53 bits, 172 bits in all. n · p1 and n · p2 are exact
    (n < 2^20), x - n · p1 is exact by Sterbenz, and n · p3 is an exact
    product.
  - Above 2^20 it uses Payne–Hanek with 32-bit limbs, which avoids 128-bit
    integers on Arm.
    - x = m · 2^e. The window starts at the bit of 2/π whose weight times
      2^e is 4, since earlier bits add whole turns.
    - Seven words (224 bits) times the 53-bit m give a 277-bit product with
      at least 191 fraction bits. The truncation error, 2^-138, is far
      below the 2^-61 · 2^-70 the worst case needs.
    - A fraction of ½ or more becomes 1 - fraction and moves to the next
      quadrant.
    - The leading 106 bits become a double-double, which is multiplied by
      π/2.
  - The 2/π table holds 40 words (1280 bits), enough for x up to `DBL_MAX`.
- **`__libc_sin_core(r)`** and **`__libc_cos_core(r)`**: superseded in P11.07m
  (D228, [`trig-cores.md`](trig-cores.md)). The original cores (r − r³/6 plus a
  Taylor tail from r⁵ in double; 1 − r²/2 plus a tail from r⁴ in double, with the
  low part through first-order terms) were 0.02 and 0.10 ulp from exact, which
  misrounded 0.3–0.6 % of results. sin now carries r³(1/6 − r²/120) in
  double-double with a five-term fitted tail from r⁷, and cos is 1 − 2 sin²(r/2).
- **sin, cos, tan** select the core by quadrant. tan is sin/cos, or
  -cos/sin in the odd quadrants, as a double-double quotient, so it stays
  accurate near the poles. Tiny arguments (below 2^-27) round x or its
  neighbour by hand (D47), and cos of a tiny x is 1 - tiny.
- **sinpi, cospi, tanpi** reduce exactly: n = roundeven(2x) and
  r = x - n/2 (|r| ≤ ¼). The angle πr is an exact product plus r · π.lo.
  - Zeros and ±1 at the integers and half-integers are returned directly,
    with no inexact, following the IEEE sign rules.
  - Beyond 2^52 every x is an integer, with the parity taken from
    `fmod(x, 2)`.
  - A tiny x gives π · x, rounded once through the 2^200 scaling of
    `__libc_exp_scale` (subnormal results included).
- **`__libc_atan2_core(y, x)`** takes double-double coordinates.
  - It divides the smaller absolute value by the larger, giving t ∈ [0, 1].
  - Then atan(t) = atan(k/32) + atan(u), with u = (t - c)/(1 + tc) and
    |u| ≤ 1/64. t - c is exact by Sterbenz, and atan(u) is the series to
    u^11.
  - The result is adjusted to π/2 - a if the coordinates were swapped, to
    π - a for a negative x, and takes the sign of y.
  - Below t = 2^-30 it returns t, because the cube would underflow.
- **Functions built on the atan2 core:**
  - asin(x) = atan2(x, √((1-x)(1+x))) and acos(x) = atan2(√((1-x)(1+x)), x).
    The radicand is a double-double product of two exact sums, and its square
    root is corrected by the residual.
  - atan(x) = atan2(x, 1), and ±π/2 beyond 2^60.
  - atan2 scales both coordinates by one power of two.
    - A quotient beyond 2^60 gives ±π/2.
    - A quotient below 2^-60 with x < 0 gives ±π.
    - Below 2^-60 with x > 0 the result is y/x, nudged toward zero when the
      quotient is exact (atan(q) < q) and rounded once through the 2^200
      scaling.
    - Below 2^-1080 it is an underflow.
- **The pi-scaled inverses** multiply the angle by 1/π as a double-double.
  The exact cases are returned directly: ±½, ±¼, ±¾, 1 and 0.
- **Hyperbolic functions:**
  - sinh and cosh below 1 use Taylor series (x³/6 and x²/2 exact), up to
    x^21 and x^20.
  - Between 1 and 22 they use (e^x ∓ e^-x)/2, with e^x a double-double from
    the exp core.
  - Beyond 22 they use e^|x|/2, through `__libc_exp_scale` with exponent
    k - 1, so the overflow threshold is exact.
  - tanh is sinh/cosh below 1, then 1 - 2/(e^2|x| + 1), and ±(1 - tiny)
    beyond 22.
  - asinh(x) = log1p(|x| + x²/(1 + √(1 + x²))), and log(2|x|) beyond 2^28.
  - acosh(x) = log1p(t + √(2t + t²)) with t = x - 1, and log(2x) beyond
    2^28.
  - atanh(x) = log1p(2|x|/(1 - |x|))/2.
  - `__libc_log1p_dd` is log1p of a double-double: 1 + u is formed exactly,
    then log(s) + t/s.
- **Float** functions return `(float)double_function(x)`. Below 2^-30, the
  functions with f(x) ≈ x use `__libc_math_tinyf(x, direction)`, where the
  direction is the sign of f(x) - x:
  - -x for sin, atan, tanh and asinh;
  - +x for tan, asin, sinh and atanh.

  At ±FLT_MIN a nudge toward zero is tiny before rounding (Arm's rule) even
  though it rounds back to FLT_MIN. In that case x is returned with inexact
  only, unless the rounding direction takes the result below FLT_MIN.
  `atan2f` nudges a tiny quotient toward zero by 2^-40 relative before the
  conversion.
- **Long double** functions call the double ones (D7).

## Edge cases

- sin(±0) and tan(±0) are ±0. sin(±∞), cos(±∞) and tan(±∞) are domain
  errors (NaN, invalid).
- cos(±0) is exactly 1, and cos(tiny) is 1 with inexact (nextdown(1) when
  rounding downward).
- acos(1) is +0 in every direction. asin(±1) and acos(-1) are π/2 and π,
  rounded.
- atan2 follows the full F.10.1.4 table: atan2(±0, -0) is ±π,
  atan2(±∞, -∞) is ±3π/4, and so on.
- sinh and cosh overflow exactly at log(2 · DBL_MAX) ≈ 710.4758.
  atanh(±1) is ±∞ with divide-by-zero. acosh(x < 1) is a domain error.
- tanh(±large) is ±1 with inexact, and nextdown(1) when rounding toward
  zero or downward for x > 0.

## Tests and vectors

- libc-test: the 39 `math/` tests of these functions and
  `functional/strtod_simple` (which needed `sin`) pass on all six boards.
- `test/unit/libm_trig/vectors.inc` holds 6000 vectors, 150 for each of the
  20 functions in double and float, generated from mpmath at 1400 bits. They
  include hard reductions: the doubles nearest to multiples of π/2, the
  2^797 worst case, `DBL_MAX` and arguments around 2^20.
- `test/unit/libm_trig` uses the bounds of D47. The worst cases seen to
  nearest are:
  - 0.51 ulp for double (tanh, tanpi);
  - 0.50 ulp for float.

  It also checks:
  - the F.10.1 and F.10.2 special cases;
  - the IEEE rules for the pi functions at every integer and half-integer
    in [-6, 6] and beyond 2^52;
  - the tiny arguments in every direction;
  - the subnormal results of the pi-scaled functions and of atan2;
  - the overflow thresholds.
