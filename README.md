# rtoskit-libc

A clean-room ISO C23 (N3220) standard library for deeply embedded systems.
It binds to an RTOS through a single link-time seam
([`include/rtos/rtos-port.h`](include/rtos/rtos-port.h)). Licence: [MPL-2.0](LICENSE).

- **Standard and language:**
  - Implements every clause 7 header in scope: everything except `<wchar.h>`, `<wctype.h>`, `<uchar.h>`, Annex K and decimal floating point.
  - Includes the C23 additions: `<stdbit.h>`, `<stdckdint.h>`, `memset_explicit`, `strfromd`, `timegm` and so on.
  - Includes `<threads.h>` and `<stdatomic.h>`.
  - Written in C23 itself (`nullptr`, `constexpr`, `thread_local`).
- **Built for small parts:**
  - One function per file and section, so `--gc-sections` drops everything a program does not call.
  - Per-thread state (`errno`, `strtok`, `rand`, the `<time.h>` buffers) is `thread_local`.
  - There is no hidden global state and no syscalls.
  - Variation comes from the build selecting files, not from `#if`.
- **One seam to the OS:**
  - Locks and futexes, threads, TLS, time, I/O, the heap regions and exit are all `__rtos_*` functions.
  - A port implements them in its own archive.
  - `port/none` (bare metal, single thread) and `port/linux` (the test port) are included.
- **Targets:**
  - Armv6-M (Cortex-M0/M0+), Armv7-M without an FPU (Cortex-M3), Armv7E-M with FPv4-SP (Cortex-M4F) or with FPv5-D16 (Cortex-M7), and Armv8-M Mainline with FPv5-SP (Cortex-M33).
  - x86-64, used as the development host.
- **Flavours:**
  - a size build (`release`) and a speed build (`speed`) of the same sources;
  - drop-in flavour archives: `libc-nofloat.a`, `libc-minimal-printf.a`, `libc-tinymalloc.a`;
  - three allocators: nano, TLSF and tiny.

