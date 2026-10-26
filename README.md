# rtoskit-libc

rtoskit-libc is a C standard library for small embedded systems. It
implements ISO C23 (N3220), was written clean-room from the standard text,
and reaches the operating system through one link-time interface, so the
same library runs on bare metal or under any RTOS that provides a dozen or
so functions. It is licensed under the [Mozilla Public License 2.0](LICENSE).

It targets Arm Cortex-M, from the Cortex-M0 to the Cortex-M7 and Cortex-M33,
and builds for x86-64 as well, which is where it is developed and tested.

## What it offers

1. **The whole C23 library a microcontroller can use.** Every clause 7
   header is implemented except the wide-character headers (`<wchar.h>`,
   `<wctype.h>`, `<uchar.h>`), Annex K and decimal floating point. That
   includes the C23 additions such as `<stdbit.h>`, `<stdckdint.h>`,
   `memset_explicit`, `strfromd` and `timegm`, as well as `<threads.h>` and
   `<stdatomic.h>`. The library is itself written in C23.

2. **Correct results.** `printf` and `strtod` convert floating-point values
   exactly, at any precision. The float maths functions are correctly
   rounded, and the double functions are below 0.53 ulp. Floating-point
   exceptions are raised on every core, including those without an FPU.

3. **Small by construction.** Each function lives in its own file and
   section, so `--gc-sections` keeps only what a program calls. There is no
   hidden global state: per-thread state such as `errno`, `strtok`'s position
   and `rand`'s seed is `thread_local`. Differences between cores come from
   the build choosing different files, never from `#if`.

4. **One interface to the OS.** Locks, threads, thread-local storage, time,
   I/O, the heap and exit all go through `__rtos_*` functions declared in
   [`include/rtos/rtos-port.h`](include/rtos/rtos-port.h). A port implements
   them in its own archive. Two ports are included: `port/none` for bare
   metal and `port/linux`, which the test suite runs on.

5. **A size build and a speed build.** The `release` build is as small as we
   can make it. The `speed` build compiles the same sources with faster
   string routines, conversions and, on cores without an FPU, fixed-point
   maths. Drop-in archives trim `printf` further (`libc-nofloat.a`,
   `libc-minimal-printf.a`) or swap in the smallest allocator
   (`libc-tinymalloc.a`), and `libc.a` itself can use any of three
   allocators: nano, TLSF or tiny.

The project's goals, architecture and every design decision (numbered D1 to
D117) are recorded in [`docs/plan/PLAN.md`](docs/plan/PLAN.md). The work was
done in phases: conformance first, then size, then speed, then a comparison
with Arm Optimized Routines, then a round of speed work that the size
budgets had held back.

## How it compares

We compared rtoskit-libc with picolibc 1.8.6 and newlib-nano 4.4 (the Ubuntu
24.04 packages) and with Arm Optimized Routines (AOR) at revision
`f67e7daa`. AOR is not a C library, only a collection of string, maths and
soft-float routines, so its columns below are rtoskit-libc with AOR's
routines linked in wherever AOR has one. Every library was built with the
same arm-none-eabi GCC 16.2 and the same flags. The figures here are a
summary; the linked documents have the full tables, the method and the
caveats.

### Conformance

All 219 applicable tests of musl's libc-test pass on all seven
architectures. The other 207 are out of scope: they test POSIX, wide
characters or `fork` ([`manifest.txt`](test/libc-test/manifest.txt)). On top
of that, an audit of the headers against Annex B checks 689 identifiers, 642
exact prototypes and 73 generic names across 28 headers, and a freestanding
check confirms that the freestanding headers' functions link without any
port. `<threads.h>` is implemented over the OS interface; picolibc and
newlib-nano have none. The implementation-defined behaviour is documented
clause by clause against Annex J in
[`docs/implementation-defined.md`](docs/implementation-defined.md).

