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
  - Armv6-M (Cortex-M0/M0+), Armv7E-M with FPv4-SP (Cortex-M4F) or with FPv5-D16 (Cortex-M7), and Armv8-M Mainline with FPv5-SP (Cortex-M33).
  - x86-64, used as the development host.
- **Flavours:**
  - a size build (`release`) and a speed build (`speed`) of the same sources;
  - drop-in flavour archives: `libc-nofloat.a`, `libc-minimal-printf.a`, `libc-tinymalloc.a`;
  - three allocators: nano, TLSF and tiny.

The work ran in four phases: compliance, then size, then speed, then a comparison with Arm Optimized Routines. [`docs/plan/PLAN.md`](docs/plan/PLAN.md) holds the goals, the architecture and the numbered [decision log](docs/plan/PLAN.md#2-decision-log) (D1–D106). Every Dnn cited below is an entry in that log.

## How it compares

A short summary. The full tables, methods and caveats are in the documents linked in each subsection.
- **picolibc 1.8.6 and newlib-nano 4.4:** the Ubuntu 24.04 packages, pinned by SHA-256.
- **Arm Optimized Routines (AOR), revision `f67e7daa`:** AOR is not a C library. It provides string, math and soft-float routines only. So the AOR columns below are rtoskit-libc with AOR's routines linked in place of ours wherever AOR has one ([D92](docs/plan/PLAN.md#2-decision-log), [D93](docs/plan/PLAN.md#2-decision-log)).
- **Build:** every library uses the same arm-none-eabi GCC 16.2 and the same flags.

### Compliance

| | rtoskit-libc |
|---|---|
| libc-test (musl's suite) | 219 of 219 applicable tests pass on all six architectures. 207 tests are skipped as out of scope: POSIX, wide characters, `fork` ([`manifest.txt`](test/libc-test/manifest.txt)). |
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

Code bytes, text plus rodata, of the library's share of each `test/size` program on Cortex-M4F (`armv7e-m`), built `-Os`:

| Program | rtoskit-libc | rtoskit + AOR | picolibc | newlib-nano |
|---|---:|---:|---:|---:|
| `printf("%d")` | 6397 | 6593 | 6381 | 6896 |
| `printf("%d")`, `libc-minimal-printf.a` | 3447 | — | — | — |
| `printf("%f")` | 7717 ¹ | 8181 | 6381 | 17575 |
| `puts` | 1148 ² | 5180 ³ | 212 | 3488 |
| `malloc` / `free` heavy | 1016 | 4982 ³ | 960 | 2156 |
| float math (`sinf`, `expf`, `logf`, ...) | 4408 | 7492 | 6444 | 8237 |

1. Includes the soft-double helpers. Since [D100](docs/plan/PLAN.md#2-decision-log) the library defines them itself instead of taking them from libgcc, which the picolibc and newlib-nano columns do not count.
2. rtoskit's `puts` goes through a buffered, locked `FILE`. picolibc's writes unbuffered through semihosting.
3. AOR's `strlen`/`strcmp` objects carry unwind directives that pull in libgcc's 3.6 KB EHABI unwinder. Our string routines are smaller than AOR's on every core, even without that.

Where the figures come from:
- **AOR column:** the image growth that AOR's routines cause ([`docs/size/phase-4.md`](docs/size/phase-4.md)), added to our current share.
- **Tracking:** current figures for every architecture are in `docs/size/<arch>.md`, for example [`armv7e-m.md`](docs/size/armv7e-m.md). A size budget ([`docs/size/budget.json`](docs/size/budget.json)) fails the build on growth.

See [`docs/size/phase-2.md`](docs/size/phase-2.md) (against picolibc and newlib-nano, all cores) and [`docs/size/phase-4.md`](docs/size/phase-4.md) (against AOR, all cores).

### Performance

Geometric means of guest instructions per operation, counted under qemu, as a ratio to rtoskit's speed flavour. Above 1 means more instructions than rtoskit.

| Cortex-M4F (`armv7e-m`) | rtoskit size | rtoskit speed | rtoskit + AOR | picolibc | newlib-nano |
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

You need:
- **x86-64:** GCC ≥ 14 (or Clang ≥ 19).
- **Arm boards:** `qemu-user`. On Ubuntu 24.04: `apt-get install -y gcc-14 qemu-user`.
- **libc-test:** `git submodule update --init`.

The Arm toolchain, arm-none-eabi GCC 16.2, is downloaded into `local/` on the first build. Versions are in [`docs/toolchain.md`](docs/toolchain.md).

Always run `make` from the project root:

```sh
make                                  # every architecture x every build type
make ARCH_CROSS=armv7e-m              # one architecture
make ARCH_CROSS=x86_64 BUILD_TYPE=release
make check                            # build, then the unit tests, libc-test and the libgcc check
make library                          # only the headers and archives a program links against
make bench                            # benchmarks against docs/perf (needs `make setup-bench`)
make help
```

**Architectures (`ARCH_CROSS`, [D106](docs/plan/PLAN.md#2-decision-log)):** `x86_64`, `armv6-m`, `armv6s-m`, `armv7e-m`, `armv7e-m-dp`, `armv8-m.main`.

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

The allocator inside `libc.a` is chosen with `LIBC_MALLOC=nano|tlsf|tiny` ([D32](docs/plan/PLAN.md#2-decision-log), [D54](docs/plan/PLAN.md#2-decision-log)). Run `make clean` after changing it.

## Using it

### Linking a program

Compile against the installed headers only, and link with `-nostdlib`. List the port, the library and libgcc in one group:

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

- To pick a flavour, link its archive (for example `libc-minimal-printf.a`) instead of `libc.a`.
- The compiler flags must match the architecture the library was built for.

### A GCC spec file

A spec file folds those options into one `-specs=` flag, the way `picolibc.specs` and `nano.specs` do. Save this as `rtoskit-libc.specs`:

```
%rename cpp_unique_options rtoskit_cpp_unique_options

*cpp_unique_options:
-nostdinc -isystem %:getenv(RTOSKIT_LIBC /include) -include stdc-predef.h %(rtoskit_cpp_unique_options)

%rename link rtoskit_link

*link:
-L%:getenv(RTOSKIT_LIBC /lib) --gc-sections %(rtoskit_link)

*lib:
%{-port=*:-lrtos-%*;:-lrtos-none} %{-libc=*:-lc-%*;:-lc}

*startfile:


*endfile:

```

What it does:
- **Compiling:** uses only this library's headers, and pre-includes its `stdc-predef.h`, which defines `__STDC_IEC_559__` where the hardware supports it.
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
| Decision log (D1–D106) | [PLAN.md §2](docs/plan/PLAN.md#2-decision-log) |
| Open questions | [PLAN.md §7](docs/plan/PLAN.md#7-open-questions) |
| Phase task lists | [phase 1](docs/plan/phase-1.md), [phase 2](docs/plan/phase-2.md), [phase 3](docs/plan/phase-3.md), [phase 4](docs/plan/phase-4.md) |
| Configuration: archives, flavours, cores, allocators, ports, parent builds | [`docs/configuration.md`](docs/configuration.md) |
| Toolchain versions and setup | [`docs/toolchain.md`](docs/toolchain.md) |
| Implementation-defined behaviour (Annex J) | [`docs/implementation-defined.md`](docs/implementation-defined.md) |
| Research notes: algorithms, trade-offs, sources, test vectors | [`docs/research/`](docs/research/) |
| Size reports | [phase 1](docs/size/phase-1.md), [phase 2](docs/size/phase-2.md), [phase 4 (AOR)](docs/size/phase-4.md), [picolibc/newlib baseline](docs/size/comparison.md), and per architecture: [x86_64](docs/size/x86_64.md), [armv6-m](docs/size/armv6-m.md), [armv6s-m](docs/size/armv6s-m.md), [armv7e-m](docs/size/armv7e-m.md), [armv7e-m-dp](docs/size/armv7e-m-dp.md), [armv8-m.main](docs/size/armv8-m.main.md) |
| Performance reports | [phase 3](docs/perf/phase-3.md), [phase 4 (AOR)](docs/perf/phase-4.md), and per architecture: [x86_64](docs/perf/x86_64.md), [armv6-m](docs/perf/armv6-m.md), [armv6s-m](docs/perf/armv6s-m.md), [armv7e-m](docs/perf/armv7e-m.md), [armv7e-m-dp](docs/perf/armv7e-m-dp.md), [armv8-m.main](docs/perf/armv8-m.main.md) |
| Contributor rules: workflow, code style, tests | [`CLAUDE.md`](CLAUDE.md) |
| The standard text used for the clean-room work | [`docs/standard/`](docs/standard/) |

## Licence

Mozilla Public License 2.0. See [`LICENSE`](LICENSE).
