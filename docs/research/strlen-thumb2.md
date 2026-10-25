# Research: a Thumb-2 `strlen` for the speed flavour

Step: P5.02 (`docs/plan/phase-5.md`). Decision: PLAN D112. Builds on D74 and
`docs/research/string-speed.md` (P3.02b, measured version 4), which designed and
measured this kernel and left it out of the tree for the `malloc-heavy` speed
budget (Q16). D111's 256-byte floor now leaves that image 96–144 bytes.

## Sources reviewed

- **C23 7.26.6.4** (`docs/standard/library-7.pdf`): `strlen` returns the
  number of characters before the terminating null character.
- **Armv7-M Architecture Reference Manual** (DDI 0403), A3.2.1: `LDRD`,
  `STRD`, `LDM`, `STM` and the exclusive accesses always need word alignment
  and raise a UsageFault (UNALIGNED) otherwise, whatever `CCR.UNALIGN_TRP`
  says; `LDR`/`LDRH` may be unaligned in Normal memory. Device memory needs
  natural alignment for every access (Keil knowledge base,
  <https://www.keil.com/support/docs/3777%20%20.htm>).
- **The Armv7-M and Armv8-M MPU** protect power-of-two regions of at least 32
  bytes, aligned to their size (Zephyr's Armv7-M MPU driver,
  <https://elixir.bootlin.com/zephyr/v2.1.0-rc1/source/arch/arm/core/cortex_m/mpu/arm_mpu_v7_internal.h>;
  TI MSP432 driverlib, <https://software-dl.ti.com/simplelink/esd/simplelink_msp432_sdk/1.40.00.28/docs/driverlib/msp432p4xx/html/group__mpu__api.html>;
  FreeRTOS forum, <https://forums.freertos.org/t/alignment-fault/21949>).
  Armv8-M's MPU uses 32-byte granules too.
- **The Linaro cortex-strings Thumb-2 `strlen`** (via newlib, already cited in
  `string-speed.md`): two words per iteration with the DSP `UADD8`/`SEL`
  test. Only its loop shape (two words per load) is relevant here.
- **The classic zero-byte test** `(w - 0x01010101) & ~w & 0x80808080` (D30):
  non-zero exactly when w has a zero byte, and its lowest set bit marks the
  first one, since a borrow only reaches the bytes above a zero. On Thumb-2
  both constants are modified immediates.

## Trade-offs

- **8-byte alignment, not 4.** `LDRD` needs only word alignment, but an 8-byte
  read from a word-aligned address can cross a 32-byte boundary, and so an MPU
  region boundary, after the terminator. Aligned to 8, each read lies inside
  one 32-byte granule, which also holds the terminator's word or bytes before
  it: the kernel never touches memory the caller's string does not share a
  granule with.
- **The classic test, not `UADD8`/`SEL`.** It needs no DSP (so armv7-m can use
  the same file, `libc/string/thumb2`) and no third register, so nothing is
  pushed. Per word: `SUB`, `BIC`, `TST`/`ANDS` and a branch, plus half an
  `LDRD`: 4.5 instructions against the Thumb-1 loop's 5 (`LDMIA`, `SUBS`,
  `BICS`, `TST`, `BEQ`).
- **Each word has its own exit**, so the first word's flags need not be merged
  into the second's. The exit finds the byte with `RBIT` and `CLZ` (both base
  Armv7-M): the index is the flags' trailing zeros divided by 8.
- **A byte loop to the 8-byte boundary.** At most 7 bytes, 4 instructions each.
  Masking the bytes before the start of the first aligned double word would
  save branches for misaligned starts but costs more at an aligned start, which
  most strings (from `malloc` and the stack) have.

## Chosen approach

`libc/string/thumb2/strlen.S`: r12 keeps the start; bytes until r0 is 8-byte
aligned (returning at a terminator); then `LDRD` of two words per iteration,
each tested; at the flagged word, length = word address − start + index.

Expected (P3.02b's measurement): 1186 instructions for 1024 bytes against 1315
for the Thumb-1 kernel, no length slower, about +56 bytes. It replaces the
Thumb-1 kernel in the speed flavour of every Thumb-2 core (armv7-m,
armv7e-m, armv7e-m-dp, armv8-m.main); the size flavour keeps Thumb-1.

## Edge cases

- The empty string, at every alignment.
- A terminator in each byte of the first and second word of a double word, at
  every start offset 0–7.
- Strings that end exactly at the 8-byte boundary where the byte loop stops.
- Bytes 0x80–0xff before the terminator: the test's `& ~w` keeps them from
  flagging (0x80 − 1 has the top bit clear only after the borrow from a zero
  byte).
- A string ending in the last bytes of a 32-byte granule: the read stays
  inside it.

## Test vectors

`test/unit/string_kernels` already runs `strlen` over every start alignment and
every length up to its buffer, with 0x80-filled bytes around the string, in
every flavour of every architecture; it covers the cases above. libc-test's
string tests and `make bench`'s `strlen` sweep (lengths 0–1024, aligned and the
worst misaligned offset) measure it.