All of this runs as part of `make check`. libc-test was run only against
this library, not against the others. See
[`docs/research/conformance.md`](docs/research/conformance.md).

### Accuracy

| | rtoskit-libc | picolibc / newlib | AOR |
|---|---|---|---|
| Float functions, round to nearest | Correctly rounded, checked over all 2^32 arguments on x86-64 and Cortex-M7. On soft-float and single-precision cores, a documented handful of hard cases are within 1 ulp. | fdlibm-style, up to about 1 ulp | 0.004–3.9 % of results misrounded per function, worst 1.09 ulp |
| Float functions, directed rounding | Correctly rounded | not measured | Up to 2 ulp; up to 35 % misrounded upward |
| Double functions | Below 0.53 ulp, at most 0.023 % misrounded | Up to about 1 ulp | Below 0.53 ulp, except `erf` at 0.89 ulp |
| Floating-point exceptions | Raised on every core, including the soft `<fenv.h>` on Armv6-M | — | AOR's Arm soft-float raises none |

The method and the per-function results are in
[`docs/research/optimized-routines.md`](docs/research/optimized-routines.md)
and [`docs/research/float-hard-cases.md`](docs/research/float-hard-cases.md),
and each family of maths functions has its own `libm-*.md` note in
[`docs/research/`](docs/research/).

### Size

The table gives the bytes of code (text plus rodata) that each library adds
to the `test/size` reference programs on a Cortex-M4F (`armv7e-m`), built
with `-Os`. Our figures are from the size build. The ratio columns divide
each other library by rtoskit-libc, so a ratio above 1 means larger.

| Program | rtoskit-libc | rtoskit + AOR | picolibc | newlib-nano | rtoskit + AOR ratio | picolibc ratio | newlib-nano ratio |
|---|---:|---:|---:|---:|---:|---:|---:|
| `printf("%d")` | 6369 | 6565 | 6381 | 6896 | 1.03 | 1.00 | 1.08 |
| `printf("%d")`, `libc-minimal-printf.a` | 3419 | — | — | — | — | — | — |
| `printf("%f")` | 7689 ¹ | 8153 | 6381 | 17575 | 1.06 | 0.83 | 2.29 |
| `puts` | 1120 ² | 5152 ³ | 212 | 3488 | 4.60 | 0.19 | 3.11 |
| `malloc` / `free` heavy | 988 | 4954 ³ | 960 | 2156 | 5.01 | 0.97 | 2.18 |
| float maths (`sinf`, `expf`, `logf`, ...) | 4380 | 7464 | 6444 | 8237 | 1.70 | 1.47 | 1.88 |

1. This includes the soft-double helpers, which rtoskit-libc provides itself.
   The picolibc and newlib-nano figures leave them out because libgcc
   supplies them there.
2. Our `puts` goes through a buffered, locked `FILE`; picolibc's writes
   unbuffered through semihosting.
3. AOR's `strlen` and `strcmp` objects carry unwind information that pulls in
   libgcc's 3.6 KB unwinder. Our string routines are smaller than AOR's on
   every core even without it.

The AOR column adds the growth AOR's routines cause
([`docs/size/phase-4.md`](docs/size/phase-4.md)) to our current figures.
[`docs/size/phase-2.md`](docs/size/phase-2.md) compares all the cores
against picolibc and newlib-nano. Current figures for each architecture are
in `docs/size/<arch>.md`, for example
[`armv7e-m.md`](docs/size/armv7e-m.md), and a size budget
([`docs/size/budget.json`](docs/size/budget.json)) fails the build if
anything grows.

### Performance

We count the guest instructions each operation takes under qemu and take the
geometric mean over each benchmark program. The table divides every column
by the count for our speed build, so that column reads 1.00 and anything
above 1 took more instructions.

