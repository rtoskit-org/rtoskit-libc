# Research: libm part 2, exponentials, logarithms, powers and roots

Step: P1.14. Decisions: PLAN D7, D44, D45, D46, D47.

## Sources reviewed

- P. T. P. Tang, *Table-driven implementation of the exponential function
  in IEEE floating-point arithmetic*, ACM TOMS 15(2), 1989. Reduces
  x = (32m + j) ln2/32 + r, takes 2^(j/32) from a table as a high and a low
  part, and evaluates exp(r) - 1 as a short polynomial. The error analysis
  stays below 0.54 ulp.
- P. T. P. Tang, *Table-driven implementation of the logarithm function in
  IEEE floating-point arithmetic*, ACM TOMS 16(4), 1990. Uses
  x = 2^m (c + f) with c = 1 + j/128, log(x) = m ln2 + log(c) + log1p(f/c),
  a table of log(c), and an odd/even polynomial in f/c.
- FreeBSD msun and musl `src/math`, for structure only: `e_exp.c`, `e_log.c`
  and `e_pow.c` (the fdlibm rational approximations with a 2-word ln2),
  `s_cbrt.c` (a bit-level first guess, then Newton and a rounding to 21 bits
  before the final step), and `e_hypot.c` (scaling by 2^±600 and an exact
  square).
