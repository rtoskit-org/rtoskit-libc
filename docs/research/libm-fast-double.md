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

## 14. sin, cos, sincos and tan on the double-FPU cores (P11.05h)

`libc/math/fast/sin.c`, `cos.c`, `sincos.c` and `tan.c`, on x86_64 and
armv7e-m-dp: the soft-double cores keep `dp-fixed/speed`'s files (D136, a
later layer's) until P11.05i. The strict files (the speed flavour's on these
cores) reduce by π/2 to a double-double and finish in double-double: 165
instructions for `sin` and 207 for `tan` on armv7e-m-dp, 308 and 427 on
x86_64.

### 14.1 Sources (ideas only)

- **FreeBSD msun `k_sin.c`, `k_cos.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/k_sin.c>,
  <https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/k_cos.c>,
  read 2026-11-13): kernels on [−π/4, π/4], odd and even polynomials of
  degree 13 and 14 (2^-58 relative), the reduced argument's tail y entering
  to first order, sin(x + y) ≈ sin x + (1 − x²/2)·y and cos(x + y) ≈
  cos x − x·y; cos recovers the rounding error of 1 − x²/2 as a separate
  term. Below 1 ulp, but not by much near π/4, where the polynomial part is
  a tenth of the result.
- **FreeBSD msun `e_rem_pio2.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_rem_pio2.c>):
  π/2 in three 33-bit parts with tails, used up to about 2^20·π/2; the
  exponents of x and of the remainder are compared, and a second and third
  part are applied only when the remainder lost more than 16 or 49 bits.
- **glibc `s_sin.c`** (IBM's table of sin and cos at breakpoints with
  double-double corrections) could not be read: sourceware.org answers
  automated fetches with a bot-protection page. Nothing here rests on it.
- **P11.04a's float path** (§3 of `libm-fast-float-trig.md`, D175): a
  reduction by π/32 and a table kernel whose leading terms are combined
  exactly, so only the last addition rounds; this is that design in double.

### 14.2 The method

- **x = n·π/32 + t**, |t| ≤ π/64, by Cody–Waite with π/32 in three parts:
  p1 and p2 of 29 bits, so n·p1 and n·p2 are exact for |n| < 2^24
  (|x| < 2^20), x − n·p1 is exact (Sterbenz), (x − n·p1) − n·p2 as an exact
  two-difference (t, t_lo), then n·p3 (53 bits) into t_lo. The parts leave
  2^-118 of π/32, so t is good to about 2^-91 absolutely.
- **Near a zero** (n ≠ 0, n ≡ 0 mod 16, and |t| < 2^-27), where sin, cos or
  tan is t itself to first order, 2^-91 is not enough; those arguments
  (about one in 2^24 of the range) and |x| ≥ 2^20 go to the strict reduction
  and double-double cores (`__libc_rem_pio2`, `__libc_sin_core`,
  `__libc_cos_core`), out of line in one hidden function.
- **The table:** sin(jπ/32) for j = 0..16 as exact pairs (hi, lo) (272
  bytes); cos(jπ/32) = sin((16 − j)π/32). With N = n mod 64, the quadrant
  q = N/16 and j = N mod 16, sin(qπ/2 + a) and cos(qπ/2 + a) for
  a = jπ/32 + t are ±sin a or ±cos a.
- **The kernel:** A·cos t + B·sin t, with (A, B) = (S_j, C_j) for sin a and
  (C_j, −S_j) for cos a. A + B·t is formed exactly: B_hi·t as an exact
  product (fma on armv7e-m-dp, Dekker's on SSE2), and a fast two-sum, valid
  because |A| ≥ 0.098 ≥ |B·t| for j ≥ 1 and A = 0 or B = 0 for j = 0. Every
  other term goes into the low part: the product's error, A_lo + B_lo·t,
  A·(cos t − 1), B·(sin t − t), and t_lo·(B − A·t). One rounding: hi + lo.
- **The polynomials** over |t| ≤ π/64, fitted here: (sin t − t)/(t·t²) with
  four terms (2^-65.3 relative to t) and (cos t − 1)/t² with four
  (2^-71.2). Three terms measured 2^-58.3 and 2^-55.1 in the fit; the cos
  one would be 2^-54 of sin(π/64) at j = 1, where A·(cos t − 1) enters a
  result half of A.
- **tan:** the period is π, so N = n mod 32; for q even sin a / cos a, for q
  odd −cos a / sin a, from the two kernels as pairs, then a quotient with its
  remainder (the product's error exact): the divisor is at least
  sin(π/64), so the quotient keeps the pairs' accuracy.
- **sincos:** one reduction, both kernels.
- **The ends:** NaN returns x + x, ±inf raises invalid, and |x| < 2^-27 is
  the strict files' tiny path (x, or 1 for cos, nudged so the direction and
  inexact come out right).
- **The slow path returns the result** (`__libc_fast_trig_slow(x, kind)`),
  so the callers end in a tail call. A first version passed the cores'
  pairs back through pointers: the frame and the saved d8–d11 cost `sin`
  about 6 instructions, and a table entry copied as a struct about 4 more.
- **Found along the way:**
  - The reduction's low part reached 2^-40 unnormalized (n·p3 and the
    two-difference's error), and the kernel takes t_lo to first order only:
    t_lo·t²/2 was 2^-49, 257 ulp near 2^20. t + t_lo is now normalized by one
    more fast two-sum (a regression case).
  - The sign test compared a bool with `signbit`, whose nonzero value is the
    sign bit on Arm (1 on the host), so a negative x with the half turn set
    would have come out negated on armv7e-m-dp; the host sweeps could not see
    it. It compares `signbit(x) != 0` now (`sin(-4.0)` is a case).
  - `sincos` first sent both results down the slow path when either was
    next to its zero; each now decides as `sin` or `cos` does, so the two
    stay equal to them bit for bit.

### 14.3 Measured (P11.05h)

On the host, against long double (whose `sinl`, `cosl` and `tanl` reduce
exactly), on both the SSE2 and the fma paths, which agree bit for bit:

| Samples | `sin` | `cos` | `tan` |
| --- | ---: | ---: | ---: |
| 10^9 log-uniform in [2^-30, 2^20] | 0.5063 | 0.5059 | 0.5063 |
| 2·10^8 uniform in [0, 10] | 0.5054 | 0.5054 | 0.5063 |
| 2·10^8 within 1000 ulp of a multiple of π/32 below 2^20 | 0.5000 | 0.5000 | 0.5005 |
| 2·10^7 in [2^20, 2^1020] (the strict reduction) | 0.5928 | 0.5898 | 0.6055 |

About 0.01–0.02 % of results are not correctly rounded below 2^20. `sincos`
equals `sin` and `cos` on every sample. Beyond 2^20 the results are the
strict files', bit for bit; their 0.59–0.61 ulp there is the strict
reduction's (checked against mpmath at 4000 bits: 0.593 ulp for
`sin(0x1.bd0a14269acdfp+707)`), above the README's 0.53, and outside this
step: suggested as a separate fix.

Instructions per call (speed → fast):

| Function | armv7e-m-dp | x86_64 |
| --- | ---: | ---: |
| `sin` | 165 → 96 | 308 → 145 |
| `cos` | 167 → 93 | 313 → 138 |
| `sincos` | 231 → 171 | 446 → 273 |
| `tan` | 207 → 133 | 427 → 253 |

AOR has no scalar double `sin`, `cos` or `tan`, so P11.06 has no AOR count
to gate them on.

**Size:** on armv7e-m-dp `sin` is 676 bytes, `cos` 592, `sincos` 860 and
`tan` 772, with the 272-byte table and the 242-byte slow path shared.
`math-mix`, which calls `sin`, `cos` and `tan`, grows from 17,548 to 19,286
bytes: each file inlines the reduction and the kernel, where the strict files
shared one double-double core. That is +58 % over speed, above D191's +20 %
line; P11.06 decides. A shared out-of-line kernel would save about 1 KiB
there for a call per evaluation. `math-float` shrinks from 5950 to 5710:
P11.04a's `sinf`, `cosf`, `sincosf` and `tanf` now reach the strict
reduction from 2^20 through the slow path (`__libc_fast_trig_large`), with
the same results, instead of pulling the double files. Both fast figures are
re-locked by D187's rule.

## 15. sin, cos, sincos and tan on the soft-double cores (P11.05i)

`libc/math/dp-fixed/fast/sin.c`, `cos.c`, `sincos.c` and `tan.c`, shadowing
`dp-fixed/speed`'s files (D133, D136, D142) on the soft-double cores.

### 15.1 Sources

The kernel is this library's own (`docs/research/sincos.md`, D136): x·2/π
from a 2/π table in fixed point, sin and cos of the reduced argument as
64-bit significands with a measured error below 18 units of their last bit
(`tools/scripts/trig-fixed-check`), and `tan` as sin/cos by a 64-bit
reciprocal, below 56 units (D142). The speed files keep a result only when
its 11 bits below the double's significand are more than that bound from a
halfway point, and otherwise run the double-double path (`__libc_trig_dd`,
`__libc_tan_dd`): about 1.8 % of arguments (Q44), each a whole double-double
evaluation on soft helpers.

### 15.2 The method

- **No rounding test:** the kernel's 64-bit significand is rounded to
  nearest by its 54th bit (`__libc_dp_fixed_double`), straight into the
  double's bits. 18 units of 2^-63 relative are at most 18/2048 of an ulp, so
  the result is within 0.5088 ulp; `tan`'s 56 units, within 0.5274. The
  largest errors measured for D136 and D142 are 7.08 units (`sin`, `cos`)
  and 12.5 (`tan`): about 0.5035 and 0.5061 ulp. The fallback this removes
  ran on 1.8 % of `sin` and `cos` arguments and 5.6 % of `tan`'s.
- **The strict path stays** where the kernel declines: |x| ≥ 2^20, and the
  reductions that cancel more than 64 bits (the arguments next to a multiple
  of π/2). The entry tests (NaN, infinity, zero, |x| < 2^-27) are the speed
  files', on the bits.
- **sincos** makes one kernel call for both results, rounded the same way, so
  each equals the fast `sin` and `cos`, bit for bit.

### 15.3 Measured (P11.05i)

On the host (the speed files' `umull` product), against long double, with
§14's sweep: 10^9 log-uniform in [2^-30, 2^20], 2·10^8 in [0, 10] and 2·10^8
within 1000 ulp of a multiple of π/32. At most 0.5020 ulp for `sin` and
`cos` and 0.5049 for `tan`; 0.03–0.07 % of `sin` and `cos` results and
0.06–0.09 % of `tan`'s are not correctly rounded (the speed files: none,
by their test). `sincos` equals `sin` and `cos` on every sample. Beyond 2^20
the results are the strict files' (§14.3).