| Cortex-M4F (`armv7e-m`) | rtoskit size build | rtoskit speed build | rtoskit + AOR | picolibc | newlib-nano |
|---|---:|---:|---:|---:|---:|
| string (603 cases) | 1.44 | 1.00 | 0.98 | 1.40 | 1.12 |
| stdio (23) | 1.22 | 1.00 | 1.00 | 1.15 | 0.92 |
| `strto*` (21) | 5.61 | 1.00 | 0.97 | 0.85 | 1.52 |
| malloc (9) | 1.07 | 1.00 | 1.00 | 7.21 | 0.68 |
| maths, throughput | 1.00 | 1.00 | 0.95 | 0.36 | 0.38 |

The picolibc and newlib-nano columns come from the phase 3 comparison
([`docs/perf/phase-3.md`](docs/perf/phase-3.md)), the AOR column from the
phase 4 records ([`docs/perf/phase-4.md`](docs/perf/phase-4.md)). AOR only
replaces `memchr`, `memset`, `strlen`, 15 maths functions and the
soft-double helpers, so every other case counts as 1.00; over the maths
cases it does replace, it takes 0.81 of our instructions, and its double
`exp`, `log` and `pow` take 0.16–0.45.

Our speed build is the fastest of the three at bulk `memcpy` and `memset`,
at decimal conversions and at `realloc` (against picolibc), and its
introsort `qsort` saves the others' 35–49 % extra. On single-precision and
soft-float cores AOR's float maths takes 4–7 times our instructions. We are
slower where we chose exactness: our maths functions take 2–3 times
picolibc's instructions because they are correctly rounded, and `%f` and
`%g` cost more because they format exactly. newlib-nano's allocator is
faster because it takes no lock; ours is always thread-safe.

Instruction counts say nothing about pipelines or memory; see
[`docs/research/benchmarks.md`](docs/research/benchmarks.md) for the method
and its limits, and `docs/perf/<arch>.md` (for example
[`armv7e-m.md`](docs/perf/armv7e-m.md)) for current figures.

## Building

### What you need

- **GNU make 4.4 or newer.** The makefiles rely on `.NOTINTERMEDIATE`, and
  an older make stops with a message. Ubuntu 26.04 has 4.4. On Ubuntu 24.04,
  whose make is 4.3, build it from <https://ftp.gnu.org/gnu/make/>
  (`./configure --prefix=/usr/local && make && sudo make install`).
- **GCC 14 or newer for the host**, to build the x86-64 library. The build
  uses plain `gcc` if it is new enough and otherwise the newest `gcc-NN` it
  finds; set `HOST_CC` to choose. On Ubuntu 24.04, whose `gcc` is 13,
  `apt-get install -y gcc-14`.
- **arm-none-eabi GCC 14 or newer** for the Arm builds. You don't have to
  install one: the first build downloads GCC 16.2 into `local/`. To use your
  own, pass `CROSS_COMPILE=<prefix>`.
- **qemu-user** to run the Arm tests (`apt-get install -y qemu-user`), and
  the libc-test submodule (`git submodule update --init`). Neither is needed
  just to build the library.

