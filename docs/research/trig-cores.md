# Research: the double sin and cos cores below 0.53 ulp

Step: P11.07m. Decisions: PLAN D228 (supersedes D48's cores; the reduction stays).
Earlier note: [`libm-trig.md`](libm-trig.md).

## The report and what was actually wrong

The report: the strict double `sin` returned `0x1.6e1cbd6c025f6p-1` for
`0x1.bd0a14269acdfp+707` (0.593 ulp; the correctly rounded result ends in `…f7`),
and `tan(0x1.f3605e569c782p+525)` returned `-0x1.fa789308d09f6p-1` (0.606 ulp). On a
sample of 2·10^7 arguments in [2^20, 2^1020], 0.31 % of `sin` and 0.41 % of `tan`
results were misrounded. The suspect was the Payne–Hanek reduction.

The reduction is not the cause. Its r for both arguments matches the exact
x − nπ/2 (mpmath at 4000 bits) to 2^-107 and 2^-115 relative, with the right
quadrant. The same sampling below 2^20 (Cody–Waite) was just as bad: 0.32 % of
`sin`, 0.30 % of `cos` and 0.42 % of `tan` misrounded, by up to 0.57 ulp.
The reductions only decide which core runs; every misrounded result came from
`__libc_sin_core` or `__libc_cos_core`. Those cores are shared by `sin`, `cos`,
`tan`, `sincos`, `sinpi`, `cospi`, `tanpi`, `lgamma` (its reflection),
`cpow` and the speed flavour's double paths.

The cores were measured on their own, with r uniform in [−π/4, π/4] and its low
part uniform in ±½ ulp, against MPFR (§Measurement):

| | core error (ulp of the result) | misrounded |
|---|---|---|
| old `__libc_sin_core` | 0.0191 | 0.096 % |
| old `__libc_cos_core` | 0.1000 | 0.554 % |

There were three error sources, all near |r| = π/4 where the results sit in [½, 1)
and an ulp is 2^-53:

1. **The low part's derivative.** cos(x + y) = cos x − y sin x − …, and the core
   used −y·x for the middle term, dropping y·x³/6: up to 2^-54 · 0.081 = 2^-57.6,
   0.04 ulp. The sin core used y·(1 − x²/2) for y cos x, dropping y·x⁴/24, up to
   2^-60, 0.008 ulp.
2. **cos's tail rounded in double.** Everything past 1 − x²/2 (the r⁴/24 term
   reaches 0.016 = 2^-6) was a plain double sum. Its roundings (x² is rounded
   before it is squared, then the Horner steps and the product) cost up to about
   5 · 2^-53 · 2^-6 = 2^-56.7, 0.03–0.06 ulp, and adding it into the low word,
   whose magnitude it set, rounded at 2^-60 more.
3. **sin's tail.** x⁵/120 (up to 2^-8.6) was in the double tail too, with the
   same squared rounding of x²: 0.01 ulp.

The fixed bound the header claimed ("to about 2^-60") was not met by either core.

## Sources reviewed

