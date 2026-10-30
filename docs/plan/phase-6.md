# Phase 6 — Every remaining Arm architecture

Prerequisite: PLAN D118 (scope, naming, order; issue #90), D106 (the
architecture loop), D108 (how `armv7-m` was added: the template for each step).
Targets: every architecture in `ARCHS`, plus the one each step adds.

**Goal:** build, test and measure rtoskit-libc on every Arm architecture GCC
16.2 targets: the rest of the M profile, the R profile, the A profile in
AArch32 state, and AArch64.

**Every step's definition of done (DoD)** is the Phase 1 DoD plus, for each
architecture the step adds:

- `tools/makefiles/<arch>.mk` (on `arm-none-eabi.mk` or the new
  `aarch64-none-elf.mk`), the architecture in `ARCHS`, and its `setjmp`,
  `<fenv.h>` variant (D44), `LIBC_MATH_LAYERS` and `libc-fpu.h` (D46, D82) and
  string kernel lists for both flavours (D69, D73).
- `make` and `make check` pass in all three build types on every architecture,
  the new one included, with the in-tree conformance tests (D126), and
  `tools/scripts/libgcc-check` is clean (D102).
- Size: `docs/size/baseline-<arch>.json`, a `docs/size/budget.json` entry and
  `docs/size/<arch>.md`. Speed: `docs/perf/baseline-<arch>[-speed].json` and
  `docs/perf/<arch>.md`. The existing architectures' size and speed figures are
  unchanged unless a decision entry says why.
