# Research: libm part 4, erf, erfc, lgamma and tgamma

Step: P1.16. Decisions: PLAN D7, D45, D47, D49.

## Sources reviewed

- W. J. Cody, *Rational Chebyshev approximations for the error function*,
  Math. Comp. 23 (1969), and the fdlibm/musl `s_erf.c` (structure only).
  They use rational approximations on [0, 0.84], [0.84, 1.25], [1.25, 1/0.35]
  and [1/0.35, 28], with erfc(x) = exp(-x² - 0.5625) · R(x)/x for large x.
  This is about 1 ulp; each interval needs its own minimax fit.
- M. Abramowitz and I. Stegun, 7.1.5 and 7.1.14 (the Maclaurin series of erf
  and the continued fraction of erfc), and DLMF 7.10 (the derivatives of
  erf through Hermite polynomials).
- W. Gautschi, *Efficient computation of the complex error function*
  (SIAM J. Numer. Anal. 1970): convergence of the Laplace continued fraction
  for large arguments.
- Stirling's series and the Bernoulli numbers (DLMF 5.11.1), the Taylor
  series of log Γ(1 + z) and log Γ(2 + z) in ζ(k) (DLMF 5.7.3), and the
  reflection Γ(x)Γ(1 - x) = π/sin(πx) (DLMF 5.5.3).
- C. Lanczos (1964) and P. Godfrey's coefficients, used by Boost and glibc's
  `tgamma`: rejected. The rational form loses relative accuracy at the zeros
  of lgamma, and the error analysis is harder than for Stirling with a
  shift.
