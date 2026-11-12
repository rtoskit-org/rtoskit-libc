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
