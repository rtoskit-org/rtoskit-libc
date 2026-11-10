# The fast build type: what relaxing libm buys (P11.01)

Research note for phase 11 ([`docs/plan/phase-11.md`](../plan/phase-11.md),
PLAN D162). It records the sources reviewed, what each guarantee the library
gives today costs and where, the proposed contract of a `fast` build type,
how the tests express it, and the gain expected per core with the method to
measure it. It contains no code from any other implementation (D1, D2).

## 1. The request and the starting point

The maintainer wants a fourth build type, `fast`, next to `debug`, `release`
(the size flavour) and `speed` (D73), in which the maths library relaxes its
accuracy and rounding guarantees so that its instruction counts come close to,
or beat, Arm Optimized Routines (AOR) and picolibc.

Today (README "Performance", `docs/perf/phase-3.md`, `docs/perf/phase-4.md`):

- On armv7e-m, picolibc takes 0.39 and newlib-nano 0.42 of our speed build's
  maths instructions (geometric mean, throughput); AOR, which replaces 15
  functions and the soft-double helpers, 0.95 overall and 0.93 over the cases
  it changes.
- Our float functions are correctly rounded (D98 on the double-FPU cores,
  D57/D125 elsewhere, where float-float is faithful and almost always
  correctly rounded); our double functions are below 0.53 ulp (D125). On the
  cores whose `<fenv.h>` is complete (x86_64, armv7e-m-dp, D44) both hold in
  every rounding direction, with invalid, divide-by-zero, overflow and
  deserved underflow raised exactly (D124's harness checks all of it).
- `math_errhandling` is `MATH_ERREXCEPT` (D45): the library **never sets
  `errno`** from `<math.h>`, in any flavour, today.

Speed-flavour throughput, instructions per call (`docs/perf/baseline-<arch>-speed.json`
of the current tree; picolibc from `docs/perf/phase-3.md`, AOR from `docs/perf/phase-4.md`):

| Function | armv6-m ours | picolibc | armv7e-m ours | picolibc | AOR | armv7e-m-dp ours | x86_64 ours |
|---|---:|---:|---:|---:|---:|---:|---:|
| `expf` | 375 | 2,659 | 160 | 76 | 681 | 172 | 237 |
| `logf` | 2,210 | 2,912 | 165 | 85 | 649 | 162 | 267 |
| `sinf` | 2,933 | 2,697 | 575 | 101 | 756 | 187 | 337 |
| `powf` | 1,222 | 9,269 | 367 | 234 | 1,478 | 406 | 643 |
| `erff` | 2,967 | 6,949 | 1,383 | 141 | 550 | 421 | 511 |
| `exp` | 10,317 | 5,013 | 3,813 | 2,085 | 1,090 | 169 | 230 |
| `log` | 16,964 | 5,631 | 6,184 | 2,207 | 1,245 | 150 | 252 |
| `pow` | 31,592 | 17,110 | 11,564 | 7,037 | 3,576 | 401 | 635 |
| `sin` | 2,833 | 5,001 | 765 | 1,767 | 5,660 ¹ | 165 | 308 |

¹ AOR has no `sin`; this is ours through AOR's soft-double helpers (phase 4).
Phase 3's picolibc figures predate phases 8 and 9 (`sin` and `sinf` were
since cut); P11.06 re-runs `bench-runner compare` on the current tree.

The gaps fall in three groups:

1. **Double on the cores without a double FPU** (armv6-m, armv6s-m, armv7-m,
   armv7e-m, armv8-m.main): `exp`, `log`, `pow` take 2–5 times picolibc's
   and 3–9 times AOR's instructions. The cause is the double-double cores
   (D47, D48) run on software double arithmetic: every TwoSum or exact
   product is 6–17 soft-double calls.
2. **Float on the single-precision FPU cores** (armv7e-m, armv8-m.main):
   `sinf`, `erff`, `expf`, `logf` take 2–10 times picolibc's. picolibc's
   float code (the SunPro routines, `newlib-obsolete-math` defaults to true)
   computes in float on the FPU at about 1 ulp; ours computes in float-float
   (`libc/math/sp-nearest`, D57) for correct rounding, and `sinf` reduces
   through `__libc_rem_pio2f`. AOR loses here (4×) because it computes float
   functions in double, which is soft on these cores.
3. **Float and double on the double-FPU cores** (armv7e-m-dp, x86_64): we are
   already close to AOR for double (AOR/ours 0.28–0.45 on armv7e-m-dp in
   phase 4, `exp` 169 now); the remaining cost is the double-double finish
   and, for float, the hard-case check (D98) and the rounding-mode save and
   restore.

## 2. Sources reviewed

