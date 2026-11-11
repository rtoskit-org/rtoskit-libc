# Fast float exponentials, logarithms and powers (P11.03)

Research note for P11.03 ([`docs/plan/phase-11.md`](../plan/phase-11.md),
PLAN D162, D163, D168), extended by each sub-step: P11.03a the
exponentials, P11.03b the logarithms, P11.03c `powf`, P11.03d the `m1`/`p1`
forms and `cbrtf`, P11.03e the cores without an FPU. It records the
sources, the design per core, the error analysis and the measured bounds.
It contains no code from any other implementation (D1, D2): the ideas below
are described in our own words, and the coefficients are fitted here.

## 1. The target

The fast contract (fast-math.md §4): below 1 ulp to nearest for the
exponentials, logarithms and `powf`, round to nearest only, every special
value, overflow and underflow result and exception as in the other build
types. Nothing corrects the last bit: no hard-case tables, no float-float,
no rounding-mode switching (D162). The cores differ in what is cheap:

| Cores | Hardware | Today (speed) | Fast evaluation |
|---|---|---|---|
| x86_64, armv7e-m-dp | double FPU | the double function rounded to float, plus D98's hard-case correction | double, one final rounding to float |
| armv7e-m, armv8-m.main | single-precision FPU with `VFMA` | float-float on the FPU (D57) | float only, with fused multiply-adds |
| armv6-m, armv6s-m, armv7-m | none | fixed point where `nofp/speed` has a file (D86–D88), float-float otherwise | P11.03e |

## 2. Sources (ideas only)

- **FreeBSD msun `e_expf.c`** (Sun-derived; <https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_expf.c>,
  read 2026-11-10 through a summary): float arithmetic, no table; x = k ln2 + r
  with ln2 split into a high part with trailing zeros (so k ln2_hi is exact)
  and a low correction; a short rational approximation in r² on
  [−0.3466, 0.3466] and one division to rebuild e^r; 2^k by the exponent
  field, with an extra factor below k = −125 and two steps at k = 128.
  Thresholds 88.72 (overflow) and −103.97 (underflow). No ulp bound stated.
- **FreeBSD `s_exp2f.c`** (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_exp2f.c>):
  a 16-entry table of 2^(i/16) in double, a degree 4 polynomial in double,
  index and integer from one large-constant addition, 0.501 ulp claimed.
- **Arm Optimized Routines** (fast-math.md §2): `expf`/`exp2f` evaluate in
  double with a 2^(j/32) table and one final rounding (0.51 ulp claimed);
  on our single-precision cores that double arithmetic is software.
- **picolibc**: the float functions are the Sun float ones on these cores
  (fast-math.md §2); the source page fetched for `ef_exp.c` was not found
  (404) and is not used.