The work ran in four phases: compliance, then size, then speed, then a comparison with Arm Optimized Routines. [`docs/plan/PLAN.md`](docs/plan/PLAN.md) holds the goals, the architecture and the numbered [decision log](docs/plan/PLAN.md#2-decision-log) (D1–D112). Every Dnn cited below is an entry in that log.

## How it compares

A short summary. The full tables, methods and caveats are in the documents linked in each subsection.
- **picolibc 1.8.6 and newlib-nano 4.4:** the Ubuntu 24.04 packages, pinned by SHA-256.
- **Arm Optimized Routines (AOR), revision `f67e7daa`:** AOR is not a C library. It provides string, math and soft-float routines only. So the AOR columns below are rtoskit-libc with AOR's routines linked in place of ours wherever AOR has one ([D92](docs/plan/PLAN.md#2-decision-log), [D93](docs/plan/PLAN.md#2-decision-log)).
- **Build:** every library uses the same arm-none-eabi GCC 16.2 and the same flags.

### Compliance

| | rtoskit-libc |
|---|---|
| libc-test (musl's suite) | 219 of 219 applicable tests pass on all seven architectures. 207 tests are skipped as out of scope: POSIX, wide characters, `fork` ([`manifest.txt`](test/libc-test/manifest.txt)). |
| Annex B header audit | Checks 689 identifiers, 642 exact prototypes and 73 generic names across 28 headers. It runs in `make check` (`test/unit/annexb`). |
| Freestanding check | The freestanding headers' functions link without any port (`test/unit/freestanding`). |
| `<threads.h>` | Implemented over the seam. picolibc and newlib-nano have none. |
| Formatted I/O | `printf` `%f`/`%e`/`%g` and `strtod` are exact (correctly rounded) for any length ([D38](docs/plan/PLAN.md#2-decision-log), [D39](docs/plan/PLAN.md#2-decision-log)). |
| Implementation-defined behaviour | Documented clause by clause against Annex J.3 and J.4 in [`docs/implementation-defined.md`](docs/implementation-defined.md). |

The results come from `make check`. libc-test was run only against this library, not against the reference libraries. See [`docs/research/conformance.md`](docs/research/conformance.md).

### Accuracy

| | rtoskit-libc | picolibc / newlib | AOR |
|---|---|---|---|
| Float functions, round to nearest | Correctly rounded. On x86-64 and Cortex-M7 this is checked exhaustively over all 2^32 arguments ([D98](docs/plan/PLAN.md#2-decision-log)). The soft-float and single-precision cores are within the allowance documented in [D57](docs/plan/PLAN.md#2-decision-log). | fdlibm-style, up to about 1 ulp | 0.004–3.9 % of floats misrounded per function, worst 1.09 ulp |
| Float functions, directed modes | Correctly rounded | not measured | Up to 2 ulp; up to 35 % misrounded upward |
| Double functions | Below 0.53 ulp; at most 0.023 % misrounded | Up to about 1 ulp | Below 0.53 ulp, except `erf` at 0.89 ulp |
| Exceptions | Raised by arithmetic on every core, including the soft `<fenv.h>` on Armv6-M | — | Its Arm soft-float raises no flags |

See:
- [`docs/research/optimized-routines.md`](docs/research/optimized-routines.md) (Accuracy section);
- [`docs/research/float-hard-cases.md`](docs/research/float-hard-cases.md);
- the `libm-*.md` notes for each function family in [`docs/research/`](docs/research/).

### Size

Code bytes, text plus rodata, of the library's share of each `test/size` program on Cortex-M4F (`armv7e-m`), built `-Os`.
- **Bytes:** our figures come from the `release` build (the size flavour).
- **Ratios:** each other library divided by rtoskit-libc. Above 1 means larger than rtoskit-libc.

| Program | rtoskit-libc | rtoskit + AOR | picolibc | newlib-nano | rtoskit + AOR ratio | picolibc ratio | newlib-nano ratio |
|---|---:|---:|---:|---:|---:|---:|---:|
| `printf("%d")` | 6369 | 6565 | 6381 | 6896 | 1.03 | 1.00 | 1.08 |
| `printf("%d")`, `libc-minimal-printf.a` | 3419 | — | — | — | — | — | — |
| `printf("%f")` | 7689 ¹ | 8153 | 6381 | 17575 | 1.06 | 0.83 | 2.29 |
| `puts` | 1120 ² | 5152 ³ | 212 | 3488 | 4.60 | 0.19 | 3.11 |
| `malloc` / `free` heavy | 988 | 4954 ³ | 960 | 2156 | 5.01 | 0.97 | 2.18 |
| float math (`sinf`, `expf`, `logf`, ...) | 4380 | 7464 | 6444 | 8237 | 1.70 | 1.47 | 1.88 |

1. Includes the soft-double helpers. Since [D100](docs/plan/PLAN.md#2-decision-log) the library defines them itself instead of taking them from libgcc, which the picolibc and newlib-nano columns do not count.
2. rtoskit's `puts` goes through a buffered, locked `FILE`. picolibc's writes unbuffered through semihosting.
3. AOR's `strlen`/`strcmp` objects carry unwind directives that pull in libgcc's 3.6 KB EHABI unwinder. Our string routines are smaller than AOR's on every core, even without that.

Where the figures come from:
- **AOR column:** the image growth that AOR's routines cause ([`docs/size/phase-4.md`](docs/size/phase-4.md)), added to our current share.
- **Tracking:** current figures for every architecture are in `docs/size/<arch>.md`, for example [`armv7e-m.md`](docs/size/armv7e-m.md). A size budget ([`docs/size/budget.json`](docs/size/budget.json)) fails the build on growth.

See [`docs/size/phase-2.md`](docs/size/phase-2.md) (against picolibc and newlib-nano, all cores) and [`docs/size/phase-4.md`](docs/size/phase-4.md) (against AOR, all cores).

### Performance

Geometric means of guest instructions per operation, counted under qemu. Each column is divided by the count of rtoskit's speed flavour, so that column is 1.00. Above 1 means more instructions than the speed flavour.
- **release (size flavour):** rtoskit's `release` build, the smallest code and the default to ship. The column shows what it costs in speed.
- **speed flavour:** rtoskit's `speed` build, the reference.
- **The other columns:** the same programs built against each library: rtoskit's speed flavour with AOR's routines linked in, picolibc and newlib-nano.

| Cortex-M4F (`armv7e-m`) | rtoskit release (size flavour) | rtoskit speed flavour | rtoskit + AOR | picolibc | newlib-nano |
|---|---:|---:|---:|---:|---:|
| string (603 cases) | 1.44 | 1.00 | 0.98 | 1.40 | 1.12 |
| stdio (23) | 1.22 | 1.00 | 1.00 | 1.15 | 0.92 |
| `strto*` (21) | 5.61 | 1.00 | 0.97 | 0.85 | 1.52 |
| malloc (9) | 1.07 | 1.00 | 1.00 | 7.21 | 0.68 |
| math, throughput | 1.00 | 1.00 | 0.95 | 0.36 | 0.38 |

How the columns were measured:
- **picolibc and newlib-nano:** the phase 3 comparison ([`docs/perf/phase-3.md`](docs/perf/phase-3.md)).
- **AOR:** the phase 4 records against today's speed baseline. AOR only substitutes `memchr`, `memset`, `strlen`, 15 math functions and the soft-double helpers, so every other case counts as 1.00. Over the math cases AOR does replace, the ratio is 0.81, and the double `exp`/`log`/`pow` take 0.16–0.45 of our instructions.

What the ratios show:
- **Faster:**
  - Bulk `memcpy`/`memset`: the speed flavour is the fastest of the three.
  - Decimal conversions.
  - `realloc`, against picolibc.
  - The speed flavour's introsort `qsort` ([D90](docs/plan/PLAN.md#2-decision-log)): the others take 1.35–1.49 times its instructions.
  - Float math on single-precision and soft-float cores: AOR takes 4–7 times our instructions there.
- **Slower:**
  - Math functions take 2–3 times picolibc's instructions. That is the price of correct rounding.
  - `%f`/`%g`, because formatting is exact.
  - newlib-nano's allocator, which takes no lock, while ours is always thread-safe.

See:
- [`docs/perf/phase-3.md`](docs/perf/phase-3.md) (all cores, against picolibc and newlib-nano);
- [`docs/perf/phase-4.md`](docs/perf/phase-4.md) (all cores, against AOR);
- `docs/perf/<arch>.md` for current figures, for example [`armv7e-m.md`](docs/perf/armv7e-m.md);
- [`docs/research/benchmarks.md`](docs/research/benchmarks.md) for the method and its limits: instruction counts are blind to pipelines and memory.

## Building

### Compiler requirements

The library is written in C23 and its headers use C23. Everything that compiles against it needs a C23 compiler.

| What | Compiler | Notes |
|---|---|---|
| Building the library for x86-64 | Host GCC 14 or newer | Uses plain `gcc` when it is new enough, otherwise the newest `gcc-NN` found; set `HOST_CC` to choose one ([D107](docs/plan/PLAN.md#2-decision-log)). Ubuntu 24.04's `gcc` is 13, so install `gcc-14` there. |
| Building the library for Arm | arm-none-eabi GCC 14 or newer | By default the first build downloads GCC 16.2 into `local/`. Pass `CROSS_COMPILE=<prefix>` to use your own instead; it is checked for version 14 or newer ([D105](docs/plan/PLAN.md#2-decision-log)). |
| Compiling programs that use the library | GCC 14 or newer, in C23 mode | The installed headers need `-std=c23` or `-std=gnu23`; in C11/C17 mode they do not compile. GCC 15 and newer default to `gnu23`. GCC 14 defaults to `gnu17`, so pass the flag (the [spec file](#a-gcc-spec-file) adds `-std=gnu23` when no `-std` is given). |

- **GCC only:** the build relies on GCC options (`-fcallgraph-info`, `-muclibc` on x86-64), so Clang cannot build the library today. Compiling programs against it with Clang 19 or newer in C23 mode is untested.
- **Versions:** the versions this project is tested with are in [`docs/toolchain.md`](docs/toolchain.md).

### Other tools

- **Arm test runs:** `qemu-user`. On Ubuntu 24.04: `apt-get install -y gcc-14 qemu-user`.
- **libc-test:** `git submodule update --init`.

### Selecting the architecture

Each build targets one architecture, named by `ARCH_CROSS`. Without it, `make` builds all seven ([D106](docs/plan/PLAN.md#2-decision-log)). The architecture fixes the compiler flags, the FPU code, `<fenv.h>` and the string kernels; there is nothing else to configure.

| `ARCH_CROSS` | Cores | Compiler flags | FPU and `<fenv.h>` |
|---|---|---|---|
| `armv6-m` | Armv6-M parts without the OS extension (no `SVC`) | `-mthumb -march=armv6-m -mfloat-abi=soft` | none; soft float, round to nearest only |
| `armv6s-m` | Cortex-M0, M0+ | `-mthumb -march=armv6s-m -mfloat-abi=soft` | none; soft float, round to nearest only |
| `armv7-m` | Cortex-M3; a Cortex-M4 or M33 built without its FPU | `-mthumb -march=armv7-m -mfloat-abi=soft` | none; soft float, round to nearest only, with the library's own Thumb-2 float and double helpers ([D108](docs/plan/PLAN.md#2-decision-log), [D109](docs/plan/PLAN.md#2-decision-log)) |
| `armv7e-m` | Cortex-M4F, and Cortex-M7 with the single-precision FPU | `-mthumb -march=armv7e-m+fp -mfloat-abi=hard` | FPv4-SP; round to nearest only |
| `armv7e-m-dp` | Cortex-M7 with the double-precision FPU | `-mthumb -march=armv7e-m+fp.dp -mfloat-abi=hard` | FPv5-D16; every rounding direction and exception flag |
| `armv8-m.main` | Cortex-M33 (and M35P) with FPU and DSP | `-mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=hard` | FPv5-SP; round to nearest only |
| `x86_64` | the development host | `-mlong-double-64` | SSE2; full. Runs only through the Linux test port. |

- **Another core:** use the build whose instruction set and float ABI the core supports. For example, a Cortex-M23 (Armv8-M Baseline) can link `armv6s-m`, and a Cortex-M55 or M85 can link `armv8-m.main`.
- **Program flags must match:** compile your program with the same `-march` and `-mfloat-abi` as the build it links.
- **Per-core detail:** the libm layers and string kernels each core uses are in [`docs/configuration.md`](docs/configuration.md#choosing-the-core).

```sh
make -j$(nproc) ARCH_CROSS=armv8-m.main                          # Cortex-M33, all three build types
make -j$(nproc) ARCHS="armv6s-m armv8-m.main" BUILD_TYPE=release # two cores, size flavour only
make ARCH_CROSS=cortex-m33                                       # stops: lists the valid architectures
```

### Make goals

Always run `make` from the project root. Without a goal, `make` builds everything.

| Goal | What it does |
|---|---|
| `all` (the default) | Builds and installs the library, the ports, the tests, the examples and the size and benchmark programs |
| `library` | Builds and installs only what a program links against: the headers, the `libc*.a` archives and the port archives ([D105](docs/plan/PLAN.md#2-decision-log)) |
| `check` | Builds, then runs the unit tests, libc-test against its baseline, and the libgcc check |
| `bench` | Builds the `release` and `speed` flavours and runs the benchmarks against [`docs/perf`](docs/perf/); needs `make setup-bench` once |
| `aor` | Fetches and builds Arm Optimized Routines for comparison, and runs its smoke test (measurement only) |
| `setup`, `setup-bench` | Downloads the Arm toolchain into `local/` (the first build does this itself); builds the instruction-counting qemu |
| `info` | Prints the configuration one architecture and build type resolve to |
| `clean`, `distclean`, `realclean` | Removes build artifacts; also `build/` and `images/`; also the downloaded tools in `local/` |
| `db` | Writes `compile_commands.json` (needs `bear`) |
| `<module>`, e.g. `libc/string`, `test/unit/printf` | Rebuilds one module, with no dependencies. Needs one `ARCH_CROSS`, one `BUILD_TYPE` and a previous full build |
| `help` | Lists the goals and variables |

### Build options

Pass these on the `make` command line.

| Variable | Default | Effect |
|---|---|---|
| `ARCH_CROSS` | `all` | One architecture: `x86_64`, `armv6-m`, `armv6s-m`, `armv7-m`, `armv7e-m`, `armv7e-m-dp` or `armv8-m.main` ([D106](docs/plan/PLAN.md#2-decision-log)). An unknown name stops with the list. |
| `ARCHS` | all seven | The architectures `ARCH_CROSS=all` loops over, e.g. `ARCHS="armv6-m armv8-m.main"` |
| `BUILD_TYPE` | `all` | One build type: `debug`, `release` or `speed` (see [Build types](#build-types)) |
| `BUILDS` | `debug release speed` | The build types `BUILD_TYPE=all` loops over |
| `LIBC_MALLOC` | `nano` | The allocator inside `libc.a`: `nano`, `tlsf` or `tiny` ([D32](docs/plan/PLAN.md#2-decision-log), [D54](docs/plan/PLAN.md#2-decision-log)). Run `make clean` after changing it. Every allocator is still built and tested. |
| `HOST_CC` | found | The host GCC for x86-64; must be GCC 14 or newer ([D107](docs/plan/PLAN.md#2-decision-log)) |
| `CROSS_COMPILE` | downloaded GCC 16.2 | The prefix of your own arm-none-eabi toolchain, e.g. `/opt/arm/bin/arm-none-eabi-`; must be GCC 14 or newer |
| `OUTPUT_ROOT` | `build` | Where the build trees go |
| `PREFIX` | `images` | Where the installs go |
| `V=1` | off | Shows the full commands |
| `LIBC_TEST_UPDATE=1` | off | With `check`: records libc-test improvements in the architecture's baseline. It refuses while anything regressed. |
| `BENCH_UPDATE=1` | off | With `bench`: records the results as the new benchmark baseline |

Parallel builds (`make -j`, with or without a job count) are supported. More variables, and the ones a parent build must pass, are in [`docs/configuration.md`](docs/configuration.md#build-variables).

### Examples

```sh
# Everything: all seven architectures x debug, release and speed
make -j$(nproc)

# One architecture, all three build types
make -j$(nproc) ARCH_CROSS=armv7e-m

# Only the size flavour, for two Cortex-M cores
make -j$(nproc) ARCHS="armv6-m armv8-m.main" BUILD_TYPE=release

# Only what a program links against, for one core, into your own directories
make -j$(nproc) ARCH_CROSS=armv8-m.main BUILD_TYPE=release \
    OUTPUT_ROOT=$HOME/tmp/rtoskit-build PREFIX=$HOME/opt/rtoskit library

# libc.a with the TLSF allocator
make clean
make -j$(nproc) LIBC_MALLOC=tlsf

# Your own compilers instead of the found host GCC and the downloaded Arm GCC
make -j$(nproc) HOST_CC=gcc-15 CROSS_COMPILE=/opt/arm/bin/arm-none-eabi-

# Test one configuration, then record any libc-test improvements in its baseline
make -j$(nproc) ARCH_CROSS=x86_64 BUILD_TYPE=debug check
make ARCH_CROSS=x86_64 BUILD_TYPE=debug check LIBC_TEST_UPDATE=1

# After a full build: rebuild one module, or one unit test, showing the commands
make ARCH_CROSS=armv7e-m BUILD_TYPE=release V=1 libc/string
make ARCH_CROSS=x86_64 BUILD_TYPE=debug test/unit/printf

# The configuration one build resolves to (toolchain, FPU, paths)
make ARCH_CROSS=armv7e-m BUILD_TYPE=release info
```

### Leaving out the tests

`make` (`all`) always builds the unit tests, libc-test, the examples and the size and benchmark programs; `make check` needs them. To build only the library, use the `library` goal. It builds and installs the headers, `libc*.a`, `libm.a` and the port archives, and nothing else. It works on a fresh tree, downloading the Arm toolchain first if needed, and loops over architectures and build types like `all`.

```sh
make -j$(nproc) library                                          # every architecture and build type, no tests
make -j$(nproc) ARCH_CROSS=armv7e-m BUILD_TYPE=release library   # one core, size flavour, no tests
```

The libc-test submodule is only needed for `make check`; `make library` builds without it.

### Build types

`BUILD_TYPE` picks one of three builds of the same sources. Without it, `make` builds all three. Each build has its own build tree and install.

| `BUILD_TYPE` | Optimisation | What it is for |
|---|---|---|
| `debug` | `-O0 -g3`, `assert` enabled | Stepping through the library in a debugger. Not size- or speed-tracked. |
| `release` | `-Os -g3`, `NDEBUG` | **The size flavour**, and the default to ship. The smallest code. The size budget ([`docs/size/budget.json`](docs/size/budget.json)) is measured on it. |
| `speed` | `-Os -g3`, `NDEBUG`, plus speed kernels | **The speed flavour** ([D73](docs/plan/PLAN.md#2-decision-log), [D77](docs/plan/PLAN.md#2-decision-log)). The same library with faster replacements: 32-byte-per-iteration `memcpy`/`memmove`/`memset` and word-at-a-time `strcmp`/`strchr` on Thumb-2; block copies on Armv6-M; `strtol`, `printf` and `strtod` fast paths; fixed-point float math on Armv6-M. Code stays within 10 % of the size flavour (20 % for the math programs). |

Both optimised flavours install the same headers and archive names, so a program switches flavour by pointing at the other install.

One caveat: the speed flavour's Thumb-2 kernels use unaligned `LDR`/`STR`, as GCC's own code does. Link the size flavour on a system that:
- sets `CCR.UNALIGN_TRP`; or
- copies Device memory with `memcpy`.

Details are in [`docs/configuration.md`](docs/configuration.md#size-or-speed).

### Output

- Build trees go to `build/<arch>/<build-type>/`.
- Installs go to `images/<arch>/<build-type>/`:
  - `include/`;
  - `lib/` with `libc.a`, `libm.a`, the flavour archives, `librtos-none.a` and `librtos-linux.a`.
- `OUTPUT_ROOT` and `PREFIX` move the two roots; the `<arch>/<build-type>/` layout below them stays the same.

## Using it

### Linking a program

Compile against the installed headers only, and link with `-nostdlib`. List the port, the library and libgcc in one group:

```sh
arm-none-eabi-gcc -march=armv7e-m+fp -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -nostdinc -isystem images/armv7e-m/release/include \
    -std=c23 -Os -ffunction-sections -fdata-sections -c app.c
arm-none-eabi-gcc -march=armv7e-m+fp -mfloat-abi=hard -mfpu=fpv4-sp-d16 \
    -nostdlib -static -Wl,--gc-sections -T board.ld app.o startup.o \
    -Wl,--start-group images/armv7e-m/release/lib/librtos-none.a \
    images/armv7e-m/release/lib/libc.a \
    $(arm-none-eabi-gcc -march=armv7e-m+fp -mfloat-abi=hard -print-libgcc-file-name) \
    -Wl,--end-group
```

- To pick a flavour, link its archive (for example `libc-minimal-printf.a`) instead of `libc.a`.
- Compile in C23 mode (`-std=c23` or `-std=gnu23`): the headers need it.
- The compiler flags must match the architecture the library was built for.

### Choosing printf

The `printf` and `scanf` flavour is chosen when you link, not when you build: every build installs all the flavour archives. Link one of them in place of `libc.a` ([D53](docs/plan/PLAN.md#2-decision-log), [D54](docs/plan/PLAN.md#2-decision-log)):

| Archive | `printf`/`scanf` | Whole `printf("%d")` program on Cortex-M4F / M0+ |
|---|---|---:|
| `libc.a` | everything, including exact `%a %e %f %g` | 7165 / 7757 bytes |
| `libc-nofloat.a` | no floating conversions: `%a %e %f %g` fail with `EINVAL`. `strtod` still works. | 4451 / 4975 bytes |
| `libc-minimal-printf.a` | as `libc-nofloat.a`, and `%n` and lengths wider than `long` (`ll`, `j`, 64-bit `z`) fail with `EINVAL`, so no 64-bit division is linked | 4211 / 4707 bytes |

- **Unsupported conversions fail loudly:** the call returns an error (`EINVAL`) instead of printing nothing.
- **Allocator:** `libc-tinymalloc.a` is a fourth flavour archive. It swaps the allocator for the smallest one, `tiny`, without rebuilding. `LIBC_MALLOC` changes the allocator inside `libc.a` itself.
- **With the spec file:** select a flavour with `--libc=`.

```sh
# Directly
arm-none-eabi-gcc ... -Wl,--start-group images/armv7e-m/release/lib/librtos-none.a \
    images/armv7e-m/release/lib/libc-minimal-printf.a $(arm-none-eabi-gcc ... -print-libgcc-file-name) -Wl,--end-group
# With the spec file
arm-none-eabi-gcc ... -specs=rtoskit-libc.specs --libc=nofloat -T board.ld startup.o app.o -o app.elf
```

Sizes are whole images of the `test/size` reference programs, release build, from [`docs/size/baseline-armv7e-m.json`](docs/size/baseline-armv7e-m.json) and [`baseline-armv6-m.json`](docs/size/baseline-armv6-m.json). More detail is in [`docs/configuration.md`](docs/configuration.md#flavour-archives-d53-d54).

### A GCC spec file

A spec file folds those options into one `-specs=` flag, the way `picolibc.specs` and `nano.specs` do. Save this as `rtoskit-libc.specs`:

```
%rename cpp_unique_options rtoskit_cpp_unique_options

*cpp_unique_options:
-nostdinc -isystem %:getenv(RTOSKIT_LIBC /include) -include stdc-predef.h %(rtoskit_cpp_unique_options)

%rename cc1 rtoskit_cc1

*cc1:
%{!std=*:-std=gnu23} %(rtoskit_cc1)

%rename link rtoskit_link

*link:
-L%:getenv(RTOSKIT_LIBC /lib) --gc-sections %(rtoskit_link)

*lib:
%{-port=*:-lrtos-%*;:-lrtos-none} %{-libc=*:-lc-%*;:-lc}

*startfile:


*endfile:

```

What it does:
- **Compiling:**
  - uses only this library's headers, and pre-includes its `stdc-predef.h`, which defines `__STDC_IEC_559__` where the hardware supports it;
  - compiles as `-std=gnu23` unless you pass your own `-std`, which GCC 14 needs (its default is `gnu17`).
- **Linking:**
  - puts the library's `lib/` ahead of the toolchain's own, so `-lc` is this `libc.a` and not the toolchain's;
  - garbage-collects unused sections;
  - links the port and the library inside arm-none-eabi GCC's `--start-group -lgcc ... --end-group`.
- **Options:**
  - `--port=<name>` picks the port archive (default `none`, i.e. `librtos-none.a`);
  - `--libc=<flavour>` picks a flavour archive, for example `--libc=minimal-printf` for `libc-minimal-printf.a`.
- **Startup:** the toolchain's crt files are dropped, so your startup code and linker script provide `_start` and the memory layout.
- **Library path:** `RTOSKIT_LIBC` names the install to use. GCC stops with an error if it is not set.

```sh
export RTOSKIT_LIBC=$PWD/images/armv7e-m/release
arm-none-eabi-gcc -mthumb -march=armv7e-m+fp -mfloat-abi=hard -specs=rtoskit-libc.specs \
    -std=c23 -Os -ffunction-sections -fdata-sections -c app.c
arm-none-eabi-gcc -mthumb -march=armv7e-m+fp -mfloat-abi=hard -specs=rtoskit-libc.specs \
    -T board.ld startup.o app.o -o app.elf
```

This was tested with arm-none-eabi GCC 16.2 on `armv7e-m`, using the Linux test port: `--port=linux` plus that port's `headers.ld`. A threaded `printf`/`sinf` program ran under `qemu-arm`, both with `libc.a` and with `--libc=minimal-printf`.

### Binding to your RTOS

To bind to an RTOS:
1. Copy `port/none`.
2. Implement the `__rtos_*` functions declared in [`include/rtos/rtos-port.h`](include/rtos/rtos-port.h): locks and futexes, thread creation, TLS setup, time, I/O, heap regions and exit ([D4](docs/plan/PLAN.md#2-decision-log)).
3. Link the port's archive in place of `librtos-none.a` (with the spec file, `--port=<name>` for `librtos-<name>.a`).

On bare metal, `port/none` needs the following:
- **Startup:** your startup code calls `__rtos_none_start()`.
- **Heap:** the linker script defines the heap bounds.
- **Output:** optionally, define `__rtos_none_output()` to send `stdout` somewhere, such as a UART.

See:
- [`docs/configuration.md`](docs/configuration.md#ports-d4) (Ports);
- [`docs/research/threads.md`](docs/research/threads.md) (the seam);
- [`docs/research/tls-and-startup.md`](docs/research/tls-and-startup.md) (TLS and startup).

### Building from another project

A parent build runs `make library` as a sub-make ([D105](docs/plan/PLAN.md#2-decision-log)). It passes its roots, the architecture and, optionally, its own compiler:

```make
$(MAKE) -C $(LIBC_DIR) \
	PROJECT_ROOT=$(LIBC_DIR) TOOLS_ROOT=$(LIBC_DIR)/tools \
	OUTPUT_ROOT=<build dir> PREFIX=<install dir> \
	ARCH_CROSS=armv8-m.main BUILD_TYPE=release \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	library
```

Then it links `<install dir>/armv8-m.main/release/{include,lib}` as shown above, or sets `RTOSKIT_LIBC` to that install and uses the spec file.
- Pass every root on the command line: a parent with the same conventions would otherwise leak in its own values.
- An external `CROSS_COMPILE` is used as given, and must be GCC 14 or newer ([D9](docs/plan/PLAN.md#2-decision-log)).

[`docs/configuration.md`](docs/configuration.md#building-from-another-project-d105), "Building from another project", covers the details and the module glue for a pi-pico-style parent. A sysroot and crosstool-NG integration are tracked in issue #65.

## Documentation

| Topic | Where |
|---|---|
| Goals, architecture, scope | [`docs/plan/PLAN.md`](docs/plan/PLAN.md) |
| Decision log (D1–D112) | [PLAN.md §2](docs/plan/PLAN.md#2-decision-log) |
| Open questions | [PLAN.md §7](docs/plan/PLAN.md#7-open-questions) |
| Phase task lists | [phase 1](docs/plan/phase-1.md), [phase 2](docs/plan/phase-2.md), [phase 3](docs/plan/phase-3.md), [phase 4](docs/plan/phase-4.md), [phase 5](docs/plan/phase-5.md) |
| Configuration: archives, flavours, cores, allocators, ports, parent builds | [`docs/configuration.md`](docs/configuration.md) |
| Toolchain versions and setup | [`docs/toolchain.md`](docs/toolchain.md) |
| Implementation-defined behaviour (Annex J) | [`docs/implementation-defined.md`](docs/implementation-defined.md) |
| Research notes: algorithms, trade-offs, sources, test vectors | [`docs/research/`](docs/research/) |
| Size reports | [phase 1](docs/size/phase-1.md), [phase 2](docs/size/phase-2.md), [phase 4 (AOR)](docs/size/phase-4.md), [picolibc/newlib baseline](docs/size/comparison.md), and per architecture: [x86_64](docs/size/x86_64.md), [armv6-m](docs/size/armv6-m.md), [armv6s-m](docs/size/armv6s-m.md), [armv7-m](docs/size/armv7-m.md), [armv7e-m](docs/size/armv7e-m.md), [armv7e-m-dp](docs/size/armv7e-m-dp.md), [armv8-m.main](docs/size/armv8-m.main.md) |
| Performance reports | [phase 3](docs/perf/phase-3.md), [phase 4 (AOR)](docs/perf/phase-4.md), and per architecture: [x86_64](docs/perf/x86_64.md), [armv6-m](docs/perf/armv6-m.md), [armv6s-m](docs/perf/armv6s-m.md), [armv7-m](docs/perf/armv7-m.md), [armv7e-m](docs/perf/armv7e-m.md), [armv7e-m-dp](docs/perf/armv7e-m-dp.md), [armv8-m.main](docs/perf/armv8-m.main.md) |
| Contributor rules: workflow, code style, tests | [`CLAUDE.md`](CLAUDE.md) |
| The standard text used for the clean-room work | [`docs/standard/`](docs/standard/) |

## Licence

Mozilla Public License 2.0. See [`LICENSE`](LICENSE).
