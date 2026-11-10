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
