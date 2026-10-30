# Research: libm conformance vectors from a correctly rounded oracle

Issue #95 (part of #96, removing libc-test). Decision: PLAN D124.

## Goal

Replace libc-test's 174 `math/*` tests with vectors the project generates
itself, so the libm gate no longer depends on libc-test's tables. The new gate
covers every function those tests covered, in float, double and long double,
with the exception flags, at bounds no looser than libc-test's.

## Sources reviewed

- **C23 7.12 and Annex F** (N3220): the special values of each function
  (F.10.x) and which exceptions it raises. F.10 leaves inexact unspecified
  for most functions, and also "undeserved" underflow. F.10.9.2 and F.10.9.3
  let `fmax(-0, +0)` and `fmin` return either zero. F.10.10.1 leaves invalid
  unspecified for `fma(inf, 0, NaN)`.
- **IEEE 754-2019**, 7.5: underflow is signalled when a result is tiny and
  inexact. Tininess may be detected before or after rounding; x86 SSE and Arm
  VFP make different choices.
- **MPFR** (<https://www.mpfr.org/mpfr-current/mpfr.html>, sections
  "Rounding", "Exceptions", "mpfr_subnormalize"): correctly rounded results
  in every rounding mode, with flags. To emulate an IEEE format, the exponent
  range is set to the format's, then `mpfr_check_range` and
  `mpfr_subnormalize` are applied. MPFR's own underflow flag means "below the
  smallest representable", not IEEE tininess, so IEEE underflow is derived
  separately (below).
- **libc-test's `math/` harness** (`src/common/mtest.h`), read only for the
  bounds and flag rules it applies, not for data:
  - to nearest, an error below 1.5 ulp, and exceptions equal apart from
    inexact;
  - in the directed modes, below 3 ulp, and exceptions equal apart from
    inexact and underflow.