Instructions per call (speed → fast; the speed files were the fast
flavour's until now):

| Core | `sin` | `cos` | `sincos` | `tan` |
| --- | ---: | ---: | ---: | ---: |
| armv7e-m | 765 → 751 | 1067 → 755 | 1390 → 1065 | 1005 → 987 |
| armv8-m.main | 754 → 740 | 1057 → 743 | 1379 → 1054 | 1000 → 982 |
| armv7-m | 770 → 755 | 1074 → 760 | 1400 → 1072 | 1012 → 996 |
| armv6-m, armv6s-m | 2833 → 2775 | 3663 → 2779 | 5177 → 4248 | 4970 → 4915 |

The bench's 32 arguments are where the counts come from: `cos` and `sincos`
gain most because more of their arguments fell back. The kernel itself is
about 740 instructions on the Thumb-2 cores; a table-driven fixed-point
kernel (§7's style) could cost less, and is noted for P11.06.

**Size:** `math-mix`, which calls `sin`, `cos` and `tan`, changes by +6 to
+44 bytes on these cores; every budget holds.

## 16. asin, acos, atan and atan2 on the double-FPU cores (P11.05j)

`libc/math/fast/asin.c`, `acos.c`, `atan.c` and `atan2.c`, on x86_64 and
armv7e-m-dp (the soft-double cores keep the strict files until P11.05k). The
strict files reach everything through the double-double `__libc_atan2_core`
(asin and acos over a double-double square root): 195–360 instructions on
armv7e-m-dp, 378–558 on x86_64.

### 16.1 Sources (ideas only)

- **FreeBSD msun `s_atan.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_atan.c>,
  read 2026-11-13): five intervals with breakpoints near 7/16, 11/16, 19/16
  and 39/16, each subtracting a reference angle by a Möbius map, then an
  11-term odd polynomial with the angle added back as a pair; no stated
  bound.
- **FreeBSD msun `e_asin.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_asin.c>):
  split at 1/2, x + x·R(x²) with a rational R (2^-58.75) below, and
  π/2 − 2 asin(√((1 − |x|)/2)) above, with √ split into a high part with its
  low word cleared plus a correction (t − f²)/(s + f), so its rounding error is
  kept.
- **AOR's AdvSIMD `asin`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/asin.c>):
  the same split, one polynomial P of order 11 in z for both halves
  (2^-57 for (asin √z − √z)/(z√z)), 1.01 ulp observed below 1/2 and 2.69
  above (its harness tolerance says 2.20).
- **P11.04b's double path** (D176): nine breakpoints j/8 and the Taylor
  series, then one rounding to float; that has 29 spare bits, this none.

### 16.2 The method

- **The atan kernel** (`__libc_fast_atan_kernel`), for a + a_lo in [0, 1]:
  - c = j/16 with j = ⌊16a + 1/2⌋, and atan(j/16) from a 17-entry table of
    exact pairs (272 bytes);
  - t = (a − c)/(1 + ac): a − c is exact (Sterbenz), 1 + ac is formed as a
    pair (the product exact by fma or Dekker, its rounding exact by a fast
    two-sum), and the quotient's remainder is exact, so t + t_lo carries the
    quotient to about 2^-106, a_lo included; |t| ≤ 1/32;
  - atan t = t + t·z·P(z), z = t², P of five terms fitted here (2^-65.6
    relative to t);
  - atan(c) + t by a fast two-sum (atan(1/16) > 1/32), every other term in
    the low part, one rounding by the caller.
- **atan:** |x| ≤ 1 directly; above, 1/|x| with its exact remainder and
  π/2 − atan(1/|x|) with π/2 as a pair (the result is above π/4, so the fast
  two-sum is exact). Below 2^-27 the strict tiny path; from 2^60, ±π/2.
- **atan2:** for normal coordinates from 2^-959 to 2^960 whose exponents
  differ by at most 60, the smaller magnitude over the larger with its exact
  remainder (no underflow there), the kernel, then the quadrant's offset 0,
  π/2 or π as a pair, and y's sign. Everything else (zeros, infinities, NaN,
  subnormals, the far exponents where the result is a tiny quotient or ±π/2
  or ±π) goes to the strict file's code, kept as a hidden slow path.
- **asin and acos up to 1/2:** asin x = x + x·z·P(z), z = x², with P of
  fourteen terms fitted here over z ≤ 1/4 (2^-58.5 relative to x; twelve
  terms leave 2^-54); acos x = π/2 − asin x, π/2 a pair, by a fast two-sum
  (|x| ≤ 1/2).
- **Above 1/2:** z = (1 − |x|)/2 exactly (Sterbenz), s = √z correctly rounded
  with its exact remainder s_lo, asin s by the same polynomial; asin |x| =
  π/2 − 2 asin s (at least 0.52, so the fast two-sum loses nothing), acos x =
  2 asin s for x > 0 (relative however close x is to 1) and π − 2 asin s for
  x < 0. ±1 return π/2, 0 and π directly (s = 0 would divide by zero).
- **Found along the way:**
  - The first build rounded the tail s·z·P(z) in plain double: next to 1/2
    it is 8 % of the result, and its four or five roundings cost asin 0.66
    ulp (1.4 % not correctly rounded). The tail is now w = z/6 + z²·Q(z),
    with z/6 by an exact product with 1/6 as a pair and s·w_hi by another,
    so only z²·Q (below 1 %) rounds in plain double: 0.55.
  - The square root's remainder S − s entered as itself, but the polynomial
    is evaluated at the exact z = S² and multiplied by s, so asin S = s +
    (S − s)(1 + w) + s·w: the factor 1 + w (1.045 at z = 1/4) was missing,
    2^-57.6 of the result next to 1/2. With it, 0.5288. Taking the
    derivative 1/√(1 − z) instead, a first guess, measured 0.566.

### 16.3 Measured (P11.05j)

On the host, against long double, 1.4·10^9 samples per function on the
SSE2 and the fma paths, which agree bit for bit (|x| uniform in [0, 1] and
log-uniform within 2^-30 of 1 for asin and acos; |x| log-uniform over
2^±40 and 2^±70 for atan and the atan2 coordinates; [−10, 10]²):

| Function | worst | not correctly rounded |
| --- | ---: | ---: |
| `asin` | 0.5288 | 0.01–0.14 % |
| `acos` | 0.5225 | 0.005–0.06 % |
| `atan` | 0.5010 | below 0.0001 % |
| `atan2` | 0.5010 | below 0.0001 % |

Instructions per call (speed → fast):

| Function | armv7e-m-dp | x86_64 | armv7e-m (helpers) | armv6-m (helpers) |
| --- | ---: | ---: | ---: | ---: |
| `asin` | 220 → 135 | 481 → 204 | 13,029 → 7141 | 34,625 → 20,137 |
| `acos` | 228 → 127 | 504 → 196 | 13,628 → 7192 | 35,718 → 20,151 |
| `atan` | 195 → 108 | 378 → 203 | 7741 → 6081 | 21,175 → 17,848 |
| `atan2` | 360 → 141 | 558 → 212 | 8314 → 6168 | 22,603 → 18,388 |

The soft-double cores run these double files on their helpers until
P11.05k; they are already about half the strict files' cost there. AOR has
no scalar double `asin`, `acos`, `atan` or `atan2`.