- **CORE-MATH** (<https://core-math.gitlabpages.inria.fr/>): correctly
  rounded binary32 functions, checked by exhaustive search; the page
  describes no algorithm. The exhaustive check is the method taken here.

What we take: the table-and-double method where double is hardware (it
costs one rounding, so a bound of about 0.51 ulp is cheap), and on the
single-precision FPU a Sun-style reduction without the table or the
division, which a fused multiply-add makes accurate enough on its own.

## 3. The double-FPU cores: `libc/math/fast/`

`__libc_fast_exp2(t)` (`libc/internal/math-fast.h`) computes 2^t in double:

- n = round(32 t) by the 1.5·2^52 shift (the sum's low 32 bits are n in two's
  complement); r = t − n/32, exact by Sterbenz's lemma (|r| ≤ 1/64);
- 2^t = 2^((n − j)/32) · T[j] · 2^r, j = n mod 32, T the 32 correctly rounded
  values of 2^(j/32) (`__libc_fast_exp2_table`, 256 bytes, computed with
  200-bit arithmetic); the power of two goes into T[j]'s exponent field;
- 2^r ≈ 1 + r(c1 + r(c2 + r c3)), Taylor coefficients of degree 3: relative
  error 2^-30.7 on |r| ≤ 1/64 (computed at 4,001 points with 200 bits).

`expf`, `exp2f` and `exp10f` pass t = x/ln2, x and x·log2(10) (the product's
2^-53 relative error is 2^-46 absolute at most for |t| < 150, negligible)
and round the result to float once. The total error is the final rounding
plus 2^-30 relative: below 0.51 ulp. Integers of `exp2f` are exact (r = 0).

## 4. The single-precision FPU cores: `libc/math/sp-nearest/fast/`

`__libc_fast_expf_kernel(u_hi, u_lo, k)` computes e^(u_hi + u_lo)·2^k in
float. Each function supplies an exact u_hi and a small u_lo:

- `expf`: k = round(x/ln2) by the 1.5·2^23 shift; u_hi = x − k·ln2f by one
  fused multiply-add, exact (k·ln2f has at most 32 significant bits and the
  difference, a multiple of 2^-24 below 2^-1.5, fits a float); u_lo =
  −k·(ln2 − ln2f), the low part of ln2 (−0x1.05c61p-29).
- `exp2f`: k = round(x), r = x − k exact, u = r·ln2: u_hi = r·ln2f rounded,
  u_lo = its rounding error by a fused multiply-add plus r·(ln2 − ln2f).
- `exp10f`: k = round(x·log2 10); r = x − k·log10(2)f exact by a fused
  multiply-add (|r| ≤ 0.1505, a multiple of 2^-25); r_lo = −k·(log10 2 −
  log10(2)f); u = (r + r_lo)·ln10 as u_hi = r·ln10f and u_lo = the product's
  rounding error + r·(ln10 − ln10f) + r_lo·ln10f.

Then e^u = 1 + (u_hi + (u_lo + u²q(u))) with q(u) ≈ (e^u − 1 − u)/u² of
degree 5, fitted here by Chebyshev interpolation on [−ln2/2, ln2/2]·1.0005
and rounded to float: relative error 2^-30.3 on e^u (degree 4 gave 2^-26.5,
too close to the bound). The error budget: the final addition rounds once
(½ ulp); u_hi + (…) rounds once, worth at most ¼ ulp of the result
(|u| ≤ 0.35 against a result in [0.70, 1.42]); the polynomial and the
reduction add far less. Sum below 0.8 ulp before the scaling, which is exact
for normal results and rounds once for subnormal ones (2^(k+64) first, then
2^-64). The layer rounds to nearest only (D57), so the shift trick and the
kernel need no mode handling.

On these cores the soft-float double is what made the float-float paths
cost 140–160 instructions; the fast files use no double at all (the module
refuses `__aeabi_d*` helpers, as the layer does). They are not used on the
cores without an FPU: there the six fused multiply-adds are calls to the
software `fmaf`, and `exp10f` measured 11,103 → 15,885 instructions on
armv6-m (+43 %), 5,461 → 6,818 on armv7-m. The arch file lists the layers
whose `fast/` directory applies (`LIBC_MATH_FAST_LAYERS`, D168): armv7e-m
and armv8-m.main list `sp-nearest`.

## 5. Special values and thresholds

The thresholds are the existing files' (89 and −104 for `expf`, 128 and −151
for `exp2f`, 39 and −46 for `exp10f`): above, x·2^127 overflows to +∞ (and
leaves +∞ exact); below, the result is +0 for −∞ exactly and otherwise zero
with underflow. A constant product such as 2^-100·2^-100 is folded by the
compiler once `-frounding-math` is gone (D163), and then raises nothing: the
first build failed 54 of `libm_vectors`' exception checks on x86_64 and
armv7e-m-dp. The double files return `(float)__libc_math_underflow(1.0)`
(zero or 2^-149 by the mode, with underflow); the float files, whose cores
round to nearest and keep no flags, call `__libc_math_raise_underflow()` and
return +0. `test/unit/libm_thresholds` checks every one of these values in
every build type.

## 6. Measured

Exhaustive host sweep (every float argument the function computes, against
glibc's double function; x86_64 with hardware FMA, the same arithmetic as
`VFMA`):

| Function | Arguments | double path (max ulp, % not CR) | float path (max ulp, % not CR) |
|---|---:|---:|---:|
| `expf` | 2,239,889,410 | 0.5095, 0.016 % | 0.8527, 0.80 % |
| `exp2f` | 2,249,654,273 | 0.5096, 0.016 % | 0.8443, 0.81 % |
| `exp10f` | 2,220,097,538 | 0.5095, 0.016 % | 0.8472, 0.81 % |

The libm vectors, largest error to nearest: 0.486, 0.496, 0.498 ulp on
x86_64 and armv7e-m-dp (all four directions and the exceptions checked);
0.526, 0.625, 0.696 on armv7e-m.

Instructions per call (`make bench`, fast against speed):

| Function | x86_64 | armv7e-m-dp | armv7e-m, armv8-m.main |
|---|---:|---:|---:|
| `expf` | 237 → 48 | 172 → 52 | 160 → 58 |
| `exp2f` | 302 → 47 | 195 → 50 | 141 → 59 |
| `exp10f` | 284 → 48 | 188 → 52 | 160 → 66 |