- **Arm Optimized Routines**, `math/math_config.h`
  (<https://github.com/ARM-software/optimized-routines/blob/master/math/math_config.h>,
  read 2026-11-10) and our own measurement of the pinned commit
  ([`optimized-routines.md`](optimized-routines.md), `aor-accuracy-*.txt`):
  - `WANT_ROUNDING` (default 1): special-case results are right in the
    non-nearest modes (the header's example: `logf(1.0f)` must be +0 under
    `FE_DOWNWARD`, not −0). Setting it to 0 is allowed "if there is no fenv
    support or if the functions are only called in round-to-nearest mode".
    It does not make the main path honour the mode; even at 1, AOR states
    only about 1 ulp outside nearest (`TEST_ULP_NONNEAREST`), and phase 4
    measured `expf`, `exp2f`, `powf` 2 ulp upward.
  - `WANT_ERRNO` (default 0): set `errno` as ISO C specifies when
    `math_errhandling & MATH_ERRNO`. `WANT_ERRNO_UFLOW` (default
    `WANT_ROUNDING && WANT_ERRNO`): `ERANGE` on underflow to zero. D92
    builds AOR with `WANT_ERRNO=0`, matching our `MATH_ERREXCEPT`.
  - `TOINT_INTRINSICS`, `HAVE_FAST_ROUND`, `HAVE_FAST_FMA`: 1 on AArch64
    only; on M-profile the round-to-integer trick is the 1.5·2^52 shift
    (`0x1.8p52`), and without fast FMA the `log` family carries a second
    table (`tab2`) to keep the error at about 0.52 ulp.
  - Error bounds claimed (each routine's header comment, `TEST_ULP`):
    `expf`/`exp2f` 0.51, `logf` 0.82, `log2f` 0.76, `log10f` 0.80, `powf`
    0.90, `sinf`/`cosf` 0.56, `erff` 1.09; `exp`/`exp2` 0.51, `exp10` 0.52,
    `log` 0.52, `log2` 0.55, `pow` 0.54, `erf` 1.01 ulp. P4.05 measured the
    float ones 0.51–1.1 ulp and up to 3.9 % misrounded (`erff`).
  - Method worth noting (ideas only): a table of 2^(j/N) with N = 32 (float)
    or 128 (double), a short polynomial in double, one final rounding, the
    exponent added by integer arithmetic; `log` through a table of 1/c and
    log c indexed by the top mantissa bits, then a degree 4–6 polynomial;
    `pow` as exp(y · log x) with log x carried to about 2^-68 (an extra
    tail word) only where it pays; special cases routed by one unsigned
    comparison on the top bits of the argument.
  - The AOR README page could not be fetched (404 at the path tried); its
    contents are not used here.
- **picolibc** `doc/build.md`
  (<https://github.com/picolibc/picolibc/blob/main/doc/build.md>, read
  2026-11-10): `newlib-obsolete-math` (default true) keeps the old SunPro
  float and double routines; `-float` and `-double` variants pick per type;
  the "new" routines are ARM's (faster where double is in hardware, but the
  float ones compute in double), and the page says the old ones are more
  accurate, hence the default. `want-math-errno` (default false): without
  it, errors are reported through `<fenv.h>` only. The Ubuntu 1.8.6 package
  our comparison uses (D51) is built with the defaults, so its `sinf`,
  `expf`, `erff` are the SunPro float routines: argument reduction in float
  (Cody–Waite, Payne–Hanek above), a minimax polynomial in float, about 1 ulp.
- **newlib** libm (`newlib/libm/math`, `common`): the same SunPro lineage
  (`e_expf.c`, `k_sinf.c`), plus ARM's routines in `common/` selected by
  `__OBSOLETE_MATH`; `errno` through `_LIB_VERSION`/`__math_*` helpers in
  the newer code.
- **musl** (`src/math`): the double functions are ARM's (exp, log, pow, with
  musl's own wrappers), the float ones ARM's or FreeBSD's; no rounding-mode
  switching, `math_errhandling` is `MATH_ERREXCEPT` (no `errno`), the same
  choice as ours.