**Size:** on armv7e-m-dp `asin` is 786 bytes, `acos` 762, `atan` 620 and
`atan2` 566, with the 272-byte table and the 736-byte strict slow path.
`math-mix` (which calls `atan2`) grows from 19,286 to 20,128 bytes there
(+65 % over speed, re-locked by D187's rule; D191 at P11.06), and by 1476 to
1552 bytes on the soft-double cores (+11 % to +15 % over speed, inside the
line).

## 17. atan and atan2 on the soft-double cores (P11.05k1)

`libc/math/dp-fixed/fast/atan.c` and `atan2.c`, shadowing §16's double files
on the soft-double cores, where they run on the soft helpers (6081 and 6168
instructions on armv7e-m, 17,848 and 18,388 on armv6-m). P11.05k was split:
`asin` and `acos` (k2) need a 64-bit square root, these a 64-bit quotient.

### 17.1 Sources (ideas only)

- **§16's double design**: atan(j/16) from a table and t = (a − c)/(1 + ac),
  |t| ≤ 1/32, and a short series; here in integers.
- **P11.04f1's `__libc_fixed_atan`** (D182): one out-of-line kernel for
  atan(a/b), a ≤ b, shared by `atanf`, `atan2f` and the π forms, with the
  quadrant's offset added in fixed point.
- **D142's 64-bit reciprocal** (`__libc_tan_fixed.c`): a 17-bit seed by one
  32/16-bit division, then Newton's y(2 − dy) by high products.

### 17.2 The method

- **One kernel, atan(y/x) for y ≤ x** (`__libc_dp_fixed_atan`, out of line),
  on the significands and exponents, with the quadrant (0, π/2 − K,
  π/2 + K, π − K) and the sign applied in fixed point and one rounding:
  - **j = round(16 y/x)** from one 32-bit division of the top bits (UDIV on
    the Thumb-2 cores, `__aeabi_uidiv` on Armv6-M); off by one at worst
    next to a breakpoint, which widens |t| a little past 1/32.
  - **j ≥ 1** (the exponents at most 6 apart): x's significand shifted by 10
    and y's by 10 − d, so c·x = j·x/16 and c·y are exact; N = y − c·x
    exactly, D = x + c·y exactly, t = N/D by a 64-bit reciprocal of D (two
    Newton steps) and one high product, |t| in Q64; atan t = t − t·V with
    V = u·G(u), u = t², G of six terms by unsigned Horner's rule in 2^10·u
    (every nested term positive); K = atan(j/16) ± atan|t| in Q64 from a
    17-entry table (136 bytes).
  - **j = 0**: t = y/x relative (the significands' quotient and the
    exponents' difference), the same series, so tiny ratios keep their
    relative accuracy (down to 2^-61, the fast path's limit).
  - The offsets π/2 and π in Q62; K enters them shifted.
- **atan:** |x| ≤ 1 is K(x, 1), and |x| > 1 is π/2 − K(1, |x|): one quotient
  either way. Below 2^-27 the strict tiny path, from 2^60 ±π/2.
- **atan2:** normal, finite, nonzero coordinates whose exponents differ by at
  most 60 go to the kernel with the smaller magnitude first; the rest to
  §16's slow path (the strict code), as on the double-FPU cores.
- **Found along the way:**
  - The first scaling of the series, w = 2^10·t², overflows Q64 at
    t = 2^-5, which j's 32-bit estimate can reach next to 1/32; w = 2^8·t²
    leaves room to t = 2^-4.
  - The kernel calls `__libc_trig_high64` out of line: `dp-fixed/fast` is
    built at `-Os`, and so are §7–§13's kernels. `-O2` for the module
    measured `atan` 523 → 422 instructions on armv7e-m and `pow` 923 →
    738, but `math-mix` +2.6 KB on armv7e-m and +16 KB on armv6-m, far past
    the budgets; it stays at `-Os` (Q53, for P11.06).

### 17.3 Measured (P11.05k1)

On the host (the speed files' `umull` product), against long double, with
§16.3's sweep (1.4·10^9 samples per function) and 2·10^8 pairs at the
breakpoints' boundaries (ratios next to (2j + 1)/32, exponents 0 to 8
apart): at most 0.5059 ulp for `atan` and 0.5093 for `atan2`; about 0.02–
0.03 % not correctly rounded. The worst cases sit just above a ratio of 1/32
on the absolute path, where 2^-64 is 1/128 of the result's ulp; a third
Newton step for the reciprocal changed 0.5093 only to 0.5083.

Instructions per call (speed → §16's double files on the helpers → fast):

| Core | `atan` | `atan2` |
| --- | ---: | ---: |
| armv7e-m, armv8-m.main | 7741 → 6081 → 523 | 8314 → 6168 → 524 |
| armv7-m | 8070 → 6435 → 529 | 8667 → 6543 → 528 |
| armv6-m, armv6s-m | 21,175 → 17,848 → 2542 | 22,603 → 18,388 → 2565 |

**Size:** the kernel is 960 bytes on armv7e-m (974 on armv6-m), `atan` 292
and `atan2` 154, with the 136-byte table. `math-mix` (which calls `atan2`)
shrinks by 200 to 304 bytes against P11.05j: +10.5 % to +13.5 % over speed.

## 18. asin and acos on the soft-double cores (P11.05k2)

`libc/math/dp-fixed/fast/asin.c` and `acos.c`, shadowing §16's double files
on the soft-double cores (7141 and 7192 instructions on armv7e-m's helpers,
20,137 and 20,151 on armv6-m).

### 18.1 Sources (ideas only)

- **§16's double design**: x + x·w up to 1/2 and the half-angle form
  π/2 − 2 asin √((1 − |x|)/2) above, with one polynomial for both halves
  (FreeBSD's `e_asin.c` and AOR's `asin`, read for §16).
- **P11.04f2's fixed-point square root** (D183): 1/√t from a 24-entry table
  of 16-bit seeds and Newton steps, then s = t·r corrected by its exact
  remainder; here widened to 64 bits.

### 18.2 The method

- **One out-of-line kernel** (`__libc_dp_fixed_asin`), for asin and acos:
  - **|x| ≤ 1/2:** z = x² in Q64 (from x's significand, absolute, which is
    all w needs), W = z·P(z), asin |x| = |x|(1 + W) relative. acos x =
    π/2 ∓ asin |x| in Q62.
  - **1/2 < |x| < 1:** z = (1 − |x|)/2 exactly in integers, normalized by an
    even shift, s = √z by the 64-bit square root, asin s = s(1 + W(z)).
    asin |x| = π/2 − 2 asin s, acos x = π − 2 asin s (x < 0) in Q62, and
    acos x = 2 asin s (x > 0) relative, however close x is to 1.
- **P(z)** = (asin √z − √z)/(z√z) for z ≤ 1/4, fifteen terms fitted here
  with Q64 coefficients (W within 2^-65.4; fourteen leave 2^-61.9). The fit
  has one negative coefficient, so Horner's rule runs in signed 64 bits
  (every partial sum is below 2^62).
- **The 64-bit square root** of t in [2^62, 2^64) (τ = t·2^-64 in [1/4, 1)):
  r ≈ 1/√τ in Q30 from the 24 seeds (2^-5.09) and three Newton steps
  r(3 − τr²)/2; s = τ·r in Q32; then twice s += (t − s²)·r/2, first in
  units of 2^-32 (s then within a unit), then in units of 2^-64: about 6
  units of 2^-64 (the second correction times r's 2^-29.5), 2^-60.5
  relatively.
- **The ends** in the callers: NaN, |x| > 1 (invalid), ±1 exactly, asin below
  2^-27 (the strict tiny path), acos below 2^-60 (π/2).
- **Found along the way:** the second remainder correction shifted its
  96-bit product right by shift − 32 = −1, so it added nothing and the root
  stayed at 32 bits: 2·10^6 ulp above 1/2. The product now shifts either way
  (the square root within 2 units of 2^-64 after it).

### 18.3 Measured (P11.05k2)

On the host (the speed files' `umull` product), against long double, with
§16.3's sweep (1.4·10^9 samples per function): `asin` at most 0.5049 ulp
and `acos` 0.5039; 0.04–0.07 % and 0.03–0.04 % not correctly rounded.

Instructions per call (speed → §16's double files on the helpers → fast):

| Core | `asin` | `acos` |
| --- | ---: | ---: |
| armv7e-m, armv8-m.main | 13,029 → 7141 → 661 | 13,628 → 7192 → 668 |
| armv7-m | 13,481 → 7295 → 664 | 14,076 → 7338 → 669 |
| armv6-m, armv6s-m | 34,625 → 20,137 → 3293 | 35,718 → 20,151 → 3293 |

AOR has no scalar double `asin` or `acos`.

**Size:** the kernel is 988 bytes on armv7e-m, `asin` 248 and `acos` 152,
with the 48-byte seed table. No size program calls them; every budget holds.

## 19. The hyperbolic functions on the double-FPU cores (P11.05l)

`libc/math/fast/sinh.c`, `cosh.c`, `tanh.c`, `asinh.c`, `acosh.c` and
`atanh.c`, on x86_64 and armv7e-m-dp (the soft-double cores run them on
their helpers until P11.05m). The strict files finish in double-double over
`__libc_exp_dd`, `__libc_log1p_dd` and a double-double square root: 192–287
instructions on armv7e-m-dp, 284–549 on x86_64.

### 19.1 Sources (ideas only)

Read 2026-11-14:

- **FreeBSD msun `e_sinh.c`, `e_cosh.c`, `s_tanh.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/e_sinh.c>,
  `.../e_cosh.c`, `.../s_tanh.c`): `expm1` of |x| up to 22 (rearranged
  below 1 so that nothing cancels), e^|x|/2 from 22 to ln(DBL_MAX), a scaled
  exponential up to the overflow threshold; `cosh` takes 1 + a term from
  `expm1` below ln 2/2; `tanh` is −t/(t + 2) with t = expm1(−2|x|) below 1,
  1 − 2/(expm1(2|x|) + 2) up to 22, ±1 beyond. No stated bounds.
- **FreeBSD msun `s_asinh.c`, `e_acosh.c`, `e_atanh.c`**
  (`.../s_asinh.c`, `.../e_acosh.c`, `.../e_atanh.c`): `asinh` as
  log1p(|x| + x²/(1 + √(1 + x²))) up to 2, then log 2|x| + 1/(√(x² + 1) + |x|),
  and log|x| + ln 2 from 2^28; `acosh` as log1p(t + √(2t + t²)), t = x − 1,
  up to 2, log 2x − 1/(x + √(x² − 1)) up to 2^28; `atanh` as
  log1p(2x + 2x²/(1 − x))/2 below 1/2, log1p(2x/(1 − x))/2 above. Each
  rearranges the argument so that the logarithm near 0 goes through `log1p`.
- **AOR's AdvSIMD `sinh` and `asinh`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/sinh.c>,
  `.../asinh.c`): `sinh` as (t + t/(t + 1))/2 with t = expm1|x|, a scaled
  path from 709; `asinh` as a polynomial of order 17 below 1 and a table log
  of |x| + √(x² + 1) above (2.52 and 2.87 ulp in their comments).

### 19.2 The method

- **sinh, cosh, tanh: one kernel over P11.05b's table**
  (`__libc_fast_sinhcosh`, 0 ≤ a ≤ 22). P11.05b's reduction gives
  a = k·ln 2/64 + u, and e^±a = S±(1 + t± + p(±u)), with S± = 2^(±k/64) and
  their tails t± from the same table entries. p(u) = E + O with E and O the
  even and odd parts of exp's polynomial: O is within 2^-60.2 of sinh u − u
  relatively, E within 2^-57.4 of cosh u − 1 (measured here). So
  - S+e^u − S−e^-u = (S+ − S−) + S+(t+ + E) − S−(t− + E) + (S+ + S−)·O;
  - the sum likewise, with the signs exchanged.

  S+ − S− and S+ + S− are fast two-sums (S+ ≥ S−). E's error stays relative
  to the difference and O's to the sum, so nothing cancels badly. In the
  difference, (S+ + S−)·u is up to 2^-6.9 of it where sinh and tanh use the
  kernel; it is an exact product added by a fast two-sum. The halving of
  sinh and cosh goes into the scales' exponents.
- **sinh:** below 1/2 the odd series x + x³/6 + x⁵·Q(x²), with 1/6 as a pair
  and Q of five terms fitted here (2^-63.4 relative); x² and x³ are exact
  pairs and x³/6 an exact product added by a fast two-sum
  (`__libc_fast_odd_series`). From 1/2 to 22 the kernel; beyond, e^|x|/2
  with exp's polynomial and the table tail in its last step, the scale
  halved and the sum doubled above 2^1023; overflow above
  0x1.633ce8fb9f87dp+9, the last double whose result is finite.
- **cosh:** the kernel from 2^-27 to 22 with no series (at k = 0 it is
  1 + E), then sinh's e^|x|/2.
- **tanh:** below 1/4 the odd series x − x³/3 + x⁵·Q(x²), Q of seven terms
  (2^-61.0); up to 22 the kernel's two pairs, normalized, and their quotient
  with its exact remainder; ±(1 − tiny) from 22.
- **asinh, acosh, atanh: the logarithm of a pair** (`__libc_fast_log_pair`):
  P11.05d's evaluation of log hi, with lo/hi added to its tail, and k + 1
  for log 2·hi. The argument is formed accurately enough (to about 2^-104)
  that no `log1p` rearrangement is needed: near a result of 0, hi is near 1,
  where the table's entry has invc = 1 and the pair is r + r²·P(r) + lo/hi,
  relative however small it is.
  - asinh |x| = log(|x| + √(x² + 1)): x² exact, 1 + x² by a two-sum, the
    square root with its remainder (`__libc_fast_sqrt_pair`), |x| + s by a
    fast two-sum; log|x| + ln 2 from 2^28 (the term 1/(4x²) is below 2^-58).
  - acosh x = log(x + √(x² − 1)): x² − 1 exact up to 2 (Sterbenz) and by a
    fast two-sum above, normalized; log x + ln 2 from 2^28.
  - atanh |x| = log((1 + |x|)/(1 − |x|))/2: both terms exact pairs, their
    quotient with its exact remainder.
- **Found along the way:**
  - The series first rounded the x³ term with the rest: 4 % of sinh at 1/2
    and 2 % of tanh at 1/4, 0.61 and 0.56 ulp next to them. With the exact
    leading term, 0.54 and 0.556.
  - The kernel first rounded (S+ + S−)·O whole: up to 2^-6.9 of the
    difference just above the series' ends, 0.537 and 0.556 ulp there. With
    its leading product exact, 0.537 and 0.520.
  - e^|x|/2 first added the table's tail and the polynomial apart, through
    `__libc_fast_exp_scale`: 0.5503 ulp near 514. With the tail in the
    polynomial's last step, as `exp.c` does, 0.5493.
  - `acosh` tested 1 < x < 2^28 by relational comparisons, which raise
    invalid for a quiet NaN; `libm_vectors` caught it in `acosh`, and in
    `acoshl` and `acoshf`, which reach the double `acosh` for a NaN. The
    comparisons are now quiet.

### 19.3 Measured (P11.05l)

On the host, against long double, 1.4·10^9 samples per function on the
SSE2 and the fma paths, which agree bit for bit (|x| uniform in [0, 1],
log-uniform over 2^-27 to 2^5 and uniform in [0, 22] for sinh, cosh and
tanh, plus [22, 710.5] for sinh and cosh; log-uniform to 2^40 and 2^1023 for
asinh and acosh, 1 + 2^-k·u for acosh, 1 − 2^-k·u for atanh), and 4·10^8
more over [22, 710.5] after e^|x|/2's fix:

| Function | worst | not correctly rounded |
| --- | ---: | ---: |
| `sinh` | 0.5493 (above 22; 0.5371 below) | 0.19–0.59 % |
| `cosh` | 0.5493 (above 22; 0.5317 below) | 0.42–0.59 % |
| `tanh` | 0.5215 | 0.15–0.20 % |
| `asinh` | 0.5151 | 0.006 % |
| `acosh` | 0.5166 | 0.006 % |
| `atanh` | 0.5161 | 0.008 % |

The worst `sinh` and `cosh` results are e^|x|/2's, exp's own error (P11.05b
measured 0.547 for `exp`).

Instructions per call (speed → fast):

| Function | armv7e-m-dp | x86_64 | armv7e-m (helpers) | armv7-m (helpers) | armv6-m (helpers) |
| --- | ---: | ---: | ---: | ---: | ---: |
| `sinh` | 200 → 107 | 305 → 134 | 6367 → 3277 | 6585 → 3328 | 17,842 → 9200 |
| `cosh` | 192 → 79 | 284 → 82 | 5954 → 1803 | 6166 → 1838 | 16,696 → 5297 |
| `tanh` | 202 → 137 | 331 → 193 | 7193 → 5831 | 7444 → 6115 | 20,152 → 16,684 |
| `asinh` | 287 → 107 | 549 → 183 | 15,215 → 6943 | 15,719 → 7166 | 39,784 → 18,468 |
| `acosh` | 246 → 101 | 479 → 177 | 13,378 → 7051 | 13,773 → 7265 | 34,248 → 18,611 |
| `atanh` | 217 → 90 | 383 → 138 | 9286 → 4056 | 9757 → 4390 | 25,834 → 12,620 |

armv8-m.main equals armv7e-m and armv6s-m armv6-m. The soft-double cores run
these double files on their helpers until P11.05m. AOR has no scalar double
hyperbolic function.

**Size:** on armv7e-m-dp `sinh` is 1094 bytes, `cosh` 720, `tanh` 1002,
`asinh` 686, `acosh` 560 and `atanh` 584, over the 1 KiB exp table and the
3 KiB log table that `exp`, `log` and `pow` already use. The strict files
are smaller (392–582 bytes) but pull the double-double exp and log cores. No
size program calls a hyperbolic function; every budget holds.

## 20. sinh, cosh and tanh on the soft-double cores (P11.05m1)

`libc/math/dp-fixed/fast/sinh.c`, `cosh.c` and `tanh.c`, shadowing §19's
double files on the five soft-double cores. P11.05m is split as P11.04g was:
m1 rests on the exponential's kernel, m2 (`asinh`, `acosh`, `atanh`) on the
logarithm's and the square root. Sources: §19.1's (FreeBSD's expm1 forms,
AOR's AdvSIMD `sinh`), and P11.04g1's float design, which takes 2^(z−1) ∓
2^(−z−1) from the exp2 core twice; no new source was read.

### 20.1 The method

- **e^a and e^-a on one scale** (`__libc_dp_fixed_exp_pair`): t = a·log2 e
  = n + f·2^-64 by §7's 128-bit product, and −t = n' + g·2^-64 by negation.
  Y1 = 2^f and Y2 = 2^g in Q63 come from §7's kernel fraction
  (`__libc_dp_fixed_exp2_fraction`), each within a few units of 2^-63. Then
  e^a = 2^(n−63)·Y1 and e^-a = 2^(n−63)·(Y2 >> (2n + [f ≠ 0])): one integer
  scale, so the sum and the difference are exact in integers.
- **sinh:** from 1/4, (Y1 − Y2')·2^(n−1), normalized and rounded once. The
  result is at least 0.25 of e^a's scale, so the kernels' errors stay below
  2^-60 of it. Below 1/4, x·(1 + V) with V = x²/6 + x⁴/120 + … in Q64 from
  w = 16x², six terms by unsigned Horner's rule (the seventh is below
  2^-68), relative however small x is.
- **cosh:** (Y1 + Y2')·2^(n−1) from 2^-27 up, the sum's carry into the
  exponent: nothing cancels, so no series.
- **Both** run to the overflow threshold 0x1.633ce8fb9f87dp+9 on one path:
  beyond about 22, Y2's shift reaches 64 and only e^a remains.
- **tanh:** one exponential, of 2|x|: e^(2|x|) = 2^(n−63)·Y and c = 2^(63−n),
  so tanh = (Y − c)/(Y + c), both exact in integers. The quotient takes
  P11.05k1's 64-bit reciprocal, now shared in `dp-fixed-fast.h`
  (`__libc_dp_fixed_reciprocal`; the atan kernel is unchanged). It is good
  to a few units of 2^-62 of the result, which is at least 0.12. Below 1/8,
  x·(1 − V) with V = x²·G(x²), G = 1/3 − (2/15)u + … in Q64 from w = 64x²:
  nine terms, alternating, by unsigned Horner's rule subtracting (each
  nested value stays positive). From 22, ±(1 − tiny).

### 20.2 Measured (P11.05m1)

On the host (the speed files' `umull` product), against long double, with
§19.3's sweep: 1.2·10^9 samples over the whole range, 2·10^8 below 0.6 and
4·10^8 over [22, 710.5].

| Function | worst | not correctly rounded |
| --- | ---: | ---: |
| `sinh` | 0.5044 | 0.03–0.04 % |
| `cosh` | 0.5029 | 0.02–0.04 % |
| `tanh` | 0.5083 | 0.06–0.10 % |

Instructions per call (speed → §19's double files on the helpers → fast):

| Core | `sinh` | `cosh` | `tanh` |
| --- | ---: | ---: | ---: |
| armv7e-m, armv8-m.main | 6367 → 3277 → 508 | 5954 → 1803 → 494 | 7193 → 5831 → 500 |
| armv7-m | 6585 → 3328 → 511 | 6166 → 1838 → 496 | 7444 → 6115 → 502 |
| armv6-m, armv6s-m | 17,842 → 9200 → 2490 | 16,696 → 5297 → 2445 | 20,152 → 16,684 → 2534 |

AOR has no scalar double hyperbolic function. `atan` and `atan2` are
unchanged by the reciprocal's move into the header.

**Size:** `sinh` is 1214 bytes on armv7e-m, `cosh` 886 and `tanh` 1486 (each
inlines exp's kernel fraction; 1042–1474 on armv6-m), over §7's 512-byte
table. No size program calls them; every budget holds.

## 21. asinh, acosh and atanh on the soft-double cores (P11.05m2)

`libc/math/dp-fixed/fast/asinh.c`, `acosh.c` and `atanh.c`, shadowing §19's
double files on the five soft-double cores. Sources: §19.1's (FreeBSD's
log1p rearrangements) and P11.04g2's float design (the argument exact in
integers, a series near 0); no new source was read.

### 21.1 The method

- **The shape:** each function forms log1p's argument w, or log's argument v
  where the result is at least 0.88, as a 64-bit fixed-point value to about
  2^-62 relatively. That is §19's idea, an accurate argument instead of a
  series, carried out in integers. Two new inlines in `dp-fixed-fast.h`
  take the value as a significand and an exponent:
  - `__libc_dp_fixed_log1p_scaled`: below 2^-5, §10's relative series
    (`__libc_dp_fixed_log_near`); otherwise 1 + w in Q63 and
    `__libc_dp_fixed_log_scaled`;
  - `__libc_dp_fixed_log_scaled`: z into [45/64, 90/64), then k·ln 2 + ln z
    from §10's table path in 128 bits, rounded once.
- **The square root:** P11.05k2's 64-bit root moves out of the asin kernel
  into a hidden function of its own (`__libc_dp_fixed_root.c`), which asin,
  acos, asinh and acosh share; `asin` and `acos` measure the same
  instruction counts within one. `__libc_dp_fixed_sqrt_scaled` evens the
  exponent and calls it.
- **asinh |x|:**
  - below 1, w = |x| + x²/(1 + √(1 + x²)): 1 + x² in Q62, its root in Q63,
    x² over (1 + r) by P11.05k1's reciprocal;
  - from 1, v = |x| + √(x² + 1), with x² + 1 in Q61 at 2^2e;
  - from 2^28, log 2|x| (the term 1/(4x²) is below 2^-58).
- **acosh x:**
  - below 2, w = t + √(t(2 + t)) with t = x − 1 exact and the product to
    2^-63, relative however close x is to 1 (w is at least 2^-25.5);
  - from 2, v = x + √(x² − 1);
  - from 2^28, log 2x.
- **atanh |x|:** log1p(s)/2 with s = 2|x|/(1 − |x|). 1 − |x| is taken in Q64,
  exact from 2^-12, and the quotient comes from the reciprocal. The halving
  is exact in the result's exponent.
- **Found along the way:**
  - The table path took 1 + w by a truncating shift. w's 64 bits, unlike
    log1p's 53, lose up to 5 there, 2 units of 2^-64 in the logarithm:
    0.533 ulp where the result is just above 2^-5, against 0.509 with
    53-bit arguments. Rounded instead, 0.523.
  - The 64-bit reciprocal measures 17 units of 2^-64 (2^-59 relatively) and
    the root 8. Neither moves the results by more than 0.01 ulp.

### 21.2 Measured (P11.05m2)

On the host (the speed files' `umull` product), against long double, with
§19.3's sweep (1.4·10^9 samples per function):

| Function | worst | not correctly rounded |
| --- | ---: | ---: |
| `asinh` | 0.5229 | 0.04 % |
| `acosh` | 0.5215 | 0.02 % |
| `atanh` | 0.5229 | 0.05 % |

The worst cases sit just above log1p's 2^-5 boundary, where the table
path's few units of 2^-64 are largest against the result.

Instructions per call (speed → §19's double files on the helpers → fast):

| Core | `asinh` | `acosh` | `atanh` |
| --- | ---: | ---: | ---: |
| armv7e-m, armv8-m.main | 15,215 → 6943 → 535 | 13,378 → 7051 → 499 | 9286 → 4056 → 524 |
| armv7-m | 15,719 → 7166 → 533 | 13,773 → 7265 → 499 | 9757 → 4390 → 524 |
| armv6-m, armv6s-m | 39,784 → 18,468 → 2415 | 34,248 → 18,611 → 2271 | 25,834 → 12,620 → 2532 |

`asin` and `acos` are within 1.5 instructions of P11.05k2's. AOR has no
scalar double hyperbolic function.

**Size:** `asinh` is 1446 bytes on armv7e-m, `acosh` 1228 and `atanh` 1066,
with the 204-byte shared root (332 on armv6-m); the asin kernel shrinks to
772. No size program calls them; every budget holds.

## 22. erf on the double-FPU cores (P11.05n)

`libc/math/fast/erf.c`, on x86_64 and armv7e-m-dp (the soft-double cores
run it on their helpers until P11.05o). The strict file finishes in
double-double over an erfc table and tail: 418 instructions on armv7e-m-dp,
504 on x86_64, against AOR's 69, which P11.06 gates on.

### 22.1 Sources (ideas only)

Read 2026-11-14:

- **AOR `math/erf.c` and `erf_data.c`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/erf.c>,
  `.../erf_data.c`): no exponential and no table, only coefficients (98 in
  all). An odd polynomial of degree 19 below 1/2, rational functions up to
  0.84375 and 1.25, then 1 minus polynomials of degree 13–17 in a shifted
  variable on four ranges up to 5.90625, ±1 beyond. The comment gives 1.01
  ulp and the test harness 0.51.
- **AOR's AdvSIMD `erf`**
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/erf.c>):
  |x| rounded to a multiple of 1/128, erf(r) and 2/√π·e^(−r²) from a table,
  then a Taylor expansion in d = |x| − r to d⁵ with coefficients in r (2.29
  ulp in its comment).
- **FreeBSD msun `s_erf.c`**
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/s_erf.c>):
  x + x·R(x²) with R rational below 0.84375, a constant plus a rational in
  |x| − 1 up to 1.25, and above 1 − e^(−x² − 0.5625 + R/S)/x, with x² split so
  that its rounding error enters the exponential's argument.

### 22.2 The method

- **No exponential.** From 1/2, erf is at least 0.52, so an absolute error
  of 2^-59 is enough. As in AOR's scalar file and P11.04e2's `erff`, erf
  itself is approximated in pieces.
- **The pieces:** 23 of width 1/4 about m = round(4|x|)/4 (1/2 to 6), the
  index by the 1.5·2^52 shift, t = |x| − m exact (|t| ≤ 1/8, m ≥ 1/2).
  - Each piece is c0 + c1·t + t²·Q(t), degree 11, fitted here by least squares
    on Chebyshev nodes: within 2^-59.24 of erf with the coefficients rounded
    (2.5 KiB).
  - c0 and c1 are pairs. c1·t is up to 0.11 against c0 ≥ 0.52, so rounding
    either alone would cost about 0.2 ulp.
  - c0 + c1·t is formed exactly. With an FMA it takes two: hi =
    fma(c1, t, c0), then its error fma(c1, t, c0 − hi), where c0 − hi is
    exact by Sterbenz. Without one, Dekker's product and a fast two-sum.
  - Only c1_lo + t·Q (below 0.03 of the result) rounds in plain double.
- **Below 1/2:** x·2/√π + x³·p0 + x⁵·Q(x²), with p0 = −2/(3√π) and 2/√π as
  pairs. x², x³ and both products are exact pairs and a fast two-sum joins
  them, so the result is relative however small x is. Q has nine terms
  fitted here (2^-61.9). This path is out of line, so the pieces' path saves
  no registers.
- **The ends:** below 2^-60, x·2/√π by the strict file's scaled product (the
  x³ term is below 2^-121 of the result); from 6, ±(1 − tiny).
- **Inlining:** the fast files build at -Os, where `__libc_dd_product`,
  called more than once, stays out of line. `__libc_fast_product` in
  `math-fast.h` is an always-inline copy (121 → 81 instructions on
  armv7e-m-dp, with the Horner chain written out).
- **Found along the way:**
  - The series rounded x³·p0 with the rest, 7 % of the result at 1/2: 0.76
    ulp. With the exact products, 0.537, and with p0 as a pair (rounded
    alone it left 2^-58.2) 0.53.
  - The strict file's tiny path from 2^-28 drops the x³ term, 0.376·x² of
    the result: 0.03 ulp there (0.532 measured). The fast file takes the
    series down to 2^-60.
  - To reach AOR's count: the range tests on the high word, the index by
    the rounding shift, the sign from the bits, c0 + c1·t by two FMAs, and
    c1's low part inside the Horner chain (81 → 68 instructions).

### 22.3 Measured (P11.05n)

On the host, against long double, 1.4·10^9 samples on each of the SSE2 and
fma paths, which agree bit for bit (|x| uniform in [0, 6.5] and [0, 3], below
1/2, and log-uniform down to 2^-70; then 4·10^8 over the pieces alone): at
most 0.5264 ulp, 0.11–0.12 % not correctly rounded.

Instructions per call (speed → fast), with AOR's:

| Core | `erf` | AOR |
| --- | ---: | ---: |
| armv7e-m-dp | 418 → 68 | 69 |
| x86_64 | 504 → 98 | – |
| armv7e-m, armv8-m.main (helpers) | 12,171 → 3206 | 8263 |
| armv7-m (helpers) | 13,408 → 3272 | – |
| armv6-m, armv6s-m (helpers) | 40,655 → 9579 | 25,491 |

The soft-double cores already run below AOR's count on this double file;
P11.05o gives them a fixed-point kernel.

**Size:** `erf` is 3388 bytes on armv7e-m-dp (the table 2576), against the
strict file's 564 over shared double-double cores. No size program calls it;
every budget holds.

## 23. erf on the soft-double cores (P11.05o)

`libc/math/dp-fixed/fast/erf.c`, shadowing §22's double file on the five
soft-double cores. Sources: §22.1's, and P11.04e2's fixed-point `erff`
(pieces of erf in signed Q31 above 1/2, x·(2/√π + u·q(u)) below); no new
source was read.

### 23.1 The method

- **Pieces, in integers, from 1/2 to 6:** 22 of width 1/4, left-aligned at
  a = (j + 2)/4. 4|x| = significand·2^(e − 50), so its integer part picks the
  piece and its fraction is u in Q64, both exact from x's bits.
  - erf(a + u/4) = b0 + u·Q(u), with b0 = erf(a) in Q64 and Q's twelve
    coefficients signed in Q63, fitted here by least squares on Chebyshev
    nodes: within 2^-61.94 of erf with the coefficients rounded (twelve leave
    2^-59.17).
  - Horner's rule runs signed: each step c + u·p takes the high product of u
    and |p| with p's sign, truncated toward zero.
  - b0 + u·Q is summed modulo 2^64; the result is below 1 and from 1/2 at
    least 0.52, so bit 63 is set and one rounding by the 54th bit finishes.
  - The truncations, a few units of 2^-63, stay below 2^-58 of the result.
- **Below 1/2:** erf x = x·K(w) with w = 4x² in Q64 and K = 2/√π − w·G(w) in
  Q62. G has nine signed Q64 coefficients (w·G within 2^-63 of the rest,
  relatively). The result is relative however small x is.
- **The ends:** below 2^-60 the strict file's scaled product (the only
  soft-double operation, and rare); from 6, ±(1 − tiny).
- **Size:** the table is 22 × 13 × 8 = 2288 bytes.

### 23.2 Measured (P11.05o)

On the host (the speed files' `umull` product), against long double, with
§22.3's sweep (1.4·10^9 samples): at most 0.5044 ulp, 0.02–0.04 % not
correctly rounded.

Instructions per call (speed → §22's double file on the helpers → fast),
with AOR's:

| Core | `erf` | AOR |
| --- | ---: | ---: |
| armv7e-m, armv8-m.main | 12,171 → 3206 → 518 | 8263 |
| armv6-m, armv6s-m | 40,655 → 9579 → 2049 | 25,491 |

AOR has no record for armv7-m (518). **Size:** `erf` is 3802 bytes on
armv7e-m and 3852 on armv6-m, the table 2288 of them. No size program calls
it; every budget holds.

## 24. Selective inlining of the high product (P11.06d, Q53, D207)

Q53: `libc/math/dp-fixed/fast/` builds at `-Os`, so each object keeps one
out-of-line copy of `__libc_trig_high64`, the high 64 bits of a 64 × 64
product, and calls it from every step of its Horner chains: 43 call sites in
`pow`, 13 in the atan kernel, 12 in the log core. P11.05k1 measured the
whole module at `-O2`: 7–20 % faster, but 2.6 KB more `math-mix` on
armv7e-m and 16 KB on armv6-m. The maintainer chose to inline selectively,
kernel by kernel, and keep only what fits the fast budgets.

### What decides it, core by core

- **On the Thumb-2 cores** (armv7-m, armv7e-m, armv8-m.main) a 32 × 32 → 64
  product is one `UMULL` (`speed/umull/trig-wide.h`). The high product is
  two or four of them and a few additions, about the size of the call's
  argument moves, branch and return. Inlining roughly halves each step.
- **On Armv6-M** each 32 × 32 product is four 16-bit `MULS` with their
  carries (`speed/muls/`). The body is several times the call, so an
  inlined copy costs much more and saves little.
- **The slack:** the fast budgets give an image no growth over its locked
  figure. At P11.06c, `math-mix` was 828 bytes below its lock on armv6-m
  and armv6s-m, 1200 on armv7-m, 1246 on armv7e-m and 1222 on
  armv8-m.main.

### What was tried

- **A loop** over a coefficient table with one inlined product, the sign of
  the alternating chains by a mask: one copy per chain, but the mask and the
  64-bit accumulator spill to the stack on every step. `log` measured 333.6
  → 438.7 instructions on armv7e-m without the shortcut for a 32-bit b (see
  below), and 429.2 with it: slower.
- **The unrolled chains with the product inlined** (kept): one copy per
  call site, the coefficients still immediates. Where b is a constant, or
  fits 32 bits as the early Horner terms do, the `b1 == 0` shortcut
  (`__libc_trig_high64`'s own: two products, the same value) folds or takes
  two `UMULL`s.

### The design

- `__libc_dp_fixed_hot_high` (`fast-high.h`) in a directory per product
  variant, picked by `fast.mk` as `speed/` picks `trig-wide.h`: in `umull/`,
  `__libc_trig_high64`'s body, always inlined; in `muls/`, a call to it, as
  before. No `#if`: the variant is a header the build selects (D22).
- Used on the hot chains only, in order of instructions saved per byte:
  - the atan kernel's series (`atan2` −36 instructions for 152 bytes);
  - the log core's two chains (`log` −38 for 256);
  - `pow`'s logarithm chains (−61 for 400);
  - the exp2 fraction's chain (`exp` −55 for 376; `pow`, `expm1` and the
    hyperbolic functions through it).
- `pow`'s two products after its chains (y³ and y³S) stay calls: inlined
  they took armv7-m 16 bytes over its lock for about 16 instructions.

### Measured

Bit for bit the same results: the inlined body is the called one (the libm
vectors, the threshold tests and `make check` in all 28 configurations agree).
Instructions per call on armv7e-m (armv7-m and armv8-m.main within two of
it), fast before → after:

| Function | before | after |
|---|---:|---:|
| `exp` | 315.6 | 260.6 |
| `exp2` | 279.9 | 224.9 |
| `expm1` | 345.6 | 292.2 |
| `log` | 333.6 | 295.3 |
| `log2` | 348.9 | 310.6 |
| `pow` | 923.3 | 816.9 |
| `atan2` | 524.1 | 488.1 |
| `sinh` | 508.0 | 394.0 |
| `cosh` | 494.2 | 380.2 |
| `tanh` | 500.5 | 445.5 |

`math-mix` (code bytes): armv7-m 40,192 → 41,328 (lock 41,392), armv7e-m
34,006 → 35,134 (35,252), armv8-m.main 33,926 → 35,046 (35,148): +13.6 %,
+17.2 % and +17.2 % over speed, inside D191's +20 % line. armv6-m and
armv6s-m are unchanged, byte for byte.

## 25. armv7e-m-dp's `math-mix`: what a reduction would save and cost (P11.06e, D207)

The maintainer asked, before settling D191's line for this core, what a
shared trigonometric kernel and slow paths that do not pull the strict cores
would save, and what they would cost (D207). At P11.06d, armv7e-m-dp's fast
`math-mix` is 20,312 bytes of code against speed's 12,174 (+66.8 %).
D191's line, +20 %, is 14,609: 5,703 bytes away.

### Where the 8,138 bytes go (`nm` on the image)

- **The fast files themselves:**
  - `pow` 836 bytes, `__libc_fast_pow_special` 568, `pow_exp_ends` 388;
  - `sin` 676, `cos` 592 and `tan` 772, each inlining its reduction and
    kernel (speed's 304, 236 and 432 share one double-double core);
  - `atan2` 540, `log` 332, `exp` 220;
  - the tables: log 3,072 (128 entries of 1/c, log c and its tail; speed's
    is 1,472), exp pairs 1,024, sin 272, atan pairs 272.
- **The strict code the fast slow paths keep:**
  - the trigonometric functions from 2^20 and next to a zero:
    `__libc_fast_trig_slow` 242, `__libc_rem_pio2` 788, `two_over_pi` 160,
    `__libc_sin_core` 304, `__libc_cos_core` 208;
  - `atan2`'s extremes: `__libc_fast_atan2_slow` 736, `__libc_atan2_core`
    272, `atan_unit` 316, the strict atan table 528;
  - `exp`'s and `pow`'s results next to overflow and underflow:
    `__libc_exp_core` 332, its table 1,024, `__libc_exp_scale` 272 (which
    the `atan2` slow path also calls).
- **The float functions** (`sinf`, `cosf`, `expf`, `logf`, `powf`) as in
  `math-float`, which is below speed's.

### The options

| Option | Bytes saved | Instructions per call | Measured or estimated |
|---|---:|---|---|
| A. One out-of-line kernel for `sin` and `cos` (`tan` needs two turns and keeps its own) | 272 | `sin` 96.1 → 115.1, `cos` 93.3 → 111.3 | prototyped and measured, then reverted |
| B. The trigonometric slow path on the fast kernels, keeping the strict reduction | about 400–500 (512 removable, less the glue) | none on the common path | estimated from the members' sizes |
| C. `atan2`'s slow path by scaling into the fast path's domain (subnormals, exponents more than 60 apart, near the overflow) | about 1,600 (1,852 removable, a replacement of about 250) | none on the common path | estimated |
| D. `exp`'s and `pow`'s ends on the fast kernel with a two-step scaling | about 1,400 (1,628 removable once C is done, the replacements about 250) | none on the common path | estimated |
| E. A 64-entry log table | 1,536 | about +2 in `log`, `log10`, `log1p` and `pow` (one more polynomial term for the same 2^-60) | estimated; the exact r = z·invc − 1 needs invc of at most 10 bits with \|r\| ≤ 2^-7.47 (§9.2), so twice the step needs a new argument, or a correction term |

The shared kernel (A) costs more than it saves: the call and the results
passed through memory add 18–19 instructions (+20 %) to `sin` and `cos`, for
272 bytes. B, C and D touch only the slow paths: they cost no instructions
on the arguments the benchmark and most programs use, but they are new code
for the extremes, each with its own sweeps.

Together, B, C, D and E would leave `math-mix` at about 15,300 bytes, still
+26 % over speed; adding A, about 15,050 (+24 %). No set of these reaches
D191's +20 % on this core. The rest of the excess is the fast files'
speed itself: inline reductions and kernels, and tables larger than speed's.

The maintainer's decision (D211): B, C, D and E, in P11.06f–i, then a
`math-mix` line of this core's own at the figure they leave.

## 26. The trigonometric slow path on fast polynomials (P11.06f, D211 B, D212)

`__libc_fast_trig_slow` takes `sin`, `cos` and `tan` from 2^20 and next to a
zero, and `sinf`, `cosf` and `tanf` from 2^20 (`__libc_fast_trig_large`). It
was the strict file's code: `__libc_rem_pio2`, then `__libc_sin_core` and
`__libc_cos_core` (512 bytes) and a double-double division in a
round-to-nearest section.

### What was tried

- **The common path's table kernel:** r by π/32 once more, and
  `__libc_fast_sin_turn`. `math-mix` lost only 104 bytes and `math-float`
  gained 120: the table (272 bytes) and the turn came back, and
  `math-float`'s floats had never needed them.
- **Plain polynomials in r**, |r| ≤ π/4, with z = r² rounded: 1.28 ulp
  (`sin`), 1.58 (`tan`) on 10^5 samples. Near π/4, r³/6 is 0.11 of sin r and
  r²/2 0.43 of cos r, so their roundings show. Making z exact took the error
  to 0.86, and −r³/6 exact to 0.68. The last 0.1 ulp was a missing term:
  r_lo·cos r_hi, not r_lo, is sin's first-order correction.

### The design (kept)

- x = q π/2 + r by `__libc_rem_pio2` (r as a pair, about 2^-109 here), unchanged.
- **sin r** = r_hi − r³/6 + r⁵ S(r²) + r_lo (1 − r²/2):
  - z = r² and r³ are exact pairs (`__libc_dd_product`);
  - −r³/6 is formed from r³'s high part with its own error, short only by
    1/6's rounding (2^-55 of itself);
  - r_hi − r³/6 is a fast two-sum;
  - S has six terms, fitted here on 400 Chebyshev nodes (2^-57.1 relative to
    sin r).
- **cos r** = (1 − z/2, a fast two-sum, exact since 1 ≥ z/2) + z² C(z) −
  r_hi r_lo, with C of six terms (2^-60.5).
- **tan**: tan r, or −cot r in an odd quadrant. Both pairs are normalized,
  then the quotient and its remainder are taken to first order, as in the
  common path's `tan`.
- No table, no rounding-mode section (the fast contract rounds to nearest),
  and no strict core.

### Measured

From 2^20 to the largest double, log-uniform with both signs, and the
doubles within 2·10^6 ulps of k π/2 for 0 < |k| ≤ 2^19, against long double
(checked against mpmath at the worst points):

| Function | samples | max ulp | not correctly rounded |
|---|---:|---:|---:|
| `sin` | 10^8 | 0.5918 | 0.46 % |
| `cos` | 10^8 | 0.5830 | 0.46 % |
| `tan` | 10^8 | 0.6489 | 0.65 % |
| `sinf`, `cosf`, `tanf` from 2^20 | every float | 0.5000 | |

That is inside the bounds (1 ulp, 1.5 for `tan`), and level with what the
strict code gave there (0.59–0.61 for the strict `sin` and `tan` from 2^20).
The fma path draws its products from the same exact `__libc_dd_product`;
10^7 other samples gave 0.5791, 0.5732 and 0.6494. The common path is
unchanged.

armv7e-m-dp: `math-mix` 20,312 → 20,112 bytes (−200), `math-float` 5,910 →
5,654 (−256). The exact tail terms cost about 140 bytes of the plain
version's saving. P11.06e estimated about 450 bytes for this option.

## 27. atan2's slow path by scaling (P11.06g, D211 C, D213)

`__libc_fast_atan2_slow` serves both fast `atan2`s: the double-FPU cores'
(`libc/math/fast/`, normal coordinates in [2^-959, 2^960) at most 60 binades
apart) and the soft-double cores' (`dp-fixed/fast/`, any normal pair at most
60 apart). It was the strict file's code: `__libc_atan2_core` with its
double-double table, `atan_unit`, a double-double division and
`__libc_exp_scale` for the tiny quotients.

### The design

- **The special cases** (C23 F.10.1.4) as before, from π as a pair.
- **Exponents more than 60 apart:**
  - y the larger: ±π/2, since π/2 − x/y rounds to it;
  - x < 0 the larger: ±π;
  - x > 0 the larger: atan(y/x) = q − q³/3 with |q| < 2^-59, so the quotient
    rounded once is atan's, with its flags and its zero's sign. When it is
    exact (q·x == y as a pair), `__libc_math_tiny` nudges it toward 0, so that
    inexact, and underflow for a subnormal, are raised as for the function.
- **Otherwise** one coordinate is subnormal, below 2^-959, or from 2^960:
  both are scaled by the same power of two, so that the larger is near 1. The
  scaling is exact (the smaller stays normal within 61 binades of 1), the
  angle does not change, and `atan2` itself, with either core's fast path,
  takes the call.

### Found along the way

The first version returned q − q·2^-80 for the far quotients, to raise
inexact for an exact one:
- it raised underflow for a normal result below 2^-942
  (`atan2(0x1p-1000, 3.0)`, `libm_thresholds`);
- it turned −0 into +0 rounding to nearest and +0 into −0 rounding downward
  (`libm_trig`, `libm_vectors`).

Hence the exactness test and `__libc_math_tiny`, which leave every inexact
quotient as the division rounded it.

### Measured

4·10^8 random pairs on the host against `atan2l`:

| Coordinates | max ulp |
|---|---:|
| both exponents below 2^-959 (subnormals included) | 0.5010 |
| both from 2^960 | 0.5010 |
| exponents 61–200 apart | 0.5000 |
| exponents 50–70 apart | 0.5000 |

That is the fast path's 0.5010, which these now go through.

`math-mix` (code bytes):

| Core | before | after |
|---|---:|---:|
| armv7e-m-dp | 20,112 | 18,758 |
| armv7e-m | 35,134 | 32,574 |
| armv8-m.main | 35,046 | 32,486 |
| armv7-m | 41,328 | 38,608 |
| armv6-m, armv6s-m | 46,100 | 43,388 |

On the soft-double cores the strict core was soft-float code, hence the
larger saving; P11.06e estimated about 1,600 bytes for armv7e-m-dp. The
exactness test costs about 120 bytes on armv7e-m-dp and 370 on the
soft-double cores. No bench case reaches the slow path.

## 28. exp's ends on the fast kernel (P11.06h, D211 D, D214)

`__libc_fast_exp_special`, `__libc_fast_exp2_special` and
`__libc_fast_exp10_special` are the slow paths of both fast families: the
double-FPU cores' `exp`, `exp2` and `exp10` (`libc/math/fast/`) and the
soft-double cores' fixed-point ones (`dp-fixed/fast/`). They handle the tiny
arguments, the infinities and NaN, and |x| from the common path's edge (512
for `exp` and `exp2`, 256 for `exp10`) to the ends. In that last range they
ran the strict file's double-double core: `__libc_exp_core` (332 bytes), its
1,024-byte table and `__libc_exp_scale` (272).

`pow`'s ends (P11.05f) already use the common path's reduction and
polynomial with `__libc_fast_exp_scale`, which scales in two steps. The three
special paths now do the same:
- `exp`: k = round(64 x/ln 2), u = x − k ln2/64 in two parts,
  `__libc_fast_expm1_poly`;
- `exp2`: k = round(64 x), r = x − k/64 exactly, the 2^r − 1 polynomial
  (integers still exact by `scalbn`);
- `exp10`: log10 2 / 64 in two parts, as its common path.

On the soft-double cores this is soft-float code on a rare path. It pulls
the fast exponential's 1,024-byte table of pairs, which `subdir.mk` now keeps
for the special paths, in place of the strict core.

### A defect of `__libc_fast_exp_scale`, found and fixed

The first sweep found `exp2` at 0.762 ulp just below 2^-1022. Below 2^-1014
the helper scales by 2^1022: the sum scale + scale·tmp rounds to 53 bits
there, and the final multiplication by 2^-1022 rounds it again, onto the
subnormal grid. `pow`'s ends share the helper: committed since P11.05f, they
reached 0.7683 ulp for subnormal results (10^8 samples between 2^-1074 and
2^-1000), which P11.05f's record of 0.5498 missed.

At the 2^1022 scale, the subnormal grid is the multiples of 2^-52, so for
y < 1:
- 1 + y rounds y onto that grid once, with y's own rounding error carried
  in by a fast two-sum (the idea of AOR's `exp`, which handles its special
  case the same way);
- then the scaling is exact, so underflow is raised by hand. It is raised
  only when the result differs from the sum as rounded to 53 bits, the old
  path's rule. Raised whenever the one rounding is inexact, it flagged exact
  results: `pow(0x1.8p-1070, 1)` lands within 2^-58 of its grid point,
  but not on it as computed (`libm_exp`);
- a zero sum is +0, not −0, rounding downward.

### Measured

2·10^8 samples from the common path's edge to each end, both signs,
subnormal results included, against long double; and 10^8 `pow` arguments
with results from 2^-1074 to 2^-1000:

| Function | max ulp |
|---|---:|
| `exp` | 0.5483 |
| `exp2` | 0.5479 |
| `exp10` | 0.5542 |
| `pow`, subnormal results | 0.5483 (was 0.7683) |

That matches the common paths' 0.5469, 0.5464 and 0.5527.

`math-mix` (code bytes):

| Core | before | after |
|---|---:|---:|
| armv7e-m-dp | 18,758 | 17,520 |
| armv7e-m | 32,574 | 31,710 |
| armv8-m.main | 32,486 | 31,622 |
| armv7-m | 38,608 | 37,848 |
| armv6-m, armv6s-m | 43,388 | 42,644 |

## 29. A 64-entry double log table (P11.06i, D211 E, D215)

`__libc_fast_log_table` and `__libc_fast_log2_table` (§9) had 128 entries of
three doubles each, 3 KiB apiece. `log`, `log10`, `log1p`, `pow` and the
inverse hyperbolic functions share the first, `log2` has the second. Both now
have 64 entries, indexed by the top 6 bits of z's fraction: 1,536 bytes off
each.

### The constraint

r = z·invc − 1 must be exact (§9.2): with an fma it is then a single
rounding-free operation, and without one z splits after its 43rd bit. invc
has at most 10 bits and r must fit 53 bits. An entry above 1 is 2^-6 wide, so
|r| needs about 2^-7. A z of ulp 2^-52 times an invc on a grid of 2^-g gives
a product on 2^-(52+g), and an r up to 2^-6.5 then takes 46 + g bits. So
g ≤ 7 above 1: invc on a grid of 2^-7, whose rounding adds up to 2^-8 to |r|.

The best invc per entry under that rule (§9's generator, rerun at 64) gives
|r| ≤ 2^-6.497. The polynomial needs one more term for the same accuracy:

| Fit for (log1p r − r)/r² | degree 5 | degree 6 |
|---|---:|---:|
| \|r\| ≤ 2^-6.497 | 2^-53.48 | 2^-61.15 |
| \|r\| ≤ 2^-6.607 (the table kept) | 2^-54.24 | 2^-62.04 |

`log2`'s fit at degree 6 is 2^-61.61, and `pow`'s Q (degree 6, its error
otherwise bounded by the rounding of 1/3) 2^-67.80.

### What cancellation costs pow

The first 64-entry table put 1 two thirds of the way into its entry, as the
128-entry one did. `log` measured 0.5229 ulp and `pow` 0.6045 (was 0.5449),
for x just above 1 with |y log x| near 700.

The cause is the entry next to 1's, where w = log c and r have opposite
signs. There |log x| can be half of |r|, and the absolute error of r³Q(r)
(four roundings of about 2^-54 of r³/3) is amplified by |y| ≤ 745/|log x|. So
the measure of an entry is max |r|³/|log z|:
- the 128-entry table: 2^-14.15;
- the first 64-entry table: 2^-12.13, four times worse.

No per-entry choice of invc alone recovers it, since the rule above fixes
invc's grid. Moving 1 within its entry does:
- with 1 half way into it (z in [0x1.6a041p-1, 0x1.6a041p+0)), the
  neighbours start 2^-8 below 1 and 2^-7 above;
- choosing each invc for the smallest |r|³/|log z| (|r| breaking ties)
  among those that keep r exact gives 2^-13.97 and |r| ≤ 2^-6.607.

1's own entry has invc = 1, w = 0 and no cancellation. The scan covered a
from 0.29 to 0.53 of the entry; below 0.29, 1's entry has r beyond the fit.

### The instructions

D211 estimated about +2 instructions. One more polynomial term in Estrin's
order cost 3 on armv7e-m-dp (a constant, a multiply-add and r⁴), which took
`log` from 66.69 to 69.69, above AOR's 68.62. Two changes win the cost back
without touching the error:
- **The polynomial** as (a0 + a1 r) + r²·(a2 + r(a3 + r(a4 + r(a5 + r a6)))):
  one multiply-add a term and no r⁴, one fewer than Estrin's order. GCC
  already contracts these into `vmla` on the Arm FPU (two roundings, one
  instruction).
- **`log`'s test for 1** (+0 rounding downward, D190) is now r == 0, one
  floating-point compare where comparing the 64-bit pattern took five
  instructions on Arm. A power of 2 returns k ln 2 from a cold, out-of-line
  helper. Inline, GCC shared k·ln2_hi between the two returns and split two
  `vmla` into `vmul` and `vadd`.
- **Always inline.** At seven terms GCC put `__libc_fast_log_poly` out of
  line on x86_64 in `log1p`, which calls it on both paths (+4 instructions);
  it is `always_inline` now. `log2`'s polynomial is in plain Horner's order,
  which saved a register move on x86_64.

### Measured

On the host, against long double, on the SSE2 and the fma paths (which
agree bit for bit), with the sampling of §9.3 and §11.3:

| Function | samples | 128 entries | 64 entries |
|---|---:|---:|---:|
| `log` | 10^9 | 0.5151 | 0.5063 |
| `log2` | 10^9 | 0.5249 | 0.5063 |
| `log10` | 10^9 | 0.5132 | 0.5112 |
| `log1p` | 10^9 | 0.5059 | 0.5063 |
| `pow` | 3·10^8 a family | 0.5513 | 0.5547 |
| `asinh`, `acosh`, `atanh` | 2·10^8 | 0.5151, 0.5166, 0.5161 | 0.5098, 0.5103, 0.5103 |

2·10^4 doubles on each side of every entry boundary: at most 0.5093. Every
power of 2 in `log2` and of 10 in `log10` is exact; `log(1)` is +0. `pow`'s
worst is in the family whose results lie near 2^±1000 from x next to 1
(§11.3's fifth), the one where cancellation counts. The logarithms gain
because 1's own entry, where w = 0, now reaches 2^-7 above 1.

Instructions per call (fast, before → after):

| Function | armv7e-m-dp | x86_64 |
|---|---:|---:|
| `log` | 66.69 → 65.69 (AOR 68.62) | 75.69 → 77.69 |
| `log2` | 70.69 → 70.69 (AOR 73.62) | 86.69 → 88.69 |
| `log10` | 75.69 → 77.69 | 100.69 → 101.69 |
| `log1p` | 86.81 → 88.81 | 91.56 → 92.56 |
| `pow` | 133.75 (AOR 135.69) | 203.62 |
| `asinh`, `acosh`, `atanh` | +2 | +1 to +2 |

The functions AOR implements stay below its counts on armv7e-m-dp, so
P11.06a's exit check holds. The soft-double cores' logarithms are
fixed-point (§10) and unchanged.

**Size:** armv7e-m-dp's `math-mix` 17,520 → 16,032 bytes (−1,488: the
table's 1,536 less the extra term and `log`'s cold helper). `log2`'s table
saves another 1,536 where it is linked; `math-float` is unchanged.

## 30. hypot and hypotf, correctly rounded (P11.07b, D219, D222)

D219 keeps `hypot` and `hypotf` correctly rounded in `fast` and takes them
from 2–4 times picolibc's instructions (`fast-math.md` §12) to below it.
Three files, ideas from musl's `hypot` (exact squares, one root) and `sqrt`
(the rounding by an exact integer comparison):
- `libc/math/sp-nearest/fast/hypotf.c` (armv7e-m, armv8-m.main);
- `libc/math/fast/hypot.c` (x86_64, armv7e-m-dp);
- `libc/math/dp-fixed/fast/hypot.c` (the five soft-double cores).

The strict files are not correctly rounded: `hypotf` within 0.509 ulp, at
most 2 in 10^5 misrounded (D59).

### 30.1 hypotf in float

For a and b in [2^-20, 2^62), every piece is normal:
- the squares as exact pairs (one fused multiply-add each for the error);
- h + l, the high parts' sum by a two-sum that needs no ordering, plus
  the low parts;
- s = sqrtf(h), d = h − s² (exact by an fma) + l, and c = d/(2s), so that
  s + c is within about 2^-46 of the root.

Outside that range a slow path takes the infinities and NaN (an infinity
wins), a zero, b below 2^-12 a (the result rounds to a), and otherwise
scales by powers of two, exactly, and back with one rounding.

**Ties.** √(x² + y²) can be exactly a midpoint between two floats:
Pythagorean triples with an odd 25-bit hypotenuse and legs below 2^24. The
correction's rounding then sends it either way, so the first version
rounded 129,452 of 260,004 such ties away from even (0.5 ulp each). Now a
result whose |c| is within 64 units of the bits of u/2 (about 2^-17 either
side) is decided exactly: a² + b² against (s ± u/2)², as 128-bit integers
at the lowest of the exponents.

Each step cost instructions on armv7e-m (bench, throughput; picolibc 53.9):

| Version | instructions |
|---|---:|
| strict | 161.4 |
| first fast version, rounding out of line | 69.4 |
| rounding forced inline, the window as an integer distance | 56.4 |
| no ordering (an unordered two-sum), no gap test on the common path | **52.4** |

A far-apart pair can no longer reach the tie test, because c is then far
below half an ulp. Both operands are in range, so b² and its error stay
normal at any gap.

### 30.2 hypot in double (x86_64, armv7e-m-dp)

The same in double: `__libc_dd_product` for the exact squares (an fma on
armv7e-m-dp, Dekker's product on SSE2), a in [2^-400, 2^500) with b within
2^27. Near a midpoint (2^-40 ulp) the sign of H − (s ± u/2)² comes from a
seven-part exact expansion (Shewchuk's grow-expansion over h − s²'s parts,
the squares' parts, ∓su and −u²/4).

A subnormal result is rounded once onto the subnormal grid g at the scaled
size: s rounded to a multiple of g by adding and subtracting 2^52·g, then
the remainder (s − that) + c. An exact tie cannot happen there: H is a
multiple of 2^-2148, and a midpoint's square is not.

### 30.3 hypot in fixed point (the soft-double cores)

Integers only, no soft-double operation on the common path:
- a ≥ b as 53-bit significands; b below 2^-61 a gives a.
- W = 4S, with S = ma² + mb²·4^-(ea−eb) exactly but for a sticky bit:
  b's square is shifted by 2(ea − eb) − 2, so that W keeps S's two
  fractional bits.
- The root of W's top 64 bits by `__libc_dp_fixed_root` (D200), within
  about 8 units of sqrt(S)·2^11, rounded at its bit 11 (more for a
  subnormal result).
- Within 32 units of a midpoint, (2q + 1)²·4^p is compared with W exactly;
  ties go to even.

The first version compared against S shifted right with a sticky bit,
which loses exactly the fraction that decides a midpoint one unit away. W
keeps it.

### 30.4 Bugs found before the commit

- **The far-apart path returned a + b**, not a: 1024 ulp at
  `hypotf(0x1.ffddfep-31, -0x1.ffc018p-45)`. Now a + b·2^-24, which rounds
  to a and raises inexact.
- **The tie test's base assumed the midpoint above b's exponent.** A near-tie
  with s in b's binade compared at a negative shift and was decided wrongly
  (56 in 10^9). The base is now the lowest exponent.
- **The scaled slow path skipped the tie test** (8 in 10^9, tiny pairs).
  It now shares the rounding and scales back exactly when the result is
  normal.
- **Both arguments zero looped forever in the scaling.** A zero b returns
  a first.
- **The subnormal grid's magic constant was 1.5·2^52·g,** whose sum
  crosses into the next binade when s exceeds 2^51·g: 1 in 10^8 on SSE2.
  It is now 2^52·g.
- **The FPU cores' flags** (`libm_exp`, `libm_vectors`): 0.5/s raised
  inexact for exact results (hypot(3, 4)), and the exact rescaling of a
  rounded subnormal result raised no underflow. The correction is now the
  quotient d/(2s), exact when d is 0, and underflow is raised by hand.

### 30.5 Measured

Against binary128 (`__float128`), random pairs at every gap and exponent
(an eighth across the whole range, subnormals included), the Pythagorean
ties, and every pair of special values:

| File and path | samples | misrounded | exact ties not to even |
|---|---:|---:|---:|
| `hypotf` (fma) | 10^10 | 0 | 0 of 260,004 |
| `hypot`, double, SSE2 | 2·10^9 | 0 | 0 of 7,876,938 |
| `hypot`, double, fma | 2·10^9 | 0 | 0 |
| `hypot`, fixed point (`umull`) | 2·10^9 | 0 | 0 |
| `hypot`, fixed point (`muls`, Armv6-M) | 3·10^8 | 0 | 0 |

Instructions per call (fast, throughput; picolibc 1.8.6):

| Core | `hypot` | picolibc | `hypotf` | picolibc |
|---|---:|---:|---:|---:|
| armv7e-m | 5835 → 346 | 1443 | 161.4 → 52.4 | 53.9 |
| armv8-m.main | 5835 → 344 | 1443 | 158.4 → 52.4 | 53.9 |
| armv7-m | 5956 → 346 | 1448 | 3336 (strict) | 767 |
| armv6-m, armv6s-m | 14,773 → 1177 | 2645 | 6293 (strict) | 1251 |
| armv7e-m-dp | 137.7 → 108.7 | 61.9 | 35.4 (strict) | 53.9 |
| x86_64 | 248.1 → 174.1 | — | 37.6 (strict) | — |

**Not closed here**, outside P11.07's exit check (armv7e-m) and recorded as
Q61:
- `hypotf` on the soft-float cores (armv6-m, armv6s-m, armv7-m) keeps the
  strict file, 4–5 times picolibc's count; an integer `hypotf` would be
  correctly rounded by construction.
- armv7e-m-dp's `hypot`, whose correct rounding costs the exact squares
  and the window, is still 1.75 times picolibc's.

**Size.** No size program calls `hypot` or `hypotf`. The members:
- the fixed-point `hypot` is 1276–1412 bytes (speed's 908–956);
- `hypotf` on the single-precision FPU 966 (376);
- the double-FPU `hypot` 1214 (388): the tie expansion and the subnormal
  rounding, out of line.

## 31. erfc at 1 ulp (P11.07c2, D219, D224)

Until this step the fast flavour kept the strict `erfc` (the double-double
`__libc_erfc_*` cores), which on the soft-double cores ran every step as
soft calls: 37,968 instructions a call on armv7e-m against picolibc's 6630,
and 783 against 159 on armv7e-m-dp (`fast-math.md` §12).

### 31.1 Sources

- musl and fdlibm (`src/math/erf.c`): below 0.84375, 1 − erf by a
  rational in x²; to 1.25, a rational about 1; beyond, e^(−z² − 0.5625) ·
  e^((z − x)(z + x) + R/S), with z = x truncated to 32 bits so that z² is
  exact, and rationals in 1/x² on [1.25, 1/0.35] and beyond. The error is
  about 1 ulp, with divisions.
- CORE-MATH (`erfc.c`, Zimmermann and Sibidanov): correctly rounded, with
  piecewise polynomials for erfc(x)·e^(x²) and double-double arithmetic,
  plus a rounding test and a slow path.
- The fast flavour's own pieces: `erf`'s (§ on erf, D204, D205) and
  `erfcf`'s kernel (`libm-fast-float-trig.md` §23).

No code was copied. The approach below is written from the identity
erfc(a) = e^(−a²) R(a), with R smooth and slowly varying (R ~ 1/(a √π) as
a grows), and from this project's exponential kernels.

### 31.2 Method

Both files share the shape:
- below 1/2 in magnitude, 1 − erf(x) from the fast `erf`'s series;
- from 1/2, e^(−a²) R(a) with a = |x|, a² carried exactly into the
  exponential's reduction;
- R by pieces in a below 4, and from 4 as v G(v²), v = 1/a, where
  G(w) = R(1/√w)/√w is close to 1/√π and analytic at w = 0;
- for a negative x, 2 − erfc(a), at least 1.5, so the subtraction loses
  nothing; below −6 the result rounds to 2;
- from 27.3 the result is below 2^-1075 (the cut is 27.2260), so +0 with
  underflow.

**Double (`libc/math/fast/erfc.c`, x86_64 and armv7e-m-dp).**
- Below 1/2: 1 − x·2/√π and then − x³p0 by two fast two-sums of exact
  products; the rest (below 2^-7 of the result) in plain double.
- e^(−a²): a² as an exact pair z_hi + z_lo; k = round(−64 z_hi/ln 2) by
  the shift constant; u = −z_hi − k ln2/64 (two parts, as `pow`) − z_lo;
  S = 2^(k/64) from the table, τ = its tail + (e^u − 1).
- R below 4: seven pieces of width 1/2, degree 13 in t = a − m (2^-57.24
  relatively), c0 and c1 as pairs, c0 + c1 t exact (two fused
  multiply-adds, or an exact product and a fast two-sum).
- From 4: v = 1/a with its remainder, w = v², three pieces of degree 10
  split at w = 1/100 and 1/36 (2^-57.68).
- The result: S R_hi as an exact product, plus S (R_lo + (R_hi + R_lo) τ),
  rounded once.
- From 26, S is scaled by 2^200 and the sum by 2^-200: one more rounding,
  onto the subnormal grid, which raises underflow itself.

**Fixed point (`libc/math/dp-fixed/fast/erfc.c`, the soft-double cores).**
- Below 1/2: 1 ∓ x K(w) in Q62, x K(w) as the fixed-point `erf`'s
  (w = 4x² in Q64).
- a²·log2(e): m² is exact in 106 bits (H 2^64 + L); times log2(e) in Q127
  as T = H c_hi + high(L c_hi) + high(H c_lo). Every dropped term is below
  one unit of T, which keeps 31 to 41 bits more than the fraction f of
  t = n + f 2^-64. Then −t = n' + g and 2^(g 2^-64) by exp's kernel
  (`__libc_dp_fixed_exp2_fraction`, Q63).
- R below 4: fourteen pieces of width 1/4 in u = 4a − ⌊4a⌋, exact from
  a's bits (as `erf`), b0 in Q64 and b1..b11 signed in Q63. The fit is
  within 2^-60.42 absolutely, with the evaluation's truncations
  simulated, which is 2^-57.6 relatively where R is least (0.137 at 4).
- From 4: v = 1/a by `__libc_dp_fixed_reciprocal` (2^-63 relatively),
  w = v² in Q64 (below 2^60), four pieces of width 1/64 indexed by w's top
  bits, degree 9 (2^-61.10).
- The products' high halves, rounded once by the result's 54th bit, onto
  the subnormal grid below 2^-1022 (by hand: these cores raise no flags,
  D44).

### 31.3 The bug the sweep found

R's pieces return c0 + c1 t as the high part and leave the t² terms (up to
2^-6 of R) in the low part. Multiplied by e^(−a²) = S (1 + τ), the first
draft dropped the low part's product with τ: a relative error of 2^-16.
The sum now multiplies (R_hi + R_lo) by τ (`regression` in
`check_erfc_ends`: erfc(1) and the switch points).

The G fit had also been built as R·√w rather than R/√w, which made it
proportional to w rather than nearly constant; refitted.

### 31.4 Measured

Against binary128 (`erfcq`), 4·10^6 random points in each range:

| Range | double (SSE2 and fma) | fixed point |
|---|---:|---:|
| [−6, −4] | 0.5000 | 0.5010 |
| [−4, −1/2] | 0.5282 | 0.5027 |
| [−1/2, 1/2] | 0.5179 | 0.5036 |
| [1/2, 1] | 0.6311 | 0.5082 |
| [1, 4] | 0.5563 | 0.5172 |
| [4, 6] | 0.5931 | 0.5054 |
| [6, 26] | 0.5453 | 0.5074 |
| [26, 27.3] (subnormal results) | 0.7633 | 0.5085 |

Both files are within the 1-ulp bound everywhere. The fixed-point file is
closer to correct rounding, because its 64-bit products carry about 10
bits more than double.

Instructions per call (fast, throughput; picolibc 1.8.6):

| Core | `erfc` | picolibc |
|---|---:|---:|
| armv7e-m, armv8-m.main | 37,968 → 843 | 6630 |
| armv7-m | 40,682 → 844 | — |
| armv6-m, armv6s-m | 112,956 → 3988 | 15,567 |
| armv7e-m-dp | 783 → 133.4 | 159 |
| x86_64 | 1443 → 208.9 | — |

On armv7e-m, a double first measured 7100 instructions: it was the double
file running on the soft-double helpers, still above picolibc's 6630. That
is why the soft-double cores take the fixed-point file.

**Size.** No size program calls `erfc`, so the size gates do not change.
The members, against speed's `erfc.o`, which also pulls in the
`__libc_erfc_*` double-double cores:

| Member | bytes | speed's `erfc.o` |
|---|---:|---:|
| fixed point, armv6-m | 3740 | 824 |
| fixed point, armv7e-m | 3928 | 900 |
| double, armv7e-m-dp | 2564 | 516 |
| double, x86_64 | 3916 | 813 |

The tables are most of each: 14 × 12 + 4 × 10 words in fixed point, and
7 × 16 + 3 × 14 doubles in double.

## 32. lgamma at 1 ulp on the double-FPU cores (P11.07d1, D219, D225)

The fast flavour had kept the strict `lgamma`, a double-double core that
ran its recurrence up to 12 and Stirling's series beyond: 499 instructions
a call on armv7e-m-dp, and 17,366 on armv7e-m, where every step is a soft
call.

### 32.1 Sources

- musl and fdlibm (`src/math/lgamma_r.c`): pieces by |x|; near zero,
  −log|x|; for negative x, the reflection through sin(πx); rationals about
  the minimum near 1.46 and on [2, 3]; the recurrence into [2, 3) below 8;
  Stirling's series in 1/x from 8.
- The strict file (`libc/math/__libc_lgamma_core.c`, `libm-special.md`):
  Taylor series about 1 and 2, the recurrence to 12, Stirling's series
  with ten terms, and the reflection.
- This project's fast kernels: `__libc_fast_log_pair` (D190, D201) and
  the fast sine's table and kernel (D196).

No code was copied.

### 32.2 Method (`libc/math/fast/lgamma.c`)

- **The zeros 1 and 2.** h(y) = lgamma(y)/((y − 1)(y − 2)) is smooth and
  positive on [1, 8): γ at 1, 1 − γ at 2. It is fitted in 24 pieces, eight
  a binade (widths 1/8, 1/4 and 1/2), each of degree 11 in t = y − m. The
  fit is within 2^-62.48 relatively; degree 10 would be 2^-58.12. A piece
  is indexed by y's exponent and top three fraction bits. From 1 to 8, the
  result is (x − 1)(x − 2)·h(x). Both factors are exact, and their product
  is an exact pair, so the result is relative however close x is to 1 or
  2 (+0 at them, tested before h so that no inexact is raised).
- **Below 1:** x(x − 1)h(1 + x) − log x. The piece is chosen from 1 + x
  (that rounding only picks the piece) and t = x − (m − 1). log x is a
  pair; |log x| is at least |lgamma(1 + x)|, so a fast two-sum combines
  them.
- **From 8, Stirling:** (x − ½)(log x − 1) + ½log 2π − ½ + S(1/x²)/x, with
  S of degree 6 (2^-57.59 absolutely, below 2^-60 once divided by x).
  x − ½ keeps its rounding error, which matters from 2^52.
- **Negative x in (−16, −1)** (D225): the recurrence
  lgamma(x) = lgamma(z) − log P, with x in (−n − 1, −n),
  z = x + n + 2 in (1, 2), exact, and P = |x (x + 1) ... (x + n + 1)|
  formed as a normalized pair (at most 16 exact pair products). Near every
  zero both terms are small: P is near 1, and z is near 1 or 2. So neither
  term cancels the other, and the result stays relative.
- **Other negative x**, from −1 and below −16: the reflection,
  log π − log(a |sin πf|) − lgamma(a), with a = −x and f the distance from
  a to the nearest integer (by masking a's fraction bits, not a rounding
  shift, so it is exact in every rounding direction: the strict `lgammaf`
  calls `lgamma`). The sine comes from the fast sine's table and kernel.
- **The ends:** NaN, the infinities, the poles (divide-by-zero), below
  2^-60 −log|x|, and from 2^60 the strict file's x(log x − 1).

### 32.3 What was tried first

The reflection everywhere, as musl does. Near the negative zeros its three
terms are about log n! and nearly cancel. With the terms at 2^-62
relatively (h of degree 11, the log kernel, the sine kernel, whose worst
case near j = 1 is 2^-60), the absolute error reached 2^-59.5 near −2.5 to
−7, and 2^-57.8 with 1.54 ulp from −8.

The strict core, tried as an accurate fallback, is no better there. Its
Stirling series runs in plain double, so its absolute error near the
negative zeros is about 2^-57, thousands of ulps of the tiny results
(Q62).

The product recurrence removed the cancellation itself, and with it the
fallback.

### 32.4 Measured

Against binary128 (`lgammaq`, checked against mpmath near the zeros):
4·10^6 random points in each range, and 4·10^5 points around each of the
34 negative zeros from −2.46 to −18, from the zero itself outwards to
2^-6.

| Range | worst |
|---|---:|
| [10^-300, 10^-10] | 0.5000 ulp |
| [0.001, 1/2], [1/2, 1] | 0.5002, 0.5008 |
| [1, 2], [2, 3], [3, 8] | 0.5026, 0.5029, 0.5032 |
| [8, 100], [100, 10^15], [10^15, 10^300] | 0.5016, 0.5000, 0.5000 |
| [−1, 0), [−2, −1], [−3, −2] | 0.5020, 0.5001, 0.5028 |
| [−8, −3], [−16, −8], [−100, −16], [−10^15, −100] | 0.5006, 0.5000, 0.5002, 0.5000 |
| within 2^-6 of each negative zero | at most 0.503 ulp, and 2^-60 absolutely where the result is below 2^-6 |

D219's promise holds as written: 1 ulp, and near the zeros 2^-60
absolutely. The soft-double cores run this file on their helpers until
P11.07d2: on armv7e-m and armv8-m.main it is 3 times the strict count
lower, but still above picolibc. On those cores the near-zero results
reach 2^-63 absolutely, inside the bound.

## 33. lgamma in fixed point on the soft-double cores (P11.07d2, D226)

P11.07d1's double file ran on the soft-double helpers at 5006 instructions
a call on armv7e-m, still above picolibc's 3901. `dp-fixed/fast/lgamma.c`
takes positive x into 64-bit integers. Negative x and the ends call the
double file's paths, which moved into their own files for that
(`__libc_fast_lgamma_positive.c`, `_negative.c`, `_ends.c`; `fast/lgamma.c`
is now their dispatch). They are not in the bench, and keep D225's
recurrence and its accuracy near the negative zeros.

Positive x needs only 1 ulp, because the zeros come from an exact product,
so the fixed-point pieces are fitted for absolute 2^-59.39 (degree 10),
not the double file's 2^-62.48.

- **From 1/2 to 8:** (x − 1)(x − 2)·h(x). Both factors are exact integers
  in units of x's last place; their product is exact in 128 bits, then
  its top 64 bits times h in Q64. h comes from 32 pieces of eight a binade
  (widths 1/16 to 1/2). The binade [1/2, 1) joins the pieces, since h's
  singularity at 0 is far enough at width 1/16; u is x's fraction bits
  below the top three.
- **Below 1/2:** −log x − x(1 − x)h(1 + x). 1 + x is in Q63, and its
  truncation only picks the piece and moves u. 1 − x is in Q64, and log x
  comes from the fixed log kernel (`__libc_dp_fixed_log_core`) plus
  e ln 2, in 128-bit Q64. The result is at least 0.57, so the absolute
  error stays relative.
- **From 8, Stirling:** (x − ½)(log x − 1) as a 128-bit product of Q63 and
  Q58, plus ½log 2π − ½ + S(1/x²)/x in Q64 (S of degree 6 in 64/x²,
  2^-60.38), with 1/x from `__libc_dp_fixed_reciprocal`.

Against binary128, 4·10^6 points a range:

| Range | worst |
|---|---:|
| [10^-300, 10^-10], [10^-10, 0.001], [0.001, 1/2] | 0.5001, 0.5003, 0.5012 ulp |
| [1/2, 1], [1, 2], [2, 4], [4, 8] | 0.5145, 0.5178, 0.5240, 0.5286 |
| [8, 16], [16, 1000], [1000, 10^15], [10^15, 10^18] | 0.5198, 0.5126, 0.5007, 0.5006 |

Instructions per call (fast, throughput):

| Core | `lgamma` before P11.07d | after d1 (on the helpers) | now | picolibc |
|---|---:|---:|---:|---:|
| armv7e-m, armv8-m.main | 17,366 | 5006 | 699.6 | 3901 |
| armv7-m | 17,908 | 5144 | 701.6 | — |
| armv6-m, armv6s-m | 47,083 | 14,761 | 3323 | — |