- ARM optimized-routines (<https://github.com/ARM-software/optimized-routines>),
  `math/exp.c`, `math/log.c`, `math/pow.c`: 2^(j/N) tables with N = 128,
  `pow` built on a log carrying about 68 bits as a double-double, and the
  observation that an exp core taking a double-double argument serves
  `exp`, `pow` and the `exp*m1` family alike.
- CORE-MATH (<https://core-math.gitlabpages.inria.fr/>) and the glibc 2.40
  imports of its `exp10m1f`, `exp2m1f`, `log2p1f`, `log10p1f`, `compoundf`,
  `rsqrtf`: correct rounding through a fast path plus Ziv's rounding test.
  This is the long-term target. The present step stops at a double-double
  fast path (see Accuracy).
- T. J. Dekker, *A floating-point technique for extending the available
  precision* (1971), and D. Knuth, TAOCP vol. 2, 4.2.2: TwoSum,
  Fast2Sum, the Veltkamp split (2^27 + 1) and the exact product. All of them
  are exact **only when rounding to nearest**, which is why the cores switch
  to that direction.
- J.-M. Muller et al., *Handbook of Floating-Point Arithmetic*, 2nd ed.
  (2018), chapters 4 and 10: exactness of x - (x ⊕ y) under Sterbenz,
  double-double error bounds, and the tininess rules.
- C. F. Borges, *An improved algorithm for hypot(a, b)* (arXiv:1904.09481,
  2019): corrects a square root by the residual (a² + b² - h²)/(2h) computed
  with an exact square, which gives a result within one rounding.
- IEEE 754-2019 9.2 and 9.2.1: the recommended operations `compound`,
  `rootn`, `pown`, `powr`, `exp2m1`, `exp10m1`, `log2p1`, `log10p1`,
  `rSqrt`, with their special cases. `powr(±0, -∞)` is +∞ without
  divide-by-zero.
- The standard: C23 7.12.6 and 7.12.7, Annex F.10.3 and F.10.4. Annex F.10
  allows an undeserved inexact.
- libc-test `src/math/{crlibm,ucb,sanity,special}`: the tables these
  functions are checked against (exp, exp2, exp10, expm1, log, log2, log10,
  log1p, pow, cbrt, hypot, sqrt and their f/l forms).

## Trade-offs

| Option | For | Against |
|---|---|---|
| fdlibm rational approximations (musl, newlib) | Small tables (none for exp/log) | Below 1 ulp only to nearest. Every function is a separate design. `pow` needs its own extra-precise log. |
| Table-driven double-double cores (Tang, ARM) | One `exp` core and one `log` core serve 20 functions. Errors stay far below 1 ulp. Directed modes come almost for free. | Tables of 64 + 92 double-doubles (2.4 KiB). Needs double-double arithmetic everywhere. |
| Correctly rounded (CORE-MATH) | Exact in every direction | A slow path for every function and much more code. Too much for this step on soft-float cores. |

Chosen: the table-driven double-double cores. The relative error of a core
is about 2^-68, so the single final rounding gives a result within
0.5 + 2^-15 ulp to nearest. Float functions call the double ones, and the
second rounding from double to float is then correct except within
2^-29 of a midpoint.

## Algorithms

- **`__libc_exp_core(xh, xl)`** handles exp of a double-double. It reduces
  with n = round(x · 64/ln2) and ln2/64 split into a 32-bit high part (so
  n · high is exact for |n| < 2^21) and a low part, giving r = x - n ln2/64
  as a double-double with |r| ≤ ln2/128. The table holds 2^(j/64) for
  j = 0..63 as double-doubles. exp(r) - 1 is the Taylor series to r^7, with
  a remainder below 2^-67. The core returns 2^(j/64) · exp(r) as a
  double-double plus the binary exponent k = (n - j)/64.
- **`__libc_exp_scale(value, k)`** does the one final rounding. A normal
  result rounds hi + lo and scales exactly with `scalbn`, which raises
  overflow. A subnormal result would be rounded twice that way, so it is
  scaled by 2^(k + 1022) into (-1, 1) instead, which is exact for hi.
  Adding ±1 moves the sum's ulp to 2^-52, which matches the subnormal ulp
  under the same scaling. One addition therefore rounds in the caller's
  direction, and the bias comes off exactly. A zero takes the sign of the
  bias, because 1 - 1 is -0 when rounding downward. Underflow is raised when
  that addition was inexact (tininess before rounding, as Arm does).
- **`__libc_log_core(x)`** uses x = 2^e · m with m in [√2/2, √2],
  c = 1 + j/128 the nearest grid point (j = -38..53, 92 table entries of
  log(c) as double-doubles), and f = (m - c)/c as a double-double
  (m - c is exact). log1p(f) is f - f²/2 + f³·P(f), the series to f^9.
  The f²/2 term is exact. ln2 is split as 0x1.62e42fefa3800p-1 (e · high is
  exact) plus 0x1.ef35793c76730p-45. Near 1 (e = 0, c = 1) the result keeps
  its relative accuracy.
- **`__libc_log1p_core(x)`** forms 1 + x = s + t exactly (TwoSum), then
  log(s) + t/s. Its correction term is below 2^-53 relative.
- **`__libc_expm1_core`**: for |z| < 2^-10 the Taylor series to z^7.
  Otherwise exp(z) - 1, with the subtraction exact on the high part
  (Sterbenz when the scale is 1, TwoSum after scaling) and the low part
  added last.
- **Derived functions:**
  - `exp2(x)` and `exp10(x)` multiply x by ln2 or ln10 (double-doubles) and
    call the exp core. `exp2` of an integer is exact.
  - `exp2m1` and `exp10m1` go through expm1 of the double-double product.
  - `log2` and `log10` multiply the log core's result by 1/ln2 or 1/ln10.
    `log2` of a power of two is exact, subnormal powers included.
  - `log2p1` and `log10p1` do the same with the log1p core. `logp1` is
    `log1p`.
- **`pow(x, y)`** handles the special cases of F.10.4.5 first. A power of
  two raised to a power that gives an integer exponent is exact, and so is
  y = 1. Otherwise it computes y · log|x| as a double-double (an exact
  product plus y · log.lo), then calls the exp core. The sign is applied
  before the final rounding, so a directed mode rounds a negative result
  the right way.
- **`powr`** is `pow` without the integer cases, defined for x ≥ 0.
  powr(0, 0), powr(∞, 0) and powr(1, ±∞) are domain errors.
- **`pown`**, **`compoundn`**: exp(n · log) for a 64-bit n, computed in
  `__libc_pow_integer`. n is split into a part with the low 11 bits clear
  (at most 52 significant bits) and the rest. Each part times log.hi is an
  exact product, so n · log keeps double-double accuracy for every n.
  `compoundn` uses the log1p core.
- **`rootn(x, n)`** is exp(log|x| / n), with the quotient carried as a
  double-double; **`rsqrt`** is exp(-log(x)/2).
- **`cbrt`**: x = y · 2^(3q) with y in [1, 8). Five Newton steps from a
  linear guess give about 2^-30, rounded to 20 bits so t² is exact. One
  Halley step t + t(s - t)/(2t + s) with s = y/t² then leaves at most
  0.62 ulp. Since D125, a last correction by the residual
  (y − t³)/(3t²), with t³ as a double-double and the sign applied before the
  final rounding, makes it correct to about 2^-100
  (`docs/research/libm-vectors.md`).
- **`hypot`** scales by 2^∓600 when an operand is beyond 2^±500. It adds
  the exact squares as double-doubles, takes the square root, and corrects
  it by the residual (S - h²)/(2h) (Borges). When the exponents differ by
  more than 60 the result is |a| + |b|. That test uses `ilogb`, because
  b · 2^60 would overflow.
- **`sqrt`**: the exact digit-by-digit core of P1.13 (correctly rounded in
  every direction). On cores with an instruction, the FPU layer replaces it:
  `vsqrt.f64` on fpv5-dp, `vsqrt.f32` on fpv5-sp and fpv4-sp, and
  `sqrtsd`/`sqrtss` on x86_64. x86_64 thereby gains a `LIBC_MATH_LAYERS`
  entry of its own.
- **Long double** functions call the double ones (D7). **Float** functions
  return `(float)double_function(x)`.

## Rounding direction (D47)

TwoSum, Fast2Sum and the Veltkamp product are exact only when rounding to
nearest. In a directed mode, 1 + x for x ≈ -2^-58 rounds to 1 - 2^-53, and
x - (s - 1) then needs 57 bits. The first version of `log1p` was 20 to
70 ulp off in that region. The cores (`exp`, `log`, `log1p`, the TwoSum
steps of `expm1` and of the subnormal path of `__libc_exp_scale`) therefore
save the direction, round to nearest, and restore the direction before the
final rounding, which happens in the caller's direction. On cores without
directed rounding `fegetround` returns `FE_TONEAREST` and nothing is set.

## Tiny arguments

When f(x) = x + e with 0 < |e| < ulp(x)/2 (expm1, log1p and their
relatives below 2^-60), `__libc_math_tiny(x, direction)` computes
(x · 2^100 + copysign(2^-1074, direction)) · 2^-100. The current direction
then picks x or its neighbour and raises inexact. Underflow is raised for a
subnormal x. Float functions cannot rely on the double path here, because
the double result is exactly x and its conversion raises nothing. Below
2^-30 they use `__libc_math_tinyf`: x ± 2^-40|x| is exact in double and lies
well inside half a float ulp, so the conversion rounds correctly and raises
underflow for a subnormal result.

## Edge cases

- exp(-745.1) is 2^-1074 to nearest. In RD exp(-1000) is +0, never -0. In
  RZ exp(1000) is `DBL_MAX`. exp2(-1075) is 0 (a tie, rounded to even).
- pow(2, -1074) and pow(2^700, -1.5) = 2^-1050 are exact, with no
  underflow. pow(-2, -1077) is -0 to nearest and -2^-1074 when rounding
  downward.
- log(1) is +0 in every direction. log(±0) is -∞ with divide-by-zero.
  log1p(-1) is -∞ with divide-by-zero.
- hypot(±∞, NaN) is +∞. hypot(1.4·2^1023, 1.2·2^1023) has no spurious
  overflow.
- compoundn(x, 0) is 1 for every x ≥ -1 and for NaN, but compoundn(-2, 0)
  is a domain error. compoundn(-1, n < 0) is +∞ with divide-by-zero.
- rootn(x, 0) and rootn(x < 0, even n) are domain errors.
  rootn(-0, odd n > 0) is -0.

## Tests and vectors

- libc-test: the 36 `math/` tests of this step's functions pass on all six
  boards, giving 123 math tests in all.
- `test/unit/libm_exp/vectors.inc` holds 6000 vectors, 150 for each of the
  20 functions in double and float. Each has random arguments over the
  whole useful range (subnormal results included) and the exact value as a
  double-double, from Python `decimal` at 200 digits (exp, ln, sqrt).
  `test/unit/libm_exp` requires at most 1 ulp (double) and correct rounding
  (float) to nearest, and 2 and 1 ulp in the directed modes. The worst
  cases seen to nearest are 0.61 ulp (cbrt) and 0.59 ulp (log2p1); the
  rest are within 0.5 ulp. It also covers:
  - the F.10.3 and F.10.4 special cases;
  - the C23 functions libc-test lacks (`exp2m1`, `exp10m1`, `logp1`,
    `log2p1`, `log10p1`, `pown`, `powr`, `rootn`, `rsqrt`, `compoundn`);
  - the exact cases;
  - tiny arguments in every direction;
  - overflow and underflow boundaries.
- `test/unit/fpu_math` compares the `sqrt`/`sqrtf` instructions with the
  generic exact code on every layered core (x86_64 included), bit for bit
  and flag for flag.