- **glibc**: its manual's "Errors in Math Functions" page
  (<https://sourceware.org/glibc/manual/latest/html_node/Errors-in-Math-Functions.html>)
  refused the fetch (403); nothing from it is used. What is relied on is
  common knowledge of its build: libm sets `errno` (`MATH_ERRNO |
  MATH_ERREXCEPT`), and since 2.28 its exp, log, pow, sinf, cosf, expf,
  logf, powf are ARM's routines. The relevant choice for us is what GCC
  does around it (next item).
- **GCC** `-f*-math` options
  (<https://gcc.gnu.org/onlinedocs/gcc/Optimize-Options.html>, read
  2026-11-10): `-ffast-math` = `-fno-math-errno -funsafe-math-optimizations
  -ffinite-math-only -fno-rounding-math -fno-signaling-nans
  -fcx-limited-range -fexcess-precision=fast`, and defines `__FAST_MATH__`.
  `-fno-trapping-math` lets the compiler assume no visible traps (and so
  speculate or if-convert floating-point operations); `-fno-signed-zeros`
  treats ±0 alike; `-ffinite-math-only` assumes no NaN or infinity. We
  compile libm with `-frounding-math -fsignaling-nans -fno-math-errno`
  (`libc/math/math.mk` and every layer's makefile; `sp-nearest` without the
  first two, because it runs only to nearest, D57). What a library may use
  is narrower than what a program may: `-ffinite-math-only` and
  `-fno-signed-zeros` would break the special cases the contract keeps
  (section 4), and `-fassociative-math` would break the error-free
  transformations the library still uses in its reductions.
- **CORE-MATH** (<https://core-math.gitlabpages.inria.fr/>, read
  2026-11-10), for contrast: correctly rounded in every IEEE mode (binary32
  checked exhaustively), but "does not necessarily" handle NaN, infinity or
  zero, nor set overflow, underflow or inexact; its binary32 functions are
  in glibc from 2.42. It is the opposite end: our release and speed builds
  already give CORE-MATH-like accuracy plus the special cases and
  exceptions; `fast` gives up the accuracy and keeps the special cases.
- **The standard** (`docs/standard/library-7.pdf`, C23 7.12p20, 7.12.1):
  `math_errhandling` is `MATH_ERRNO`, `MATH_ERREXCEPT`, both, or 0, and
  "shall not be 0 in a hosted implementation"; each function must not raise
  invalid, divide-by-zero or overflow "except to reflect the result of the
  function". Annex F (F.10, from knowledge of N3220; `annex-b.pdf` is the
  library summary and does not contain it) leaves it implementation-defined
  whether the functions honour the rounding direction, and unspecified
  whether and when they raise inexact; it does specify the special-case
  results and the invalid, divide-by-zero and overflow exceptions.
  **So a `fast` libm that is less accurate, ignores the rounding direction
  and raises inexact loosely stays conforming, with the same headers**, as
  long as it keeps the special cases and the three exceptions.

## 3. What each guarantee costs today, and where

| Guarantee | Where it costs (files, notes) | Cores that pay | Rough share of a call |
|---|---|---|---|
| **Double accurate to < 0.53 ulp** | The double-double cores: `libc/math/__libc_exp_core.c`, `__libc_log_core.c`, `__libc_log1p_core.c`, `__libc_expm1_core.c`, `__libc_pow_integer.c` (D47, [`libm-exp-log.md`](libm-exp-log.md)); `__libc_sin_core.c`, `__libc_cos_core.c`, `__libc_atan2_core.c`, `__libc_rem_pio2.c` (D48, [`libm-trig.md`](libm-trig.md)); `__libc_sinh_core.c`, `__libc_cosh_core.c`, `__libc_erf_series.c`, `__libc_erfc_*.c`, `__libc_lgamma_core.c`; the dd helpers in `libc/internal/math.h` (`__libc_dd_sum`, `__libc_dd_product`, Veltkamp split without FMA). | All, and heavily where double is software (soft-dp layer or libgcc): every dd operation is several soft calls. | 60–80 % of `exp`, `log`, `pow` on the soft-double cores (AOR, which finishes in double, takes 0.16–0.27 of ours on armv7e-m) |
| **Correctly rounded float, double-FPU cores** | `__libc_float_round` and `__libc_float_case` with each function's hard-case table (`libc/internal/math.h`, `libc/math/__libc_float_case.c`, `__libc_sinf_cases.c`, `__libc_cosf_cases.c`; D98, [`float-hard-cases.md`](float-hard-cases.md)); the float functions there are the double ones rounded (D45), so they inherit the double-double cost. | x86_64, armv7e-m-dp | The inline 29-bit test is a few instructions; the real cost is that `expf` runs `exp`'s dd core: `expf` 172 against `exp` 169 on armv7e-m-dp, where a float-only evaluation needs about 40–60 |
| **Correctly rounded float, other cores** | `libc/math/sp-nearest/` (D57, [`libm-float-exp.md`](libm-float-exp.md), [`libm-float-log.md`](libm-float-log.md), [`libm-float-trig.md`](libm-float-trig.md), [`libm-float-kernels.md`](libm-float-kernels.md)): float-float pairs (`struct __libc_ff`, `__libc_ff_product` with a Dekker split where there is no FMA), `__libc_rem_pio2f`, `__libc_ff_round_product`; the speed flavour's fixed-point paths with rounding tests and fallbacks (`libc/math/nofp/speed/`, D86–D88; `libc/math/dp-fixed/speed/__libc_trig_fixed_round*.c`, D136, D141). | armv6-m, armv6s-m, armv7-m, armv7e-m, armv8-m.main | `sinf` 575 on armv7e-m against picolibc's 101; `erff` 1,383 against 141 |
| **Directed rounding modes** | `__libc_math_nearest`/`__libc_math_restore` around every dd core (`fegetround`/`fesetround`: `VMRS`/`VMSR` on armv7e-m-dp, `STMXCSR`/`LDMXCSR` on x86_64; 32 files under `libc/math` call them); `__libc_math_tiny`/`__libc_math_tinyf` (the nudge that makes tiny-argument shortcuts round in the current mode, D125); `-frounding-math`, which stops GCC folding and reordering constant arithmetic; manual rounding of subnormal results through `FLT_ROUNDS` (`__libc_exp_scale`, D45). | x86_64, armv7e-m-dp only (the other cores have only `FE_TONEAREST`, D44, and their code is already nearest-only) | 10–30 instructions per call on the FPU cores (two mode reads, up to two writes, and the lost folding) |
| **Exceptions (invalid, divide-by-zero, overflow, deserved underflow)** | `__libc_math_raise_*`, `__libc_math_overflow`, `__libc_math_underflow`, `__libc_math_divbyzero`, `__libc_math_invalid` (`libc/internal/math.h`); `-fsignaling-nans`. Raised by arithmetic on the special-case branches that already exist for the result value. | x86_64, armv7e-m-dp (no-ops elsewhere, `FE_ALL_EXCEPT` is 0) | Nil on the common path: the branches are needed for the value anyway (`exp(x > 709.79)` must return +∞) |
| **`errno`** | None: `math_errhandling` is `MATH_ERREXCEPT` and libm never touches `errno` (D45). | — | 0 |
| **Subnormal results** | `__libc_exp_scale`, `__libc_exp2f_scale`: one rounding of a subnormal result through a bias (D47, D57); range checks on the exponent. | All | Only on the rare path; the common path pays one comparison |
| **NaN propagation and payloads** | `return x + x` (or `x + y`) for NaN inputs, which quiets a signalling NaN and keeps the payload on IEEE hardware and in our soft helpers. | All | 1–2 instructions |
| **Exact argument reduction for huge trig arguments** | `__libc_rem_pio2.c` (Payne–Hanek, 2/π table), `__libc_rem_pio2f` and the D136 integer reduction. | All | Only for |x| ≥ 2^20 (double) or the float equivalent; code size, not speed |