Programs that use the library need a compiler in C23 mode: the installed
headers do not compile as C11 or C17. GCC 15 and newer default to `gnu23`;
with GCC 14, pass `-std=c23` or `-std=gnu23` (the [spec file](#a-gcc-spec-file)
below does this for you). The library itself builds only with GCC, since
the build uses GCC-specific options; compiling programs against it with
Clang 19 or newer is untested. The exact versions we test with are listed in
[`docs/toolchain.md`](docs/toolchain.md).

### Quick start

```sh
make -j$(nproc)                       # every architecture, every build type, with tests
make -j$(nproc) ARCH_CROSS=armv7e-m   # just the Cortex-M4F builds
make check                            # build, then run the unit tests and libc-test
```

Always run `make` from the top of the tree. The results land in
`build/<arch>/<build-type>/`, and the headers and archives are installed in
`images/<arch>/<build-type>/include` and `.../lib`.

### Choosing the architecture

`ARCH_CROSS` names the architecture to build. Without it, `make` builds all
seven. The architecture decides the compiler flags, the floating-point code,
`<fenv.h>` and the string routines, so there is nothing else to configure.

| `ARCH_CROSS` | Cores | Compiler flags | FPU and `<fenv.h>` |
|---|---|---|---|
| `armv6-m` | Armv6-M parts without the OS extension (no `SVC`) | `-mthumb -march=armv6-m -mfloat-abi=soft` | none; soft float, round to nearest only |
| `armv6s-m` | Cortex-M0, M0+ | `-mthumb -march=armv6s-m -mfloat-abi=soft` | none; soft float, round to nearest only |
| `armv7-m` | Cortex-M3, and a Cortex-M4 or M33 built without its FPU | `-mthumb -march=armv7-m -mfloat-abi=soft` | none; soft float, round to nearest only |
| `armv7e-m` | Cortex-M4F, and Cortex-M7 with the single-precision FPU | `-mthumb -march=armv7e-m+fp -mfloat-abi=hard` | FPv4-SP; round to nearest only |
| `armv7e-m-dp` | Cortex-M7 with the double-precision FPU | `-mthumb -march=armv7e-m+fp.dp -mfloat-abi=hard` | FPv5-D16; every rounding direction and exception flag |
| `armv8-m.main` | Cortex-M33 (and M35P) with FPU and DSP | `-mthumb -march=armv8-m.main+fp+dsp -mfloat-abi=hard` | FPv5-SP; round to nearest only |
| `x86_64` | the development host | `-mlong-double-64` | SSE2; full. Runs only through the Linux test port. |

For a core that isn't listed, use the build whose instruction set and float
ABI it supports: a Cortex-M23 (Armv8-M Baseline) can use `armv6s-m`, and a
Cortex-M55 or M85 can use `armv8-m.main`. Compile your program with the same
`-march` and `-mfloat-abi` as the build you link. The maths and string code
each core gets is described in
[`docs/configuration.md`](docs/configuration.md#choosing-the-core).

### Build types

There are three builds of the same sources, chosen with `BUILD_TYPE`.
Without it, `make` builds all three, each in its own tree.

| `BUILD_TYPE` | Optimisation | Use it for |
|---|---|---|
| `debug` | `-O0 -g3`, `assert` enabled | Stepping through the library in a debugger |
| `release` | `-Os -g3`, `NDEBUG` | The size build, and the one to ship. A size budget ([`docs/size/budget.json`](docs/size/budget.json)) keeps it from growing. |
| `speed` | `-Os -g3`, `NDEBUG`, plus faster routines | The speed build. It adds 32-byte-per-iteration `memcpy`, `memmove` and `memset`, word-at-a-time `strlen`, `strcmp`, `strchr` and `memchr` on Thumb-2, block copies on Armv6-M, fast paths in `strtol`, `printf` and `strtod`, and fixed-point float maths on the cores without an FPU. |

Both optimised builds install the same headers and archive names, so a
program switches between them by pointing at the other install. One caution:
the speed build's Thumb-2 string routines use unaligned `LDR` and `STR`, as
GCC's own code does, so use the size build if your system sets
`CCR.UNALIGN_TRP` or copies Device memory with `memcpy`. More in
[`docs/configuration.md`](docs/configuration.md#size-or-speed).

### Make goals

| Goal | What it does |
|---|---|
| `all` (the default) | Builds and installs the library, the ports, the tests, the examples, and the size and benchmark programs |
| `library` | Builds and installs only what a program links against: the headers, the `libc*.a` archives and the port archives |
| `check` | Builds, then runs the unit tests, libc-test against its recorded results, and a check that nothing links libgcc's copy of our functions |
| `bench` | Builds the size and speed builds and runs the benchmarks against the records in [`docs/perf`](docs/perf/) (run `make setup-bench` once first) |
| `aor` | Fetches and builds Arm Optimized Routines for comparison, and runs its smoke test |
| `setup`, `setup-bench` | Downloads the Arm toolchain into `local/` (the first build does this anyway); builds the instruction-counting qemu |
| `info` | Prints what one architecture and build type resolve to: toolchain, FPU, paths |
| `clean`, `distclean`, `realclean` | Removes build products; also `build/` and `images/`; also the downloaded tools in `local/` |
| `db` | Writes `compile_commands.json` (needs `bear`) |
| `<module>`, e.g. `libc/string` | Rebuilds one module without its dependencies. Needs `ARCH_CROSS`, `BUILD_TYPE` and an earlier full build. |
| `help` | Lists the goals and variables |

`make all` always builds the tests, since `make check` needs them. If you
only want the library, use `make library`: it builds the headers and
archives and nothing else, works on a fresh tree, and does not need the
libc-test submodule.

### Build options

Set these on the `make` command line.

| Variable | Default | Effect |
|---|---|---|
| `ARCH_CROSS` | `all` | One architecture, from the table above. An unknown name stops with the list. |
| `ARCHS` | all seven | The architectures that `ARCH_CROSS=all` loops over, e.g. `ARCHS="armv6-m armv8-m.main"` |
| `BUILD_TYPE` | `all` | One build type: `debug`, `release` or `speed` |
| `BUILDS` | `debug release speed` | The build types that `BUILD_TYPE=all` loops over |
| `LIBC_MALLOC` | `nano` | The allocator inside `libc.a`: `nano`, `tlsf` or `tiny`. Run `make clean` after changing it. All three are always built and tested. |
| `HOST_CC` | found | The host GCC for x86-64, version 14 or newer |
| `CROSS_COMPILE` | the downloaded GCC 16.2 | The prefix of your own arm-none-eabi toolchain, e.g. `/opt/arm/bin/arm-none-eabi-`, version 14 or newer |
| `OUTPUT_ROOT` | `build` | Where the build trees go |
| `PREFIX` | `images` | Where the installs go |
| `V=1` | off | Shows the full commands |
| `LIBC_TEST_UPDATE=1` | off | With `check`, records libc-test improvements in the architecture's expected results. It refuses while anything has regressed. |
| `BENCH_UPDATE=1` | off | With `bench`, records the results as the new benchmark baseline |

Parallel builds (`make -j`) are supported. The less common variables,
including the ones a parent build must pass, are in
[`docs/configuration.md`](docs/configuration.md#build-variables).

Some more examples:

```sh
# The size build only, for two cores
make -j$(nproc) ARCHS="armv6-m armv8-m.main" BUILD_TYPE=release

# Only what a program links against, into your own directories
make -j$(nproc) ARCH_CROSS=armv8-m.main BUILD_TYPE=release \
    OUTPUT_ROOT=$HOME/tmp/rtoskit-build PREFIX=$HOME/opt/rtoskit library

# libc.a with the TLSF allocator
make clean
make -j$(nproc) LIBC_MALLOC=tlsf

# Your own compilers
make -j$(nproc) HOST_CC=gcc-15 CROSS_COMPILE=/opt/arm/bin/arm-none-eabi-

# Test one configuration, then record any libc-test improvements
make -j$(nproc) ARCH_CROSS=x86_64 BUILD_TYPE=debug check
make ARCH_CROSS=x86_64 BUILD_TYPE=debug check LIBC_TEST_UPDATE=1

# After a full build, rebuild one module or one unit test
make ARCH_CROSS=armv7e-m BUILD_TYPE=release V=1 libc/string
make ARCH_CROSS=x86_64 BUILD_TYPE=debug test/unit/printf
```

## Using the library

### Linking a program

Compile against the installed headers only, and link with `-nostdlib`,
putting the port, the library and libgcc in one group:

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

Your compiler flags must match the architecture the library was built for.

### Choosing a printf

You choose how much `printf` and `scanf` you get when you link, not when you
build: every build installs all the variants, and you link one of them in
place of `libc.a`.

| Archive | `printf` and `scanf` | Whole `printf("%d")` program, Cortex-M4F / M0+ |
|---|---|---:|
| `libc.a` | Everything, including exact `%a %e %f %g` | 7165 / 7757 bytes |
| `libc-nofloat.a` | No floating-point conversions; `%a %e %f %g` fail with `EINVAL`. `strtod` still works. | 4451 / 4975 bytes |
| `libc-minimal-printf.a` | As `libc-nofloat.a`, and `%n` and lengths wider than `long` (`ll`, `j`, 64-bit `z`) fail with `EINVAL`, so no 64-bit division is linked | 4211 / 4707 bytes |

A conversion that a variant leaves out fails with `EINVAL` rather than
printing nothing. A fourth archive, `libc-tinymalloc.a`, swaps in the
smallest allocator without rebuilding. The sizes are whole images of the
`test/size` programs, from the size build's baselines; more detail is in
[`docs/configuration.md`](docs/configuration.md#flavour-archives-d53-d54).

```sh
arm-none-eabi-gcc ... -Wl,--start-group images/armv7e-m/release/lib/librtos-none.a \
    images/armv7e-m/release/lib/libc-minimal-printf.a $(arm-none-eabi-gcc ... -print-libgcc-file-name) -Wl,--end-group
```

### A GCC spec file

A spec file folds all of that into one `-specs=` option, the way
`picolibc.specs` and `nano.specs` do. Save this as `rtoskit-libc.specs`:

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

Point `RTOSKIT_LIBC` at the install you want and compile and link as usual:

```sh
export RTOSKIT_LIBC=$PWD/images/armv7e-m/release
arm-none-eabi-gcc -mthumb -march=armv7e-m+fp -mfloat-abi=hard -specs=rtoskit-libc.specs \
    -Os -ffunction-sections -fdata-sections -c app.c
arm-none-eabi-gcc -mthumb -march=armv7e-m+fp -mfloat-abi=hard -specs=rtoskit-libc.specs \
    -T board.ld startup.o app.o -o app.elf
```

The spec file uses only this library's headers and pre-includes its
`stdc-predef.h`, which defines `__STDC_IEC_559__` where the hardware
supports it. It compiles as `-std=gnu23` unless you pass your own `-std`. At
link time it puts the library's `lib/` ahead of the toolchain's, so `-lc`
means this `libc.a`, and turns on `--gc-sections`. `--port=<name>` picks the
port archive (`none` by default) and `--libc=<variant>` picks a variant
archive, for example `--libc=minimal-printf`. The toolchain's start files are
dropped, so your startup code and linker script provide `_start` and the
memory layout. If `RTOSKIT_LIBC` is not set, GCC stops with an error.

We tested it with arm-none-eabi GCC 16.2 on `armv7e-m` and the Linux test
port (`--port=linux` plus that port's `headers.ld`): a threaded `printf` and
`sinf` program ran under `qemu-arm`, with `libc.a` and with
`--libc=minimal-printf`.

### Porting to your RTOS

To run on your RTOS, copy `port/none`, implement the `__rtos_*` functions
declared in [`include/rtos/rtos-port.h`](include/rtos/rtos-port.h) (locks
and futexes, thread creation, TLS setup, time, I/O, the heap regions and
exit), and link your port's archive in place of `librtos-none.a`. With the
spec file, `--port=<name>` selects `librtos-<name>.a`.

On bare metal, `port/none` works as it is: your startup code calls
`__rtos_none_start()`, your linker script defines the heap bounds, and you
can define `__rtos_none_output()` to send `stdout` to a UART or similar.

[`docs/configuration.md`](docs/configuration.md#ports-d4) describes the
ports; [`docs/research/threads.md`](docs/research/threads.md) and
[`docs/research/tls-and-startup.md`](docs/research/tls-and-startup.md) go
into the OS interface, thread-local storage and startup.

### Building from another project

A parent build can run `make library` as a sub-make, passing its own
directories, the architecture and, if it likes, its own compiler:

```make
$(MAKE) -C $(LIBC_DIR) \
	PROJECT_ROOT=$(LIBC_DIR) TOOLS_ROOT=$(LIBC_DIR)/tools \
	OUTPUT_ROOT=<build dir> PREFIX=<install dir> \
	ARCH_CROSS=armv8-m.main BUILD_TYPE=release \
	CROSS_COMPILE=$(CROSS_COMPILE) \
	library
```

It then links `<install dir>/armv8-m.main/release/{include,lib}` as shown
above, or sets `RTOSKIT_LIBC` to that install and uses the spec file. Pass
every directory on the command line, or a parent that uses the same
variable names will leak its own values in. An external `CROSS_COMPILE` is
used as given and must be GCC 14 or newer. The details, including the glue
for a pi-pico-style parent, are in
[`docs/configuration.md`](docs/configuration.md#building-from-another-project-d105).
A sysroot and crosstool-NG integration are tracked in issue #65.

## Documentation

Read these first:

- [`docs/configuration.md`](docs/configuration.md): the archives, the build
  variants, the cores, the allocators, the ports and parent builds.
- [`docs/plan/PLAN.md`](docs/plan/PLAN.md): goals, scope and architecture,
  the [decision log](docs/plan/PLAN.md#2-decision-log) (D1 to D117) and the
  [open questions](docs/plan/PLAN.md#7-open-questions).
- [`docs/toolchain.md`](docs/toolchain.md): the tool versions we build and
  test with.

For more detail:

- [`docs/implementation-defined.md`](docs/implementation-defined.md): the
  implementation-defined behaviour, clause by clause against Annex J.
- [`docs/research/`](docs/research/): a note for each algorithm, with the
  sources reviewed, the trade-offs, the edge cases and the test vectors.
- Size reports: [phase 1](docs/size/phase-1.md), [phase 2](docs/size/phase-2.md),
  [phase 4 (AOR)](docs/size/phase-4.md), the
  [picolibc and newlib baseline](docs/size/comparison.md), and one per
  architecture: [x86_64](docs/size/x86_64.md), [armv6-m](docs/size/armv6-m.md),
  [armv6s-m](docs/size/armv6s-m.md), [armv7-m](docs/size/armv7-m.md),
  [armv7e-m](docs/size/armv7e-m.md), [armv7e-m-dp](docs/size/armv7e-m-dp.md),
  [armv8-m.main](docs/size/armv8-m.main.md).
- Performance reports: [phase 3](docs/perf/phase-3.md),
  [phase 4 (AOR)](docs/perf/phase-4.md), and one per architecture:
  [x86_64](docs/perf/x86_64.md), [armv6-m](docs/perf/armv6-m.md),
  [armv6s-m](docs/perf/armv6s-m.md), [armv7-m](docs/perf/armv7-m.md),
  [armv7e-m](docs/perf/armv7e-m.md), [armv7e-m-dp](docs/perf/armv7e-m-dp.md),
  [armv8-m.main](docs/perf/armv8-m.main.md).
- The task lists of each phase: [1](docs/plan/phase-1.md),
  [2](docs/plan/phase-2.md), [3](docs/plan/phase-3.md),
  [4](docs/plan/phase-4.md), [5](docs/plan/phase-5.md).
- [`CLAUDE.md`](CLAUDE.md): how we work on the code: the workflow, the
  coding style and the tests.
- [`docs/standard/`](docs/standard/): the standard text the clean-room work
  was written from.

## Licence

rtoskit-libc is licensed under the Mozilla Public License 2.0; see
[`LICENSE`](LICENSE).