## 7. The logarithms on the double-FPU cores (P11.03b)

No table: x = 2^e m with m in [√½, √2) (subnormal x first scaled by 2^23;
the exponent field and a halving above √2 do the split), and in double

    log m = 2 atanh s = 2s (1 + s² Q(s²)),   s = (m − 1)/(m + 1)

with Q the Taylor coefficients 1/3, 1/5, …, 1/13: |s| ≤ 0.1716, relative
error 2^-39.5 (4,001 points, 200 bits). m − 1 and 2 + f are exact in
double (m has 24 bits); the division rounds once, 2^-53. Then
`logf` = e ln2 + log m, `log2f` = e + log m / ln2, `log10f` = e log10 2 +
log m / ln10, all in double, rounded once to float. One double division
counts one instruction under the bench and costs about 20 cycles on the
FPU, against the table lookups a polynomial-only form would need.

**log(1) is +0 in every mode** (F.10.3.7): rounding downward, m − 1 = 1 − 1
is −0, and so was the result; `libm_vectors` caught it on x86_64 and
armv7e-m-dp. The files return +0 for x = 1 before the split
(`libm_thresholds` checks it downward too).

## 8. The logarithms on the single-precision FPU cores (P11.03b)

Float only, so the error budget leaves no room for a second large rounding:
the result is HI + (r + LO) with HI exact.

- **Reduction** (`__libc_fast_logf_reduce`): x = 2^k z with z in
  [0.6992, 1.3984): subtracting 0x3f330000 from x's bits gives k in the
  exponent field and the entry i of 32 in the next 5 bits (each entry 2^18
  ulps of z wide); z = x's bits less k's. r = z·(1/c_i) − 1 by a fused
  multiply-add, |r| < 0.0235, with 1/c_i rounded to float and log c_i taken
  of that rounded reciprocal (−log(1/c_i)), so the identity is exact.
- **Tables** (`sp-nearest/fast/__libc_fast_logf_{invc,ln,log2,log10}.c`,
  32 floats and three 32 × 2 tables, 896 bytes in all, one object each):
  −log(1/c_i) in each base as a multiple of 2^-16 and a low part. Since
  ln2_hi and log10(2)_hi are multiples of 2^-16 too, k ln2_hi + high (and
  k + high, k log10(2)_hi + high) is exact for |k| ≤ 150: at most 23
  significant bits.
- **Near 1** the entry holding 1 has c = 1, but it spans only [0.996, 1.023);
  next to it log c_i and r cancel, and the first build measured up to 2.72 ulp
  (`log10f` at 0.996). Within 2^-5 of 1 the reduction now takes r = x − 1,
  exact by Sterbenz's lemma, against that entry; beyond, |log x| > 0.031 and
  the cancellation costs at most 1/8 ulp.
- **log1p(r) = r + r² q(r)**, q of degree 3 fitted on [−2^-5, 2^-5]·1.01:
  relative error 2^-30 on log1p.
- `logf` = HI + (r + (k ln2_lo + low + r² q)); `log2f` and `log10f` multiply
  r by 1/ln2 or 1/ln10 as a float plus its rounding error (a fused
  multiply-add) plus the constant's low part.

## 9. Measured (P11.03b)

Every positive float (2,139,095,041 arguments) against glibc's double
function, as section 6:

| Function | double path | float path |
|---|---:|---:|
| `logf` | 0.5000 ulp, all correctly rounded | 0.7507 ulp, 0.026 % not CR |
| `log2f` | 0.5000 ulp | 0.7943 ulp, 0.027 % |
| `log10f` | 0.5000 ulp | 0.8341 ulp, 0.027 % |

Instructions per call (speed → fast):

| Function | x86_64 | armv7e-m-dp | armv7e-m, armv8-m.main |
|---|---:|---:|---:|
| `logf` | 267 → 69 | 162 → 81 | 165 → 74 |
| `log2f` | 337 → 71 | 203 → 81 | 154 → 80 |
| `log10f` | 303 → 72 | 173 → 83 | 165 → 83 |

The cores without an FPU keep the speed files (D168, P11.03e).

## 10. `powf` (P11.03c)

### Sources (ideas only)

- **FreeBSD msun `e_powf.c`** (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_powf.c>,
  read 2026-11-10 through a summary): float arithmetic only, no table; log2|x|
  from s = (m − 1)/(m + 1) about two pivots (1 and 1.5) as float-float, a
  long odd polynomial in s; y split into a high part with cleared low bits so
  y·log2|x| is a float-float; the exponential by a polynomial and a rational
  reconstruction; the parity of y from its exponent and significand bits;
  |y| above 2^27 short-cut. No ulp bound stated.