- A research note (`docs/research/<arch>.md`, or a section of P6.01's note),
  a PLAN decision entry, and the architecture's row in the README's and
  `docs/configuration.md`'s architecture tables.

---

## P6.01 — Research: the architecture matrix  `[ ]`
- [ ] `docs/research/arm-architectures.md`: for every Arm `-march` GCC 16.2 accepts, its cores, ISA (Arm, Thumb-1, Thumb-2, A64), divide, exclusives, FPU options (`-mfpu`/`+fp` extensions), SIMD (NEON, MVE, Advanced SIMD, SVE), and what `qemu-arm -cpu max` and `qemu-aarch64` run.
- [ ] Fix the `ARCH_CROSS` names and the variant list (D24: one name per `-march`, suffixes for FPU or extension variants), and which variants are worth a separate build.
- [ ] Q29: record which classic cores GCC 16.2 and qemu still support, and whether any user needs them.
- [ ] Q33: measure how long `make check` and `make bench` take per architecture, and project the cost of the full list.

## P6.02 — `armv8-m.base` (Cortex-M23)  `[ ]`
- [ ] Thumb-1 plus `CBZ`/`CBNZ`, `MOVW`/`MOVT`, `SDIV`/`UDIV` and exclusives; no FPU. Reuse the Armv6-M work (`thumb1` kernels, the `nofp` layers, the fixed-point speed maths, D85–D88, D114–D116); its own `setjmp` and integer division.
- [ ] Use the hardware divide and exclusives where they pay: lock-free atomics (D43) and the speed flavour's conversions.

## P6.03 — `armv7-r` (Cortex-R4, R5, R7, R8)  `[ ]`
- [ ] Variants from P6.01: no FPU (`soft-sp`, D109), VFPv3-D16 double precision, and the single-precision-only R5F if it differs.
- [ ] Q31: Arm or Thumb state per flavour; every `.S` kernel interworks.
- [ ] Q35: the unaligned-access default.

## P6.04 — `armv8-r` (Cortex-R52) in AArch32  `[ ]`
- [ ] VFPv5 with optional Advanced SIMD; reuse P6.03's choices where they hold.

## P6.05 — `armv7-a` (Cortex-A5 to A17) in AArch32  `[ ]`
- [ ] Variants from P6.01: soft float, VFPv3-D16, VFPv4 with NEON (D32 registers). `<fenv.h>` with full rounding and exceptions on the double-precision FPUs.
- [ ] TLS through TPIDRURO (already what `port/linux/arm` reads, D36); the `port/none` policy for the A profile.

## P6.06 — `armv8-a` in AArch32 state  `[ ]`
- [ ] Cortex-A32, and A53 or A72 running 32-bit code; reuse P6.05, with VFPv5 (`VRINT`, `VCVT` with rounding) in the libm layers.

## P6.07 — `armv8.1-m.main` (Cortex-M52, M55, M85)  `[ ]`
- [ ] Variants from P6.01: no FPU, single- and double-precision FPU, MVE integer, MVE float. Resolves Q10.
- [ ] Low-overhead loops (`WLS`/`DLS`/`LE`) in the scalar kernels where they pay.
- [ ] Q34: PACBTI, `-mbranch-protection` and the landing pads in `setjmp`/`longjmp` and the `.S` kernels.

## P6.08a — AArch64 bring-up  `[ ]`
- [ ] `aarch64-none-elf` GCC 16.2 in `make setup`, `tools/makefiles/aarch64-none-elf.mk`, `ARCH_FAMILY := aarch64`; `port/linux/aarch64` (Linux AArch64 system calls), `port/none/aarch64`, `libc/tls/aarch64` (TPIDR_EL0, variant I).
- [ ] `setjmp`/`longjmp` (x19-x30, sp, d8-d15) and Q32's `jmp_buf` expression; `<fenv.h>` on FPCR/FPSR.
- [ ] `qemu-aarch64` as `TARGET_RUNNER`; `make setup-bench` builds it with plugins.
- [ ] The `long double` cases of `test/unit/libm_vectors`, `printf_float` and `strtod`, which assume binary64 (D7), are selected per architecture until P6.08b.

## P6.08b — AArch64 binary128 `long double`: conversions and limits  `[ ]`
- [ ] Q30: `<float.h>` and `<math.h>` limits; exact `printf` `%La`/`%Le`/`%Lf`/`%Lg`, `strtold` and `scanf` `%Lf` on binary128 (extending D38/D39's decimal code to 113-bit significands).
- [ ] The soft-quad arithmetic helpers: libgcc's or ours (Q30), with `libgcc-check` clean.

## P6.08c — AArch64 binary128 `long double`: libm  `[ ]`
- [ ] The `*l` functions and their `<tgmath.h>` forms at Q30's accuracy goal; `test/unit/libm_vectors` gains binary128 vectors (`tools/scripts/libm-vectors` at 113 bits) and they pass.

## P6.08d — AArch64 baselines and later levels  `[ ]`
- [ ] Conformance, size and speed baselines for `armv8-a`; the later levels (`armv8.2-a`, `armv8.5-a`, `armv9-a`) and Cortex-R82 that P6.01 found worth a variant.
- [ ] Q34 for AArch64 (BTI, PAC).

## P6.09 — NEON string kernels (A and R profile AArch32)  `[ ]`
- [ ] The speed flavour's `memcpy`, `memset`, `memchr`, `strlen`, `strcmp`, `strchr` with NEON, within the speed budget and the granule read rule (`docs/research/strlen-thumb2.md`).

## P6.10 — MVE string kernels (`armv8.1-m.main`)  `[ ]`
- [ ] As P6.09, with MVE and tail predication.

## P6.11 — AArch64 Advanced SIMD string kernels  `[ ]`
- [ ] As P6.09, with Advanced SIMD; SVE only on the variants that have it.

## P6.12 — Classic cores (only if Q29 admits them)  `[ ]`
- [ ] Armv4T, Armv5TE, Armv6/Armv6KZ as P6.01 found them supported: no Thumb-2, no exclusives before Armv6 (atomics through the seam, Q27), VFPv2 at most.

## P6.13 — AOR comparison on the new cores  `[ ]`
- [ ] Extend `make aor` and `bench-runner aor` (D92, D93) to the new architectures; AOR's AArch64 and NEON routines are its most complete. Record the gaps as Q entries or later steps.

## P6.14 — Phase 6 wrap-up  `[ ]`
- [ ] PLAN §1's target matrix, the README, `docs/configuration.md`, CLAUDE.md's architecture lists, and the multilib list of issue #65.
- [ ] Close issue #90.
