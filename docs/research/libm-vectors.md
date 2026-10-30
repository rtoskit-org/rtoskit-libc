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
| Double and long double, to nearest | below 1 ulp of the exact value (D47, D48) |
| Float, to nearest, on cores where `<fenv.h>` is complete | correctly rounded (D98) |
| Float, to nearest, on the other cores | below 1 ulp (D57) |
| Exact or IEEE-correctly-rounded functions (`ceil` … `trunc`, `rint`, `nearbyint`, `fabs`, `fmod`, `remainder`, `remquo`, `copysign`, `fmax`, `fmin`, `fdim`, `sqrt`, `fma`, `logb`, the scaling functions, `nextafter`, `nexttoward`, `frexp`, `modf`) | exact, in every direction |
| `lrint`, `llrint`, `lround`, `llround` | exact in range; out of range (with the core's own `long`), invalid |
| `ilogb` | exact; `FP_ILOGB0`, `FP_ILOGBNAN`, `INT_MAX` |
| Float, directed | within 1 ulp of the correctly rounded result |
| Double, directed | within 3 ulp (libc-test's bound) |
| Exceptions (cores with `<fenv.h>` exceptions) | invalid, divide-by-zero and overflow exactly; a deserved underflow raised to nearest |

Each bound is the documented contract or tighter, and none is looser than
libc-test's. The harness prints a note with the largest error of every
function whose error exceeds half an ulp to nearest, or which has any error
in a directed mode. The measured accuracy therefore stays visible in the
`make check` log, not only whether the bound held.

## What the vectors found

Measured on x86_64 (all four directions):

- **`cbrt`:** 0.602 ulp to nearest, which is D47's "0.61 seen", and 2 ulp in
  the directed modes. The README's "below 0.53 ulp" for the double functions
  does not hold for `cbrt`.
- **Float functions in the directed modes:** `asinf`, `tanf`, `atanhf`,
  `sinhf` and `log1pf` at −FLT_MIN, and `tgammaf` at ±FLT_MIN, are 1 ulp from
  the correctly rounded result. On the double path, the double function's
  tiny-argument shortcut returns x unchanged, and the conversion to float then
  rounds that exact value. The README's "float, directed rounding: correctly
  rounded" does not hold there.
- **A bug, fixed:** on the soft-double cores (armv7-m, armv7e-m,
  armv8-m.main), `(float)` of a NaN came out negative whatever the double's
  sign. `__libc_softdouble_d2f` put the double's bits from 29 upwards into
  the payload unmasked, so an exponent bit landed on the sign. The
  `copysignf(x, NaN)` vectors caught it, and
  `test/unit/conformance` now has `regression_nan_conversion_sign`.
- **Undeserved underflow:** `pow(2, tiny)` and `log1p(DBL_MAX)` raise one,
  which F.10 allows.

Q36 records the two accuracy findings. The harness bounds are the documented
contracts, so it passes; the notes keep the numbers in view.

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