- **Arm Optimized Routines `powf.c`** (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/powf.c>,
  read 2026-11-10 through a summary): in double, log2|x| from a 32-entry
  reciprocal table, r = z·(1/c) − 1 and a degree 5 polynomial; the product
  y·log2|x| as one double (y is a float, so it cannot overflow); 2^t by the
  same 32-entry exp2 table and a degree 3 polynomial; integer y and its
  parity by bit tests; 0.82 ulp stated (0.5 + the relative error · 2^24).

What we take: on the double-FPU cores, the double evaluation (one product,
one rounding); on the single-precision FPU cores, the reciprocal table of
section 8 with log2|x| carried as a float pair, since the product's error
grows with |y log2|x|| and float arithmetic must keep about 2^-35.

### The special cases (`__libc_fast_powf_special`, every fast powf)

C23 F.10.4.5 in the order the strict files take it: y = ±0 or x = 1 → 1
(even for a NaN); a NaN → x + y; y = ±∞ by |x| against 1 (pow(−1, ±∞) = 1);
x = ±0 → ±∞ with divide-by-zero (a division at run time, so it is not
folded) for y < 0, ±0 for y > 0; x = ±∞ → ∞ or 0; a negative x to a
non-integer → NaN with invalid; y = 1 → x. The parity: y is an integer when
truncf(y) = y, and odd only below 2^24 (every larger float is even). A
negative x to an odd power negates |x|^y at the end.

### The double-FPU cores (`libc/math/fast/powf.c`)

x = 2^e m, m in [√½, √2) (`__libc_fast_logf_split`); log2|x| = e +
log m·(1/ln2) with log m to 2^-39.5 relative (section 7); t = y·log2|x| in
double. For |t| < 150 the product's and the logarithm's errors are below
2^-32 absolute, 2^-32·ln2 relative after the exponential; `__libc_fast_exp2`
(section 3) adds 2^-30.7, and the one rounding to float ½ ulp: below 0.51
ulp. t beyond ±1000 returns `__libc_math_overflow`/`underflow` (a far
argument would round to an infinite or zero result anyway); between, the
exponential's own scaling gives ∞, a subnormal or zero.

### The single-precision FPU cores (`libc/math/sp-nearest/fast/powf.c`)

The result 2^t is within ½ + ε ulp only if t is known to about 2^-25
absolute; with |t| up to 150 that is 2^-32 relative for t and, for x near 1
where log2|x| is small and |y| large, an absolute error of about 2^-39 in
log2|x| (|y| up to 2^14 keeps x^y finite with log2 x ≈ 0.003). So
`__libc_fast_log2f_extended` returns log2|x| as hi + lo:

- the reduction of section 8 without its near-1 short cut: x = 2^k z,
  p = z·(1/c_i) rounded with its error p_lo by a fused multiply-add, r = p − 1
  exact (Sterbenz), so z/c_i − 1 = r + p_lo exactly;
- log1p(r + p_lo) = r − r²/2 + r³q₃(r) + p_lo/(1 + r): the square's error by a
  fused multiply-add, q₃ of degree 3 fitted here on the table's r range
  (relative error 2^-37.3), and p_lo/(1 + r) as p_lo(1 − r + r²). The first
  build used p_lo(1 − r): p_lo reaches 2^-24, and the missing p_lo·r² left
  2^-38 at x ≈ 0.996 (entry 18, |log2 x| = 0.003), so powf(0x1.fdd94ap-1,
  −0x1.31f942p+14) was 1.34 ulp and 15 of 400 million samples reached 1 ulp;
- the products by 1/ln2 as a float plus its rounding error (fused
  multiply-add) plus 1/ln2's low part; log2 c_i as a full float and its
  remainder from the table's two parts (a Fast2Sum, exact); k, log2 c_i, the
  r term and the r² term combined by exact two-sums, every error kept in lo.

Measured over every float in [0.5, 2): |hi + lo − log2 x| at most 2^-39.1
(2^-41.6 to 2^-43 next to 1). Then t_hi = y·hi, t_lo = fma(y, hi, −t_hi) +
y·lo; k = round(t_hi), f = t_hi − k exact; u = (f + t_lo)·ln2 as u_hi = f·ln2f
and u_lo its rounding error + f·(ln2 − ln2f) + t_lo·ln2f; then
`__libc_fast_expf_kernel` (section 4) with k. t_hi ≥ 128.5 overflows and
t_hi ≤ −151 underflows whatever the low parts (|lo| < 2^-20 |hi|), with the
raise helpers of section 5 and the sign. The kernel's own scaling covers
128 > t ≥ −151, subnormals included. No double arithmetic (the module refuses
`__aeabi_d*`).

