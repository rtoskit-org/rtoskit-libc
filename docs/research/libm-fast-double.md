# Fast double functions (P11.05)

Research note for P11.05 ([`docs/plan/phase-11.md`](../plan/phase-11.md),
PLAN D162, D163, D186). It covers the double counterparts of P11.03 and
P11.04: `exp`, `exp2`, `exp10`, `expm1`, `log`, `log2`, `log10`, `log1p`,
`pow`, `cbrt`, `sin`, `cos`, `sincos`, `tan`, `asin`, `acos`, `atan`,
`atan2`, the hyperbolic functions and `erf` (`erfc`, `lgamma` and `tgamma`
stay strict, D163, D178). Each sub-step (P11.05b–o) extends it with its
design, error analysis and measured bounds. Like the float notes it holds
no code from any other implementation (D1, D2): the ideas are described in
our own words, and the constants are computed here.

## 1. The target

- **The fast contract** (D162, D163, `test/unit/include/fast_contract.h`),
  the same for double as for float:
  - below 1 ulp to nearest for exp, log, pow, the trigonometric and the
    inverse trigonometric functions;
  - below 1.5 ulp for `tan`, the hyperbolic functions and `erf`;
  - round to nearest only;
  - the special values, exceptions and errno as in the other build types.
- **The exit target** (P11.06): at or below AOR's instructions per call for
  every case AOR implements, on every Arm core. AOR's double maths in the
  comparison (D92) is `exp`, `exp2`, `exp10`, `log`, `log2`, `pow` and `erf`.
- **Today** (instructions per call, throughput; the fast build is the speed
  build for doubles; AOR's from `docs/perf/aor-<arch>.json`, armv7-m has no
  AOR record):

| Function | x86_64 | armv7e-m-dp | AOR | armv7e-m | AOR | armv7-m | armv6-m | AOR |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| `exp` | 230 | 169 | 56 | 3813 | 1090 | 3874 | 10,317 | 3954 |
| `exp2` | 287 | 183 | 52 | 5235 | 934 | 5320 | 14,133 | 3335 |
| `exp10` | 277 | 185 | – | 5252 | – | 5334 | 14,233 | – |
| `expm1` | 242 | 172 | | 4075 | | 4128 | 10,938 | |
| `log` | 252 | 150 | 68 | 6184 | 1245 | 6405 | 16,964 | 4335 |
| `log2` | 330 | 200 | 73 | 7758 | 1514 | 8007 | 21,420 | 5211 |
| `log10` | 288 | 161 | | 7635 | | 7881 | 21,066 | |
| `log1p` | 316 | 187 | | 6727 | | 6961 | 18,025 | |
| `pow` | 625 | 393 | 135 | 11,531 | 3576 | 11,833 | 31,522 | 12,178 |
| `cbrt` | 143 | 132 | | 3765 | | 4309 | 14,702 | |
| `sin` | 308 | 165 | | 765 | | 770 | 2833 | |
| `cos` | 313 | 167 | | 1067 | | 1074 | 3663 | |
| `sincos` | 446 | 231 | | 1390 | | 1400 | 5177 | |
| `tan` | 427 | 207 | | 1005 | | 1012 | 4970 | |
| `asin` | 481 | 220 | | 13,029 | | 13,481 | 34,625 | |
| `acos` | 504 | 228 | | 13,628 | | 14,076 | 35,718 | |
| `atan` | 378 | 195 | | 7741 | | 8070 | 21,175 | |
| `atan2` | 558 | 360 | | 8314 | | 8667 | 22,603 | |
| `sinh` | 305 | 200 | | 6367 | | 6585 | 17,842 | |
| `cosh` | 284 | 192 | | 5954 | | 6166 | 16,696 | |
| `tanh` | 331 | 202 | | 7193 | | 7444 | 20,152 | |
| `asinh` | 549 | 287 | | 15,215 | | 15,719 | 39,784 | |
| `acosh` | 479 | 246 | | 13,378 | | 13,773 | 34,248 | |
| `atanh` | 383 | 217 | | 9286 | | 9757 | 25,834 | |
| `erf` | 504 | 418 | 69 | 12,171 | 8263 | 13,408 | 40,655 | 25,491 |

  armv8-m.main is within a few instructions of armv7e-m, and armv6s-m equals
  armv6-m. AOR's `exp10` has no bench case. Where the AOR column is empty,
  `bench-runner aor` measured our own function (ratios near 1.0 on the FPU
  core, about 1.15 on armv7e-m, where AOR's link brings libgcc's
  soft-double helpers; P11.06 does not gate on those).

- **What a soft-double operation costs** (the `softfloat` bench, our
  helpers):

| | `dadd` | `dmul` | `ddiv` | `d2f` |
| --- | ---: | ---: | ---: | ---: |
| armv7e-m, armv8-m.main | 55 | 54 | 174 | 19 |
| armv7-m | 56 | 58 | 244 | 19 |
| armv6-m | 107 | 264 | 844 | 52 |

  AOR's `exp` on armv7e-m, 1090 instructions, is about 20 such operations.
  The strict files cost 3.5 to 6 times that because they finish in
  double-double (D60 and its successors) and test the rounding.

## 2. Sources (ideas only)

Read 2026-11-12:

- **Arm Optimized Routines `math/exp.c`, `log.c`, `pow.c`, `erf.c`,
  `math_config.h`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/exp.c>,
  `.../log.c`, `.../pow.c`, `.../erf.c`, `.../math_config.h`):
  - `exp`: a table of 2^(j/128) (7 bits) stored as the bits of the scale
    with the exponent adjusted, a degree-5 polynomial in the reduced
    argument after a two-constant reduction by ln 2/128, and the result as
    scale + scale·tmp with one rounding; the overflow and underflow ends
    handled apart, without intermediate rounding.
  - `log`: near 1 a degree-10 to 12 polynomial in x − 1 with its leading
    term split (0.516 ulp); elsewhere 1/c and log c from a 128-entry table,
    r = z/c − 1 by a fused multiply-add or a second table of c's split, and
    a degree-6 polynomial.
  - `pow`: log(x) to about 15 bits beyond double (the table's log c as two
    parts and r exact), y·log x as a pair, then exp of the pair with the
    tail added to the polynomial; 0.54 ulp, the exp error plus 1024 ln 2
    times the log's relative error times 2^53.
  - `erf`: rational and polynomial pieces below 0.84375 and up to 1.25,
    above an expansion that takes one `exp` (1.01 ulp measured).
