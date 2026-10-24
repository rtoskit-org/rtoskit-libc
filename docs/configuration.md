# Configuring rtoskit-libc

This page is for people who build programs against the library: what is
built, which archive to link, and what changes per core. The design
decisions behind each choice are in `docs/plan/PLAN.md` (the D numbers).

## What a build installs

`make` builds every architecture in `ARCHS` and every build type. A release
build of one architecture installs into `images/<arch>/release/`:

| Path | Contents |
|---|---|
| `include/` | the public headers (identical for every core of a target triple, D15) |
| `lib/libc.a` | the full library, libm included (D46) |
| `lib/libm.a` | empty, so that `-lm` links |
| `lib/libc-nofloat.a` | `libc.a` without floating-point formatting (D53) |
| `lib/libc-minimal-printf.a` | `libc-nofloat.a` with a smaller printf/scanf (D53) |
| `lib/libc-tinymalloc.a` | `libc.a` with the smallest allocator (D54) |
| `lib/librtos-<port>.a` | the OS seam for one port: `linux` (tests) or `none` (bare metal) |

Link exactly one of the `libc*.a` archives. They are complete libraries,
not add-ons: a flavour archive replaces members of `libc.a`, it is not
linked in front of it.

## Size or speed

Each architecture builds two optimised libraries (D73):