The conclusion: almost all of the gap is **accuracy** (the double-double and
float-float evaluation, and computing float through double on the cores
where double is software), a little is **directed rounding** on the two
cores that have it, and **essentially none is exceptions or `errno`**. On the
five cores the 0.39 figure is about, exceptions and directed modes cost
nothing today, because those cores have neither.

## 4. The proposed fast contract

One contract for every core and every function family. It keeps everything a
program can test cheaply for (special values, exceptions on error, ranges)
and relaxes what only an oracle can see (the last bit, the rounding
direction). The bounds are measured against the correctly rounded result,
in ulps of its binade (the subnormal ulp below the normal range), like D124.

### 4.1 What stays exactly as today

- **Headers unchanged** (D15): `math_errhandling` stays `MATH_ERREXCEPT`,
  `<fenv.h>` and `__STDC_IEC_559__` are as today. A program cannot tell a
  `fast` library from a `speed` one by its headers; only by the results.
- **Special values**, in every rounding mode: a NaN argument gives a quiet
  NaN (the payload of a NaN argument is kept where the function returns
  `x + x`, as today, but not guaranteed); the C23 F.10 results for ±∞ and ±0
  (`exp(−∞) = +0`, `log(±0) = −∞`, `log(1) = +0`, `sin(±0) = ±0`,
  `atan(±∞) = ±π/2` rounded, `pow`'s F.10.4.5 table, `hypot(±∞, NaN) = +∞`,
  ...); signed zeros wherever F.10 gives a sign; exact results F.10 or the
  function's definition fixes (`exp(0) = 1`, `pow(x, ±0) = 1`, `pow(1, y) = 1`,
  `cos(0) = 1`, `sqrt` of a square, `cbrt(±8) = ±2` is *not* required).
- **Overflow and underflow results**: a result beyond the range is ±∞ (or
  the largest finite value where the mode says so, on the cores with modes),
  never a large finite number; a result in the subnormal range is within
  the bound of the correctly rounded subnormal value (gradual underflow, no
  flush to zero); a result that underflows to zero has the right sign.
- **Range and symmetry**: functions whose mathematical range is bounded never
  return outside it after rounding (`|sin x| ≤ 1`, `|tanh x| ≤ 1`,
  `0 ≤ acos x ≤ π` rounded up, `erf` in [−1, 1], `erfc` in [0, 2]); odd
  functions are odd and even functions even (computed on |x| with the sign
  applied last).