### A strict-library bug found on the way

`libm_thresholds`' new powf cases found that the strict `pow` (and so the
strict `powf` and `powl` on the double cores) returned +1 for (−1)^y with an
odd y beyond 2^20, and for |x| = 1 and |y| beyond 2^20 rounded 1 − 2^-60
downward to 1 − 2^-53: the exact power-of-two path stops at |y| = 2^20, and
the path after it, exp(z) = 1 + z for |z| < 2^-60, kept neither the sign
nor exactness. The exact path now bounds the power y·ilogb(x) by 2^20
instead of y, so ±1 to any integer power ends there, exactly and with its
sign, at no cost in size (`pow.o` 4 bytes smaller on x86_64, 8 on armv7e-m, the
same elsewhere; an explicit |x| = 1 test
cost 24–32 bytes and broke the no-FPU cores' locked speed and fast budgets).
`regression_pow_minus_one` checks it in three rounding directions.

## 11. Measured (P11.03c)

Random samples against glibc's double `pow` (a double reference: exact
enough for an ulp measure of a float), four groups of 400 million: (0) any
positive finite x and y with t uniform in [−150, 128]; (1) x within 2^-4 of 1
and t uniform (|y| up to 2^18), plus a second run within 2^-12 of 1; (2) a
negative x to an odd integer power; (3) uniform bit patterns of x and y.

| Group | double path | float path |
|---|---:|---:|
| any x, t in range | 0.5095 ulp | 0.8531 ulp |
| x near 1, large y | 0.5096 ulp | 0.8749 ulp |
| negative x, odd y | 0.5094 ulp | 0.8382 ulp |
| uniform bits | 0.5095 ulp | 0.8398 ulp |

None reached 1 ulp. The libm vectors, every special case and the
thresholds (`libm_thresholds`) pass in every build type.

Instructions per call (`powf`, speed → fast): x86_64 633 → 181,
armv7e-m-dp 398 → 159, armv7e-m 367 → 215,
armv8-m.main 353 → 203. The `pow` fix also shortened the strict `pow`
in every build type (a power of two is now tested before y's size, which
skips a double comparison): by 8–10 instructions on x86_64 and armv7e-m-dp
(where the strict `powf` calls it), 31–70 on the soft double cores. The cores without an FPU keep the
speed files (D168, P11.03e).

## 12. The m1 and p1 forms (P11.03d)

`expm1f`, `exp2m1f`, `exp10m1f`, `log1pf`, `log2p1f`, `log10p1f` over the
kernels of sections 3, 4, 7 and 8. Near 0 the result is about x (or x ln b,
x / ln b), so an error relative to 1, harmless for e^x or log x, would be
large relative to the result: each form keeps its accuracy relative to |x|.

### Sources (ideas only)

- **FreeBSD `s_expm1f.c`** (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_expm1f.c>,
  read 2026-11-10 through a summary): float only; x = k ln2 + r with a split
  ln2; a short rational approximation with one division and a correction
  term near 0; x itself below about 2^-25; 2^k by the exponent field with
  special cases for very negative k, large k and k = 128. No bound stated.
- **FreeBSD `s_log1pf.c`** (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_log1pf.c>):
  float only; x itself (or x − x²/2) for tiny x; otherwise u = 1 + x with a
  correction term for the rounding of the sum, u's exponent and mantissa in
  [√½, √2), s = f/(2 + f) and a polynomial in s². No bound stated.
- **Arm Optimized Routines `expm1f`, `log1pf`** (AdvSIMD;
  <https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/expm1f.c>,
  `.../log1pf.c`): expm1(r) = r + r² P(r) with P of degree 5, the result as
  p·2^n + (2^n − 1) (no cancellation near 0); 1.13 and 1.20 ulp stated.

What we take: the result built as (2^k − 1) + 2^k(…) so that k = 0 needs no
subtraction of 1; 1 + x with its rounding error carried; x itself as m − 1
near 0 where the double path can.

### The double-FPU cores (`libc/math/fast/`)

