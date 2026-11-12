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
