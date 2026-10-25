# Research: an Armv7-M architecture (Cortex-M3, Cortex-M4 without FPU)

Decision: PLAN D108.

## The gap

The library builds for Armv6-M (Thumb-1, soft float) and for the Thumb-2 cores that have the DSP extension and an FPU (armv7e-m, armv7e-m-dp, armv8-m.main). A Cortex-M3, or a Cortex-M4 or M33 built without its FPU, has Thumb-2 but no FPU. Until now such a core had to link `armv6s-m`, which runs but gives up what Thumb-2 brings:
- hardware divide (`SDIV`/`UDIV`);
- `IT` blocks, 32-bit encodings with wide immediates, and all registers in most instructions;
- the speed flavour's Thumb-2 string kernels;
- libgcc's hand-written Thumb-2 soft-float helpers.

`armv7-m` (`-mthumb -march=armv7-m -mfloat-abi=soft`) fills it. GCC 16.2 ships the `thumb/v7-m/nofp` multilib, so libgcc is there.

## Sources reviewed

- **The Armv7-M Architecture Reference Manual** (Arm DDI 0403), via the LLVM discussions below. Armv7E-M is Armv7-M plus the DSP extension: the saturating and SIMD instructions (`UADD8`, `SEL`, `QADD`, ...) and `UMAAL`, `SMLAL*` variants.
- **LLVM review D25890**, <https://reviews.llvm.org/D25890>: "UMAAL is a DSP instruction and it is not available on thumbv7m (Cortex-M3) and thumbv6m"; the reviewer asks for a test that Cortex-M4 (v7-M plus DSP) does generate it.
- **LLVM commit on the DSP feature** (mirror at <https://repo.hca.bsc.es/gitlab/rferrer/llvm-epi/-/commit/e7d9505c08017b6952c5f32015d185b5819094bc>): DSP instructions are an optional extension; with them the core is Armv7E-M (Cortex-M4), without them Armv7-M (Cortex-M3).
- **Arm's "Armv6-M vs Armv7-M" blog**, <https://developer.arm.com/community/arm-community-blogs/b/architectures-and-processors-blog/posts/armv6-m-vs-armv7-m---unpacking-the-microcontrollers>: Armv7E-M adds saturating and SIMD instructions to Armv7-M.
- **libgcc's `config/arm/ieee754-sf.S` and `ieee754-df.S`** (<https://gnu.googlesource.com/gcc/+/0a77c7033ae4ed05a2f7e78600522610a8d82225/libgcc/config/arm/ieee754-sf.S>): the add, subtract and reverse-subtract entry points share one routine under `L_arm_addsubsf3`, followed by the integer-to-float conversions.
- **The toolchain itself** (GCC 16.2, `thumb/v7-m/nofp/libgcc.a`), checked with `nm`. One object per group:

  | Object | Symbols |
  |---|---|
  | `_arm_addsubsf3.o` | `fadd fsub frsub i2f ui2f l2f ul2f` |
  | `_arm_muldivsf3.o` | `fmul fdiv` |
  | `_arm_addsubdf3.o` | `dadd dsub drsub f2d i2d ui2d l2d ul2d` |
  | `_arm_muldivdf3.o` | `dmul ddiv` |
  | `_arm_cmpsf2.o`, `_arm_cmpdf2.o` | the five comparisons each |
  | one each | `f2iz f2uiz d2iz d2uiz d2f dcmpun fcmpun`, the 64-bit conversions |

  The assembler rejects `umaal r0, r1, r2, r3` for `-march=armv7-m` ("selected processor does not support ... in Thumb mode").

## What each piece of the library does on Armv7-M

| Piece | Armv6-M (today) | Thumb-2 with FPU and DSP (today) | Armv7-M (chosen) |
|---|---|---|---|
| C code | Thumb-1 | Thumb-2 | Thumb-2 (`-march=armv7-m`) |
| `<fenv.h>`, `LIBC_FPU` | soft, `nofp` | soft or full | soft, `nofp` |
| float helpers | `nofp` layer for `fsub fmul fdiv f2uiz`, libgcc for the rest | hardware | libgcc's Thumb-2 assembly, all of them (since D109 the `soft-sp` layer, `docs/research/softfloat-thumb2.md`) |
| double helpers | `nofp` (`dsub d2uiz`), libgcc's per-function C | `soft-dp` layer | `soft-dp`, with a DSP-free `dmul` |
| integer `sqrtf`, fixed-point speed paths | `nofp`, `nofp/speed` | — | `nofp`, `nofp/speed` |
| string kernels, size | `thumb1` | `thumb1` | `thumb1` |
| string kernels, speed | `thumb1-speed thumb1` | `thumb2 thumb1` | `thumb2 thumb1`, without the DSP `strchr` |
| `setjmp` | Thumb-1 | Thumb-2 plus `d8-d15` | Thumb-2, no VFP |

### Why the Armv6-M float helpers cannot come along

On Armv6-M libgcc's helpers are one function per object, so the `nofp` layer can replace any of them (D55, D85). On Armv7-M they come in groups. The `nofp` layer's `__aeabi_fsub` is `__aeabi_fadd(a, -b)`: a program that subtracts links it, it calls `__aeabi_fadd`, that pulls `_arm_addsubsf3.o`, and that object defines `__aeabi_fsub` again, which is a duplicate definition. The same holds for `__aeabi_dsub` and `_arm_addsubdf3.o`, which is the problem `soft-dp` already solves on the FPU cores (D99, D100: the layer defines every symbol of each group it replaces). So:
- the six Armv6-M helpers move out of `nofp` into a layer of their own, `nofp-thumb1`, which only armv6-m and armv6s-m list;
- `nofp` keeps what suits any core without an FPU: the integer `sqrtf` (D85) and the fixed-point speed paths in `nofp/speed`;
- armv7-m takes every float helper from libgcc (Thumb-2 assembly; none clash, since the library defines none of them there), and `soft-dp` for doubles, as the FPU cores do.

`tools/scripts/libgcc-check` (D102) proves the outcome on every link map.

### `soft-dp` without `UMAAL`

`soft-dp`'s `__aeabi_dmul.S` forms the 117-bit product from one `UMULL`, one `UMLAL` and two `UMAAL`s. Everything else in `soft-dp` assembles for Armv7-M. `UMAAL` computes `{hi:lo} = a * b + lo + hi`; both uses can be rewritten with base instructions:
- the first `UMAAL` has `hi` zero, so it is a `UMLAL`;
- the second needs both addends: `ADDS lo, lo, hi` (carry into C), `MOV hi, #0`, `ADC hi, hi, #0`, then `UMLAL lo, hi, a, b`. The sum of a 32 × 32 product and two 32-bit words is below 2^64, so no carry is lost.

That is three instructions more, so the DSP cores keep their `UMAAL` version. The variant is a whole file in a layer of its own, `soft-dp-nodsp`, listed after `soft-dp` on armv7-m; `soft-dp`'s new `subdir.mk` drops a file that a later layer replaces, as `sp` and `sp-nearest` already do. The code is otherwise the same routine, so `test/unit/softdp` (which runs the assembly against reference C on every core with `soft-dp`) and `tools/scripts/softdp-diff` check it.

### The string kernels

`libc/string/thumb2`'s `memcpy`, `memmove`, `memset` and `strcmp` use only Armv7-M instructions; `strchr` uses `UADD8`/`SEL`. `strchr.S` moves to `libc/string/thumb2-dsp`, and the DSP cores' speed lists become `thumb2-dsp thumb2 thumb1`: the same code, so their images and counts do not change. armv7-m's speed list is `thumb2 thumb1`, and `strchr` is the C version there.

### `setjmp`

armv7e-m's routine minus the VFP part: `mov ip, sp; stmia r0!, {r4-r11, ip, lr}` and the mirror in `longjmp`. The jmp_buf layout is the common prefix of the other Arm variants, and `jmp_buf` is already sized for the largest (D15).

## Edge cases

- **libgcc's float addition on Armv7-M.** D99 found libgcc's Armv7E-M double addition misrounding. armv7-m's doubles use `soft-dp`, which fixed that. Its floats use libgcc's `_arm_addsubsf3.o`, which no other core used before: the libm tests (`libm_dense`, `float_accuracy`, `float_exact`) and `test/unit/softfp` run on it.
- **The fixed-point speed paths** were tuned for Thumb-1. On Armv7-M they are compiled as Thumb-2 and checked by the same tests in the speed flavour.
- **Unaligned access.** The speed flavour's Thumb-2 kernels use unaligned `LDR`/`STR`, which Armv7-M supports in Normal memory (same caveat as the other Thumb-2 cores: `CCR.UNALIGN_TRP`, Device memory).

## Test vectors

No new unit test: every existing one runs on armv7-m.
- **`test/unit/softdp`:** the DSP-free `dmul` against reference C. Its hash of 30,000 random cases per operation must equal x86_64's.
- **`test/unit/softfp`:** libgcc's float helpers.
- **`test/unit/string_kernels`:** every alignment and length for the Thumb-2 kernels without `strchr`.
- **`test/unit/setjmp`:** the new routines.
- **Baselines:** libc-test must pass in full, and `libgcc-check` must stay clean.