- **m1:** u = x ln b in double. Within |u| ≤ 1/8, e^u − 1 = u(1 + u P(u))
  with the Taylor series to u⁶ (`__libc_fast_expm1_series`, relative error
  2^-30.3 however small u is; u(1 + uP) rather than u + u²P, which turns −0
  into +0); beyond, `__libc_fast_exp2`(x log2 b) − 1, whose 2^-30.5 relative
  to 2^t is at most 2^-28 relative to a result at least 0.11 from 0. One
  rounding to float: at most 0.5473 ulp measured. `expm1f` keeps the other
  build types' tiny path (x itself with underflow below 2^-30).
- **p1:** within 1/4 of 0, log(1 + x) from x itself
  (`__libc_fast_log1p_double`, the series of section 7 taking f = m − 1
  directly); beyond, 1 + x in double, exact there, split as 2^e m
  (`__libc_fast_log_split`). The first build formed 1 + x everywhere: below
  2^-29 the sum is not exact in double, and `log1pf`, `log2p1f`, `log10p1f`
  measured 1.02, 1.94 and 2.24 ulp near 2^-30. Now every one is at most
  0.5000 ulp.

### The single-precision FPU cores (`libc/math/sp-nearest/fast/`)

- **m1** (`__libc_fast_expm1f_kernel`, over the reductions of section 4):
  for |k| ≤ 24, e^u − 1 = u_hi + u_hi²/2 + (the square's error + u_lo(1 +
  u_hi) + u_hi³ q₃(u)), u_hi² exact by a fused multiply-add and q₃ the
  upper part of section 4's polynomial (shared, `__libc_fast_expf_q3`). For
  k = 0 that is the result; otherwise (2^k − 1) + 2^k u_hi + 2^k u_hi²/2 by
  exact two-sums, then 2^k times the rest. For k < −24 the power is below
  2^-24 beside −1; above 2^24 the 1 joins the kernel's tail as 2^-k. Three
  builds measured on the way: the tail rounded whole gave 1.12 ulp (2^k
  times its rounding, k = 1, a result near 0.43); squaring u rounded gave
  1.04 (a quarter ulp of a result near 0.24); `exp10m1f`'s reduction low
  part k (log10(2)f − log10 2) reaches 2^-21 at k = 24, and u_lo(1 + u_hi)
  dropped a third of an ulp of it (1.25 ulp at x = 7.07), so `exp10m1f` now
  takes r + r_lo as a fresh pair by a two-sum first.
- **p1** (`__libc_fast_log1pf_reduce`): within 2^-5 of 0, r = x exactly
  against the entry holding 1; otherwise 1 + x = u + c by a two-sum, u
  reduced as section 8 (`__libc_fast_logf_split_z`), and c/(1 + x) carried
  into the low part as c·2^-k·(1/c_i)(1 − r). Then the bases as `logf`,
  `log2f`, `log10f`.
- **Below 2^-90** `exp10m1f`, `log2p1f` and `log10p1f` scale x by 2^64 first
  and round once on the way back: the constants' low parts underflowed and
  the results reached 0.98–0.9999 ulp near FLT_MIN. Zero returns itself:
  where the low part is negative, −0 + +0 would be +0.

Under `-Os` and `-fno-builtin`, `fabs` and `fabsf` are calls and a helper
used twice is not inlined: the hot paths use `__builtin_fabsf` and
`__builtin_fabs`, the kernel evaluates `__libc_fast_expf_q3` once for both
of its paths, and the double p1 files call `__libc_fast_log1p_double`
once (with f = x near 0, or m − 1, and e added only when nonzero: its +0
would turn log(1 + −0) = −0 into +0).

## 13. `cbrtf` (P11.03d)

- **FreeBSD `s_cbrtf.c`** (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_cbrtf.c>,
  read through a summary): an initial guess of about 5 bits from the bit
  pattern (the exponent divided by 3), then two Halley steps in double; the
  final float rounding said to be correct to nearest.
- **Arm Optimized Routines `cbrtf`** (AdvSIMD, `.../advsimd/cbrtf.c`): m in
  [0.5, 1), a degree 3 polynomial, one Newton step, 2^(i/3) from a 5-entry
  table; 1.15–1.64 ulp stated.

Ours, on both paths: |x| = 2^(3q + j) f with f in [1, 2) and j = 0, 1, 2
(q = floor(e/3) by a division of a positive number), m = 2^j f, and the
start cbrt(f)·cbrt(2^j) from a polynomial fitted here by Chebyshev
interpolation (degree 2: 2^-10.1; degree 3: 2^-13.2):

- **double:** degree 3, one Halley step t(t³ + 2m)/(2t³ + m), cubic, to about
  2^-39; rounded once to float (every float is a normal double: no scaling).
  The start uses plain products: the first build's three `__builtin_fma`
  calls are a software `fma` on x86_64, whose baseline has no FMA, and
  measured 1,057 instructions against speed's 150.