- **CORE-MATH** (<https://core-math.gitlabpages.inria.fr/>) and the
  table-maker's-dilemma literature (`docs/research/float-hard-cases.md`):
  random arguments almost never land on hard cases, so the known hard float
  arguments are tested separately, by `test/unit/float_hard`.

## Approach

### The generator

`tools/libm-vectors/libm-vectors.c` is a host program, run by
`tools/scripts/libm-vectors`.

- **Arguments**, per function and precision:
  - 30 special values: ±0, ±∞, NaN, ±1, ±0.5, ±2, ±3, the smallest subnormal,
    the largest subnormal, the smallest normal, the largest finite value,
    π and π/2, overflow and underflow thresholds, and an integer plus 0.5;
  - 96 deterministic pseudo-random arguments. A third are any finite bit
    pattern, a third have a log-uniform magnitude in 2^-30 … 2^30, and a third
    lie in the function's own interval.
- **Two-argument functions** add 10 × 10 special pairs and 128 random pairs.
  A quarter of the pairs have close values, to exercise cancellation.
- **`fma`** adds a third of its cases with z ≈ −x·y. The scaling functions
  use exponents from −1100 to 1100. The rounding functions add the halfway
  points k + 0.5.
- **Each vector holds:**
  - the correctly rounded result in all four directions;
  - the offset of the exact value from the to-nearest result, in ulps;
  - the second outputs: the `frexp` exponent, the `modf` integral part, and
    the `remquo` quotient with its sign;
  - the flags of each direction.
- **Computing a result:**
  - The exact value is computed at 400 bits in MPFR's widest exponent range.
  - The result is computed at the target precision (24 or 53 bits) and
    exponent range, then put through `mpfr_check_range` and
    `mpfr_subnormalize`.
  - Exact functions (`frexp`, `modf`, `ilogb`, `logb`, `nextafter` and the
    integer conversions) are computed from their definitions.
- **Flags:**
  - invalid is expected for a NaN result from non-NaN arguments;
  - divide-by-zero and overflow come straight from MPFR;
  - IEEE underflow is expected when the result is inexact and both the exact
    value and the rounded result are below the smallest normal. An exact
    value below even MPFR's widest range counts as tiny;
  - where tininess before and after rounding disagree, underflow is marked
    "either". The cases F.10 leaves open are marked "either" too.
- **Determinism:** the output depends only on the generator and the MPFR
  version, which the files record. The committed vectors are
  `test/unit/libm_vectors/vectors-double.inc` and `vectors-float.inc`,
  9,467 lines each.

### The harness

`test/unit/libm_vectors` runs every function of the set in each precision:
float, double, and long double, which is binary64 (D7) and so shares the
double vectors. It runs in each rounding direction the core has, and checks
the following.

| What | Bound |
|---|---|
| Double and long double, to nearest | below 0.53 ulp of the exact value (the README's claim; D47 and D48 promise 1 ulp, D125) |
| Float, to nearest, on cores where `<fenv.h>` is complete | correctly rounded (D98) |
| Float, to nearest, on the other cores | below 1 ulp (D57) |
| Exact or IEEE-correctly-rounded functions (`ceil` … `trunc`, `rint`, `nearbyint`, `fabs`, `fmod`, `remainder`, `remquo`, `copysign`, `fmax`, `fmin`, `fdim`, `sqrt`, `fma`, `logb`, the scaling functions, `nextafter`, `nexttoward`, `frexp`, `modf`) | exact, in every direction |
| `lrint`, `llrint`, `lround`, `llround` | exact in range; out of range (with the core's own `long`), invalid |
| `ilogb` | exact; `FP_ILOGB0`, `FP_ILOGBNAN`, `INT_MAX` |
| Float, directed | correctly rounded (D125; the cores with directed modes are those whose float functions are correctly rounded) |
| Double, directed | within 3 ulp (libc-test's bound) |
| Exceptions (cores with `<fenv.h>` exceptions) | invalid, divide-by-zero and overflow exactly; a deserved underflow raised to nearest |

Each bound is the documented contract or tighter, and none is looser than
libc-test's. The harness prints a note with the largest error of every
function whose error exceeds half an ulp to nearest, or which has any error
in a directed mode. The measured accuracy therefore stays visible in the
`make check` log, not only whether the bound held.

## What the vectors found

Measured on x86_64 (all four directions) and the Arm cores. Each finding is
fixed (D125, resolving Q36), and `test/unit/conformance` keeps a regression
case for it.

- **`cbrt`:** 0.602 ulp to nearest (D47's "0.61 seen"), against the README's
  "below 0.53 ulp", and 2 ulp in the directed modes. The directed error had
  its own cause: `cbrt` computed |cbrt(x)| in the caller's direction and
  then applied the sign, so upward and downward were swapped for a negative
  x.
- **Float functions at −FLT_MIN, rounding downward:** `asinf`, `tanf`,
  `atanhf`, `sinhf` and `log1pf` returned −FLT_MIN instead of its neighbour
  away from zero. Their tiny-argument helper, `__libc_math_tinyf`, asked
  whether the nudge pointed toward zero with
  `signbit(x) != signbit(direction)`. `signbit` is only zero or nonzero
  (C23 7.12.3.6), and ours gives 0x80000000 for a float and 1 for a double,
  so every negative x looked nudged toward zero.
- **`tgamma` of a tiny argument:** for |x| < 2^-54 it returned 1/x. That
  drops −γ (Euler's constant), which is below half an ulp of 1/x and so
  matters only in the directed modes. It was 1 ulp off there, and
  `tgammaf(±FLT_MIN)` inherited the error.
- **`pow(2, 2^-1074)`, rounding downward:** gave 1 − 2^-53. `pow` formed
  y·log|x| in the caller's direction, and the product of two subnormal-scale
  values rounded away from its sign.
- **`(float)` of a NaN** on the soft-double cores (armv7-m, armv7e-m,
  armv8-m.main) came out negative whatever the double's sign:
  `__libc_softdouble_d2f` put the double's bits from 29 upwards into the
  payload unmasked, so an exponent bit landed on the sign. The
  `copysignf(x, NaN)` vectors caught it.
- **Undeserved underflow:** `log1p(DBL_MAX)` raises one, which F.10 allows.

### The fixes

- **`cbrt`** truncates its Newton estimate (good to about 2^-40) to 26
  bits, so that t² is exact. Splitting t² = a + b, with a holding 27 bits,
  makes t³ = a·t + b·t exactly as two doubles, so the residual
  r = y − t³ comes out exact; y − a·t is exact by Sterbenz's lemma. One
  Halley step, t + t·r/(2t³ + y), then leaves about 2^-78 relative error,
  below 0.5 + 2^-25 ulp. Halley's method is cubically convergent, and
  computing on an exactly computed residual is the usual last correction of
  an iterative root (Brent and Zimmermann, *Modern Computer Arithmetic*,
  chapter 4, <https://members.loria.fr/PZimmermann/mca/mca-cup-0.5.9.pdf>).
  No step needs round to nearest. The sign goes on before the one rounding
  of t + d, so the caller's direction applies to the signed result. A first
  version used a double-double product and switched to round to nearest; it
  was 75 % slower on x86_64 and added about 580 bytes on the cores without
  fma. A Python model of the algorithm, in double arithmetic with the
  error measured against a 60-digit reference, saw at most 0.499997 ulp
  over 200,000 random y in [1, 8), with no misrounding.
- **`__libc_math_tinyf`** compares `!signbit(x) != !signbit(direction)`.
- **`tgamma`**, for 2^-1024 < |x| < 2^-54, takes 1/x as a double-double in
  round to nearest. It divides by x · 2^64, so the divisor is normal and
  Dekker's split of the quotient stays finite. It then adds −γ to the low
  part and rounds once in the caller's direction. At or below 2^-1024 the
  result overflows, and 1/x already does that correctly.
- **`pow`** forms y·log|x| in round to nearest, as its cores do. When
  |y·log|x|| < 2^-60 (the product may even have underflowed to zero), the
  result is 1 ± 2^-60, with the sign taken from y and log|x|, rounded in the
  caller's direction.

With these, the harness's bounds rise to the README's claims: double below
0.53 ulp to nearest, and float correctly rounded in every direction where
the core has directed modes.

## Edge cases

- NaN results are compared by `isnan`, because the sign and payload of a NaN
  are unspecified. Zeros are compared with their sign, except where F.10 says
  either zero may be returned.
- On a 32-bit `long`, `lrint` and `lround` treat any value beyond 2^31 as out
  of range. The harness takes the range from the core's own `LONG_MAX`.
- `remquo` checks only the sign and the low three bits of the quotient, which
  is all C23 7.12.10.3 requires.
- Cores without exceptions (`FE_ALL_EXCEPT == 0`) skip the flag checks, and
  cores without the directed modes run to nearest only.

## Size and time

About 37,000 calls to nearest and 111,500 in all four directions. They take
0.02 s on x86_64 and a few seconds under `qemu-arm`. The source is 2.6 MB of
generated vectors.
