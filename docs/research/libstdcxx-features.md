# Research: telling libstdc++'s configure what the library provides (issue #125, D147)

libstdc++'s `configure` runs no link tests on a cross target. For newlib and
picolibc it has its own branches that say what those libraries provide; for
this library, sgstreet/rtoskit-gcc's patch
`0002-libstdcxx-add-rtoskit-libc-os-port.patch` adds a `--with-rtoskit-libc`
branch that hard-codes 57 `AC_DEFINE`s (the float and `long double` math
functions, `hypot`, `strtof`, `aligned_alloc`, `quick_exit`,
`at_quick_exit`, TLS, `S_ISREG`, `<sys/types.h>`, `<inttypes.h>`,
`<fcntl.h>`, `<unistd.h>`), and its defconfig adds
`-D_GLIBCXX_HAVE___CXA_THREAD_ATEXIT=1`. The list states facts about this
library from another repository, and nothing here tests it.

## Sources reviewed

- GCC 16 `libstdc++-v3/configure.ac`
  (<https://github.com/gcc-mirror/gcc/blob/releases/gcc-16/libstdc++-v3/configure.ac>),
  the `--with-picolibc` branch: a fixed list of `AC_DEFINE`s (`HAVE_HYPOT`,
  `HAVE_STRTOF`, the float math functions, `HAVE_MEMALIGN`,
  `HAVE_ALIGNED_ALLOC`, `HAVE_AT_QUICK_EXIT`, `HAVE_LINK`, `HAVE_SYS_STAT_H`,
  `HAVE_SYS_TYPES_H`, `HAVE_SETENV`, `HAVE_STRERROR_L`, `HAVE_S_ISREG`,
  `HAVE_UNISTD_H`), plus two compile probes against `<picolibc.h>`:
  `HAVE_ICONV` when `_ICONV_ENABLED` is defined, `HAVE_TLS` when
  `__THREAD_LOCAL_STORAGE` is. A compile test needs only headers, so it works
  where link tests do not.
- picolibc `picolibc.h.in`
  (<https://github.com/picolibc/picolibc/blob/main/picolibc.h.in>): a header
  generated at build time with one `#cmakedefine` per build option
  (`__THREAD_LOCAL_STORAGE`, `__TINY_STDIO`, `__IO_LONG_DOUBLE`, ...) and
  `_NEWLIB_VERSION`.
- GCC 16 `acinclude.m4`, `GLIBCXX_ENABLE_LIBSTDCXX_TIME`: on a target whose
  OS it does not know (`arm-none-eabi`), the `auto` default enables no clock
  and no sleep; with `=yes` it probes `_POSIX_TIMERS` in `<unistd.h>`, which
  this library does not define (see `docs/research/posix-subset.md`). A
  configure branch that defines `_GLIBCXX_USE_CLOCK_MONOTONIC` and friends
  directly does not need either.
- rtoskit-gcc `origin/main` (`bf788d2`): the patch above, the defconfig
  (`--with-rtoskit-libc --enable-clocale=generic --enable-cstdio=stdio
  --disable-wchar_t`), and `tools/scripts/validate-toolchain`.

## Data file or header

Issue #125 offered both. The header is chosen:

- it is what libstdc++'s configure already does for picolibc, so the
  rtoskit-libc branch reads like its neighbour and a GCC maintainer
  recognises it;
- `AC_TRY_COMPILE` against the installed headers needs no new configure
  option to find a data file, only the sysroot the build already uses;
- the probe and the definition cannot disagree about which header declares
  a function, because the probe compiles against the same headers libstdc++
  will.

## The header

`include/rtoskit-libc.h`, installed with the other headers:

- `__RTOSKIT_LIBC__` identifies the library, as `__PICOLIBC__` does.
- `__LIBC_HAVE_<NAME>`, defined as 1, one per feature, `<NAME>` in autoconf's
  spelling: a function or macro in upper case (`SINF`, `S_ISREG`), a header as
  `FOO_H` or `SYS_FOO_H`. So libstdc++'s `HAVE_<NAME>` maps by name, and the
  four time names map to `_GLIBCXX_USE_<NAME>` (`CLOCK_REALTIME`,
  `CLOCK_MONOTONIC`, `NANOSLEEP`, `SCHED_YIELD`). `__CXA_THREAD_ATEXIT`
  answers `_GLIBCXX_HAVE___CXA_THREAD_ATEXIT`, which the defconfig passes by
  hand today.
- The 57 names of the patch, plus `SYS_STAT_H`, `GETENTROPY` (D146),
  `__CXA_THREAD_ATEXIT` and the four time names: 64 features.
- **One header for every architecture and flavour.** Nothing in the list
  varies between the cores of a triple, or between `libc.a` and its flavour
  archives (which differ only in printf, scanf and malloc), and D15 forbids a
  public header that differs per multilib. `test/unit/features` runs on every
  architecture, so a feature that one architecture lacked would fail there.
- **The port is chosen at link time**, so the header cannot say whether the
  clocks, sleeping or the entropy source work: a port without them returns
  `-ENOSYS` (`port/none` without a board's clock). The header says the
  library has them.

## The configure side (for rtoskit-gcc)

The `--with-rtoskit-libc` branch can replace its `AC_DEFINE` list with a loop
over libstdc++'s names, defining each whose `__LIBC_HAVE_` macro the header
defines. In `configure.ac` (`AC_DEFINE_UNQUOTED` takes a shell variable; the
names already have templates in `config.h.in`):

```sh
for name in HYPOT STRTOF ACOSF ... TANL TLS ALIGNED_ALLOC AT_QUICK_EXIT \
    QUICK_EXIT SYS_TYPES_H SYS_STAT_H INTTYPES_H FCNTL_H UNISTD_H S_ISREG \
    GETENTROPY; do
  AC_TRY_COMPILE([#include <rtoskit-libc.h>], [
    #ifndef __LIBC_HAVE_$name
    #error
    #endif], [AC_DEFINE_UNQUOTED([HAVE_$name], 1)], [])
done
```

and the same for `CLOCK_REALTIME CLOCK_MONOTONIC NANOSLEEP SCHED_YIELD` into
`_GLIBCXX_USE_$name`, and `__CXA_THREAD_ATEXIT` into
`_GLIBCXX_HAVE___CXA_THREAD_ATEXIT` (replacing the defconfig's
`CT_CC_GCC_LIBSTDCXX_TARGET_CXXFLAGS`). Which names libstdc++ asks about
stays in the GCC patch; whether the library has each stays here.

## Test (`test/unit/features`)

- `static_assert(__LIBC_HAVE_<NAME> == 1)` for every feature, so a name
  removed from the header fails to compile.
- The address of every function, in a `volatile` table, so each must link
  from `libc.a` on every architecture and build type.
- The ones libstdc++ calls at run time, on the Linux test port:
  `thread_local`, `strtof`, `aligned_alloc`, both clocks, `nanosleep`,
  `sched_yield` and `thrd_yield` (libstdc++'s `_GLIBCXX_USE_SCHED_YIELD` goes
  through `gthr-c11.h`'s `__gthread_yield`), `getentropy`.
- `features.run` fails when the header names a feature that `features.c`
  does not check, so the header cannot grow ahead of the test.
- `test/unit/api` compiles the header alone with `-pedantic-errors`, and
  `test/unit/cxx/headers98.cpp` as `gnu++98` (D131).