- **float:** degree 2, one Newton step in float (about 2^-21), then one with
  the residual t³ − m from exact products (t² and its error by a fused
  multiply-add), to about 2^-40 before the last rounding; subnormals scaled
  by 2^24 first.

Both measured on every finite float: at most 0.5000 ulp, 0.0001 % not
correctly rounded (against `cbrtl`).

## 14. Measured (P11.03d)

Every float argument of the function's range (the m1 forms from just below
their −1 threshold to their overflow, the p1 forms from −1 to FLT_MAX,
`cbrtf` all finite floats) against glibc's long double `expm1l`, `log1pl`
and `cbrtl` (glibc 2.39 has no `exp2m1` and the like; x ln b and the
division by ln b in long double):

| Function | Arguments | double path | float path |
|---|---:|---:|---:|
| `expm1f` | 2,220,097,538 | 0.5473 ulp | 0.8432 ulp |
| `exp2m1f` | 2,228,813,826 | 0.5473 | 0.8345 |
| `exp10m1f` | 2,200,961,026 | 0.5468 | 0.8472 |
| `log1pf` | 3,204,434,336 | 0.5000 | 0.7539 |
| `log2p1f` | 3,204,434,336 | 0.5000 | 0.8061 |
| `log10p1f` | 3,204,434,336 | 0.5000 | 0.8404 |
| `cbrtf` | 4,278,162,238 | 0.5000 | 0.5000 |

Instructions per call (speed → fast):

| Function | x86_64 | armv7e-m-dp | armv7e-m, armv8-m.main |
|---|---:|---:|---:|
| `expm1f` | 264 → 61 | 187 → 66 | 178 → 78 |
| `exp2m1f` | 275 → 56 | 176 → 56 | 173 → 75 |
| `exp10m1f` | 293 → 57 | 194 → 59 | 186 → 92 |
| `log1pf` | 343 → 80 | 209 → 88 | 197 → 96 |
| `log2p1f` | 369 → 75 | 211 → 83 | 186 → 101 |
| `log10p1f` | 369 → 76 | 211 → 84 | 197 → 104 |
| `cbrtf` | 150 → 71 | 135 → 73 | 149 → 81 |

On armv8-m.main `cbrtf` is 78. The cores without an FPU keep the speed files (D168, P11.03e).

## 15. The cores without an FPU: the fixed-point files without their test (P11.03e)

On armv6-m, armv6s-m and armv7-m the speed flavour's `expf`, `exp2f`,
`logf`, `log2f`, `log10f` and `powf` are already fixed point
(`nofp/speed/`, `docs/research/libm-fixed.md`, PLAN D86, D115): the exp2 and
log2 cores give the result with 9 guard bits, within 6 (exp2) or 8 (log2)
units of 2^-9 ulp, and a rounding test sends the results within that
distance of a halfway point to the float-float path. The fast contract
needs no correct rounding, so `nofp/fast/` takes the same paths and rounds
to nearest without the test (`__libc_fixed_nearest`): at most 0.5 + 8/512
ulp. The logarithms' fixed path then covers every positive finite x, and
their float-float code goes; the exponentials keep it for the arguments the
fixed path does not cover (subnormal results, beside the overflow), and
`powf` for those and the carry into the infinities.