- CORE-MATH (<https://core-math.gitlabpages.inria.fr/>): correctly rounded
  `erff`, `erfcf`, `lgammaf`, `tgammaf` (float) and their hard cases.
- The standard: C23 7.12.8 and Annex F.10.5.
- libc-test `src/math/{sanity,special,ucb}` for erf, erfc, lgamma and tgamma
  (f/l). The lgamma tests read the XSI `signgam`.

## Trade-offs

| Option | For | Against |
|---|---|---|
| fdlibm rational approximations | Small, fast | About 1 ulp; each interval needs a minimax fit, which would have to be regenerated |
| Series, table and continued fraction with double-double heads | Uses only exactly derivable coefficients (Taylor, Hermite, Bernoulli, ζ); error below 2^-60 before the final rounding | More arithmetic; a 57-entry erfc table (1.8 KiB) |

Chosen: the second option. Its coefficients come from the definitions, not
from fits, which suits clean-room work and makes the accuracy argument
direct.

## Algorithms

- **erf, below ½**: the Maclaurin series (2/√π) Σ (-1)^n x^(2n+1)/(n!(2n+1)).
  x - x³/3 is computed as double-doubles, and the terms n = 2 … 13 in
  double, since the next term is below 2^-62 relative.
- **erfc, from 15/32 to 129/32**: a table of erfc(c) and (2/√π)e^(-c²) for
  c = ½ + k/16 (57 entries).
  - erfc(c + h) = erfc(c) - (2/√π)e^(-c²) · S, with
    S = Σ (-1)^(m-1) H_(m-1)(c) h^m/m!, which is the Taylor series of
    ∫ e^(-t²).
  - The Hermite polynomials come from their recurrence
    H_(n+1) = 2cH_n - 2nH_(n-1).
  - With |h| ≤ 1/32 the terms fall below 2^-60 of S by m = 15.
  - The cancellation erfc(c) - … loses at most 2 bits at c = 4.
- **erfc, from 4 to 27.3**: erfc(x) = e^(-x²)/(√π K(x)), where K is the
  Laplace continued fraction x + ½/(x + 1/(x + (3/2)/(x + …))).
  - K is evaluated backwards from 30 levels as double-doubles. Python
    experiments show below 2^-68 relative from x = 4; 2 needs 80 levels,
    which is why the table covers [½, 4).
  - x² is an exact product. e^(-x²) comes from the D47 exp core, and the
    result is scaled once, so subnormal results round once.
  - Beyond 27.3 the result underflows.
- **The other cases**:
  - erf(x) = 1 - erfc(x) above ½;
  - erfc(x) = 1 - erf(x) below ½;
  - erfc(-x) = 2 - erfc(x);
  - erf saturates to ±(1 - tiny) from 6, and erfc to 2 - tiny below -6;
  - for |x| < 2^-28, erf(x) = (2/√π)x rounded once, through the 2^200
    scaling (subnormal results included).
- **lgamma of positive x**:
  - From 12 up, Stirling's series:
    (y - ½) log y - y + ½ log 2π + Σ B_2k/(2k(2k - 1)y^(2k-1)). The
    product and the logarithm are double-doubles; ten terms are below 2^-66
    of the result at 12.
  - Below 12, it shifts up: log Γ(x) = log Γ(x + n) - log(x(x+1)…(x+n-1)).
    x + k and the product are double-doubles, so no rounding of x + n enters.
  - Within ¼ of 1 and 2, it uses the ζ series of log Γ(1 + z) and
    log Γ(2 + z), to k = 40. This keeps the relative accuracy at the zeros 1
    and 2. ζ(k) - 1 is tabulated, the first two terms are double-doubles.
  - Beyond 2^60, x(log x - 1) is computed as a scaled double-double with one
    rounding. Stirling's other terms are below 2^-60 relative there.
- **lgamma of negative x**: the reflection log π - log|sin πx| -
  log Γ(1 - x).
  - 1 - x is exact as a double-double.
  - sin πx comes from the exact reduction x = n + f, |f| ≤ ½, through the
    D48 kernels, which also give the sign of Γ.
- **tgamma**:
  - The factorials up to 22! are exact products (23! needs 56 bits).
  - |x| < 2^-54 gives 1/x.
  - Otherwise tgamma is exp(lgamma) with the reflection's sign, through the
    exp core and one scaled rounding. Overflow comes past 171.624 and
    underflow below about -184, with the sign of Γ.
- **Special cases (F.10.5)**:
  - erf(±0) = ±0; erf(±∞) = ±1; erfc(-∞) = 2; erfc(+∞) = +0.
  - lgamma(1) = lgamma(2) = +0. Poles give +∞ with divide-by-zero.
    lgamma(±∞) = +∞.
  - tgamma(±0) = ±∞ with divide-by-zero. A negative integer or -∞ is a
    domain error.
- **Float functions** return `(float)double_function(x)`; **long double**
  ones call the double ones (D7).

## signgam

libc-test's lgamma tests read `signgam` (XSI). C does not reserve the name,
so the library cannot define it. The test shim provides it:
`test/libc-test/shim/include/signgam.h` is forced into the math suite with
`-include`, and wrappers record the sign of Γ(x) beside the library's
unchanged lgamma.

## Tests and vectors

- libc-test: `math/erf`, `math/erfc`, `math/lgamma`, `math/tgamma` (f/l,
  12 tests) pass on all six boards.
- `test/unit/libm_special/vectors.inc` holds 2400 vectors, 300 for each of
  erf, erfc, lgamma and tgamma, in double and float, generated from mpmath
  at 600 bits. They include the regions near the zeros of lgamma at 1 and 2,
  negative arguments, and x up to 2^1000.
- The worst error to nearest is 0.50 ulp, in double and float.
- Also checked: the F.10.5 special cases, the exact factorials 0! … 22!,
  and the overflow and underflow boundaries (erfc(27), erfc(30), tgamma
  past 171.6 and below -190, and the sign of Γ there).

## Phase 12: the strict `erf` below 2^-28 and `lgamma` (P12.01, Q57, Q62)

The vectors above (600-bit mpmath at chosen points) found neither defect.
These measurements took the x86_64 release `libc.a` with every symbol
renamed (`objcopy --prefix-symbols`), linked into a host program and
compared with `libquadmath`. `lgammaq` was itself checked against 300-bit
mpmath at the 3562 points around the negative zeros, and none was
misrounded.

### Which files

- **`erf`:** `libc/math/erf.c` alone, on every core in debug, release and
  speed. `erfl` calls it. In `fast`, `libc/math/fast/erf.c` and
  `dp-fixed/fast/erf.c` replace it everywhere.
- **`lgamma`:** `libc/math/__libc_lgamma_core.c`, through `lgamma.c` and
  `lgammal.c`, on every core in debug, release and speed (`fast` has its own
  files, D225 and D226). The generic `lgammaf` rounds the double result once
  more: its sweeps below show no misrounding, since the double's error is far
  below a float's ulp everywhere sampled. The single-precision FPU cores'
  `lgammaf` has its own core (`sp-nearest/__libc_lgammaf_core.c`).

### `erf` below 2^-28 (Q57)

erf(x) = (2/√π)(x − x³/3 + …). Below 2^-28 the strict file returns x·2/√π,
an exact pair rounded once (`__libc_dd_scale_product`, through the 2^200
scaling so that subnormal results round once). It drops x³·p0 with
p0 = −2/(3√π) = −0x1.812746b0379e7p-2, which is x²/3 of the result relative,
up to 2^-57.6 at 2^-28, about 0.03 ulp.

| Range (samples) | Worst error | Misrounded |
|---|---:|---:|
| [2^-28, 2^-27) (10^6) | 0.5000 ulp | 0 |
| [2^-34, 2^-28) (4·10^6) | **0.5325 ulp** (−0x1.c51f10a1c1f6bp-29) | 14,566 (0.36 %) |
| [2^-60, 2^-34) (2·10^6) | 0.5000 ulp | 1 |
| [2^-1022, 2^-60) (10^6) | 0.5000 ulp | 0 |
| subnormal (2·10^5) | 0.5000 ulp | 0 |
| [2^-27, 1/2) (2·10^6, the series) | 0.5106 ulp | 143 |

**Fix:** add x·(x·x)·p0 to the scaled pair's low part before its one
rounding, in plain double. The term is at most 2^-57.6 of the result, so
its own rounding is about 2^-110 of the result. x·x underflows to 0 below
2^-538, where the term is far below the result's last place anyway. The next
term, x⁵/10, is below 2^-115 relative. A host prototype (an fma pair for
x·2/√π plus the term) gave a worst of 0.500000 ulp, with none misrounded in
8·10^6 samples over [2^-34, 2^-28). The cost is three multiplies and an add
on the tiny path only. No bench case reaches it.

### `lgamma` (Q62, and the positive range)

Sampled against `lgammaq`:

| Range (samples) | Worst error | Misrounded | Worst absolute |
|---|---:|---:|---:|
| (0, 3/4) (4·10^5) | 0.5566 ulp | 0.62 % | 2^-50 |
| [3/4, 5/4) (4·10^5) | 0.6005 ulp | 0.95 % | 2^-55.8 |
| [5/4, 7/4) (4·10^5) | **0.6785 ulp** (0x1.565cf5c906bfcp+0) | 4.31 % | 2^-56.6 |
| [7/4, 9/4) (4·10^5) | 0.5276 ulp | 0.28 % | 2^-56.9 |
| [9/4, 3) (4·10^5) | 0.5910 ulp | 1.23 % | 2^-54 |
| [3, 6) (4·10^5) | 0.5203 ulp | 0.22 % | 2^-51 |
| [6, 12) (4·10^5) | 0.5024 ulp | 0.04 % | 2^-49 |
| [12, 10^6) (7·10^5) | 0.5003 ulp | ≤ 0.01 % | — |
| (−2, 0) (10^6) | 0.5195 ulp | 0.22 % | 2^-50 |
| (−24, −2) (2·10^6) | 3266 ulp | 0.29 % | 2^-48 |
| (−171, −24) (10^6) | 0.5000 ulp | 0.001 % | 2^-44 |
| `lgammaf` on (−24, 0) (3·10^6) | 0.5000 ulp | 0 | — |

**Near the negative zeros:** 44 zeros from −2.4570 to −24, found by
bisection in 300-bit mpmath. At ±40 ulps of each zero's nearest double, and
at 2^6 to 2^38 ulps away, 2453 of 3562 points are misrounded. The worst is
2.3·10^13 ulp at −0x1.5fb410a1bd901p+1 (a result of 1.7·10^-16), and the
largest absolute error is 2^-49.0. lgamma(−2.5) is −0x1.ccbf9f5ed0f15p-5, at
0.535 ulp. Q62 recorded 2^-57 from P11.07d1's sample; the zeros themselves
are worse.

**Causes:**
- **Negative x:** the reflection log π − log|sin πx| − lgamma(1 − x). Near
  a zero the result is the small difference of terms near log n!, so their
  absolute errors are the result's. D225 measured the same for the fast
  file's first form.
- **(5/4, 7/4) and (9/4, 3):** the shift to x + n ≥ 12, Stirling there, then
  − log(x(x+1)…). The result (about −0.12 at the minimum, 0.28 at 5/2) is
  the difference of terms near 18, and Stirling's series, in plain double,
  is about 2^-60 absolutely.
- **Near 1 and 2 (|z| ≤ 1/4):** the ζ series' tail, coefficients and Horner
  steps in plain double, reaches 0.60 ulp at z = 1/4.

**The maintainer's choice (D252):** an absolute bound near the negative
zeros, as the fast contract has it, and below 0.53 ulp elsewhere.

**The plan for the core** (P12.03a, then b):
- **(a) Positive x below 3.** Reduce to [1, 2]:
  - lgamma(x) = lgamma(1 + x) − log x below 1 (both relative near x = 1,
    where lgamma(2 + z) and log are small);
  - lgamma(x) = lgamma(x − 1) + log(x − 1) on [2, 3), lgamma(x − 1) ≤ 0 and
    log(x − 1) ≥ 0 there. They cancel most near 2, by at most a factor
    1/(1 − γ) ≈ 2.4.

  On [1, 2]:
  - within 1/4 of 1 and 2, the ζ series with its third term as a pair and
    the tail's coefficients as constants (not formed at run time);
  - on (5/4, 7/4), a Taylor series about 3/2: ψ^(k)(3/2)/k! fitted in
    mpmath. Their ratio is about 1/6 for |t| ≤ 1/4, so about 25 terms reach
    2^-65 absolutely (2^-62 of the result near the minimum, −0.1215), with
    the first three as pairs.

  Estimated at about 200 bytes of coefficients. From 3 up, the shift and
  Stirling stay (0.52 ulp, below the 0.53 figure).
- **(b) Negative x in (−24, −1), not an integer:** D225's recurrence
  lgamma(x) = lgamma(z) − log|P|, z = x + n + 2 in (1, 2), P = x(x + 1)…
  as a normalized pair product (up to 24 products), lgamma(z) from (a).
  Near a zero both terms are small, so the result keeps about 2^-60
  absolutely (D226 measured 2^-60 for the fast files to −18). Below −24 and
  in (−1, 0) the reflection stays: its worst there is 0.52 ulp.

**Q60 (D253):** each step re-locks its own size and bench growth in release
and speed, recorded in `docs/size/` and `docs/perf/`. None of the
`test/size` programs calls `erf` or `lgamma`, so only the archive members
change.