- **Qfplib-M0-full** (<https://www.quinapalus.com/qfplib-m0-full.html>):
  hand-written Armv6-M double functions in fixed point. Its table gives
  Cortex-M0 cycles for `exp` 811, `ln` 464 and `atan2` 2187, against
  6655, 4375 and 9973 for GCC's library. So integer kernels are an order of
  magnitude below a soft-double evaluation on that core (the sources were
  not read).
- **Our own D136** (`libc/math/dp-fixed/speed/`): the double `sin`, `cos`,
  `sincos` and `tan` already run a 64-bit fixed-point kernel on the
  soft-double cores (an integer reduction for |x| < 2^20, Taylor
  polynomials in Q63), with a rounding test and a double-double fallback.
  That is why they cost 765–1067 instructions on armv7e-m where AOR's would
  cost about 5660.
- **The float steps of this phase**: P11.03e–g and P11.04e–g measured that a
  fixed-point kernel with one final rounding and no test serves a float
  result at 0.50–0.58 ulp. A double result needs the same with 64-bit
  words: errors of a few units of 2^-63 are below 2^-9 of a double's ulp.

## 3. The choice per core

**The double-FPU cores (x86_64, armv7e-m-dp): double only, no double-double
finish.**
- AOR's structure is the one to match: a 128-entry table and a short
  polynomial, every operation in double, and the last one rounding the
  result. Clean-room versions of these are standard numerical analysis (the
  table of 2^(j/N), the reduction by ln 2/N in two parts, 1/c and log c),
  and the strict files already hold most of the pieces in double-double
  form.
- The fast bound (below 1 ulp) leaves room that AOR does not use: a
  degree-4 exp polynomial (2^-60 relative over |r| ≤ ln 2/256) or a 64-entry
  table where it measures below about 0.55 ulp.
- pow keeps AOR's one extra piece of precision, the log's tail (the bound
  is relative to the result, and y·log x reaches 1024 ln 2). Each sub-step
  measures whether an 11-bit or 15-bit tail is needed.

**The soft-double cores (armv6-m, armv6s-m, armv7-m, armv7e-m,
armv8-m.main): 64-bit fixed-point kernels, one final rounding, no test, no
fallback** (D136's style without its test, as P11.03e and P11.04e did for
float).
- A double-only evaluation on the soft helpers costs about one `dmul` (54
  instructions on armv7-m, 264 on armv6-m) per operation. A 64 × 64 → 128
  product's high word costs about four 32 × 32 → 64 products (`UMULL` on
  armv7-m, about 12 instructions; four 16-bit multiplies each on armv6-m,
  about 80). So the same polynomial costs 4–5 times less in fixed point on
  armv7-m and about 3 times less on armv6-m. It also needs no rounding of
  intermediate results: one rounding at the end, from a 64-bit significand
  (`__libc_trig_fixed_round`'s nearest-even step without its distance test).
- Arguments, results and ranges are integers: the exponent from the bits,
  the significand as a Q52/Q63 integer, the reductions exact.
- Estimated: `exp` about 250–350 instructions on armv7e-m (AOR 1090) and
  1000–1400 on armv6-m (AOR 3954); `log` about 300–400 and 1200–1600 (AOR
  1245, 4335); `pow` the sum of the two plus the 128-bit product of y and
  the logarithm (AOR 3576, 12,178).
- The trigonometric functions already have the kernel. Fast drops the test
  and keeps the strict path only beyond 2^20, as P11.04e1 did for float.

## 4. Error budget and verification

- **Double results cannot be swept exhaustively.** Each sub-step samples at
  least 10^9 random arguments over the whole range, plus 10^8 over each
  piece or table interval and every edge, against `long double` (64-bit
  significand: its own error is about 2^-11 of a double ulp, so the
  measurement is good to 0.001 ulp) and mpmath for the worst points.
- `test/unit/libm_vectors` (D124, the MPFR vectors) checks the fast bounds
  in every build type (`fast/bounds.c`). `libm_thresholds` gains each side of
  every new edge and each regression found.
- **The target error is 0.55 ulp or below**, leaving room under the bound for
  whatever the samples miss. For the fixed-point kernels: at most 2^-61 of
  the result before the final rounding, so the rounding adds 1/256 ulp.

## 5. The split

P11.05 is split by family, each with the FPU cores and the soft-double cores
apart. The two have nothing in common but the contract, and each half is
about one P11.04 sub-step:

| Sub-step | Functions | Cores |
| --- | --- | --- |
| P11.05a | this note, the decisions (D186) | – |
| P11.05b | `exp`, `exp2`, `exp10`, `expm1` | x86_64, armv7e-m-dp |
| P11.05c | the same | the soft-double cores |
| P11.05d | `log`, `log2`, `log10`, `log1p` | x86_64, armv7e-m-dp |
| P11.05e | the same | the soft-double cores |
| P11.05f | `pow`, `cbrt` | x86_64, armv7e-m-dp |
| P11.05g | the same | the soft-double cores |
| P11.05h | `sin`, `cos`, `sincos`, `tan` | x86_64, armv7e-m-dp |
| P11.05i | the same (D136 without the test) | the soft-double cores |
| P11.05j | `asin`, `acos`, `atan`, `atan2` | x86_64, armv7e-m-dp |
| P11.05k | the same | the soft-double cores |
| P11.05l | the hyperbolic functions | x86_64, armv7e-m-dp |
| P11.05m | the same | the soft-double cores |
| P11.05n | `erf` | x86_64, armv7e-m-dp |
| P11.05o | `erf` | the soft-double cores |

The order puts AOR's functions first (b–g, n, o), as they gate P11.06. The
layers follow D168: `libc/math/fast/` for the double-FPU cores (where the
generic double files live), and a new `libc/math/dp-fixed/fast/` for the
soft-double cores, listed in their arch files' `LIBC_MATH_FAST_LAYERS`.
`libc/math/fast/` applies on every core where no later layer has the
function. So between P11.05b and c, the soft-double cores run the
double-only files on their soft helpers. Each FPU sub-step measures what
they cost there; they should be near AOR's, since the structure is the
same.

## 6. exp, exp2, exp10 and expm1 on the double-FPU cores (P11.05b)

`libc/math/fast/exp.c`, `exp2.c`, `exp10.c`, `expm1.c` and the table
`__libc_fast_exp_pairs.c`. No layer has a double exp, so these files apply on
every core. The soft-double cores run them on their helpers until P11.05c.

### 6.1 The method

- **The table:** for j = 0..63, the bits of T = 2^(j/64) correctly rounded
  and of its relative tail (2^(j/64) − T)/T, side by side, so that one base
  address serves both (1 KiB). 2^(k/64) = 2^n T (1 + tail), with
  n = ⌊k/64⌋ added to T's exponent field.
- **The reductions** (k by the 1.5·2^52 shift, which rounds to nearest; in
  a directed mode k may be one off and |u| twice as large, still within the
  polynomial's reach):
  - `exp`: k = round(64x/ln 2), u = (x − k c1) − k c2. c1 = ln 2/64 to 33
    bits, so k c1 is exact for |k| < 2^17 and x − k c1 is exact (Sterbenz);
    c2 is the rest.
  - `exp2`: k = round(64x), r = x − k/64 exactly.
  - `exp10`: k = round(64x log2 10), r = (x − k d1) − k d2 with d1 =
    log10(2)/64 to 31 bits, and u = r ln 10 (rounded: 2^-59.5 of the
    result).
- **The polynomials** (Chebyshev fits in 256-bit arithmetic, coefficients
  rounded to double):
  - e^u − 1 = u + u² q(u), q of degree 3 for |u| ≤ ln 2/128 (u² q within
    2^-57.7);
  - 2^r − 1 = r ln 2 + r² q(r), q of degree 3 for |r| ≤ 1/128 (2^-57.7), so
    exp2 needs no product r ln 2 first.
  - On the common paths both run by Horner's rule as tail + u(1 + u(…)),
    the tail added by the last multiply-add.
  - Measured on armv7e-m-dp: the degree-4 fits over a 32-entry table were 2
    instructions per call more; the separate square and tail addition 2
    more.
- **One rounding:** scale + scale·(tail + p). Before the last addition the
  error is about 2^-57 of the result.
- **The common path** (2^-54 ≤ |x| < 512, 256 for exp10) is guarded by one
  integer test on the exponent bits (`__libc_fast_exp_outside`). It
  builds the scale from the table with no test of the ends.
- **The slow path** (tiny arguments, the ends, the infinities and NaN) is the
  strict file's: `__libc_exp_core` and `__libc_exp_scale`, the double-double
  core that `pow` shares, so a program that calls both carries no second
  copy. It is out of line (`[[gnu::noinline]]`): inlined, its `fabs` call
  (the files build with `-fno-builtin`) made the whole function save
  registers, 6 instructions per call on armv7e-m-dp.
- **The ends, as first built:** the first build scaled by 2^1022 below
  2^-1014 and rounded twice there (0.76 ulp on subnormal results).
  Unscaled, scale·tmp went subnormal near 2^-1020, which also measured
  0.76 ulp. The strict core removed both, and `__libc_fast_exp_scale` is
  left only for `expm1` above 2^60.
- **The thresholds:** overflow above ln(DBL_MAX), 1024 and log10(DBL_MAX).
  Zero (with underflow) below ln 2^-1075, −1075 and log10 2^-1075. The first
  build used log10 2^-1074 for exp10, which returned 0 where 2^-1074 is the
  nearest (1 ulp).
- **Tiny arguments:** 1 + x for exp and exp2. For exp10, 1 + x ln 10: 1 + x
  alone was 1.15 ulp just below 1 (found by the sweep).
- **expm1** keeps exp's reduction with u = hi + lo, hi = x − k c1 exact:
  - For k = 0, the polynomial alone, relative however small x.
  - Otherwise (2^(k/64) − 1), exactly as a pair, plus 2^(k/64)·hi as an exact
    product (one fma where the FPU has it, Dekker's splits on SSE2), plus
    2^(k/64)(lo + u² q + tail (1 + hi)), summed as pairs so that only the
    last addition rounds.
  - Where the two first terms cancel (|x| near ln 2/128), a rounded u or
    tail + p cost up to 2.1 ulp, a dropped tail·p 0.17 and a degree-4 q 0.13.
    expm1 has its own degree-5 q (2^-71.5).
  - From 2^60 the 1 is below 2^-8 of an ulp and exp's result stands.
  - The special values are the strict file's: below −40, −1 + 2^-100 at run
    time, so that it is inexact and rounds in every direction (the fast
    files drop `-frounding-math`, which would otherwise fold it); below
    2^-54, `__libc_math_tiny` (underflow for a subnormal).

### 6.2 Measured (P11.05b)

On the host, against long double (10^9 samples per function, half uniform
over the domain and half with random exponents):

| Function | sampled worst | every binade's 2^16 significands |
| --- | ---: | ---: |
| `exp` | 0.5469 | 0.5381 |
| `exp2` | 0.5464 | 0.5464 |
| `exp10` | 0.5527 | 0.5425 |
| `expm1` | 0.5146 | 0.5093 |

The bound is 1.

Instructions per call (speed → fast, the bench's plugin):

| Function | x86_64 | armv7e-m-dp | AOR (armv7e-m-dp) |
| --- | ---: | ---: | ---: |
| `exp` | 230 → 51 | 169 → 53 | 56 |
| `exp2` | 287 → 47 | 183 → 51 | 52 |
| `exp10` | 277 → 52 | 185 → 55 | – |
| `expm1` | 242 → 134 | 172 → 106 | – |

How the counts came down on armv7e-m-dp, for `exp2`: 68 instructions with
the first build, 62 with the integer guard, 56 with the slow path out of
line, 54 with the 64-entry table and degree 3, 53 with one pointer to the
table entry, and 51 with Horner's rule and the tail in its last step.

The soft-double cores run these files on their helpers until P11.05c, and
they gain there too: `exp` 3813 → 1170 instructions on armv7e-m (AOR 1090),
10,317 → 3648 on armv6-m (AOR 3954); `exp2` 5235 → 1035 (AOR 934) and
14,133 → 3233 (AOR 3335); `expm1` 4075 → 3549 and 10,938 → 9720.

**Size.** The `math-mix` size program calls `exp` and `pow`, and `pow` still
pulls in the strict core until P11.05f/g. So the fast `exp` and its 1 KiB
table come on top of it: against the locked fast figures, armv6-m and
armv6s-m +1212 bytes, armv7-m +1176; armv7e-m-dp 15,478 bytes, against speed's
12,302 (+25.8 %, over the +20 % budget). The
maintainer decided (D187) that the fast figures may grow through P11.05:
each step re-locks the fast figures of the cores it grows and records the
growth, and P11.06 re-locks them all with the final figures. This step
re-locked armv6-m, armv6s-m, armv7-m and armv7e-m-dp (`size-report --flavour
fast --lock`). armv7e-m-dp now has its own fast figures, as the cores without
an FPU do.

## 7. exp, exp2 and exp10 on the soft-double cores (P11.05c1)

`libc/math/dp-fixed/fast/`, listed in the soft-double cores' arch files
(`LIBC_MATH_FAST_LAYERS`: armv6-m, armv6s-m and armv7-m add it to nofp,
armv7e-m and armv8-m.main to sp-nearest). As a later layer it shadows §6's
double-FPU files there. P11.05c was split: expm1 (c2) needs relative
accuracy where 2^(k/64) − 1 and the polynomial cancel, which the kernel's
absolute 2^-62 does not give.

### 7.1 The method

- **No soft-double operation on the common path.** Everything is done on
  x's bits, the 53-bit significand m and the exponent e, with the speed
  files' 32 × 32 → 64 product (`__libc_trig_wide`: `UMULL` inline on the
  Thumb-2 cores, four 16-bit products out of line on Armv6-M) and its
  high-64 product (D136).
- **t = |x|·c as n + f·2^-64** (`__libc_dp_fixed_scaled`): c is log2(e) or
  log2(10) to 128 bits, in two 64-bit words (log2(e) in Q63, log2(10) in
  Q62). m·c_hi is exact in 117 bits (four products), plus the high part of
  m·c_lo (one product: below a unit of what the shift keeps). The shift by
  51 − e (50 − e) leaves n, below 2^10, and the 64-bit fraction; the dropped
  bits are below 2^-64. For x < 0, n + f is negated. exp2's t is x itself: m
  shifted, no product.
- **The kernel** (`__libc_dp_fixed_exp2`, out of line and shared by the
  three functions): 2^f = 2^(j/64)·2^r with j the top 6 bits of f.
  - 2^(j/64) comes from a table in Q63 (`__libc_dp_fixed_exp2_table`, 512
    bytes).
  - For r in [0, 1/64), 2^r − 1 is its Taylor polynomial in u = 64r, Q64,
    to degree 7 (the eighth term is below 0.09 units of 2^-64). All the
    coefficients are positive, so Horner's rule runs unsigned, and the three
    highest coefficients fit in 32 bits, so their products take two partial
    products rather than four.
  - y = T + T·P in Q63, in [2^63, 2^64), is rounded to 53 bits by its 54th
    bit straight into the double's bits: ((n + 1022) << 52) + (y >> 11) +
    the round bit. The implicit bit adds the last 1 to the exponent, and a
    carry from rounding goes into it too.
  - The error before that rounding is a few units of 2^-63 (the table's half
    unit, the products' floors): below 2^-9 of an ulp.
- **The common path** (2^-54 ≤ |x| < 512, 256 for exp10, `__libc_fast_exp_outside`)
  keeps every result normal. The slow path is §6's, moved into
  its own hidden functions in `libc/math/fast/` (`__libc_fast_exp_special.c`,
  `__libc_fast_exp2_special.c`, `__libc_fast_exp10_special.c`) so that both
  layers share it: the strict file's double-double core, which `pow` shares.
  As `static` functions in `math-fast.h` they were emitted at -O0 into every
  file that includes it, and the float-only sp-nearest layer's check refused
  their double helpers on armv6-m debug (`inline` with `noinline` is a GCC
  warning, so the out-of-line form is a separate file, D6).

### 7.2 Measured (P11.05c1)

On the host (the files compiled with the library's headers and the speed
files' `umull` product; armv6-m's `muls` product gives the same bits), against
long double: 2·10^8 samples per function and every binade's 2^16
significands.

| Function | sampled worst | every binade |
| --- | ---: | ---: |
| `exp` | 0.5029 | 0.5020 |
| `exp2` | 0.5063 (slow path) | 0.5024 |
| `exp10` | 0.5073 | 0.5039 |

Instructions per call (speed → fast; §6's double-FPU files on the soft
helpers were in between, see the PR):

| Function | armv7e-m, armv8-m.main | armv7-m | armv6-m, armv6s-m |
| --- | ---: | ---: | ---: |
| `exp` | 3813 → 317 (AOR 1090) | 3874 → 318 | 10,317 → 1410 (AOR 3954) |
| `exp2` | 5235 → 281 (AOR 934) | 5320 → 282 | 14,133 → 1210 (AOR 3335) |
| `exp10` | 5252 → 316 | 5334 → 317 | 14,233 → 1412 |

## 8. expm1 on the soft-double cores (P11.05c2)

`libc/math/dp-fixed/fast/expm1.c`, over §7's kernel.

### 8.1 Sources (ideas only)

Read 2026-11-12:

- **FreeBSD msun `s_expm1.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_expm1.c>,
  fdlibm): x = k ln 2 + r with |r| ≤ ln 2/2 and the reduction's rounding
  error carried as a correction; e^r − 1 from a degree-5 rational form in r²
  (2^-61); and 2^k (1 + p) − 1 never formed as a subtraction from a value
  near 1: the −1 is folded into a per-range constant (1 − 2^-k) or handled
  apart for k = ±1. Under 1 ulp.
- **Arm Optimized Routines, SVE `expm1`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/sve/expm1.c>):
  the reduction by ln 2/64, a degree-6 polynomial, and, near zero (below
  about 15 ln 2/64), a table of 2^(i/64) − 1, so that scale − 1 does not
  cancel; elsewhere (scale − 1) + scale·poly. 2.81 + 0.5 ulp. AOR has no
  scalar double expm1, so P11.06 has no AOR count to gate it on.
- **CORE-MATH `expm1`**: the host refused automated access; not read.

Both handle the cancellation the same way in essence: the subtraction of 1
is either exact or taken out of the range where the result is small. In
fixed point the subtraction is exact in integers anyway, so only the
kernel's absolute error remains, and it matters only where the result is
small.

### 8.2 The method

- **Below 1/16** (`|x| < 2^-4`), the series: expm1(x) = x (1 + q) with
  q = x/2 + x²/6 + … in Q64, from y = 16|x| in Q64 and the coefficients
  2^-4k/(k + 1)! in Q64, k = 1..9 (the tenth is below 0.43 units of 2^-64).
  Horner's rule runs unsigned, subtracting each step for x < 0 (every
  partial value stays positive, since the coefficients fall by more than 16
  each step). The four innermost steps' operands fit in 32 bits, so they take
  two partial products. q's absolute error of a few units of 2^-64 is a
  relative error of x (1 + q), however small x is, down to 2^-54. Then
  m·(1 + q) in Q63, normalized (at most two shifts) and rounded once by its
  54th bit.
- **From 1/16**, the kernel: t = x log2(e) as in §7 (`__libc_dp_fixed_scaled`),
  y = 2^f in Q63 from the kernel's polynomial, now an inline
  (`__libc_dp_fixed_exp2_fraction`) that `__libc_dp_fixed_exp2` wraps (its
  object is unchanged), and the 1 subtracted exactly in integers:
  - x > 0, n ≥ 0: y − 2^(63−n), or y alone from n = 64, where the 1 is below
    a unit of y;
  - x < 0, n ≤ −1: 2^63 − (y >> −n), negated.

  The result is then at least 1 − e^-1/16 = 0.0606, so the kernel's few
  units of 2^-63 are at most 0.03 ulp of it. A 1/32 threshold measured
  0.5513 ulp at the edge, one step less in the series; 1/16 measured 0.5254.
- **The ends:** NaN, below 2^-54 (x nudged, underflow for a subnormal) and
  from 512 (§6's exp slow path: the 1 is below 2^-500 of the result) go out
  of line; below −40, e^x is below 2^-57 and the result −1 + 2^-100, at run
  time so that it is inexact and rounds in every direction.

### 8.3 Measured (P11.05c2)

On the host, as §7.2: 2·10^8 samples over (−45, 710) at most 0.5215 ulp, and
every binade's 2^16 significands from 2^-56 to 2^9, both signs, at most
0.5239; 10^6 floats on each side of 1/16, 2^-54, 64 ln 2, ±ln 2, ±1, −40 and
512 at most 0.5254 (just below −1/16). `exp`, `exp2` and `exp10` measured as
in §7.2, bit for bit.

Instructions per call (speed → fast; P11.05b's double file on the soft
helpers was 3549 on armv7e-m and 9720 on armv6-m):

| Function | armv7e-m, armv8-m.main | armv7-m | armv6-m, armv6s-m |
| --- | ---: | ---: | ---: |
| `expm1` | 4075 → 346 | 4128 → 347 | 10,938 → 1468 |

The kernel's polynomial as an inline with `constexpr` coefficients saved
one instruction in `exp`, `exp2` and `exp10` on the Thumb-2 cores. The
`expm1` member is 1622 bytes on armv6-m (the series, the kernel's
polynomial inline, and the reduction); no size program calls it.

## 9. log, log2, log10 and log1p on the double-FPU cores (P11.05d)

`libc/math/fast/log.c`, `log2.c`, `log10.c`, `log1p.c`, the tables
`__libc_fast_log_table.c` and `__libc_fast_log2_table.c` (3 KiB each) and
`__libc_fast_log_special.c`; on every core with no layer of its own, so the
soft-double cores run them on their helpers until P11.05e.

### 9.1 Sources (ideas only)

AOR's `log` (§2): 1/c and log c from a 128-entry table, r = z/c − 1 by a fused
multiply-add (or a second table of c split in two without one), a degree-5
polynomial after r², and near 1 a separate polynomial of degree 10 to 12 in
x − 1, because there the table's rounding of log c and r's rounding would show.
AOR's `log2` has its own table and r/ln 2 as a pair.

### 9.2 The method

- **An exact r everywhere, so no separate path near 1.** x = 2^k z with z in
  [0x1.68aaap-1, 0x1.68aaap+0), the entry by the top 7 bits of z's fraction.
  The offset puts 1 two thirds of the way into its entry in bit order (its
  lower part is half as wide), and that entry's invc is 1. Every other invc is
  chosen on a grid of 2^-6 to 2^-10, with at most 10 significant bits, to
  make |r| smallest while z·invc − 1 still fits 53 bits for every z of the
  entry: |r| ≤ 2^-7.47 (the entries just above 1, where the grid must be
  coarse). r = z·invc − 1 is then exact:
  - with a fused multiply-add (armv7e-m-dp);
  - without one (x86_64), z's top 43 bits times invc is exact, minus 1 is
    exact (Sterbenz), the rest of z times invc is exact, and their sum is
    exact because r is representable.
- **logc = −log(invc), rounded to a multiple of 2^-43** (2^-42 for log2) with
  its tail, so that w = k·ln 2_hi + logc is exact (ln 2_hi has 42 bits; for
  log2, k + logc). hi + lo = w + r by a fast two-sum: wherever w is not 0,
  |w| ≥ |r| (checked for every entry).
- **The polynomial:** (log1p(r) − r)/r² by a Chebyshev fit of degree 5 in
  double on [−2^-7.47, 2^-7.47]: r² times it is within 2^-60.1 of log1p(r),
  relatively (log2's, scaled by 1/ln 2, 2^-59.7). The result is
  lo + r²·P(r) + hi, one rounding. Near 1, k = 0 and invc = 1, so w = 0 and the
  result is r + r²·P(r), relative however small r is.
- **log2:** its own table in base 2, and r/ln 2 as t + te: a fused
  multiply-add for the product's error, or without one r's top 21 bits times
  1/ln 2 to 32 bits (exact) plus the rest, which leaves an error near 2^-74 of r.
- **log10:** log's hi and lo, times 1/ln 10 as a pair with the product's error
  exact (fma or Dekker), so that only the last addition rounds. Powers of 10
  come out exact: the error is far below half an ulp of the integer.
- **log1p:** for |x| ≤ 2^-7.5, x + x²·P(x) with log's polynomial; otherwise
  u = 1 + x and its error e by a fast two-sum (the larger magnitude first),
  log(u) as log's, and e/u added to the tail. One division: e·invc·2^-k
  would leave e·r, up to 2^-53 of the smallest result there (2^-7.5).
- **The ends:** one integer test sends x ≤ 0, +inf and NaN out of line
  (`__libc_fast_log_special`) and scales a subnormal by 2^52 in place. log(1)
  must be +0 in every direction, but r = z·invc − 1 is −0 rounding downward.
  `log` tests x = 1. `log2` and `log10` test r = 0 instead, which happens only
  for z = 1 (no other invc is a power of 2), and return k (k log10 2, rounded
  once) at once. On armv7e-m-dp that test saved 2 instructions in `log2`
  (73.6 to 71.6, AOR 73) but cost 1 in `log` (67.6 to 68.6, AOR 68), so `log`
  keeps the test for x = 1.
- **log1p's ends:** quiet comparisons (`isgreater`, `isless`), so that a quiet
  NaN raises nothing; above 2^53 u = x and e = 1 (1 + x rounding upward would
  overflow DBL_MAX to infinity), and from 2^60 e = 0, so that e/u never goes
  subnormal (the strict file raises a spurious underflow there).
- **Found along the way:**
  - A first build returned 0 from the scaling helper for the special values;
    2^-1023, scaled, has the bit pattern 0, and log2 returned the argument
    (a regression case in `libm_thresholds`).
  - `log1p` compared x with relational operators, which raise invalid for a
    quiet NaN, and rounding upward 1 + DBL_MAX overflowed to infinity and gave
    NaN (`libm_vectors`).

### 9.3 Measured (P11.05d)

On the host, against long double, 10^9 samples per function (every positive
double, [0.5, 2], within 2^-40 of 1, and [10^-3, 10^3]; log1p over x − 1 of
the same), on both paths (SSE2 and a fused multiply-add, which agree bit for
bit): `log` 0.5151 ulp, `log2` 0.5249, `log10` 0.5132, `log1p` 0.5059. 2·10^5
to 10^6 doubles on each side of the subnormal range's ends, DBL_MAX, 1, the
table's ends and log1p's switch at ±2^-7.5: at most 0.5083. Every power of 2
in `log2` and of 10 (to 10^22) in `log10` exact.

Instructions per call (speed → fast):

| Function | armv7e-m-dp | x86_64 | armv7e-m, armv8-m.main | armv7-m | armv6-m, armv6s-m |
| --- | ---: | ---: | ---: | ---: | ---: |
| `log` | 150 → 68 (AOR 68) | 252 → 77 | 6184 → 1649 (AOR 1245) | 6405 → 1683 | 16,964 → 4902 (AOR 4335) |
| `log2` | 200 → 72 (AOR 73) | 330 → 88 | 7758 → 1845 (AOR 1514) | 8007 → 1884 | 21,420 → 5441 (AOR 5211) |
| `log10` | 161 → 77 | 288 → 102 | 7635 → 2685 | 7881 → 2743 | 21,066 → 8108 |
| `log1p` | 187 → 86 | 316 → 92 | 6727 → 2108 | 6961 → 2149 | 18,025 → 5808 |

The soft-double cores run these files on their helpers until P11.05e gives
them fixed-point kernels; they are already 3 to 4 times faster than speed.

**Size:** the table is 3 KiB (128 entries of three doubles); `log` is 340
bytes on armv7e-m-dp and 556 on armv6-m. `math-mix`, which calls `log` and
`pow`, grows by 3.3 KiB in fast on the Arm cores while `pow` keeps the
strict log core (armv6-m 42,552 → 45,992 bytes, armv7-m 37,552 → 40,832,
armv7e-m-dp 15,478 → 18,806, armv7e-m 31,284 → 34,828, armv8-m.main 31,188 →
34,724); by the maintainer's decision for P11.05 (D187) those fast figures are
re-locked. A table of floats for invc and the tail (2 KiB) would cost two
conversions per call, and `log` would miss AOR's count.

## 10. log, log2, log10 and log1p on the soft-double cores (P11.05e)

`libc/math/dp-fixed/fast/log.c`, `log2.c`, `log10.c`, `log1p.c`, over two
out-of-line kernels (`__libc_dp_fixed_log_core.c`, `__libc_dp_fixed_log_near.c`)
and a 90-entry table (`__libc_dp_fixed_log_table.c`, 1440 bytes). They shadow
§9's double files on armv6-m, armv6s-m, armv7-m, armv7e-m and armv8-m.main.
Sources as §9.1 and §2 (Qfplib-M0-full's cycle counts: an integer `ln` an
order of magnitude below a soft-double one).

### 10.1 The method

- **No soft-double operation.** x = 2^k z with z in [45/64, 90/64) in Q63,
  from x's bits (a subnormal normalized with a count of leading zeros).
- **The table path** (|z − 1| ≥ 2^-5 or k ≠ 0): the entry by (z − 45/64) in
  steps of 2^-7; invc = 1/c in Q31 (c the entry's middle) and −log(invc) in
  Q64, for that invc exactly.
  - r = z·invc − 1: the Q63 by Q31 product is exact in 96 bits (two 32 × 32
    products), less 1, kept in Q64 (one floor), |r| ≤ 2^-7.5.
  - ln z = −log(invc) + r + r·q, q = Σ (−r)^n/(n + 1) for n ≤ 7 in Q64 from
    y = 2^7|r| by Horner's rule, unsigned (subtracting for r > 0; every
    partial value stays positive). The three innermost steps' operands fit in
    32 bits. r·q is always negative. The error is a few units of 2^-64.
  - The result as a 128-bit value: k·ln 2 (ln 2 in Q64) + ln z for `log`;
    k + ln z·log2(e) (as ln z + ln z·(log2(e) − 1), since the sum can reach
    2^-1) for `log2`; k·log10(2) + ln z·log10(e) for `log10`. Then its top 64
    bits, normalized, rounded once by the 54th bit. The result is at least
    2^-6.3 in magnitude on this path, so the few units of 2^-64 are at most
    0.03 ulp.
- **Strictly within 2^-5 of 1** (k = 0): s = z − 1 is exact; normalized, the
  series s·(1 + q) with q = Σ (−s)^n/(n + 1) for n ≤ 11 in Q64 from
  y = 2^5|s| (the six innermost steps fit in 32 bits), times the base's
  constant (log2(e), or 4 log10(e), normalized in Q63) for `log2` and `log10`:
  relative however close x is to 1. log(1) returns +0.
- **log1p:** below 2^-5 the same series on x itself (below 2^-60, x nudged
  toward −inf); otherwise 1 + x as 2^k z in integers: exact for x < 0
  (1 − |x|, normalized) and up to 2^11, else to 64 bits (2^-64 relatively, at
  most 2^-64 on the logarithm), and the table path.
- **The ends:** x ≤ 0, +inf and NaN by one unsigned test, out of line to §9's
  `__libc_fast_log_special`, which therefore stays built on every core.
- **Found along the way:** the band around 1 was closed at 1 − 2^-5: there
  |s| = 2^-5 made the series' shift negative (10^14 ulp at that one double).
  The band is now open at both ends (a regression case).

### 10.2 Measured (P11.05e)

On the host (the speed files' `umull` product; Armv6-M's `muls` product gives
the same bits), against long double, 10^9 samples per function as §9.3: `log`
0.5098 ulp, `log2` 0.5161, `log10` 0.5171, `log1p` 0.5098. 10^6 doubles on
each side of 1 ± 2^-5, 45/64, 90/64, 1 and log1p's ±2^-5: at most 0.5190
(log10 just outside the band, where its result is smallest). Every power of 2
in `log2` and of 10 in `log10` exact.

Instructions per call (speed → §9's double files on the soft helpers → fast):

| Function | armv7e-m, armv8-m.main | armv7-m | armv6-m, armv6s-m |
| --- | ---: | ---: | ---: |
| `log` | 6184 → 1649 → 334 (AOR 1245) | 6405 → 1683 → 336 | 16,964 → 4902 → 1454 (AOR 4335) |
| `log2` | 7758 → 1845 → 349 (AOR 1514) | 8007 → 1884 → 351 | 21,420 → 5441 → 1535 (AOR 5211) |
| `log10` | 7635 → 2685 → 354 | 7881 → 2743 → 357 | 21,066 → 8108 → 1601 |
| `log1p` | 6727 → 2108 → 367 | 6961 → 2149 → 371 | 18,025 → 5808 → 1479 |

**Size:** the table is 1440 bytes, the two kernels 422 and 582 bytes on
armv7e-m (570 and 682 on armv6-m), each function 656 to 944. `math-mix`
shrinks against §9 (armv6-m 45,992 → 45,752 bytes, armv7-m 40,832 → 40,296,
armv7e-m 34,828 → 34,300, armv8-m.main 34,724 → 34,188): not re-locked; P11.06
does (D187, D191).

## 11. pow and cbrt on the double-FPU cores (P11.05f)

`libc/math/fast/pow.c` and `cbrt.c`, on every core with no layer of its own
(the soft-double cores until P11.05g).

### 11.1 Sources (ideas only)

- **AOR's `pow`** (§2): log x to about 15 bits beyond double from its own
  table with log c in two parts, y·log x as a pair, exp of the pair with the
  tail folded into the polynomial; 0.54 ulp.
- **FreeBSD msun `s_cbrt.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_cbrt.c>,
  read 2026-11-13): the exponent divided by 3 on the bit pattern for a 5-bit
  guess, a degree-4 polynomial to 23 bits, t rounded to 23 bits (away from
  zero, deliberately), then one Halley step with rounded operations; under
  0.667 ulp.
- **AOR's AdvSIMD `cbrt`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/cbrt.c>):
  m in [1/2, 1) and e/3 by a multiply, a table of 2^(i/3), a degree-3
  polynomial and two Newton steps with no exact residual; 1.79 ulp observed.
  AOR has no scalar double cbrt, so P11.06 has no AOR count to gate it on.

### 11.2 The method

- **pow's logarithm as a pair**, from §9's table (no second table, so a
  program that calls `log` and `pow` carries one): r = z·invc − 1 exact,
  w = k·ln 2_hi + logc exact, hi = (w + r) − r²/2 by two fast two-sums with
  r² exact (a fused multiply-add, or Dekker's product), and lo the two sums'
  errors, r²'s error, r³·Q(r) (a degree-6 fit of (log1p(r) − r + r²/2)/r³,
  2^-54 relatively, the rounding of 1/3 itself) and the tails. lo is not
  normalized: r³ Q reaches 2^-16 of hi. The error times |y| is what reaches
  the result: below 2^-61 of it, since |y log x| ≤ 745 and Q's error is
  relative to r³/3. Degree 5 (2^-51.2) measured 0.5591 ulp on 10^9 samples at
  y log x ≈ −319, two instructions fewer.
- **y·(hi + lo)** as zh + zl, the product's error exact.
- **exp of the pair** with §6's table and polynomial: the index from zh + zl
  (lo's size could otherwise push the reduced argument beyond the
  polynomial's range), u = ((zh − k c1) − k c2) + zl, and the sign put into
  the scale's bits, so that only the last addition rounds.
- **The ends:** one test on |y log x| > 700 sends overflow, underflow and the
  scaled path (§6's `__libc_fast_exp_scale`) out of line; below it the scale's
  exponent needs no check.
- **The special cases** (F.10.4.5 and the strict file's) that need no
  logarithm are out of line: y = ±0, x = 1, NaN, an infinite y, x = ±0 or ±inf,
  x < 0 with a non-integer y, and |y| below 2^-65 or from 2^63 (beyond the
  ends for every x other than ±1). A negative x with an integer y and a
  subnormal x are fixed in line and take the common path. A first version
  passed the fix-ups back through pointers: x, the sign and the result then
  lived in memory, and GCC saved d8–d15 and five integer registers on every
  call (152 instructions on armv7e-m-dp; 131 without them).
- **cbrt:** x = 2^(3q + j) m with m in [1, 2) (q by a multiply), cbrt(m) by a
  degree-4 Chebyshev fit (2^-16.2) times cbrt(2^j), one Newton step (2^-31),
  and one with the residual t³ − y exact (two exact products by fused
  multiply-adds; t³ − y itself exact by Sterbenz): about 2^-62 before the one
  rounding, t − d. Without a fused multiply-add (x86_64, and the soft-double
  cores until P11.05g), t is cut to 26 bits so that t² and its split make t³
  exact with plain products, and Halley's step (cubic) leaves 2^-78: Dekker's
  two products had cost those cores 1.1 % over speed's cbrt (3807 against 3765
  instructions on armv7e-m), past the bench gate. The sign goes on t and d
  before the rounding; the exponent afterwards, exactly.
- **Found along the way:**
  - lo's r³ Q, up to 2^-16 of hi, made zl about 2^-7, so the reduced argument
    left exp's polynomial range: 130 ulp at y log x ≈ 662 (a regression case).
  - The first test for the ends allowed |y log x| up to 746, but above
    ln(DBL_MAX) the scaled path's exponent leaves its field and wrapped into
    the sign: a tiny negative result instead of +inf (a regression case).
  - `conformance`'s directed-rounding regressions (Q36) ran against the new
    files: `cbrt` put the sign on after rounding, which swaps upward and
    downward for a negative x, and `pow`'s nudge for |y| below 2^-65, 1 +
    2^-60, folded to 1 at compile time (the fast files drop
    -frounding-math). The sign now goes on t and d before the rounding, and
    the nudge is a volatile.

### 11.3 Measured (P11.05f)

On the host, against long double, 10^9 samples each on the SSE2 and the fma
paths (which agree bit for bit): `pow` at most 0.5498 ulp (x in [1/2, 4] with
y in [−8, 8], x across the range with |y| ≤ 1, x within 2^-50 of 1 with y up
to 2^50, results across the range, and x in [e^-30, e^30] with |y| ≤ 25);
`cbrt` 0.5000 ulp over every finite double (both paths), every exact cube
to 100³ exact.

Instructions per call (speed → fast):

| Function | armv7e-m-dp | x86_64 | armv7e-m, armv8-m.main | armv7-m | armv6-m, armv6s-m |
| --- | ---: | ---: | ---: | ---: | ---: |
| `pow` | 393 → 133 (AOR 135) | 625 → 204 | 11,531 → 5600 (AOR 3576) | 11,833 → 5717 | 31,522 → 16,457 (AOR 12,178) |
| `cbrt` | 132 → 90 | 143 → 93 | 3765 → 1919 | 4309 → 2106 | 14,702 → 6896 |

The soft-double cores run these files on their helpers until P11.05g; `pow`
there is still above AOR's, which P11.05g has to bring below.

**Size:** `pow` is 1724 bytes on armv7e-m-dp, `cbrt` 380. `math-mix` (log
and pow) shrinks on armv7e-m-dp from 18,806 to 17,548 bytes now that pow no
longer pulls the strict log and exp cores (+44 % over speed, D191's line is
+20 %). On the soft-double cores it grows (armv6-m 45,752 → 46,928,
armv7-m 40,296 → 41,392, armv7e-m 34,300 → 35,252, armv8-m.main 34,188 →
35,148): their pow carries §9's 3 KiB table besides §10's; re-locked by
D187's rule until P11.05g.

## 12. pow on the soft-double cores (P11.05g1)

`libc/math/dp-fixed/fast/pow.c`, shadowing §11's double file on the
soft-double cores. Sources as §11.1 (AOR's pow: log x to about 15 bits beyond
double, y·log x as a pair, exp of the pair); the arithmetic here is integer.
P11.05g was split: cbrt (g2) shares none of this.

### 12.1 The error budget

|y·log2 x| is at most 1075 for a finite nonzero result, and the exponential
kernel (§7) needs y·log2 x to about 2^-62 absolutely. So log2 x must be good
to about 2^-72 relatively: beyond one 64-bit word, but not by much. A
relative bound (not an absolute one) matters near 1, where log2 x is small
and y can be up to 2^62.

### 12.2 The method

- **x = 2^k z** with z in [45/64, 90/64) in Q63, as §10 (a subnormal x
  normalized there).
- **The table path** (k ≠ 0 or z strictly beyond 2^-5 of 1): ln z in Q126,
  in two 64-bit words, to about 2^-82 absolutely:
  - −log(invc) from §10's table, extended to Q96 by a signed 32-bit tail in
    what was the entry's padding (the table's size and §10's results are
    unchanged);
  - r = z·invc − 1 exact, from the 96-bit product;
  - r²/2 from r to 2^-71 (|r|·2^71 squared, 128 bits);
  - r³·S(r), S = Σ (−r)^m/(m + 3) for m ≤ 8, by unsigned Horner's rule in
    y = 2^7|r| in Q64 (the four innermost steps' operands fit in 32 bits).
  ln z · log2(e) is one product of normalized 96-bit values (below); k is
  added in Q96, then the sum normalized again. On this path |log2 x| is at
  least 2^-4.6, so the absolute bound is a relative one.
- **Within 2^-5 of 1** (k = 0): s = z − 1 exactly, and log1p(s) = s·W with
  W = 1 − s/2 + s²·S(s) in Q126 (s/2 exact, s² exact in 128 bits, S of
  thirteen terms in y = 2^5|s|): relative however close x is to 1. Then
  times log2(e).
- **The products**: a normalized value m·2^(e−95), m of 96 bits; a product
  keeps the 192-bit product's columns from 2^64 up (six 32 × 32 products; the
  lower ones change it by 2^-125) and renormalizes by one bit. Three of them
  per call: ·log2(e), ·y, and on the near path s·W.
- **The exponential**: y·log2 x as n + f·2^-64 (one shift of the 96-bit
  significand), then §7's kernel, one rounding. From 2^10 (positive) the
  result overflows; for n below −1022 a subnormal is rounded once at its own
  last place from the kernel's 64 bits (`__libc_dp_fixed_exp2_fraction`);
  below −1075 it underflows.
- **The special cases** that need no logarithm are §11's, now shared as
  `__libc_fast_pow_special` (`libc/math/fast/`); a negative x with an
  integer y is fixed in line.
- **Found along the way:**
  - k went into the high word's bit 0 (Q64's units place) instead of
    bit 32 (Q96's): 2^10 came out as 1.0000000069.
  - Products from −1024 to −1075 went to the underflow branch (0 where a
    subnormal was due); then the subnormals were rounded twice, by the kernel
    to 53 bits and by a multiply by 2^-64 (0.7502 ulp). Both are regression
    cases.

### 12.3 Measured (P11.05g1)

On the host (the speed files' `umull` product), against long double, 10^9
samples as §11.3: at most 0.5029 ulp, 0.05 % not correctly rounded (§11's
double file: 0.54 %). 4·10^6 edge cases (subnormal and underflowing
results, both ends, a negative x to integer powers, a subnormal x, x within
2^-40 of 1 with y to 2^60, the band around 1): at most 0.5020, subnormal
results included.

Instructions per call (speed → §11's double file on the helpers → fast):

| Core | `pow` | AOR |
| --- | ---: | ---: |
| armv7e-m, armv8-m.main | 11,531 → 5600 → 923 | 3576 |
| armv7-m | 11,833 → 5717 → 923 | |
| armv6-m, armv6s-m | 31,522 → 16,457 → 3938 | 12,178 |

**Size:** the member is 3828 bytes on armv7e-m and 4118 on armv6-m (its
helpers inline). `math-mix` (log and pow) shrinks against §11 by 2100 to
2454 bytes (armv6-m 46,928 → 44,828, armv7-m 41,392 → 38,980, armv7e-m
35,252 → 32,806, armv8-m.main 35,148 → 32,694): §11's 3 KiB double table and
the exp pairs leave these cores. Against speed it is now +7 % to +9 %, inside
D191's +20 %.

## 13. cbrt on the soft-double cores (P11.05g2)

`libc/math/dp-fixed/fast/cbrt.c`, shadowing §11's double file on the
soft-double cores. §11's file runs there on the soft-double helpers: a
handful of soft multiplies and two divisions, 1919 instructions on armv7e-m
and 6896 on armv6-m.

### 13.1 Sources (ideas only)

- **§11.1's** FreeBSD `s_cbrt.c` and AOR's AdvSIMD `cbrt`: the exponent
  divided by 3 on the bit pattern, a short polynomial seed, then Newton or
  Halley steps.
- **CORE-MATH's `cbrt`** (`gitlab.inria.fr/core-math`) could not be read on
  2026-11-13: the host answers automated fetches with a bot-protection page.
  Nothing here rests on it.
- **P11.03g's `cbrtf`** (`libc/math/nofp/fast/cbrtf.c`, D174): w ≈ M^(-1/3)
  by multiplication-only Newton steps w += w(1 − M·w³)/3 in Q31, and
  cbrt M = M·w². Its degree-2 seed is reused here.

### 13.2 The method

- **x = 2^(3q + j)·M** with M = 2^j·f, f in [1, 2) the 53-bit significand
  (a subnormal x normalized), q by a multiply (n·21846/2^16 is ⌊n/3⌋ below
  32,768; Armv6-M has no divide).
- **u ≈ f^(-1/3)**, not M^(-1/3): u lies in (0.79, 1], so u, u² and u³ keep
  31 bits in Q31. P11.03g's w = M^(-1/3) reaches 0.5 and w³ 0.125, which
  keep only 30 and 28 bits: the first build iterated on w and measured T to
  only 2^-27.5 and 1.03 ulp. The seed (2^-8.9) and two Newton steps give u to
  2^-29.7, the limit of Q31 here: rounding the products instead of
  truncating them changed nothing.
- **T = f·u²·cbrt(2^j)** in Q31, within about 2^-28 of cbrt M, clamped below
  2^32 (cbrt M is within 2^-28 of 2 for M near 8).
- **The residual R = M − T³ exactly**: M·2^93 is the significand shifted by
  41 + j (96 bits), T³ is three 32 × 32 products; R is below 2^-23.
- **First correction:** d = R/(3T²) with 1/T² ≈ u²·2^(-2j/3) (one product with
  a per-j constant that includes the 1/3), in Q62. T + d is within about
  2^-55 of cbrt M: Newton's own error d²/T and d times 1/T²'s 2^-28. A second
  build stopped here and measured 0.7319 ulp, 3 % not correctly rounded.
- **Second correction, again from an exact residual:** M − (T + d)³ =
  R − d(3T² + 3Td + d²). It is below 2^-48, so in units of 2^-100 it fits 64
  bits and every term is formed modulo 2^64: R exactly, 3T²d exactly before
  its floor (T² is already exact from the cube, d an integer in Q62), 3Td² to
  2^-12 of itself (|d| in Q56 squared), and d³ (2^-78) left out. Times the
  same 1/(3T²) it leaves about 2^-80 before the rounding, at the cost of six
  32 × 32 products.
- **One rounding**, to nearest by the 54th bit straight into the double's
  bits (`__libc_dp_fixed_double`), the exponent q added exactly. These cores
  have no other direction: their `<fenv.h>` defines `FE_TONEAREST` alone
  (D44), so `conformance`'s directed-rounding cases (Q36) do not run on them,
  and to nearest is the whole of P11.05g2's "the caller's direction".
- **The special cases** are F.10.4.1's: ±0, ±inf and NaN return x + x.
- **A shared helper:** `__libc_dp_fixed_high_signed` in `dp-fixed-fast.h`,
  the high word of a signed 32 × 32 product from the unsigned one (no
  `__aeabi_lmul` on Armv6-M), as nofp's `__libc_fixed_high_signed`, which this
  layer cannot reach.

### 13.3 Measured (P11.05g2)

On the host (the speed files' `umull` product), against long double: every
one of 1.2·10^9 random doubles (a quarter of them in [2^-6, 2^6)) and of
2^14 significands in every binade, subnormal ones included, correctly
rounded (worst 0.5000 ulp); every cube of an integer to 200,000 below 2^53,
and of its half, exact; ±0, ±inf and NaN as F.10.4.1.

Instructions per call (speed → §11's double file on the helpers → fast):

| Core | `cbrt` |
| --- | ---: |
| armv7e-m, armv8-m.main | 3765 → 1919 → 296 |
| armv7-m | 4309 → 2106 → 299 |
| armv6-m, armv6s-m | 14,702 → 6896 → 1230 |

AOR has no scalar double `cbrt`, so P11.06 has no AOR count to gate it on.

**Size:** the member is 852–868 bytes on the Thumb-2 cores and 1022 on
Armv6-M, against 380 for §11's file on armv7e-m-dp, but it pulls no
soft-double helper. No size program calls `cbrt`; every budget holds.
