# Research: fixed-point exponentials and logarithms on Armv6-M

Step: P3.05f. Decision: PLAN D86.

## Starting point (P3.05e speed baseline, armv6-m, instructions per call)

| Function | Instructions |
|---|---:|
| `exp2f` | 9269 |
| `expf` | 11034 |
| `log2f` | 10547 |
| `logf` | 12367 |

The `sp-nearest` layer (D57, D58) computes in float-float to about 2^-40 and rounds once. On a core without an FPU, each float operation is a soft-float call. An exact product takes about ten of them, so `expf` costs about 70 calls of 100–150 instructions each.

## Sources reviewed

- **CORE-MATH** (P. Zimmermann et al., <https://core-math.gitlabpages.inria.fr/>), the `binary32` exponentials and logarithms:
  - a fast path in double precision whose error bound is known;
  - a rounding test that accepts the result when the error interval holds no rounding boundary;
  - otherwise an accurate path (Ziv's strategy).
- **A. Ziv, *Fast evaluation of elementary mathematical functions with correctly rounded last bit*** (ACM TOMS 17, 1991): the rounding test and the retry with more precision.
- **J.-M. Muller, *Elementary Functions: Algorithms and Implementation*** (3rd ed., 2016), ch. 6 (table-based range reduction, Tang's method) and ch. 11 (the table maker's dilemma).
- **P. T. P. Tang, *Table-driven implementation of the exponential function in IEEE floating-point arithmetic*** (ACM TOMS 15, 1989), and the logarithm counterpart (1990): 2^(j/32) tables, and log tables at 1 + j/N with a short polynomial.
- **ARM optimized-routines** `expf`/`exp2f`/`logf`/`log2f` (<https://github.com/ARM-software/optimized-routines>): table-driven in double precision for cores with a double FPU, which Armv6-M lacks.
- **Qfplib-M0** (M. Owen, <https://www.quinapalus.com/qfplib.html>): fixed-point `exp` and `ln` for Cortex-M0 by shift-and-add (CORDIC-like) iterations. They are fast, but not correctly rounded. It is GPL and was only read about, not read as code.
- **FLIP** (C.-P. Jeannerod, G. Revy et al.): polynomial evaluation in 32-bit integers for integer-only cores, written to schedule well on VLIW machines.

## Design

The functions sit in the `nofp` layer's `speed/` directory (D77), so only the speed flavour on armv6-m and armv6s-m uses them:
- `libc/libc.mk` archives `libc/math/<layer>/speed` in the speed flavour;
- `sp-nearest/subdir.mk` then drops its own files of the same name.

The size flavour is byte-identical.

Each function keeps the float-float code as its fallback (Ziv):
- **fast path:** computes 2^n (1 + F·2^-32) in 32-bit fixed point;
- **rounding test:** F's 9 bits below the float's last bit are compared with the halfway point. If they differ from it by more than the error bound, rounding F rounds the exact value the same way, so the result is returned;
- **otherwise:** the float-float path runs, giving exactly the result the size flavour gives.

Arguments outside the fast range also take the float-float path: subnormal or overflowing results, NaN, infinities, zero and negative logarithm arguments.

### Exponentials (`__libc_exp2f_fixed`)

- **Reduction:** z = x (`exp2f`) or x / ln 2 (`expf`), as a signed 64-bit integer in units of 2^-37.
  - `expf` builds it from the 24-bit significand times 1/ln 2 to 64 bits, with three 32 × 32 products.
  - n = floor(z) is the result's exponent, j the next 5 bits, and r ∈ [0, 2^-5) the low 32 bits.
  - Every value is unsigned, so the core needs no signed multiplication.
- **2^(j/32) − 1:** a 32-entry table, in units of 2^-32.
- **2^r − 1:** the Taylor polynomial of degree 4 in r·ln 2; the next term is below 2^-34. It is evaluated by Horner's rule, with each coefficient scaled to fill 32 bits, and the products' high halves shifted by 7 between steps.
- **Combination:** (1 + t)(1 + p) − 1 = t + p + t·p. It is saturated below 2: the true value is below 2, and the rounding goes up as it must.
- **Fast range:** |x| < 126 for `exp2f` and |x| < 87 for `expf`, where every result is normal.

### Logarithms (`__libc_log2f_fixed`)

- **Reduction:**
  - x = 2^e·m, with m ∈ [√½, √2) as an unsigned Q31 number;
  - c' is the reciprocal of a Q31 number near 1/c, for c = 1 + j/32 (j = −9 … 13), the same reduction as D58;
  - r = m / c' − 1 is exact: the Q62 product of m and the table's 1/c, less 1.
- **log2(1 + r):** r·Q(r), with Q the series (1 − r/2 + r²/3 − …) / ln 2 to the r⁵ term. It is evaluated by Horner's rule in signed Q31, with r in Q36 and every step rounded to nearest. The final step and the constant 1/ln 2 are unsigned.
- **Relative accuracy:**
  - r·Q uses r normalised to 32 bits;
  - near 1 (e = 0, j = 0) the result is that product alone, so it keeps r's relative accuracy however small r is;
  - elsewhere |log2 x| ≥ 0.022, and e + log2 c' + r·Q is summed in units of 2^-55.
- **`logf`:** multiplies the normalised result by ln 2 to 64 bits.

### Arithmetic

- **`__libc_fixed_wide`:** Armv6-M's `MULS` keeps 32 bits, so the 64-bit product comes from four 16-bit half products. It is one shared out-of-line function: `-Os` already kept a local copy of it per object, and four copies broke the speed budget.
- **Signed high half:** from the unsigned one, corrected for each negative operand.
- **libgcc:** the 64-bit variable shifts and the leading-zero counts call `__aeabi_llsl`/`__aeabi_llsr` and `__clzsi2`/`__clzdi2`.

## Error bounds, verified over every argument

The core code compiles unchanged on the host. A harness ran every float argument in each fast range through it and compared F with `exp2l`/`expl`/`log2l`/`logl` on x86-64 (64-bit significand, error far below 2^-32 of F):

| Function | Arguments | Largest error of F (units of 2^-32) | Bound used | Falls back | Not correctly rounded on the fast path |
|---|---:|---:|---:|---:|---:|
| `exp2f` | 2,247,622,656 | 2.68 | 6 | 0.62 % | 0 |
| `expf` | 2,237,399,040 | 2.71 | 6 | 0.60 % | 0 |
| `log2f` | 2,139,095,039 | 4.49 | 8 | 3.32 % | 0 |
| `logf` | 2,139,095,039 | 4.23 | 8 | 3.32 % | 0 |

Every argument is covered, so these bounds are proven for this code, not estimated. Each bound used is above the measured largest error. Where F is spread evenly, the fallback rate is (2·bound + 1) / 512, as for the logarithms. The exponentials fall back less often: about three quarters of their arguments are below 2^-24 in magnitude, where F stays clear of the halfway point.

The logarithms' error is largest just below 1 and at the bucket edges near |log2 x| = 0.022:
- the term r·Q and the table entry cancel there;
- before every Horner step was rounded, the largest error was 5.45 units.

## Measured and not taken

- **Inline helpers:** with `__libc_fixed_wide` inline (four local copies) and the table as an array of `{uint32, int64}` structures (padded to 16 bytes each), `math-float` was 10728 bytes against a speed budget of 10604. One shared helper and two arrays give 10520 bytes, for about 40 instructions more per `expf` (336 → 375).
- **The size flavour:** gains nothing from this step. The fast paths cost about 1.5 KB of code, which the size flavour's +0 % budget does not have.

## Results (armv6-m speed flavour, instructions per call; armv6s-m the same)

| Function | Before | After |
|---|---:|---:|
| `exp2f` | 9269 | 316 |
| `expf` | 11034 | 375 |
| `log2f` | 10547 | 583 |
| `logf` | 12367 | 2210 |

`logf`'s figure is the mean over the benchmark's 16 arguments, about two of which fall back (12,000 instructions each); its fast path costs about 650. Code: `libc.a` +3.5 KB in the speed flavour; `math-float` 9036 → 10520 (limit 10604), `math-mix` within its limit.

## Tests

- **`test/unit/libm_dense` (new, every board and flavour):**
  - every 127th float in [0.906, 1.094) and every 31st in [1.39, 1.44) for `logf` and `log2f`;
  - every 1021st in ±[16, 32) and every 2^17th across ±[2^-30, 87) or ±[2^-30, 126) for `expf` and `exp2f`;
  - each against the double function rounded to float, allowing one in ten thousand plus one, as the float accuracy test does.
  - The three results not correctly rounded on armv6-m come from the float-float path and are the same in the size flavour.
  - A mutant with both bounds set to 0, so that it never falls back, fails on all four functions; `test/unit/float_accuracy` alone missed it for the logarithms.
- **`test/unit/float_accuracy` and libc-test** run every board and flavour.

## P3.05g: sine, cosine and tangent

Decision: PLAN D87.

### Starting point (speed flavour, armv6-m, instructions per call)

| Function | Instructions |
|---|---:|
| `sinf` | 18350 |
| `cosf` | 18281 |
| `tanf` | 36223 |

### Design

- **Reduction:** the existing integer Payne–Hanek reduction (D60) is kept, with its float-float output s, |s| ≤ π/64. Its float-float work, two `scalbnf` calls and a product by π/32, is now most of the remaining cost.
- **`__libc_sinf_fixed(k, s)`:**
  - s becomes a signed integer in units of 2^-48, exact for |s| ≥ 2^-24;
  - u = s² in units of 2^-38;
  - cos s − 1 = u(−1/2 + u/24 − u²/720) and sin s = s(1 + u(−1/6 + u/120)), each by Horner's rule in Q31 (the next terms are below 2^-40);
  - the angle sum uses a 9-entry table of sin and cos of jπ/32 in units of 2^-62, and the same quadrant logic as the float-float core: first·(1 + cm) + second·sin s, summed in units of 2^-62.
  - second·s uses four 32-bit partial products, so the table's and s's low bits count.
  - Sine quadrants at j = 0, where the result is s itself, decline when |s| < 2^-14, because 2^-48 units no longer give 32 bits of relative accuracy there.
- **`tanf`:** the sine over the cosine of the same core:
  - divide by the cosine's top 32 bits (Armv6-M's `__aeabi_uldivmod`);
  - take one more quotient bit from the remainder when the sine is the smaller;
  - subtract the quotient times the cosine's low 32 bits over the cosine, at most 3 by a short loop.
  - A first version divided by the cosine's top 31 bits alone and measured 8.16 units of error; this one measures 1.39.

### Error bounds, verified over every argument

As for P3.05f, the host ran the core code, together with the real `__libc_rem_pio2f` (x86-64 with `-mlong-double-64`, which the library's headers require), over every float from 2^-12 up (2^-13 for `tanf`), against the double functions:

| Function | Arguments | Largest error (units of 2^-32) | Bound used | Falls back | Not correctly rounded on the fast path |
|---|---:|---:|---:|---:|---:|
| `sinf` | 1,174,405,120 | 1.28 | 3 | 1.38 % | 0 |
| `cosf` | 1,174,405,120 | 1.29 | 3 | 1.37 % | 0 |
| `tanf` | 1,182,793,728 | 1.39 | 4 | 1.75 % | 0 |

For `tanf`, the first version's divisor of 31 bits had a largest error of 8.16 units, above its bound of 8. Its fallback rate was 3.3 %, and no result was wrong, but only by luck; the exhaustive run found it.

### Size: the speed budget

- `math-float` links `sinf` and `cosf`. With the fast paths it is 11460 bytes, against 10604 under the speed flavour's +10 % (Phase 2 + 10 %).
- The cost is the core (782 bytes, with its 144-byte table) and 76 bytes each in `sinf` and `cosf`.
- The user chose to raise the budget for the speed flavour's math images, `math-float` and `math-mix`, to +20 % (`docs/size/budget.json`, `program_margins_percent`; D87), rather than leave the trigonometric functions out.
- `math-float` is at +18.9 %. The other images keep +10 %, and the size flavour +0 %.

### Results (armv6-m speed flavour; armv6s-m the same)

| Function | Before | After |
|---|---:|---:|
| `sinf` | 18350 | 4729 |
| `cosf` | 18281 | 3742 |
| `tanf` | 36223 | 5140 |

### Tests

- **`test/unit/libm_dense`:**
  - sweeps `sinf`, `cosf` and `tanf` over every 2039th float in ±[2^-12, 8) and every 2^17th float from 8 to 2^127;
  - 46 arguments that the fast path rounds the wrong way without its error test, found on the host over every binade, must be correctly rounded on every board (expected values from mpmath at 200 bits).
  - A mutant with every bound at 0 misses all of them. The sweeps alone tolerated it (7 of 123424 wrong, within the 1/10000 allowance).
  - Two `tanf` arguments the float-float quotient misses on every float-float board (D57's allowance) are left out.

## P3.05h: the error functions

Decision: PLAN D88.

### Starting point (speed flavour, armv6-m, instructions per call)

| Function | Instructions |
|---|---:|
| `erff` | 141024 |
| `erfcf` | 119604 |

The float-float `erff` (D63) sums the Maclaurin series of e^(x²)·erf(x), up to 55 float-float terms with a division each. `erfcf` from 1 runs Laplace's continued fraction, with 150/x² + 10 float-float divisions.

### Sources reviewed

- **The C23 standard**, 7.12.8.1–2 and F.10.5.
- **W. J. Cody, *Rational Chebyshev approximations for the error function*** (Math. Comp. 23, 1969): piecewise rational approximations for erf and erfc in three ranges. These are what fdlibm and musl implement.
- **NIST DLMF §7** (<https://dlmf.nist.gov/7>):
  - erf' = (2/√π)e^(-x²);
  - erfc(x)·e^(x²) is smooth and behaves as 1/(x√π) for large x (7.12.1);
  - the continued fraction 7.9.2.
- **CORE-MATH `erff`/`erfcf`:** polynomials on subintervals in double precision, with a rounding test.

A rational or minimax fit per interval needs a set of coefficients for each interval. Instead, the Taylor coefficients are generated from an ODE recurrence. That needs only the function and one derivative per node, so the tables are small, and every coefficient comes from the same short loop.

### Design (`__libc_erff_fixed`)

- **|x| < 2:**
  - erf(x0 + h) = erf(x0) + g(x0)·h·(1 + S), with nodes x0 = i/8, |h| ≤ 1/16, and g = (2/√π)e^(-x0²);
  - S is the integral series of e^(-2x0t − t²), whose coefficients follow (k + 1)·q_{k+1} = a·q_k + b·q_{k−1}, with a = −2x0h and b = −2h²;
  - 12 terms; past them the terms are below 2^-40;
  - at the node 0 the result is (2/√π)·x·(1 + S) from x's own significand, so it keeps x's relative accuracy for small x;
  - erfc(x) is 1 − erf(x), or 1 + erf(|x|) for negative x, in units of 2^-62.
- **2 ≤ x < 9.25:**
  - erfc(x) = e^(-x²)·R(x), with R = erfc·e^(x²);
  - R(x0 + h) = R(x0)·(1 + T), with nodes x0 = 2 + i/8;
  - T comes from (k + 1)·r_{k+1} = 2x0·r_k + 2·r_{k−1}, which follows from R' = 2xR − 2/√π, with the tabulated R'/R as the first coefficient;
  - e^(-x²) comes from the exp2 core of P3.05f, with x² exact to 2^-40 and z = x²/ln 2 to 2^-37;
  - erf(x) is 1 − erfc(x) there;
  - results below the normal range decline to the float-float path.
- **Tables:** erf and g at 17 nodes, R and R'/R at 59 nodes, and 1/k for k = 3 … 13, about 1 KB in all.
- **The series** keeps its terms in Q32 (the erf branch) or Q35 (the R branch, where every term is below 0.03). a and b are in Q30, and every product is exact and then rounded to nearest.
  - **b·q0:** b is passed again at the series' own precision for the first step, where it multiplies q0 = 1.
  - **Rounding:** each step rounds to nearest; truncation bias was measured at 11 units.
  - **h(1 + S):** formed in units of 2^-62 before the product with g.

### Error bounds, verified over every argument

The host ran the core over every float from 2^-19 to 3.92 (`erff`) and from 2^-19 to 9.25 (`erfcf`, both signs), against the double functions:

| Function | Arguments | Largest error (units of 2^-32) | Bound used | Falls back | Not correctly rounded on the fast path |
|---|---:|---:|---:|---:|---:|
| `erff` | 351,650,448 | 3.92 | 8 | 3.3 % | 0 |
| `erfcf` | 361,601,435 | 5.69 | 8 | 3.3 % | 0 |

The first versions measured 11.5 and 33.3 units, with 110 and 191,682 wrong results. Three precision losses caused this, and fixing them gave the figures above:
- a floored series in which vanishing terms stuck at −1 unit;
- b in Q30 in the first step (2 units);
- h(1 + S) floored to 2^-34 before the product with g.

### Results (armv6-m speed flavour; armv6s-m the same)

| Function | Before | After |
|---|---:|---:|
| `erff` | 141024 | 2967 |
| `erfcf` | 119604 | 2874 |

`__libc_erff_fixed` is 2328 bytes in the speed flavour's `libc.a`. No size image links the error functions.

### Tests

- **`test/unit/libm_dense`:**
  - sweeps `erff` and `erfcf` over both branches and both signs, and `erfcf` up to its subnormal results;
  - 32 more arguments that the fast path rounds the wrong way without its error test.
  - These were chosen with F at least one unit from the halfway point: at exactly 0x100 the test falls back even with a bound of 0, so such a vector cannot catch the mutant.
  - The bounds-at-0 mutant now misses all 78 vectors; the real library rounds all 78 correctly on every board and flavour.

## P5.04a: `log10f`

Decision: PLAN D114 (Q25). Step: `docs/plan/phase-5.md`.

### Approach

`libc/math/nofp/speed/log10f.c` is `logf`'s fixed-point path with a different
constant: the D86 `log2f` core's normalised result times log10 2 to 64 bits.

- **The constant:** log10 2 = 0.30103... lies in [1/4, 1/2), so it fills 64
  bits as 0x9a209a84fbcff799 · 2^-65 (mpmath, 200 bits, rounded to nearest),
  one place lower than `logf`'s ln 2 · 2^-64; the exponent drops by one more.
- **The product:** the top 64 bits of v · c from three of the four 32-bit
  partial products, as for `logf`; it lies in [2^62, 2^64) and is normalised
  to bit 63.
- **The rounding test and fallback:** `__libc_fixed_round` with the logarithm
  bound (8 units of 2^-32); otherwise the float-float `log10f` (D58).

### Error bound, verified over every argument

The host ran the core and the product over every positive finite float
(x = 1 returns 0 exactly before), against `log10l` on x86-64:

| Function | Arguments | Largest error of F (units of 2^-32) | Bound used | Falls back | Not correctly rounded on the fast path |
|---|---:|---:|---:|---:|---:|
| `log10f` | 2,139,095,038 | 5.38 (at 0x1.f7e0bap-1) | 8 | 3.32 % | 0 |

Without the test, 3,334 arguments would be rounded the wrong way with the fast
path confident (F not at the exact halfway point, which falls back even with a
bound of 0). Twelve of them, spread over the range, are in
`test/unit/libm_dense/near.inc`; a mutant with the bound at 0 fails all twelve.
A thirteenth (0x1.c6c5cep+127) was left out: the float-float `log10f` that the
size flavour uses on every float-float board misses it too, within D57's
allowance. `libm_dense` also sweeps `log10f` around 1, across sqrt(2) and over
every binade.

### Size

`log10f` in the speed flavour is 488 bytes against the float-float version's
324 (armv6-m). No reference image links `log10f`, so the speed budget is
unchanged.

### Results (speed flavour, instructions per call, throughput)

| Core | Before | After |
|---|---:|---:|
| armv6-m, armv6s-m | 12365 | 678 |
| armv7-m | 6181 | 466 |

## P5.04b: `powf`

Decision: PLAN D115 (Q25). Step: `docs/plan/phase-5.md`.

### Sources reviewed

- C23 7.12.7.5 and F.10.4.5: the special cases, which stay with the
  float-float `powf` (D59).
- Arm Optimized Routines' `powf` (via FreeBSD,
  <https://src.rcs.uwaterloo.ca/xref/freebsd-src/contrib/arm-optimized-routines/math/powf.c>):
  log2 x by a table of 1/c and log2 c and a polynomial in r = x/c − 1, then
  exp2 of y·log2 x, all in double, with a relative error of the exp2 argument
  of about 2^-34. Only the structure is relevant; the arithmetic here is fixed
  point and no code is used.
- CORE-MATH (Sibidanov, Zimmermann, <https://members.loria.fr/PZimmermann/talks/core-math-raim2022.pdf>):
  a fast phase with a rounding test and a slower accurate phase (Ziv), which
  is this library's structure since D86; binary32 `pow` has hard cases that no
  fixed precision settles, so the fallback stays.

### Error budget

The D86 exp2 core takes z in units of 2^-37. For a normal result |z| < 2^7,
and an error δz in z adds about δz · ln 2 · 2^33 units of 2^-32 to F. Keeping
that near 0.05 units asks |δz| ≤ 2^-37, so log2 |x| must be accurate to about
2^-44 relative: the D86 log2 core (about 2^-29) is far from enough.

### The wide log2 core

`__libc_log2f_wide` keeps the D86 reduction (23-entry tables, now shared in
`__libc_log2f_table.c`, r exact in Q62, |r| < 2^-5.4) and evaluates
log2(1 + r) = r · U(r), U = log(1 + r) / (r ln 2), to the term in r^7 (the
next is below 2^-46 relative), its coefficients divided by ln 2:

- the terms from r^3 on by Horner's rule in signed Q31 with r in Q36, as in
  D86 (their error is multiplied by r^3 < 2^-16);
- the last three steps in Q62: r times a 64-bit value from three 32-bit
  partial products (the low one dropped, less than 2^-60);
- r · U from r normalised to 64 bits, so near 1 the result keeps r's relative
  accuracy; elsewhere |log2 x| ≥ 0.022 and the sum is formed in units of
  2^-55.

Run on the host over every positive float against `log2l`: the largest
relative error is 0.040 · 2^-44 (about 2^-48.6).

### `powf`

- **Fast path:** finite non-zero x other than ±1, finite non-zero y, and for
  a negative x an integer y (an odd one gives the sign). Everything else goes
  to the float-float `powf` unchanged.
- **z = y · log2 |x|:** y's 24-bit significand times the 64-bit logarithm
  (two 32 × 32 products), shifted to units of 2^-37 with rounding. A scale
  that puts |z| at 2^7 or more declines (overflow or underflow, left to the
  fallback).
- **2^z:** the D86 exp2 core; a result exponent outside [−126, 127] or a
  rounding that reaches infinity declines; otherwise `__libc_fixed_round` with
  the exp2 bound (6 units).

### Validation (two arguments cannot be run exhaustively)

The host ran the fast path against `powl` on x86-64 over four structured
sets: x of every binade with y = t / log2 x (t uniform over the normal range),
x near 1 with the same, small integer y, and random y bits.

| Set | Fast-path results | Largest error of F (units of 2^-32) | Falls back | Wrong |
|---|---:|---:|---:|---:|
| x any binade, y = t / log2 x | 249,999,997 | 2.69 | 2.54 % | 0 |
| x near 1 | 249,999,943 | 2.70 | 2.54 % | 0 |
| integer y in [−30, 30] | 32,591,372 | 2.67 | 2.22 % | 0 |
| random y | 125,851,672 | 2.66 | 0.65 % | 0 |

The largest error equals the exp2 core's own (2.68 units, D86): the logarithm
and the product add almost nothing. Without the rounding test, about one
result in a thousand would be confidently misrounded; twelve of them, from all
four sets, are in `test/unit/libm_dense/near2.inc`, and a mutant with the
bound at 0 fails all twelve. Every one is correctly rounded by the float-float
path too (armv7e-m release) and by x86-64.

### Measured and not taken

- **One logarithm core for every function:** with `logf`, `log2f` and
  `log10f` on the wide core, `__libc_log2f_fixed` goes and `math-float`
  shrinks by 516 bytes, but `log2f` is 73 % and `log10f` 62 % slower (the
  wide core's 64-bit steps), against the 1 % benchmark gate. The two cores
  stay.
- **1/ln 2 as a separate product:** folding it into the coefficients removed
  one 64-bit product: `powf` 1322 → 1222 instructions on armv6-m, and the
  error fell from 0.053 to 0.040 · 2^-44.

### Results (speed flavour, instructions per call, throughput)

| Core | Before | After |
|---|---:|---:|
| armv6-m, armv6s-m | 22158 | 1222 |
| armv7-m | 11060 | 865 |

`math-float` and `math-mix` grow by about 1.1 KB on armv6-m and armv6s-m and
1.0 KB on armv7-m (`powf` 978 bytes, the wide core 788, the table moved, not
added); D116 settles the budget.
