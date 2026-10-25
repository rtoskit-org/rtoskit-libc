# Research: single-precision helpers in Thumb-2 for Armv7-M

Decision: PLAN D109. Builds on D44 (soft `<fenv.h>`), D85 (integer float helpers on Armv6-M), D99–D101 (`soft-dp`) and D108 (`armv7-m`).

## The problem

On armv7-m every float operation is a run-time ABI call (RTABI 4.1.2). D108 took them from libgcc's Thumb-2 assembly, which is correctly rounded (38 million `fadd`/`fsub` cases checked against the double result, and `test/unit/softfp` for `fmul`/`fdiv`) but slow. Instructions per operation on armv7-m, the speed build's `softfloat` benchmark, with Arm Optimized Routines' `at32` helpers relinked in (`bench-runner aor`) as a measure of what Thumb-2 allows:

| Operation | libgcc | AOR |
|---|---:|---:|
| `fadd` / `fsub` | 59.3 / 58.2 | 38.1 / 37.6 |
| `fmul` | 39.9 | 36.9 |
| `fdiv` | 156 | 58.9 |
| `fcmp` | 42.9 | 21.3 |
| `i2f` | 29.9 | 20.9 |
| `l2f` | 32.9 | 27.3 |

The math benchmark took 0.71 of its instructions with AOR's helpers. `ddiv` is also slow on armv7-m (354 against 174 on armv7e-m): `soft-dp`'s C division computes its reciprocal with a float division, which is an FPU instruction on armv7e-m and a call to libgcc's `fdiv` on armv7-m.

## Sources reviewed