- FreeBSD msun `k_cos.c` and `k_sin.c`
  (<https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/k_cos.c>,
  <https://raw.githubusercontent.com/freebsd/freebsd-src/main/lib/msun/src/k_sin.c>),
  and musl's `__cos.c`, which derives from them
  (<https://git.musl-libc.org/cgit/musl/plain/src/math/__cos.c>). These are
  the fdlibm kernels for results within 1 ulp, finishing in double. They use the same
  first-order handling of the low part we had: cos(x+y) ≈ cos x − x·y and sin(x+y) ≈ sin x + (1 − x²/2)·y.
  cos recovers the rounding of 1 − x²/2 exactly and adds everything else as one small
  term. The stated polynomial bounds are 2^-58, with fitted (not Taylor) coefficients
  of degree 13–14. That is good enough for 1 ulp but not for 0.53, which is the
  level our cores had.
- glibc `sysdeps/ieee754/dbl-64/s_sin.c`
  (<https://raw.githubusercontent.com/bminor/glibc/master/sysdeps/ieee754/dbl-64/s_sin.c>):
  a table of sin and cos at grid points, each with a tail, plus short
  Taylor-like corrections and the argument's low part as a separate term. The
  header claims about 0.55 ulp, with branch bounds from 0.501 to 0.548. The table is
  several kilobytes, too much for this library's size flavour.
- Arm optimized-routines' AdvSIMD `cos.c`
  (<https://raw.githubusercontent.com/ARM-software/optimized-routines/master/math/aarch64/advsimd/cos.c>):
  states 3.3 ulp worst. It is only a contrast: AOR's scalar double trig are not
  part of the D92 comparison.
- J.-M. Muller, *Elementary Functions*, 3rd ed., chapter 3 (double-word
  arithmetic) and the half-angle identity cos r = 1 − 2 sin²(r/2).

## Trade-offs

- **Keep the series, carry more of cos in double-double (chosen for the speed
  flavour).** x⁴ needs one more exact product (a Veltkamp split on every core
  without fma). 1/24 can't multiply it exactly, so it is taken as 1/32 + 1/128 +
  1/512 + 1/1536: the first three are exact scalings, summed by fast two-sums, and
  only x⁴/1536 (below 2^-12) is rounded. Adding x⁴/384 rounded would leave
  0.002 ulp, and rounding the whole x⁴/24 0.008. The tail from x⁶ needs six fitted
  terms (2^-74). y·sin x is taken as y·x(1 − z/6 + z²/120). That is two exact
  products in place of the half angle's four, but more code than the old core:
  measured below.
- **A table (glibc).** Kilobytes of data, against D51's size priority.
- **Half angle (chosen for cos).** cos r = 1 − 2 sin²(r/2). Halving is exact,
  |r/2| ≤ 0.4, and there the sin core's double tail is below 2^-13, so its roundings
  are near 2^-66. The square is one `__libc_dd_multiply`, and 1 − 2s² is an exact
  two-sum (2s² ≤ 0.3). The cos core becomes a short wrapper: −104 to −156 bytes. The
  cost is a nested call, and with it stack depth (below).
- **For sin, move x⁵/120 into the double-double part.** r − r³(1/6 − r²/120)
  puts the bracket in a two-sum. Rounding r²/120 as a whole still cost 2^-61.5
  (0.004 ulp), enough to misround `tan(0x1.6133c93d93e1ep+2)`, a hard case of
  `test/unit/libm_dense/near_double.inc` 0.001 ulp from a midpoint. So the bracket takes 1/120 as
  1/128 + 1/1920: r²/128 is exact (also its low word) and goes into the two-sum,
  and only r²/1920, at most 2^-11.5, is rounded (2^-65.6 after the cube). The tail left in double starts at r⁷/5040 (below 2^-14.7). A
  near-minimax fit then needs five terms where the old Taylor tail had seven,
  which pays for most of the added work.
- **The low part:** y·cos x through 1 − x²/2 + x⁴/24 (its next term is 2^-65 at
  most). For cos the half angle carries y exactly, so no derivative is needed.

## Algorithm (as implemented)

- `__libc_sin_core(r)`, |r| ≤ 0.8: z = x², with x²'s exact product and
  x³ = (x²)·x as double-double. bracket = 1/6 − x²/128 by a fast
  two-sum of 1/6 and the exact square's high word over 128, whose low word takes 1/6's low
  word, −(x²'s low word)/128 and −z/1920, term = x³ · bracket in double-double, then
  x − term.hi exactly. The double tail is x·z·z²·q(z) with q's five coefficients
  fitted here (Lawson-weighted least squares, the weight x⁶ for an error relative
  to x). The fit's error is 2^-67.0 relative on |r| ≤ 0.8. y·(1 − z/2 + z²/24) is
  added to the low word.
- `__libc_cos_core(r)`, generic (the size flavour): s = `__libc_sin_core`(r/2),
  q = s·s in double-double, then 1 − 2q.hi (exact) and its low word − 2q.lo.
- `libc/math/speed/__libc_cos_core.c` (the speed and fast flavours, D77): 1 − h/2
  for the exact square h + l, then h²/32, h²/128 and h²/512 by fast two-sums, then
  h²/1536 plus the tail by one more. The low word collects the two-sums' errors,
  −l/2, (the low word of h² + 2hl)/24 and −y·x(1 − z/6 + z²/120). The tail is z³·p(z)
  with p's six coefficients fitted the same way (absolute error, 2^-74.0):
  `-0x1.6c16c16c16c14p-10`, `0x1.a01a01a019530p-16`, `-0x1.27e4fb765ed98p-22`,
  `0x1.1eed8cf977dc8p-29`, `-0x1.9393843b348bbp-37`, `0x1.aae026013157cp-45`.

The q coefficients (z = x², tail = x⁷·q(z)):
`-0x1.a01a01a019b60p-13`, `0x1.71de3a546ec13p-19`, `-0x1.ae6453fda193dp-26`,
`0x1.61215b2e295d9p-33`, `-0x1.aafc87d0f6ea0p-41`.

## Measurement

The cores alone, 2·10^6 random r (x86_64, no fma):

| | core error (ulp) | final error (ulp) | misrounded |
|---|---|---|---|
| old sin core | 0.0191 | 0.5140 | 0.096 % |
| old cos core | 0.1000 | 0.5776 | 0.554 % |
| sin core, bracket 1/6 − z/120 rounded whole | 0.0040 | 0.5027 | 0.024 % |
| sin core, bracket from an exact (long double) z/120, for comparison | 0.0004 | 0.5002 | 0.002 % |
| sin core, as above but without the y·x⁴/24 term | 0.0111 | 0.5101 | 0.081 % |
| **new sin core** (1/128 + 1/1920) | 0.0008 | 0.5003 | 0.004 % |
| **new cos core**, half angle (on the new sin core) | 0.0001 | 0.5000 | 0.001 % |
| cos series, x⁴/384 rounded whole | 0.0038 | 0.5017 | 0.016 % |
| **new cos core**, series (speed flavour) | 0.0016 | 0.5010 | 0.007 % |

The functions through the library (x86_64, MPFR reference), with arguments random
in exponent and significand and both signs. Release (half-angle cos):

| | range | samples | worst (ulp) | above 0.53 | misrounded | before |
|---|---|---|---|---|---|---|
| `sin` | [2^20, 2^1020] | 2·10^7 | 0.5005 | 0 | 0.0024 % | 0.593 ulp, 0.31 % |
| `cos` | [2^20, 2^1020] | 2·10^7 | 0.5005 | 0 | 0.0022 % | 0.588 ulp, 0.35 % |
| `tan` | [2^20, 2^1020] | 2·10^7 | 0.5006 | 0 | 0.0044 % | 0.606 ulp, 0.41 % |
| `sin` | [2^-1, 2^20] | 4·10^6 | 0.5003 | 0 | 0.0025 % | 0.570 ulp, 0.32 % |
| `cos` | [2^-1, 2^20] | 4·10^6 | 0.5004 | 0 | 0.0023 % | 0.562 ulp, 0.30 % |
| `tan` | [2^-1, 2^20] | 4·10^6 | 0.5005 | 0 | 0.0041 % | 0.569 ulp, 0.42 % |

("before" is the old cores on 3·10^5 samples of the same ranges.)

With the speed flavour's cos core, on the same samples: `sin`, `cos` and `tan` at
most 0.5011, 0.5010 and 0.5015 ulp beyond 2^20 (0.5008, 0.5008 and 0.5018 below),
none above 0.53, and 0.005 % of `sin` and `cos` and 0.008 % of `tan` misrounded.

## Speed

Guest instructions per call (release, `make bench`), old → new. The old cores'
speed came with their errors, so every accurate design is slower:

| | armv7e-m half angle | armv7e-m series | armv6-m half angle | armv6-m series | x86_64 half angle | x86_64 series |
|---|---|---|---|---|---|---|
| `sin` | +46.1 % | +27.9 % | +46.0 % | +27.9 % | +30.8 % | +17.7 % |
| `cos` | +47.0 % | +28.4 % | +46.4 % | +28.0 % | +30.4 % | +17.4 % |
| `tan` | +53.3 % | +32.5 % | +50.0 % | +30.3 % | +44.5 % | +22 % |
| `sincos` | +56.8 % | +34.4 % | +55.1 % | +33.3 % | +42.6 % | +22 % |

The series cos costs about 750 more bytes on the soft-double cores (images +240 to
+740 bytes against the old baselines, against −48 to +4 for the half angle). So the
maintainer chose the half angle for the size flavour and the series for the speed
flavour (D228). The speed flavour's own gate figures are in `docs/perf/<arch>.md`.

## Edge cases

- |r| > π/4 slightly: the cores accept |r| ≤ 0.8, and the fit covers it.
- r/2 for a subnormal r.lo loses at most half of its last bit, far below an ulp of
  cos r ≈ 1.
- `sincos` stays bit-identical to `sin` and `cos`, since both still call the same
  cores on the same r.
- tan = sin/cos in either order: both cores are now accurate on the whole range.

## Size and stack

Release, against the old baselines (`docs/size/<arch>.md`, P11.07m): `__libc_sin_core`
+56 (armv7e-m-dp) to +140 bytes (the Armv7-M cores), `__libc_cos_core` −104 to
−156. `libc.a` and the images are 2–48 bytes smaller everywhere except Armv6-M and
Armv6S-M, where they are 4 bytes larger. The nested call deepens the stack of `sin`,
`cos` and `tan` by 68 (armv7e-m-dp) to 144 bytes (armv7e-m, armv8-m.main); on x86_64
the depth is unchanged. The maintainer accepted the growth for the accuracy, and the
baselines were rewritten. Every image stays within `docs/size/budget.json`. The series cos
put `math-mix` 692–736 bytes over its speed and fast budgets on armv6-m, armv6s-m
and armv7-m (both flavours) and on armv7e-m and armv8-m.main (fast). Those eight
entries of `docs/size/budget.json` were re-locked, and nothing else changed.

## Tests

- `test/unit/conformance`: `regression_trig_cores`, seven arguments the old cores
  misrounded (the two reported, two more beyond 2^20, three below it), compared to
  MPFR's correctly rounded results through `libm_exact` (strict: equality; fast:
  its 1-ulp contract).
- The `libm_vectors` and `libm_trig` suites are unchanged and pass.
