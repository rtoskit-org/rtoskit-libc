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

## P5.03 — Thumb-2 word-at-a-time `memchr`  `[ ]`
- [ ] Q21: `memchr` is C and takes about twice AOR's instructions from 16 bytes up. A word-at-a-time kernel (`UADD8`/`SEL` on the DSP cores, as `strchr`, D74; a DSP-free version for armv7-m), researched from first principles.
- [ ] printf links `memchr`, and `hello-printf-int-nofloat` has 97–101 bytes of speed budget: measure the kernel's growth against that first; if it does not fit, record the trade-off and ask for a budget decision rather than shrinking the test.

## P5.04 — Fixed-point `powf` and `log10f` on the cores without an FPU  `[ ]`
- [ ] Q25: armv6-m, armv6s-m and armv7-m. `powf` (about 22,000 instructions on armv6-m, 11,000 on armv7-m) and `log10f` have no fixed-point path in the speed flavour, and take 4–4.5 times AOR's instructions. Extend `nofp/speed` (D86–D88) with the same rounding test and float-float fallback; research note with the error bounds and hard cases.
- [ ] `math-float` links `powf` and has 112 bytes of speed budget on armv6-m and none on armv7-m: record the growth, then a decision entry for the new budget (a larger `math-float`/`math-mix` margin as D87, or locked speed figures as D110), before the step is ticked.
