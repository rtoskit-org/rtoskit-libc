# Research: thread-local storage layout and static startup

Step: P1.01. Decisions: PLAN D10 (static local-exec TLS), D4 (seam).

## Sources reviewed

- Ulrich Drepper, *ELF Handling For Thread-Local Storage*, v0.21 (2013):
  <https://www.akkadia.org/drepper/tls.pdf>. §3.4 gives the variant I and
  variant II layouts and the `tlsoffset` formulas. §4.4 shows the x86-64
  local-exec code sequences that read `%fs:0`.
- Fangrui Song (MaskRay), *All about thread-local storage* (2021):
  <https://maskray.me/blog/2021-02-14-all-about-thread-local-storage>. It
  covers the exact TP offset formulas when `p_vaddr % p_align != 0`, the
  2-word reserved gap on Arm, and the known glibc and FreeBSD bugs in that
  corner case (glibc BZ24606).

Not re-read in this session, and summarised from general knowledge. Review
these before P1.12 (threads), where per-thread TLS allocation matters:

- The x86-64 psABI TLS section. The `%fs:0` self-pointer requirement used
  here is confirmed by Drepper's local-exec code sequences.
- Static TLS setup in other libcs: musl `__init_tls` (finds `PT_TLS` from
  `AT_PHDR`); glibc `__libc_setup_tls`; picolibc `_init_tls` (the linker
  script exports the `.tdata`/`.tbss` bounds, and on bare-metal Arm
  `__aeabi_read_tp` returns a global); Bionic `__libc_init_main_thread`.

## Findings

### Layouts

| | Variant I (Arm, AArch64, RISC-V) | Variant II (x86, x86-64) |
|---|---|---|
| Thread pointer (TP) | Points at a reserved control block. TLS data follows *above* it. | Points at the control block. TLS data lies *below* it. |
| Reserved at TP | 2 words (8 bytes on AArch32) | the TCB, of implementation size; word 0 = TP |
| Offset of the executable's TLS block | `+ (tcb + ((p_vaddr - tcb) & (p_align - 1)))` | `- (p_memsz + ((-p_vaddr - p_memsz) & (p_align - 1)))` |
| TP alignment | ≥ `p_align` | ≥ `p_align` |

In the normal case (`p_vaddr % p_align == 0`) these reduce to
`round(tcb, align)` above TP and `round(memsz, align)` below TP. The general
formulas are what the linker uses to resolve local-exec offsets, so the runtime
has to use them too. Otherwise the misaligned-`p_vaddr` case breaks, as it did
in glibc.

### Initialization

Copy `p_filesz` bytes from the image at `p_vaddr`, then zero the remaining
`p_memsz - p_filesz` bytes (`.tbss`). The image address supplies the alignment
phase.

### x86-64 specifics

- GCC and Clang compute `&tls_var` in local-exec as `movq %fs:0, %rax; leaq
  off(%rax)`, so `*(void **)TP == TP` is mandatory.
- `-fstack-protector` reads its guard at `%fs:0x28`. A control block of at
  least 0x30 bytes, zero filled, lets such code run (with a zero canary)
  instead of faulting.

### Finding the image

- **Static Linux executable:** walk the program headers through
  `__ehdr_start`, which GNU ld and lld define when the ELF header is in a
  loaded segment (always true for the default static layout). Using `AT_PHDR`
  from the auxiliary vector also works and covers more exotic layouts.
  `__ehdr_start` needs no auxv parsing.
- **Bare metal:** the linker script must export the `.tdata`/`.tbss` bounds
  (as picolibc does). The port turns them into the same image description.

### Startup

The kernel enters `_start` with `argc` at `%rsp`, followed by `argv[]`,
`NULL`, `envp[]`, `NULL` and the auxv. `%rsp` is 16-byte aligned at entry. A
`call` needs `%rsp ≡ 0 (mod 16)` immediately before it. Constructors run from
`__preinit_array_*` and `__init_array_*`, which the default linker scripts
provide.

**Who runs which table (D104).** Neither C nor the compilers know
`.preinit_array`: C23 only requires static objects to be initialised before
`main`, and GCC and Clang put constructors (`[[gnu::constructor]]`, C++
static initialisation) in `.init_array`. `.preinit_array` is an ELF gABI
section (`DT_PREINIT_ARRAY`, executables only, run before `.init_array`) that
gets entries only from code that places pointers there by hand; newlib,
picolibc, glibc and musl run it at startup by convention. An RTOS may use it
for its own early tiers (pi-pico's `PREINIT_*` macros, walked by its own
`_start`), so the table belongs to the port: the port's startup runs it,
after the initial thread's TLS is in place, and `__libc_start_main` runs only
`.init_array`. Both ports here run it, so code that follows the ELF
convention works unchanged; an RTOS port that walks the table itself earlier
simply does not run it again.

## Chosen approach (implemented in P1.01)

- The seam struct `__libc_tls_image { init, init_size, size, align }`
  carries the program-header data from the port to libc. `init` doubles as
  the alignment phase (`p_vaddr`).
- `__libc_tls_block_size(image)` returns a size that includes slack for
  aligning TP anywhere in the block, so ports pass an unaligned block (from
  mmap, a heap, or a static array).
- `__libc_tls_init(block, image)` lays out the data plus control block and
  returns TP. The variant is chosen **per `ARCH_CROSS` variant subdirectory**
  (`libc/tls/<arch>/*.c`) with shared helpers in `libc/internal/tls.h`.
  There is no `#if`.
- x86_64: a 64-byte TCB (`self` + 7 zero words), TP aligned to
  `max(p_align, 16)`. The data offset uses the exact variant II formula.
- Arm (P1.06): variant I with an 8-byte gap, the same helper
  `__libc_tls_variant1_offset(image, 8)`. TP is read through the
  port-provided `__aeabi_read_tp`.
- Linux port: `_start` (asm) → `__rtos_linux_start(sp)`. It parses
  argc/argv/envp, finds `PT_TLS` via `__ehdr_start`, mmaps the block, calls
  `__libc_tls_init` and `arch_prctl(ARCH_SET_FS)`, then
  `__libc_start_main(main, argc, argv)`, which runs constructors and calls
  `exit(main(...))`. The image is kept in `__rtos_linux_tls_image` for thread
  creation (P1.12).

## Edge cases and test vectors (test/unit/smoke)

- No TLS at all (no `PT_TLS`): size 0, but the TCB is still created.
- `.tdata` non-zero initializer (42), `.tbss` zero (64 bytes).
- An over-aligned TLS variable (`alignas(64)`) aligned in every block.
- A second block initialised by `__libc_tls_init` and switched to: `errno` gets
  a new address, starts at 0 and is isolated from the first block; `*(void
  **)TP == TP`.
- Not yet covered: `p_vaddr % p_align != 0`. GNU ld always aligns `p_vaddr`,
  so a test needs a hand-made linker script. Add it with the bare-metal port
  linker scripts (P1.12).
