# Research: a Thumb-2 `memchr` for the speed flavour

Step: P5.03 (`docs/plan/phase-5.md`, Q21). Decision: PLAN D113. Builds on D74
(the DSP `strchr`), D112 (the Thumb-2 `strlen`) and the P4.06 analysis
(`docs/research/optimized-routines.md`, gap 4).

## The gap

`memchr` is the generic C word loop (`libc/string/memchr.c`, D30) in both
flavours: about 10 instructions a word on Thumb-2 (`strlen`'s loop needs 4.5),
2615 instructions for 1024 bytes on armv7e-m. Arm Optimized Routines' kernel
takes 0.51 of that at 1 KiB and 0.66 at 16 bytes (P4.03), and is 160 bytes
larger than our C. printf links `memchr` (line-buffered writes look for
`'\n'`, `libc/stdio/__libc_file_write.c`), so `hello-printf-int-nofloat`'s
speed budget (97–101 bytes left on the FPU cores) limits the kernel's growth.

## Sources reviewed

- **C23 7.26.5.2** (`docs/standard/library-7.pdf`): `memchr` finds the first
  occurrence of c, converted to unsigned char, in the initial n characters;
  "the implementation shall behave as if it reads the characters sequentially
  and stops as soon as a matching character is found". So n may exceed the
  object when c occurs in it: a kernel must not read beyond what it could
  prove exists, except inside a granule it already touches.
- **The Linaro cortex-strings / newlib Arm `memchr`** (Dave Gilbert, 2011;
  <https://sourceware.org/pipermail/newlib/2011/009451.html>,
  <https://www.sourceware.org/legacy-ml/newlib/2011/msg00517.html>): Armv6T2
  and later, a fast path for short sizes and an optimised path for large data;
  it relies on `CBZ`/`CBNZ` and `UADD8`/`SEL`, and other targets use the C
  version. glibc's `sysdeps/arm/armv6t2/memchr.S` derives from it
  (<https://git.stg.centos.org/source-git/glibc/blob/87c41c2382094eca815874b1d105a1e443f4dcd8/f/sysdeps/arm/armv6t2/memchr.S>).
  Only the structure (bytes to alignment, a double-word loop, a tail) is
  relevant; no code is used.
- **The classic zero-byte test** on `w ^ pattern` (D30): zero bytes of the XOR
  are matches, and the lowest flag marks the first one exactly.
- **Armv7-M ARM** A3.2.1 (`LDRD` needs word alignment) and the 32-byte MPU
  granule (`docs/research/strlen-thumb2.md`).

## Trade-offs

- **The classic test, not `UADD8`/`SEL`.** `strchr` tests two conditions per
  word, where `UADD8`/`SEL` merges them cheaply (D74). `memchr` tests one:
  `EOR`, `SUB`, `BIC`, `ANDS` and the branch take 5 instructions either way,
  and the classic form needs no DSP, so the kernel goes in `libc/string/thumb2`
  and armv7-m takes it as well.
- **Two `LDR`s per iteration, not `LDRD`.** An `LDRD` version (two words per
  load, 6.5 instructions a word) needed 8-byte alignment first, a separate
  word step to reach it, and r4/r5 pushed for the second data word: 188 bytes,
  100 more than the C, which put `hello-puts` and `hello-printf-int-nofloat`
  3–12 bytes over the speed budget on the FPU cores (printf and `puts` both
  link `memchr`). Two word loads per iteration (7 instructions a word) reuse
  one data register, need only word alignment and push nothing: 154 bytes.
  The pattern lives in r1, and the byte loops compare against its top byte
  (`CMP r3, r1, LSR #24`), so they stay at 3 instructions a byte.
- **Reads.** Only words wholly inside the n bytes are read, and they are word
  aligned, so every read lies in one 32-byte granule. When the match comes
  before the end of the object a read may extend past the match, but never
  past the granule holding it.
- **Short sizes.** The byte loop to the first word boundary is at most 3
  bytes; fewer than 8 bytes after it skip the loop for at most one word and
  the byte tail.

## Chosen approach

`libc/string/thumb2/memchr.S`: c to unsigned char, replicated into r1; bytes
until the pointer is word aligned (or n is 0); two aligned words per iteration
while 8 or more bytes remain, one more word if 4 remain; then the byte loop for
the last 0–3. At a flagged word, `RBIT`/`CLZ` give the byte index, and the
address is the word's plus the index. 154 bytes against the C version's 88.

Measured on every Thumb-2 core (instructions per call, C → kernel): 1024 bytes
2615 → 1821, 256 bytes 695 → 477, 16 bytes 95 → 57, 8 bytes 75 → 43, 0 bytes
29 → 24; 0.57–0.75 from 4 bytes up, 49 cases faster, none slower (two 3-byte
cases within noise, ±1 instruction). AOR's kernel takes 0.51 of the C at 1 KiB
(P4.03), so part of the gap remains: the price of the 66-byte growth that the
budget allows.

## Edge cases

- n = 0 at any alignment: a null pointer, nothing read.
- c outside 0–255 (converted to unsigned char), c = 0, and 0x80–0xff bytes
  that trip a careless zero test (`0x80 ^ c`).
- The match in every byte of the first word, the second word of a pair, the
  tail word, the tail bytes, and just past n (not found).
- Every start offset 0–7 and every n up to the test buffer.
- The match a few bytes before the end of a readable area, with n far larger.

## Test vectors

`test/unit/string_kernels` gains `memchr`: every offset 0–7, every n up to its
limit, the match at every position (and absent, with the character just past
n), the tricky byte values around it, c given as an int above 255; and a page
end case with the match within the last 32 bytes of the buffer and n =
`SIZE_MAX`-sized.