- **Exact functions stay exact** in every mode: `sqrt`, `fma`, the rounding
  functions, `fmod`, `remainder`, `remquo`, scaling, `fabs`, `copysign`,
  `fmax`/`fmin`, `nextafter`, the narrowing functions, `logb`/`ilogb`,
  `frexp`, `modf`. They are cheap already and `fast` must not change them.
- **Argument reduction stays exact** for every argument (trigonometric
  functions of huge arguments are within the bound, not garbage). Its code
  is reached only for large |x|.
- **`errno` and exceptions as in every other build type** (the maintainer's
  decision, Q48, D162): `math_errhandling` stays `MATH_ERREXCEPT` and libm
  leaves `errno` as debug, release and speed do; every domain, pole and range
  error goes through the same `__libc_math_raise_*` and
  `__libc_math_overflow`/`underflow`/`divbyzero`/`invalid` helpers on every
  core, the soft-`<fenv.h>` cores included, so a program sees the same
  `errno` and the same flags (invalid, divide-by-zero and overflow exactly as
  F.10 says, a deserved underflow) in `fast` as in `speed`. Inexact stays
  unspecified (as F.10 allows), as today.

### 4.2 What is relaxed

| Family | Functions (float and double, `l` = double, D7) | Bound to nearest, fast | Today (speed) | Notes |
|---|---|---|---|---|
| exp | `exp`, `exp2`, `exp10`, `expm1`, `exp2m1`, `exp10m1` | < 1 ulp | float CR, double < 0.53 | AOR's are 0.51–0.52; a float table of 2^(j/32) and a degree 3–4 polynomial in float on the SP FPU cores |
| log | `log`, `log2`, `log10`, `log1p`, `log2p1`, `log10p1` | < 1 ulp | same | AOR 0.52–0.82; `log1p` near 0 must keep relative accuracy (no `log(1 + x)`) |
| pow | `pow`, `powr`, `pown`, `compoundn`, `rootn`, `cbrt` | < 1 ulp (D163: no 1.5 ulp allowance for `powf`) | same | `pow` needs log x to about 2^-(p+10) relative; the cheapest route on each core is the open design point of P11.03/P11.05 |
| trig | `sin`, `cos`, `tan`, `sincos`, `sinpi`, `cospi`, `tanpi` | < 1 ulp (`tan` < 1.5 ulp) | same | Cody–Waite reduction in working precision for small |x|, the existing exact reduction above |
| inverse trig | `asin`, `acos`, `atan`, `atan2`, the `pi` forms | < 1 ulp | same | |
| hyperbolic | `sinh`, `cosh`, `tanh`, `asinh`, `acosh`, `atanh` | < 1.5 ulp | same | built on the fast exp and log1p |
| error and gamma | `erf`, `erfc`, `lgamma`, `tgamma` | `erf` < 1.5 ulp; `erfc`, `lgamma`, `tgamma` keep speed's code in phase 11 | same | AOR's `erff` is 1.09; `lgamma` near its zeros needs high precision in any contract |
| everything else (`hypot`, `fdim`, `nan`, complex functions, C23 additions not listed) | | as speed | | inherited until a later step measures a gain worth a fast file |