`exp10f` was float-float on soft-float on these cores (11,103 instructions
on armv6-m against `expf`'s 375): `nofp/fast/exp10f.c` is `expf`'s fixed
path with log2 10 for 1/ln2 (0xd49a784bcd1b8afe 2^-62, the product below
2^56 as before), for |x| < 37.5 (|t| < 125, a normal result).

The arch files list `nofp` in `LIBC_MATH_FAST_LAYERS` (D168); `sp-nearest`
stays off, measured: with the float-only files of P11.03a–d (their fused
multiply-adds are software `fmaf` calls here) against speed's float-float,

| Function | armv6-m | armv7-m |
|---|---:|---:|
| `expm1f` | 12,310 → 14,544 | 6,065 → 6,396 |
| `exp2m1f` | 10,068 → 14,220 | 4,979 → 6,229 |
| `exp10m1f` | 12,406 → 18,424 | 6,135 → 8,111 |
| `log1pf` | 12,789 → 10,690 | 6,386 → 4,761 |
| `log2p1f` | 11,003 → 11,284 | 5,545 → 4,992 |
| `log10p1f` | 13,202 → 13,883 | 6,619 → 6,095 |
| `cbrtf` | 8,590 → 7,985 | 3,545 → 3,243 |

slower for the m1 forms, a few percent to a quarter faster for the others:
too little to take a layer for, and P11.03f and P11.03g write fixed-point
ones (the fixed-point `exp10f` is 30 to 47 times faster than either).

Measured on the host (the cores compiled natively, the multiply in C as
on armv7-m), every argument of the fixed path:

| Function | Arguments | max ulp |
|---|---:|---:|
| `expf` | 2,237,399,042 (|x| < 87) | 0.5051 |
| `exp2f` | 2,247,596,444 (|x| < 125.9) | 0.5050 |
| `exp10f` | 2,217,423,670 (|x| < 37.4) | 0.5051 |
| `logf` | every positive float | 0.5062 |
| `log2f` | every positive float | 0.5060 |
| `log10f` | every positive float | 0.5062 |
| `powf` | 1.6 billion samples (section 11's groups) | 0.5052 |

Instructions per call (speed → fast):

| Function | armv6-m, armv6s-m | armv7-m |
|---|---:|---:|
| `expf` | 375 → 368 | 125 → 119 |
| `exp2f` | 316 → 310 | 118 → 112 |
| `exp10f` | 11,103 → 367 | 5,461 → 117 |
| `logf` | 2,210 → 670 | 1,026 → 253 |
| `log2f` | 583 → 577 | 247 → 240 |
| `log10f` | 678 → 670 | 259 → 253 |
| `powf` | 1,222 → 1,216 | 374 → 368 |

## 16. The cores without an FPU: the m1 forms (P11.03f)

`expm1f`, `exp2m1f`, `exp10m1f` were float-float on soft-float here
(4,979–12,406 instructions). `nofp/fast/` computes them in 32- and 64-bit
integers over the exp2 core of section 15 (`nofp/fast/fixed-m1.h`):

- **Near 0** (|u| < 1/8 with u = x ln b): e^u − 1 = u(1 + u p(u)), p the
  Taylor series of (e^u − 1 − u)/u² to u⁵ (the next term below 2^-33
  relative) by Horner's rule in signed Q31 with u in Q34; then the 32-bit
  significand of u times (1 + u p) as a 64-bit product, normalised by its
  leading zeros and rounded once (`__libc_expm1f_series_fixed`). u is x
  itself for `expm1f`, and x's significand times ln 2 or ln 10 to 64 bits
  (0xb17217f7d1cf79ac 2^-64, 0x935d8dddaaa8ac17 2^-62), kept to 32, for the
  others: the relative error stays about 2^-30 however small u is. Below
  2^-60 the result is x, or x times the float constant rounded once (the
  subnormal results included). The first build normalised the product by
  one bit: (1 + u p) < 1 with u's significand near 2^31 leaves it below
  2^62, and the sweep found 256,767 ulp at u = −2^-4.
- **Elsewhere** (`__libc_exp2m1f_fixed`): t = x log2 b in units of 2^-37 as
  `expf`, `exp2f`, `exp10f` (sections 15 and the speed files); the core
  gives 2^n(1 + f 2^-32) within 6 units of f; for |n| ≤ 30, (2^n − 1) +
  2^n f 2^-32 is exact in 64 bits (units of 2^-32, or of 2^(n−32) below 1),
  normalised and rounded once. Its relative error is at most
  6·2^(n−32)/|2^t − 1|, below 2^-28.4 since |t| ≥ 0.18 here. From n = 31
  the 1 is below 2^-30 of the result: the exp2 result itself.
- **Beyond:** −1 below −18, −26 and −8 (as the other build types); above
  87, 126 and 37.5, `expf`, `exp2f` or `exp10f` itself (the 1 far below the
  result's ulp), with their overflow.

Measured on the host (the cores compiled natively), every argument from
the −1 threshold to where `expf` and the like take over:

| Function | Arguments | max ulp |
|---|---:|---:|
| `expm1f` | 2,219,691,215 | 0.5361 |
| `exp2m1f` | 2,228,473,039 | 0.5460 |
| `exp10m1f` | 2,200,279,452 | 0.7328 |

`exp10m1f`'s worst is below 2^-62, x times ln 10 as a float (relative
error 2^-26.5) rounded once; elsewhere it is below 0.55 ulp.

Instructions per call (speed → fast):

| Function | armv6-m, armv6s-m | armv7-m |
|---|---:|---:|
| `expm1f` | 12,310 → 461 | 6,065 → 151 |
| `exp2m1f` | 10,068 → 429 | 4,979 → 150 |
| `exp10m1f` | 12,406 → 463 | 6,135 → 152 |