| Build type | Install | What it is |
|---|---|---|
| `release` | `images/<arch>/release/` | the size flavour: the smallest code, the default |
| `speed` | `images/<arch>/speed/` | the speed flavour: the same `-Os` library with faster kernels (on the Thumb-2 cores, `memcpy`/`memmove`/`memset` that copy 32 bytes per iteration and use unaligned word access, and word-at-a-time `strcmp`/`strchr`; on Armv6-M, block copies and shift-merged misaligned copies; on every core, `strtol` and printf integer conversions without 64-bit division and a `strtod` fast path; on Armv6-M, fixed-point `expf`, `exp2f`, `logf`, `log2f`, `sinf`, `cosf`, `tanf`, `erff` and `erfcf` that fall back to the size flavour's code when their rounding is not certain), within 10 % of the size flavour's code size, 20 % for the math reference programs (`docs/size/budget.json`, D87) |

Both flavours ship the same headers and archives (`libc.a`, the flavour
archives, `libm.a`). The speed flavour's Thumb-2 kernels use unaligned
`LDR`/`STR`, as GCC's own code does on these cores; a system that sets
`CCR.UNALIGN_TRP`, or copies Device memory with `memcpy`, should link the
size flavour. Speed figures: `docs/perf/<arch>.md`; the phase 3 summary, with picolibc and newlib-nano for
context, is `docs/perf/phase-3.md`.

## Choosing the core

`ARCH_CROSS` selects the core. The core then fixes everything below
automatically; there is nothing to configure per function.

| Core (`ARCH_CROSS`) | FPU | `<fenv.h>` | libm layers (D46, D56, D57) | String kernels (D69, D70, D73) |
|---|---|---|---|---|
| `armv6-m`, `armv6s-m` | none | soft: round to nearest, no flags | `sp sp-nearest nofp` | `thumb1`; speed: `thumb1-speed thumb1` |
| `armv7e-m` | FPv4-SP | soft | `sp sp-nearest fpv4-sp` | `thumb1`; speed: `thumb2 thumb1` |
| `armv8-m.main` | FPv5-SP | soft | `sp sp-nearest fpv5-sp` | `thumb1`; speed: `thumb2 thumb1` |
| `armv7e-m-dp` | FPv5-DP | full: every direction and flag | `sp fpv5-sp fpv5-dp` | `thumb1`; speed: `thumb2 thumb1` |
| `x86_64` (tests) | SSE | full | `x86_64` | none (C); speed: `x86_64` (SSE2) |

What the layers mean for a program:

- **float functions on the single-precision cores** (`sp`, `sp-nearest`):
  - they compute in float or float-float only and link no double
    arithmetic;
  - a float-only program on armv7e-m links no libgcc at all;
  - they are faithfully rounded and almost always correctly rounded
    (0.500 ulp measured, D57–D68);
  - the float complex functions follow the same rule (D65–D67).
  - They assume round to nearest, the only direction these cores'
    `<fenv.h>` has.
- **double functions** on those cores use the soft double helpers from
  libgcc (D55 trims them on armv6-m).
- **armv7e-m-dp** computes float functions in hardware double, with every
  rounding direction and exception flag. Its float transcendental functions
  are the double ones. A float-only program there is larger than on
  armv7e-m (`math-float` below), in exchange for full `<fenv.h>`
  semantics.

## Linking a program

The library builds with `-ffreestanding -nostdinc` against its own headers
(D8). Programs are linked with `-nostdlib`. The archives go in a group,
because the library and the port refer to each other:

```sh
arm-none-eabi-gcc -march=armv7e-m+fp -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -nostdinc -isystem images/armv7e-m/release/include \
    -Os -ffunction-sections -fdata-sections -c app.c
arm-none-eabi-gcc -march=armv7e-m+fp -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -nostdlib -static -Wl,--gc-sections -T board.ld app.o startup.o \
    -Wl,--start-group images/armv7e-m/release/lib/librtos-none.a \
    images/armv7e-m/release/lib/libc.a \
    $(arm-none-eabi-gcc -march=armv7e-m+fp -mfloat-abi=hard -print-libgcc-file-name) \
    -Wl,--end-group
```

- Replace `libc.a` with a flavour archive to pick that flavour.
- `--gc-sections` matters: every function has its own section and file
  (D6), and unused ones disappear.
- The compiler flags must match the core the library was built for: the
  `-march` and float ABI of its `ARCH_CROSS` file.

## Flavour archives (D53, D54)

| Archive | What changes | When to use it |
|---|---|---|
| `libc.a` | everything | default |
| `libc-nofloat.a` | `printf`/`scanf` floating conversions (`%a %e %f %g`) fail with `EINVAL`; `strtod` still works | programs that never format floating point |
| `libc-minimal-printf.a` | as `libc-nofloat.a`, and `%n` and the length modifiers wider than `long` (`ll`, `j`, `z` when 64-bit) fail with `EINVAL`, so no 64-bit division is linked | small programs that print `int`/`long` |
| `libc-tinymalloc.a` | the `tiny` allocator: an implicit free list, 13 % smaller than `nano`, O(chunks) `malloc` | the smallest heap code, few allocations |

A conversion the flavour does not support fails loudly (`EINVAL`) rather
than printing nothing.

Code size of the reference programs (`test/size`, release, bytes of code
in the whole image, `docs/size/<arch>.md`):

| Program | armv6-m | armv7e-m | armv8-m.main | armv7e-m-dp |
|---|---:|---:|---:|---:|
| `hello-puts` | 2163 | 1903 | 1883 | 1903 |
| `hello-printf-int` (`libc.a`) | 7769 | 7185 | 7165 | 7220 |
| `hello-printf-int` (`libc-nofloat.a`) | 4987 | 4471 | 4439 | 4471 |
| `hello-printf-int` (`libc-minimal-printf.a`) | 4719 | 4231 | 4203 | 4231 |
| `hello-printf-float` | 10374 | 9170 | 9150 | 7244 |
| `malloc-heavy` (`nano`) | 2214 | 1934 | 1914 | 1934 |
| `malloc-heavy` (`libc-tinymalloc.a`) | 2134 | 1874 | 1854 | 1874 |
| `math-float` | 9640 | 5282 | 5214 | 9494 |
| `math-mix` | 34116 | 27312 | 27232 | 12302 |

## The allocator inside `libc.a` (D32, D54)

`libc.a` takes the allocator named by `LIBC_MALLOC` when the library is
built:

| `LIBC_MALLOC` | Design | Notes |
|---|---|---|
| `nano` (default) | first fit over an address-ordered free list, merging | small and general |
| `tlsf` | two-level segregated fit | O(1) `malloc`/`free`, for bounded-time systems |
| `tiny` | implicit free list | smallest; also always shipped as `libc-tinymalloc.a` |

```sh
make clean && make LIBC_MALLOC=tlsf
```

`make clean` is needed when changing it, because archives are updated in
place. The heap comes from the port: `__rtos_heap_region` describes one or
more regions (`tiny` uses only the first).

## Ports (D4)

The library reaches the OS only through the `__rtos_*` functions declared in
`include/rtos/rtos-port.h`. A port implements them in its own archive.

- **`port/none`** (bare metal, single thread), as a board uses it:
  - **Startup:** the reset code initialises memory and calls
    `__rtos_none_start(&tls_image)`. That places the TLS block in a static
    1024-byte buffer, runs `.preinit_array` and runs `main` through
    `__libc_start_main`, which runs only `.init_array` (D104).
  - **Heap:** the linker script defines `__rtos_none_heap_start` and
    `__rtos_none_heap_end`. Or the board defines its own
    `__rtos_heap_region` for several regions.
  - **Output:** `stdout`/`stderr` bytes go to the weak
    `__rtos_none_output(buffer, count)`, which discards them. Define it,
    for example over a UART.
  - Locks are empty. The thread functions report that threads cannot be
    created.
- **`port/linux`** runs the library on Linux system calls. It is the test
  port (`make check` runs everything through it, natively or under
  `qemu-arm`), not a production target.
- A new port copies `port/none` and implements the seam for its RTOS (see
  `include/rtos/rtos-port.h` and `docs/research/threads.md`).
- **An RTOS whose heap is an `sbrk` break** (pi-pico's `sys/sbrk.c`, D103):
  the port's `__rtos_heap_region` takes one block from that `sbrk` at index
  0 and returns false after it. libc calls it once, at the first allocation,
  and owns the block from then on, so the port decides how much to take:
  everything up to the heap's end, or all but a reserve for the RTOS's other
  `sbrk` users, which keep working from the break above the block.

## Building from another project (D105)

`make library` builds and installs only what a program links against: the
headers, the `libc*.a` archives and the port archives. It skips the tests,
the examples and the size and benchmark programs, and keeps the full build's
ordering (the toolchain, then `libc` and `port`), so it works on a fresh tree.
Like `all`, it loops over `ARCHS` and `BUILDS` unless `ARCH_CROSS` and
`BUILD_TYPE` name one.

### The sub-make

A parent build runs `make library` in this tree with two rules:

- **Pass every root and the architecture explicitly.** The top-level
  `Makefile` takes `PROJECT_ROOT`, `TOOLS_ROOT`, `OUTPUT_ROOT`, `PREFIX`,
  `ARCH_CROSS` and `BUILD_TYPE` from the environment when they are set there. A parent
  with the same conventions (pi-pico exports all six) would otherwise hand
  over its own values (pi-pico's `ARCH_CROSS` is a CPU name such as
  `cortex-m33`): the sub-make fails to find `tools/`, or stops because
  `ARCH_CROSS` names no toolchain file. Values on the sub-make's command line win over the
  environment and over the parent's `MAKEFLAGS`. The architecture files and
  the build-type logic assign everything else (`ARCH_FAMILY`, `BUILD_ROOT`,
  `INSTALL_ROOT`, `RTOS_PORT`, ...) with `:=`, so the parent's exports of
  those do not leak in.
- **Pass `CROSS_COMPILE` to use the parent's compiler.** A `CROSS_COMPILE`
  given on the command line or in the environment is used as it is: setup
  downloads nothing into `local/`, stops if `${CROSS_COMPILE}gcc` does not
  exist, and stops if it is older than GCC 14 (D9). Without it, the first
  build downloads GCC 16.2 into this tree's `local/` as usual.

```make
$(MAKE) -C $(LIBC_DIR) \
	PROJECT_ROOT=$(LIBC_DIR) TOOLS_ROOT=$(LIBC_DIR)/tools \
	OUTPUT_ROOT=<build dir> PREFIX=<install dir> \
	ARCH_CROSS=armv8-m.main BUILD_TYPE=release \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	library
```

The libraries install into `<install dir>/<arch>/<build type>/{include,lib}`;
link them as in "Linking a program" above. A parent picks the architecture of
its core; the `linux` port archive is installed too and simply not linked.

### Module glue in a pi-pico-style parent

A parent that uses this project's own conventions (pi-pico) wraps the
sub-make in a module makefile of its own, the way pi-pico's `libc/libc.mk`
wraps picolibc. With this tree checked out as `<parent>/rtoskit-libc/` and the
parent's glue as `rtoskit-libc.mk` at the root of that checkout, the parent's
module search (`<dir>/<dir>.mk`, one level down) finds it as the module
`rtoskit-libc`. This project's own build never reads that file: its module
search also starts one level down, inside the checkout. The glue has to:

1. **Stop the parent's tree walk at the module.** The parent's `target.mk`
   derives `SUBDIRS` from every `*/*.mk` below the module, which here means
   this project's `libc/`, `port/`, `test/`, ... Set `SUBDIRS :=` before
   including it, and prune the checkout from the parent's module search, as
   pi-pico's `tree.mk` already prunes `libc/picolibc`.
2. **Run the sub-make above** in its build-directory branch, with
   `PROJECT_ROOT` and `TOOLS_ROOT` pointing at the checkout and
   `OUTPUT_ROOT`/`PREFIX` inside the module's build directory, so a parent
   `make clean` removes them.
3. **Map the parent's core to an architecture:** pi-pico's `cortex-m33` to
   `armv8-m.main` and `cortex-m0plus` to `armv6s-m`, with an override
   variable for anything else.
4. **Give the parent fixed paths,** for example by linking
   `<install dir>/<arch>/<build type>/{include,lib}` to
   `${BUILD_ROOT}/rtoskit-libc/{include,lib}`. The parent's core files then
   compile with `-nostdinc -isystem ${BUILD_ROOT}/rtoskit-libc/include` in
   place of picolibc's `--specs`, and link `libc.a` (or a flavour archive),
   its own port archive and libgcc in a group.
5. **Hand over the parent's compiler** with `CROSS_COMPILE`, so nothing is
   downloaded twice; it must be GCC 14 or newer.
6. **Order the parent's build** so that the modules which used to depend on
   `libc` depend on `rtoskit-libc`.

A sysroot layout (`make ARCH_CROSS=<arch> ... install` with `MULTIDIR`) and
`install-headers` are the rest of D16, tracked in issue #65.

## Build variables

| Variable | Default | Effect |
|---|---|---|
| `ARCH_CROSS` | every architecture in `ARCHS` | one architecture, e.g. `armv7e-m` |
| `BUILD_TYPE` | `debug`, `release` and `speed` | one build type (`speed` is the speed flavour) |
| `LIBC_MALLOC` | `nano` | the allocator in `libc.a` |
| `HOST_CC` | `gcc` if it is GCC 14 or newer, else the newest `gcc-NN` | the host compiler for the x86_64 build; it must be GCC 14 or newer (D107) |
| `V=1` | off | show the commands |

`ARCH_CROSS` names the architecture's toolchain file under
`tools/makefiles/`; the per-core variables it sets (`LIBC_FENV`,
`LIBC_MATH_LAYERS`, `LIBC_STRING_KERNELS`) are not meant to be overridden on
the command line. A crosstool-NG build drives the same makefiles per multilib (D16,
issue #65).