- **Rounding: round to nearest only** (the maintainer's decision, D162):
  the fast functions compute as if rounding to nearest and do not read or
  change the mode. On x86_64 and armv7e-m-dp, in a directed mode, no error
  bound is promised; the special values, signed zeros, exceptions, ranges
  and exact functions of 4.1 still hold in every mode (AOR's
  `WANT_ROUNDING=1` behaviour: `log(1)` is +0 downward). On the other cores
  there is nothing to relax: they have only `FE_TONEAREST` (D44).
- **Compiler flags** for the `fast/` sources (decided, D163): drop `-frounding-math`
  (the functions no longer promise directed results, so GCC may fold
  constants); keep `-fsignaling-nans` and the default `-ftrapping-math`
  (the three exceptions must not be raised spuriously, 7.12.1p1); never
  `-ffinite-math-only`, `-fno-signed-zeros` or `-fassociative-math`.
- **No monotonicity promise** (neither AOR nor picolibc gives one; nor do
  we today, formally).

### 4.3 `errno` and exceptions (Q48, decided: as the other build types)

| Option | What changes | Cost | Gain | Conformance |
|---|---|---|---|---|
| **A. As today** (recommended): no `errno`, exceptions on the cores that have them | Nothing in the headers; the fast files keep the special-case branches they need for the values and raise by arithmetic there | 0 on the soft cores; on x86_64 and armv7e-m-dp a few instructions on rare branches only | — | Unchanged, Annex F still claimed |
| **B. No exceptions** in `fast` | The fast files skip `__libc_math_raise_*`; libm could build `-fno-trapping-math` | `math_errhandling` would have to become 0 (forbidden for a hosted implementation, 7.12p20) or lie; `__STDC_IEC_559__` would have to go; both are header changes per build type, which D15 and a toolchain's shared `include/` rule out | A handful of instructions on error paths of two cores; `-fno-trapping-math` lets GCC if-convert some branches (unmeasured, small) | Breaks the claim, or needs per-build-type headers |
| **C. Add `errno`** (`MATH_ERRNO | MATH_ERREXCEPT`) | Every domain, pole and range error writes the `thread_local` `errno` | A TLS access (`__aeabi_read_tp`) on each error path, a header change per build type | POSIX-style programs that test `errno` after maths | Conforming, but no flavour sets it today and nothing asked for it |

The options as weighed in P11.01; the recommendation was **A**. **The
maintainer decided that `fast` keeps `errno` and the floating-point
exceptions as the other build types do, on every core** (Q48, D162). Read as
written, that is A: no build type sets `errno` from `<math.h>` today (D45),
so `fast` does not either, and the exceptions are raised by the same helpers
everywhere. If the intent was option C, that every build type should start
setting `errno` (`MATH_ERRNO | MATH_ERREXCEPT`), that is a change to the
shared header and to debug, release and speed as well, and needs its own
decision superseding D45; it is not part of phase 11. **The maintainer
confirmed the first reading** (D163): `fast`, like every build type, does not
set `errno`.

## 5. How the tests express it

- **No test is weakened** (CLAUDE.md, D126): debug, release and speed run
  every test exactly as today. `fast` is a separate build type with its own
  documented contract, and its tests check that contract.
- **`test/unit/libm_vectors`** (D124): the bounds move from the expressions
  in `check_value` into a small table per flavour, `bounds.c` (today's
  values, byte-for-byte the same checks) and `fast/bounds.c` (section 4's
  table), selected by the test module's `subdir.mk` on `LIBC_FLAVOUR`, like
  `libc/math/subdir.mk` selects `speed/` files: no `#if` (D5, D77). The fast
  table gives a bound per function to nearest; in the directed modes the
  fast build is checked on the special values, exact functions and
  exceptions only (nearest-only rounding), and those keep exact-bit checks
  in every flavour. The exception checks run unchanged in `fast`. The
  vectors themselves (`vectors-{double,float}.inc`) are unchanged and need
  no MPFR at `make check`.
- **Tests that check the strict contract** (`float_hard`, `float_accuracy`,
  `libm_dense`'s near-halfway cases, parts of `libm_exp`, `libm_trig`,
  `libm_special`, `fpu_math`, `float_exact`): P11.02 audits each and gives it
  the same flavour split: the strict expectation in the existing source, the
  fast one in a `fast/` replacement file, so every assertion that is not
  about the last bit (special values, exceptions, ranges, symmetry) still
  runs in `fast`. The maintainer accepted this split (Q51, D163).
- **Measured accuracy stays visible**: the harness keeps printing the worst
  error per function, and P11.06 adds a sampled accuracy run of the fast
  build (16,384 arguments per function, as P4.05's `aor-accuracy`) to
  `docs/research/fast-math.md` and `docs/perf/phase-11.md`.
- **`regression_*` cases** for every bug the fast work finds (CLAUDE.md
  step 6), in `test/unit/conformance`, in every flavour where they apply.

## 6. Expected gain per core, and how to measure it

Estimates, from AOR's and picolibc's counts on the same cores and from how
much of each of our calls the dd/ff evaluation takes (section 3). They are
targets for P11.03–P11.05, not results. They assume Q48's decision: `errno`
and exceptions as in the other build types. That costs nothing on the
common path anywhere (the special-case branches are needed for the result
values, and the raise helpers are no-ops on the soft-`<fenv.h>` cores), and
a few instructions on the error branches of x86_64 and armv7e-m-dp; so the
figures below are unchanged by it, and no estimate counts a gain from
dropping exceptions or from `-fno-trapping-math`. The gain from
nearest-only rounding is counted only on x86_64 and armv7e-m-dp (no mode
save and restore, no `__libc_math_tiny` nudges, no subnormal rounding
through `FLT_ROUNDS`): about 10–30 instructions per call there, nothing
elsewhere.

| Core | float exp/log/pow | float trig, erf | double exp/log/pow | double trig | Main lever |
|---|---|---|---|---|---|
| armv7e-m, armv8-m.main (SP FPU, soft double) | `expf` 160 → ~70, `logf` 165 → ~80, `powf` 367 → ~200 | `sinf` 575 → ~120, `erff` 1,383 → ~150 | `exp` 3,813 → ~1,000, `log` 6,184 → ~1,200, `pow` 11,564 → ~3,500 | `sin` 765 → ~650 (drop the rounding test and fallback) | float evaluated in float on the FPU; double finished in double, AOR-style tables |
| armv7-m (no FPU, UMULL) | fixed-point already (D86–D88); 10–30 % from dropping rounding tests | `sinf` 2,499-class → ~1,500 | 3–4× fewer | 10–20 % | the same, on the soft helpers; 32×32→64 fixed point where it beats soft float |
| armv6-m, armv6s-m (no FPU, no UMULL) | `expf` 375 already beats picolibc; `logf` 2,210 → ~1,200, `powf` 1,222 → ~900 | `sinf` 2,933 → ~2,000 | `exp` 10,317 → ~4,000, `log` 16,964 → ~5,000, `pow` 31,592 → ~14,000 | `sin` 2,833 → ~2,400 | fixed-point double kernels (D136's style) with one final rounding and no fallback |
| armv7e-m-dp (double FPU) | `expf` 172 → ~60 (float-only evaluation, no hard-case table) | `sinf` 187 → ~80 | `exp` 169 → ~120, `pow` 401 → ~250 | `sin` 165 → ~120 | no dd finish, no mode save/restore |
| x86_64 (SSE2) | as armv7e-m-dp, relative | | `exp` 230 → ~120 instructions | | the same; measured but not the phase's gate |

Target for the phase exit (P11.06): on armv7e-m the fast build's maths
throughput geometric mean at or below picolibc's (≤ 0.39 of the speed
build), and at or below AOR's on every case AOR implements, on every Arm
core; the other cores recorded, not gated.

**Method** (all existing tools, D72, D92):

1. `make BUILD_TYPE=fast bench` against `docs/perf/baseline-<arch>-fast.json`
   (created in P11.02 as a copy of the speed figures, re-locked by every
   step with `BENCH_UPDATE=1`), guest instructions per call under the plugin
   qemu, throughput and latency.
2. `bench-runner compare` (picolibc 1.8.6, newlib-nano 4.4.0) and
   `bench-runner aor`, extended in P11.02 to take the fast build as the
   reference column (`--build-type fast`), giving "picolibc / fast" and
   "AOR / fast" per function and the geometric means of the README table.
3. Accuracy of each fast function by the libm vectors (gate) and the
   sampled run (record), on every core and, on x86_64 and armv7e-m-dp,
   every direction.
4. Size: `size-report --build-type fast budget --flavour fast` per
   reference program, and the members of each new `fast/` file recorded in
   `docs/size/<arch>.md`. Tables smaller than the dd cores' and no hard-case
   tables should make `math-mix` smaller than in speed; the budget is the
   speed flavour's rule (D73).

## 7. Design of the build type (for P11.02)

- `tools/makefiles/fast.mk`: the same flags as `speed.mk` (`-Os`, `-g3`,
  `LIBC_SPEED_CFLAGS := -O2`), `-DNDEBUG -DBUILD_TYPE_FAST`, `export
  LIBC_FLAVOUR := fast`. `BUILDS ?= debug release speed fast`. It builds into
  `build/<arch>/fast/` and installs into `images/<arch>/fast/` through the
  existing `BUILD_ROOT`/`INSTALL_ROOT` rules, with no change to them.
- **The fallback chain**: a new exported `LIBC_FLAVOURS`, the list of
  flavour directories in increasing priority (`size` → empty; `speed` →
  `speed`; `fast` → `speed fast`), set beside `LIBC_FLAVOUR`. Every
  `$(filter speed,${LIBC_FLAVOUR})` in `libc/libc.mk`, `libc/math/subdir.mk`,
  `libc/stdio/subdir.mk`, `libc/stdlib/subdir.mk` and the layers' makefiles
  becomes `$(filter speed,${LIBC_FLAVOURS})`, so `fast` is exactly `speed`
  until a `fast/` file exists. Each arch file adds
  `LIBC_STRING_KERNELS_fast := ${LIBC_STRING_KERNELS_speed}`.
- **`fast/` directories** (D77's rule, one level up): `libc/math/fast/` and
  `libc/math/<layer>/fast/` hold files that replace the generic, layer or
  `speed/` file of the same name in the fast flavour only. Precedence:
  `<layer>/fast/` > `fast/` > `<layer>/speed/` > `<layer>/` > generic. The
  same `subdir.mk` mechanism drops the shadowed names; D117's stale-object
  rule applies when a `fast/` file is added.
- **Why a build type, not a flavour archive** (D53, D54): a
  `libc-fastmath.a` linked ahead of `libc.a` would also work, since only
  libm changes, and would cost no extra build tree or `make check`
  configuration. The maintainer asked for a build type, which is also what a
  toolchain multilib or a parent build selects most simply (one
  `images/<arch>/fast/`), and lets a later step relax other modules
  (`strtod`'s and `printf`'s exact paths) without a new mechanism. The
  maintainer kept the build type (Q50, D163); the archive remains possible
  later.

## 8. Edge cases and test vectors the fast steps must keep

- ±0, ±∞, quiet and signalling NaN for every function; `pow` F.10.4.5's
  table (in `test/unit/libm_special` already); `atan2` of every signed-zero
  and infinity pair; `hypot(∞, NaN)`.
- The overflow and underflow thresholds of exp, exp2, exp10, expm1, sinh,
  cosh, pow (largest finite result, first ∞, smallest normal, first
  subnormal, last nonzero, first zero), for float and double.
- `log` family at 1 (+0 in every mode), at the smallest subnormal, at the
  largest finite; `log1p` at −1 (−∞, divide-by-zero), below −1 (NaN,
  invalid), at tiny x (returns x within the bound, not 0).
- Trig at multiples of π/2 near the reduction thresholds (2^20 double; the
  float path's switch to the integer reduction), at the D48 worst cases for
  reduction (the closest doubles to multiples of π/2), and at 2^1023.
- Odd-function symmetry and range checks over the libm vectors' random
  arguments.
- In the directed modes (x86_64, armv7e-m-dp): every special value above
  with its F.10 sign in each mode.

## 9. What building the type found (P11.02a)

Notes from implementing section 7 (D164); no outside code was consulted, the
mechanism is the project's own build system.

- **An archive keeps two members of the same name.** `ar rcs` given
  `a/x.o` and `b/x.o` stores both, and the link takes whichever the symbol
  index names first. The library has relied on every shadowing `subdir.mk`
  filtering the shadowed file, and on D117's documented delete of a stale
  object; a missed filter or a stale object linked the wrong code silently.
  With `fast/` files about to shadow generic, layer and `speed/` files, a
  check before archiving (`libc.unique`: the sorted member names have no
  duplicate) turns both mistakes into a build error, in every build type.
  It found the stale objects of the test below at once.
- **Which modules must filter.** Every module that holds a libm function a
  `fast/` file may replace: the generic `libc/math`, the `sp` and
  `sp-nearest` layers and the two `speed/` directories (`nofp/speed`,
  `dp-fixed/speed`). The other layers hold instructions (`fpv4-sp`,
  `fpv5-sp`, `fpv5-dp`, `x86_64`: `sqrt`, `fma`, rounding), the soft-float
  helpers or conversions, which the fast contract keeps exact; a layer that
  gains a shadowed file gets a `subdir.mk` then, and `libc.unique` stops the
  build until it does. The stems come from one place,
  `LIBC_MATH_FAST_STEMS` in `tools/makefiles/common.mk`, empty unless
  `LIBC_FLAVOURS` has `fast`, so the other build types parse to the same
  sources.
- **The precedence, tested and removed.** With temporary files
  `libc/math/fast/{exp,expf,sin,logf}.c` and
  `libc/math/sp-nearest/fast/logf.c`, each defining a marker symbol, the fast
  `libc.a` of armv7e-m and armv6-m took the generic fast `exp`, `expf`
  (over `sp-nearest` and `nofp/speed`) and `sin` (over `dp-fixed/speed`) and
  the layer's `logf` over the generic fast one; x86_64 and armv7e-m-dp,
  which have no `sp-nearest` layer, took the generic fast `logf`; no speed
  `libc.a` had a marker.
- **Comparing two build types' objects.** With `-g3`, the objects of the
  speed and fast builds differ in `.debug_macro` (the command line's
  `-DBUILD_TYPE_*`) and in the string table, which names each macro group
  `wm4.<file>.<line>.<hash>`. `size-report same` compares every section but
  the debug ones and their relocations, with those names masked. Without a
  `fast/` file every member of every architecture's fast `libc.a` is then
  equal to the speed one, in the same order; the debug, release and speed
  archives are byte-identical to the ones built before the change.
- **The bounds in the fast directed modes.** Section 4 promises no error
  bound in a directed mode, but the overflow result of the mode and the
  sign. `fast/bounds.c` checks exactly that for the relaxed families
  (`got == want` when the vector's direction overflows, the sign of the
  correctly rounded result otherwise), and the speed contract for the rest.
  To nearest, a representable exact result of a relaxed function is held to
  the bound, not to its bits (`cbrt(8) = 2` is not required, section 4.1),
  except a zero, whose value and sign are F.10's (`log(1) = +0`,
  `sin(±0) = ±0`); the nonzero exact results F.10 fixes (`exp(0) = 1`,
  `pow(x, ±0) = 1`) are `libm_special`'s, in P11.02b's audit.
