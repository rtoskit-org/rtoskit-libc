# Phase 5 — Speed follow-ups unblocked by the budget

Prerequisite: PLAN D108–D110 (`armv7-m`, `soft-sp`, armv7-m's own speed budget).
Targets: every architecture in `ARCHS`.

**Goal:** close the speed gaps that phases 3 and 4 measured but the per-image
speed budget blocked (Q16, Q21, Q25), now that a speed image may grow by at
least 256 bytes of code (D111).

**Every step's definition of done (DoD)** is the Phase 1 DoD plus:

- libc-test passes in full on every architecture and build type; `make bench`
  shows no case slower by more than 1 %, and the step's gains are recorded with
  `make bench BENCH_UPDATE=1` and in `docs/perf/<arch>.md`.
- The size flavour is unchanged (`size-report diff`), and the speed flavour stays
  within its budget (`size-report --build-type speed budget --flavour speed`). A
  step that needs more names the decision that allows it.

---

## P5.01 — A floor under the speed margin  `[x]`
- [x] `size-report budget`: a flavour's code allowance is the larger of its percentage and `floor_bytes` (256 for the speed flavour); RAM keeps the percentage; architectures with their own flavour figures (D110) keep no margin.
  *D111. Speed headroom of the small images: `malloc-heavy` 25–37 → 96–144 bytes on every core. `hello-printf-int-nofloat` (about 4.4 KB, whose 10 % already exceeds 256 bytes) is unchanged at 97–101 bytes.*

## P5.02 — Thumb-2 `strlen` in the speed flavour  `[x]`
- [x] armv7e-m, armv7e-m-dp, armv8-m.main (and armv7-m if a DSP-free version gains): the double-word `strlen` that P3.02b measured 10 % faster for +56 bytes and dropped for the `malloc-heavy` budget (Q16). Research note, `test/unit/string_kernels` coverage, budget check.
  *D112, `docs/research/strlen-thumb2.md`, `libc/string/thumb2/strlen.S` (DSP-free, so every Thumb-2 core). Bytes to a word boundary, one word to 8, then `LDRD` pairs; walking bytes to 8 first made short misaligned strings 4–12 % slower. 1024 bytes 1315 → 1187, 7 bytes 61 → 36, `mix/strlen` 113 → 103; 119 cases faster, none slower. +64 bytes per image that links `strlen`, inside the floor on the FPU cores.*
- [x] armv7-m: its speed figures are locked exactly (D110); re-lock them only with a decision entry.
  *Re-locked under D112: `hello-puts` 2051, `malloc-heavy` 2138, `malloc-heavy-tiny` 2078. `make check` passes in all 21 configurations.*

## P5.03 — Thumb-2 word-at-a-time `memchr`  `[x]`
- [x] Q21: `memchr` is C and takes about twice AOR's instructions from 16 bytes up. A word-at-a-time kernel (`UADD8`/`SEL` on the DSP cores, as `strchr`, D74; a DSP-free version for armv7-m), researched from first principles.
  *D113, `docs/research/memchr-thumb2.md`, `libc/string/thumb2/memchr.S`. One condition per word, so the classic test is as short as `UADD8`/`SEL` and needs no DSP: one file for every Thumb-2 core. Two aligned words per iteration, nothing pushed. 1024 bytes 2615 → 1821, 16 bytes 95 → 57; 49 cases faster, none slower. `string_kernels` gains `memchr` coverage.*
- [x] printf links `memchr`, and `hello-printf-int-nofloat` has 97–101 bytes of speed budget: measure the kernel's growth against that first; if it does not fit, record the trade-off and ask for a budget decision rather than shrinking the test.
  *An `LDRD` version (188 bytes, +100) did not fit (`hello-puts` and `hello-printf-int-nofloat` 3–12 bytes over); the two-`LDR` kernel (154 bytes, +66) fits on the FPU cores. armv7-m's speed figures are re-locked under D113. `make check` passes in all 21 configurations.*

## P5.04a — Fixed-point `log10f` on the cores without an FPU  `[~]`
- [ ] Q25: armv6-m, armv6s-m and armv7-m. `log10f` has no fixed-point path in the speed flavour (it takes 4–4.5 times AOR's instructions): the D86 `log2f` core times log10 2 to 64 bits, as `logf` does with ln 2. Research note section with the error bound measured on the host over every positive float, as D86 did.
- [ ] Record its growth against `math-float`/`math-mix` (armv6-m: 112 bytes of `math-float` speed budget; armv7-m locked, D110).

## P5.04b — Fixed-point `powf` on the cores without an FPU  `[ ]`
- [ ] Q25: `powf` (about 22,000 instructions on armv6-m, 11,000 on armv7-m) has no fixed-point path. y log2 |x| from a more precise logarithm core, then the D86 exp2 core, with the same rounding test and float-float fallback; an error analysis (two arguments cannot be run exhaustively), host runs over large random and structured sets, and hard cases in `test/unit/libm_dense`.

## P5.04c — The math images' speed budget  `[ ]`
- [ ] With P5.04a and P5.04b measured: a decision entry for the new budget of `math-float`/`math-mix` on armv6-m, armv6s-m and armv7-m (a larger margin as D87, or locked speed figures as D110), then `make check`, `make bench` and both budgets on every architecture.
