# rtoskit-libc

A clean-room ISO C23 (N3220) standard library for deeply embedded systems.
It binds to an RTOS through a single link-time seam
(`include/rtos/rtos-port.h`). Licence: MPL-2.0.

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

The work ran in four phases: compliance, then size, then speed, then a comparison with Arm Optimized Routines. `docs/plan/PLAN.md` holds the goals, the architecture and the numbered decision log (D1–D106) that the rest of the documentation cites.

## How it compares

A short summary. The full tables, methods and caveats are in the documents linked in each subsection.
- **Reference libraries:** picolibc 1.8.6 and newlib-nano 4.4 (Ubuntu 24.04 packages pinned by SHA-256), and Arm Optimized Routines (AOR) at `f67e7daa`.
- **Build:** the same arm-none-eabi GCC 16.2 and the same flags for every library.

### Compliance

| | rtoskit-libc |
|---|---|
| libc-test (musl's suite) | 219 of 219 applicable tests pass on all six architectures. 207 tests are skipped as out of scope: POSIX, wide characters, `fork`. |
| Annex B header audit | Checks 689 identifiers, 642 exact prototypes and 73 generic names across 28 headers. It runs in `make check` (`test/unit/annexb`). |
| Freestanding check | The freestanding headers' functions link without any port (`test/unit/freestanding`). |
| `<threads.h>` | Implemented over the seam. picolibc and newlib-nano have none. |
| Formatted I/O | `printf` `%f`/`%e`/`%g` and `strtod` are exact (correctly rounded) for any length (D38, D39). |
| Implementation-defined behaviour | Documented clause by clause against Annex J.3 and J.4 in `docs/implementation-defined.md`. |

The results come from `make check`. libc-test was run only against this library, not against the reference libraries. See `docs/research/conformance.md` and `test/libc-test/manifest.txt`.

### Accuracy

| | rtoskit-libc | picolibc / newlib | AOR |
|---|---|---|---|
| Float functions, round to nearest | Correctly rounded. On x86-64 and Cortex-M7 this is checked exhaustively over all 2^32 arguments (D98). The soft-float and single-precision cores are within D57's documented allowance. | fdlibm-style, up to about 1 ulp | 0.004–3.9 % of floats misrounded per function, worst 1.09 ulp |
| Float functions, directed modes | Correctly rounded | not measured | Up to 2 ulp; up to 35 % misrounded upward |
| Double functions | Below 0.53 ulp; at most 0.023 % misrounded | Up to about 1 ulp | Below 0.53 ulp, except `erf` at 0.89 ulp |
| Exceptions | Raised by arithmetic on every core, including the soft `<fenv.h>` on Armv6-M | — | Its Arm soft-float raises no flags |

See:
- `docs/research/optimized-routines.md` (Accuracy section);
- `docs/research/float-hard-cases.md`;
- the `docs/research/libm-*.md` notes for each function family.

### Size

Code bytes, text plus rodata, of the library's share of each `test/size` program on Cortex-M4F (`armv7e-m`), built `-Os`:

| Program | rtoskit-libc | picolibc | newlib-nano |
|---|---:|---:|---:|
| `printf("%d")` | 6397 | 6381 | 6896 |
| `printf("%d")`, `libc-minimal-printf.a` | 3447 | — | — |
| `printf("%f")` | 7717 ¹ | 6381 | 17575 |
| `puts` | 1148 ² | 212 | 3488 |
| `malloc` / `free` heavy | 1016 | 960 | 2156 |
| float math (`sinf`, `expf`, `logf`, ...) | 4408 | 6444 | 8237 |

1. Includes the soft-double helpers. Since D100 the library defines them itself instead of taking them from libgcc, which the other columns do not count.
2. rtoskit's `puts` goes through a buffered, locked `FILE`. picolibc's writes unbuffered through semihosting.

Other points:
- **Arm Optimized Routines:** our string routines are smaller than AOR's on every core. AOR's `strcmp`/`strlen` objects also pull in libgcc's 3.6 KB EHABI unwinder.
- **Tracking:** current figures for every architecture are in `docs/size/<arch>.md`. A size budget (`docs/size/budget.json`) fails the build on growth.

See `docs/size/phase-2.md` (against picolibc and newlib-nano) and `docs/size/phase-4.md` (against AOR).

### Performance

Geometric means of guest instructions per operation, counted under qemu, as a ratio to rtoskit's speed flavour. Above 1 means more instructions than rtoskit.

| Cortex-M4F (`armv7e-m`) | rtoskit size | rtoskit speed | picolibc | newlib-nano |
|---|---:|---:|---:|---:|
| string (603 cases) | 1.44 | 1.00 | 1.40 | 1.12 |
| stdio (23) | 1.22 | 1.00 | 1.15 | 0.92 |
| `strto*` (21) | 5.61 | 1.00 | 0.85 | 1.52 |
| malloc (9) | 1.07 | 1.00 | 7.21 | 0.68 |
| math (124) | 1.00 | 1.00 | 0.36 | 0.38 |

What the ratios show:
- **Faster:**
  - Bulk `memcpy`/`memset`: the speed flavour is the fastest of the three.
  - Decimal conversions.
  - `realloc`, against picolibc.
  - The speed flavour's introsort `qsort`: the others take 1.35–1.49 times its instructions (D90).
- **Slower:**
  - The math functions take 2–3 times the instructions. That is the price of correct rounding.
  - `%f`/`%g`, because formatting is exact.
  - newlib-nano's allocator, which takes no lock, while ours is always thread-safe.
- **Against AOR:** AOR is ahead on double `exp`/`log`/`pow` and on float math where the FPU has double precision. We are ahead on float math on single-precision and soft-float cores, where AOR takes 4–7 times our instructions.

See:
- `docs/perf/phase-3.md` (all cores, against picolibc and newlib-nano);
- `docs/perf/phase-4.md` (against AOR);
- `docs/perf/<arch>.md` (current figures);
- `docs/research/benchmarks.md` (method and limits: instruction counts are blind to pipelines and memory).

## Building

You need:
- **x86-64:** GCC ≥ 14 (or Clang ≥ 19).
- **Arm boards:** `qemu-user`. On Ubuntu 24.04: `apt-get install -y gcc-14 qemu-user`.
- **libc-test:** `git submodule update --init`.

The Arm toolchain, arm-none-eabi GCC 16.2, is downloaded into `local/` on the first build.

Always run `make` from the project root:

```sh
make                                  # every architecture x every build type (debug, release, speed)
make ARCH_CROSS=armv7e-m              # one architecture
make ARCH_CROSS=x86_64 BUILD_TYPE=debug
make check                            # build, then the unit tests, libc-test and the libgcc check
make library                          # only the headers and archives a program links against
make bench                            # benchmarks against docs/perf (needs `make setup-bench`)
make help
```

**Architectures (`ARCH_CROSS`):** `x86_64`, `armv6-m`, `armv6s-m`, `armv7e-m`, `armv7e-m-dp`, `armv8-m.main`.

**Output:**
- Build trees go to `build/<arch>/<build-type>/`.
- Installs go to `images/<arch>/<build-type>/{include,lib}`:
  - `libc.a`;
  - the flavour archives;
  - `librtos-none.a` and `librtos-linux.a`.

The allocator inside `libc.a` is chosen with `LIBC_MALLOC=nano|tlsf|tiny`. Run `make clean` after changing it.

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

### Binding to your RTOS

To bind to an RTOS:
1. Copy `port/none`.
2. Implement the `__rtos_*` functions declared in `include/rtos/rtos-port.h`: locks and futexes, thread creation, TLS setup, time, I/O, heap regions and exit.
3. Link the port's archive in place of `librtos-none.a`.

On bare metal, `port/none` needs the following:
- **Startup:** your startup code calls `__rtos_none_start()`.
- **Heap:** the linker script defines the heap bounds.
- **Output:** optionally, define `__rtos_none_output()` to send `stdout` somewhere, such as a UART.

See:
- `docs/configuration.md` (Ports);
- `docs/research/threads.md` (the seam);
- `docs/research/tls-and-startup.md` (TLS and startup).

### Building from another project

A parent build runs `make library` as a sub-make. It passes its roots, the architecture and, optionally, its own compiler:

```make
$(MAKE) -C $(LIBC_DIR) \
	PROJECT_ROOT=$(LIBC_DIR) TOOLS_ROOT=$(LIBC_DIR)/tools \
	OUTPUT_ROOT=<build dir> PREFIX=<install dir> \
	ARCH_CROSS=armv8-m.main BUILD_TYPE=release \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	library
```

Then it links `<install dir>/armv8-m.main/release/{include,lib}` as shown above.
- Pass every root on the command line: a parent with the same conventions would otherwise leak in its own values.
- An external `CROSS_COMPILE` is used as given, and must be GCC 14 or newer.

`docs/configuration.md`, "Building from another project", covers the details and the module glue for a pi-pico-style parent. A sysroot and crosstool-NG integration are tracked in issue #65.

## Documentation

| Topic | Where |
|---|---|
| Goals, architecture, decision log, open questions | `docs/plan/PLAN.md` |
| Phase task lists | `docs/plan/phase-{1,2,3,4}.md` |
| Configuration: archives, flavours, cores, allocators, ports, parent builds | `docs/configuration.md` |
| Toolchain versions and setup | `docs/toolchain.md` |
| Implementation-defined behaviour (Annex J) | `docs/implementation-defined.md` |
| Research notes: algorithms, trade-offs, sources, test vectors | `docs/research/` |
| Size reports and per-architecture history | `docs/size/` |
| Performance reports and per-architecture history | `docs/perf/` |
| Contributor rules: workflow, code style, tests | `CLAUDE.md` |
| The standard text used for the clean-room work | `docs/standard/` |

## Licence

Mozilla Public License 2.0. See `LICENSE`.