- **IEEE 754-2019** clauses 5.4.1 (arithmetic), 5.11 (comparisons), 6.2.3 (NaN propagation), 7.2 (invalid: ∞ − ∞, 0 × ∞, 0/0, ∞/∞), 7.3 (division by zero); round to nearest, ties to even.
- **Run-time ABI for the Arm Architecture** (Arm IHI 0043), 4.1.2, as quoted by compiler-rt and the LLVM reviews below:
  - `__aeabi_cfcmple` sets the flags as for an integer compare: Z for equal, C clear for less; unordered is C set, Z clear;
  - `__aeabi_cfcmpeq` gives the same flags (it differs only in signalling, which these cores do not have);
  - `__aeabi_cfrcmple` swaps the operands;
  - they preserve every core register except ip, lr and the flags; `cfrcmple` is read by every implementation reviewed as "swap r0 and r1, then `cfcmple`", so its r0 and r1 come back swapped in libgcc, compiler-rt and FreeBSD (<https://cgit.freebsd.org/src/tree/lib/libc/arm/aeabi/aeabi_asm_float.S>). `soft-sp` swaps them back, which satisfies either reading.

  Sources: <https://llvm.googlesource.com/llvm-project/+/890c4bece26e005cd9fa5511fe0efa7307794de5/compiler-rt/lib/builtins/arm/aeabi_cfcmp.S> and <https://reviews.llvm.org/D12089>.
- **The Armv7-M architecture.** `UDIV` (32/32 hardware divide), `UMULL`, `CLZ`, `IT` and the wide shifts are all base Armv7-M; `UMAAL` is not (D108).
- **Double rounding.** Figueroa, "When is double rounding innocuous?", and Roux, "Innocuous Double Rounding of Basic Arithmetic Operations" (<https://jfr.unibo.it/article/view/4359>; HAL <https://hal.ccsd.cnrs.fr/INRIA/hal-01091186v1>). Rounding the exact result of +, −, ×, ÷ to binary64 and then to binary32 equals rounding it to binary32 once, since 53 ≥ 2 · 24 + 2, subnormal results included. The test therefore takes the double operation on the widened operands, rounded to float, as the reference for those four. The conversions from 64-bit integers do not qualify (a 64-bit integer is not exact in a double), so their reference is computed in integers.
- **libgcc's Armv7-M objects** (GCC 16.2, `thumb/v7-m/nofp`), from `nm`. The groups a replacement must cover whole (D100):

  | Object | Symbols |
  |---|---|
  | `_arm_addsubsf3.o` | `fadd fsub frsub i2f ui2f l2f ul2f` (and the GNU names) |
  | `_arm_muldivsf3.o` | `fmul fdiv` |
  | `_arm_cmpsf2.o` | `cfcmpeq cfcmple cfrcmple fcmpeq fcmpge fcmpgt fcmple fcmplt` |

  Single-symbol objects that stay libgcc's are `f2iz`, `f2uiz`, `fneg` and `fcmpun`: they are as fast as AOR's (16–17 instructions for the conversions) and cannot clash. libgcc's own objects that call the replaced helpers are `_mulsc3.o`, `_powisf2.o`, `_divsc3.o` (already `soft-dp`'s), `_fixsfdi.o` (`soft-int`'s `f2lz`) and the fixed-point `_fract*`/`_satfract*` conversions, which are out of scope as in D100. So the layer also defines `__mulsc3` and `__powisf2`.

AOR and libgcc are only measured here; nothing of their code is used.

## Approach

A layer `libc/math/soft-sp`, listed on armv7-m only. Thumb-2 assembly for the arithmetic, the comparisons and the conversions; C for the two libgcc callers. NaN rules:
- a NaN operand gives that NaN, quietened; of two, the one with the larger magnitude, since the addition orders its operands by magnitude;
- an invalid operation gives the default NaN `0x7fc00000`;
- no exception flags (D44).

### Addition

`fsub` flips b's sign and falls into `fadd`; `frsub` flips a's.
- **Order.** The operands are ordered by magnitude, so |a| ≥ |b| and a decides the sign. A zero b, then an infinite or NaN a, are dealt with first (the math benchmark's `y * 0.0f + x` makes the zero case common).
- **Fast path: both normal.** From d ≥ 26 the result is a. Otherwise the significands are unpacked with the hidden bit at bit 31 (the low 8 bits empty); b is shifted right by d, the bits shifted out setting bit 0 (sticky). Addition and subtraction are separate branches. A carry out of the addition shifts back in through `RRX`, keeping the sticky bit, and may overflow to infinity. A subtraction normalises by `CLZ`, never below exponent 1, so a subnormal result comes out exactly; a zero difference is +0. Massive cancellation needs d ≤ 1, where no bit has been shifted out of the 32-bit word, so the normalisation is exact; from d ≥ 2 it shifts by one place at most, past the sticky bit only.
- **General path** (b subnormal, so a may be too). Significands with the hidden bit at bit 30; a subnormal is taken with exponent 1 and no hidden bit, which keeps both on one grid. The same alignment, addition (a carry into bit 31 shifts right once) and subtraction with `CLZ` normalisation.
- **Pack.** (sign | exponent − 1) · 2^23 plus the significand's top 24 bits takes the hidden bit into the exponent field; the first bit below is the guard bit, added through the carry, and the rest are sticky, cleared bit 0 on an exact tie (ties to even). A carry out of the significand reaches the exponent through the addition, and at the top it becomes exactly infinity's encoding.

### Multiplication

- **Product.** Significands with the hidden bit at bit 31, one `UMULL` for the 64-bit product. A product below 2^63 shifts left once.
- **Round.** The high word holds the 24-bit significand above the guard bit (bit 7). Its lower bits and the low word are sticky.
- **Below the normal range.** The high word, with the low word folded into its bit 0, shifts right by 1 − e (at most 31) with a sticky bit, and is rounded once with exponent field 0.
- **Subnormal operands** are normalised with `CLZ` first.

### Division

Significands with the hidden bit at bit 23; a below b doubles a, so a/b lies in [1, 2). Then four `UDIV`/`MLS` digit steps:
- the first gives 8 bits: (a · 2^7)/b, below 2^32;
- each later step divides the remainder times 2^8 (below 2^32, since the remainder is below b < 2^24), giving 8 bits.

That is 32 quotient bits with the leading one at bit 31, the same layout as the product's high word. A non-zero final remainder is the sticky bit, and the packing (overflow, subnormal shift, rounding) follows the same steps as the multiplication's, in a copy of its own.

### Comparisons

- **Unordered.** A NaN on either side gives C set and Z clear.
- **Zeros.** Two zeros are equal.
- **Order key.** Otherwise the operands map to unsigned keys whose order is the numbers' order: a non-negative x gives x | 2^31, a negative one gives ¬x. One unsigned `CMP` of the keys then sets Z and C as RTABI requires.
- **Registers.** One hidden routine, `__libc_softfloat_compare`, changes only ip and the flags: `cfcmple`/`cfcmpeq` branch to it, `cfrcmple` swaps r0 and r1 around a call, and the 0/1 forms call it and test the flags (`fcmpge`/`fcmpgt` with the operands swapped).
- **Without keys.** Rather than build keys, the routine compares the bit patterns directly: two non-negative operands as unsigned integers, otherwise with the operands swapped (two negatives order in reverse, and a negative is below a non-negative, which as unsigned is above). It needs the same flags and no spare register.

### Conversions

- `i2f`/`ui2f`: magnitude, `CLZ`, shift so the leading one is at bit 31, then the addition's packing with exponent 157 − `CLZ`.
- `l2f`/`ul2f`: a zero high word goes the 32-bit way; otherwise the high word, normalised by `CLZ` with the low word shifted in and the rest of it folded into bit 0 as sticky, then the same packing.

### `__mulsc3`, `__powisf2`

From C23 Annex G.5.2 and the definition of `powi`, mirroring `soft-dp`'s `__muldc3` and `__powidf2` for float.

## Edge cases

- **Signed zeros.**
  - x − x is +0, and −0 + −0 is −0 (6.3).
  - A product or quotient of a zero has the signs' difference.
  - A result rounded to zero from below the subnormal range keeps its sign.
- **Overflow.** It gives the signed infinity, the round-to-nearest result. A rounding carry from the largest finite value reaches infinity through the packing.
- **Subnormals throughout.** Operands and results, no flush to zero; ties on the subnormal grid.
- **Infinities.**
  - ∞ − ∞, 0 · ∞, 0/0 and ∞/∞ give the default NaN.
  - x/0 is a signed infinity.
  - x/∞ is a signed zero.
- **Comparisons.** −0 equals +0; a NaN compares unordered, including against itself.
- **Integer conversions.** The most negative `int32_t`/`int64_t` (magnitude 2^31, 2^63); `UINT64_MAX` rounds up to 2^64.

## Test vectors

A new `test/unit/softsp` runs on every Arm core, against whatever provides the helpers there: `soft-sp` on armv7-m, libgcc or `nofp-thumb1` elsewhere.
- **Random cases.** Several hundred thousand per operation from a fixed xorshift generator, mixing:
  - random bit patterns;
  - operands with close exponents, which carry or cancel;
  - ties;
  - subnormals;
  - products and quotients near both ends of the range.
- **Special values.** Every pair of ±0, ±min subnormal, ±max subnormal, ±min normal, ±1, ±max finite, ±∞ and a quiet and a signalling NaN.
- **References.**
  - The four operations: the double operation, rounded to float.
  - Comparisons: the double comparison.
  - `i2f`/`ui2f`: the conversion through double.
  - `l2f`/`ul2f`: an integer model.
  - NaN results only need to be quiet NaNs.
- **Flags.** The `cfcmp` forms are called from inline assembly (`cfcmp.c`) that reads Z and C and checks that r2 and r3 survive, and r0 and r1 too, except that `cfrcmple` may return them swapped.
- **Armv6-M.** libgcc's Thumb-1 `cfcmp` forms do not give RTABI's flags: they end with `cmp`/`cmn` of `__lesf2`'s result, so C is set for less and clear for equal. GCC never calls them on Thumb-1 (it calls the 0/1 forms), and the library does not define them there, so armv6-m and armv6s-m build `cfcmp-thumb1.c`, which checks nothing.
