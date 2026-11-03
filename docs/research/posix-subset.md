# Research: the minimal POSIX subset

Issue #106. Plan: PLAN D128 (the subset), phase 7. Decisions from this note:
D129 (descriptors are seam handles; answers Q37), D130 (the library defines
`open`, `close`, `read` and `write`; answers Q38), D132 (where the ISO
headers show the subset's names; P7.03), D134 (P7.04). Open: Q39 (how a hosted libstdc++
is configured for this library).

## Sources reviewed

- **POSIX.1-2024** (IEEE Std 1003.1-2024, Issue 8),
  <https://pubs.opengroup.org/onlinepubs/9799919799/>:
  - headers: `basedefs/unistd.h.html`, `basedefs/fcntl.h.html`,
    `basedefs/sys_types.h.html`, `basedefs/sys_stat.h.html`,
    `basedefs/strings.h.html`, `basedefs/sched.h.html`;
  - functions: `functions/fdopen.html`, `functions/fileno.html`,
    `functions/lseek.html`, `functions/open.html`, `functions/read.html`,
    `functions/write.html`, `functions/close.html`, `functions/fstat.html`,
    `functions/strnlen.html`, `functions/strcasecmp.html`,
    `functions/reallocarray.html`, `functions/clock_gettime.html`,
    `functions/nanosleep.html`, `functions/sched_yield.html`.
- **newlib** (`newlib-cygwin` `b097f95`) and **picolibc** (`b5d6050`), read
  for structure only (the header survey in issue #106):
  - newlib's `FILE` keeps the descriptor (`_file`) and reaches the OS through
    `_read_r`, `_write_r` and `_lseek_r`;
  - picolibc's tinystdio has no descriptor in the generic `FILE`; its
    POSIX glue (`stdio-posix.h`) adds one.
- **pi-pico** `origin/main` `a4055c6`: `posix/posix-io.c`, its VFS, and the
  static scan of 278 product sources from D128, re-run with the same result.
- **libstdc++**, GCC 16 release branch (`releases/gcc-16`, `224e4ac`),
  <https://github.com/gcc-mirror/gcc/tree/releases/gcc-16/libstdc++-v3>:
  - `config/io/basic_file_stdio.cc` and `config/io/basic_file_stdio.h`;
  - `configure.ac` and `crossconfig.m4`;
  - `config/os/generic`, `config/os/newlib`, `config/os/picolibc`.

## Descriptors (Q37 → D129)

The seam already defines everything a descriptor needs
(`include/rtos/rtos-port.h`, P1.09):

- `__rtos_open` returns a new `int` handle, or a negated `errno`;
- handles 0, 1 and 2 are the standard input, output and error streams;
- `__rtos_read`, `__rtos_write`, `__rtos_seek` and `__rtos_close` take
  the handle.

Our `FILE` stores that handle (`struct __libc_file`, `libc/internal/file.h`),
and `__libc_file_new(handle, flags)` already makes a stream for an open
handle. Making the handle the descriptor therefore needs no table and no
new seam call:

- `fileno(stream)` returns the stream's handle (under the stream's lock);
- `fdopen(fd, mode)` parses the mode with `__libc_file_mode`, then calls
  `__libc_file_new(fd, flags)`;
- `lseek(fd, offset, whence)` calls `__rtos_seek`.

The handle must be a non-negative `int`, which the seam already requires
(a negative return is an error). It need not be small: nothing in the
subset indexes a table by it. Both ports already behave this way:

- `port/linux` returns the Linux descriptor;
- `port/none` has only 0, 1 and 2, and `__rtos_open` returns `-ENOSYS`.

pi-pico's VFS also matches: small-integer descriptors from a fixed table,
with descriptors below `POSIX_FD_FIRST` reserved for the standard streams.
Once its stdio glue is rewritten, its `__rtos_read` and `__rtos_write` serve
0, 1 and 2, and every other handle is a VFS descriptor.

A descriptor table of our own (newlib's `_file` mapped through an array)
was rejected: it costs RAM in every program that calls `fdopen` or
`fileno`, and it would duplicate the port's own table.

## `open`, `close`, `read` and `write` (Q38 → D130)

These have two kinds of user:

1. **Ports without a VFS** (`port/linux`, `port/none`, a bare-metal board).
   Programs and libstdc++'s default I/O backend still call these functions,
   so the library must supply them.
2. **Ports with a VFS** (pi-pico). The port already defines them, and its
   seam calls route into the same VFS.

The library never relies on weak definitions. It replaces code at link time
through archive order (D13): a linker pulls an archive member only while
the symbol is still undefined. One function per file (D6) means each of
these is its own member, so:

- `libc.a` defines `open`, `close`, `read` and `write`, each over the seam:
  - `open` translates the `O_*` flags to `__RTOS_OPEN_*`;
  - each sets `errno` from a negated return and returns -1.
- A port or application that defines its own replaces them. Its object
  files, or an archive linked ahead of `libc.a`, resolve the symbol first,
  so the library's member is never pulled.

The library's own `FILE` code calls only the seam, never these functions, so
the two cannot recurse. A port's VFS must keep the same rule: its
`__rtos_read` calls into the VFS, not into `read`.

`ioctl` and `poll` stay the port's own (D128): the seam has no equivalent,
and no consumer needs a library version.

## What a hosted libstdc++ needs

`basic_file_stdio.cc` is the only I/O backend. Its POSIX use depends on the
`--enable-cstdio` choice:

| Call | `stdio` (default) | `stdio_pure` |
|---|---|---|
| `fdopen` | `sys_open(int)` | `sys_open(int)` |
| `fileno` | `fd()`, `native_handle()` | — |
| `read`, `write` | `xsgetn`, `xsputn` | — (uses `fread`, `fwrite`) |
| `lseek` | `seekoff`, `get_file_offset` | — (uses `fseek`, `ftell`) |
| `writev` | if `<sys/uio.h>` has it | — |
| `ioctl(FIONREAD)`, `poll` | if found | — |
| `fstat`, `S_ISREG` | if `<sys/stat.h>` has `S_ISREG` | same, but see the trap below |

`<fcntl.h>` and `<errno.h>` are included unconditionally in both modes.

**The `stdio_pure` trap.** `showmanyc()` calls `fstat(this->fd(), …)`
whenever configure found `S_ISREG`. But `fd()` is defined only when
`stdio_pure` is off. With `stdio_pure` and a `<sys/stat.h>` that defines
`S_ISREG`, libstdc++ fails to link with `fd()` undefined. P7.04 adds
`<sys/stat.h>` for `std::stacktrace`, so a `stdio_pure` toolchain must not
be configured with it. With D130 the default `stdio` backend is fully
supported, so the toolchain should use `stdio`, not `stdio_pure` (Q39).

**Configuring for `arm-none-eabi`.** libstdc++'s `configure.ac` runs no link
tests on a cross target. It takes one of three routes:

- `--with-newlib`: assumes newlib. It hard-codes a list of `HAVE_*`
  macros and uses `config/os/newlib`, which needs newlib's ctype table
  (`_U`, `_L`, `_ctype_`).
- `--with-picolibc`: assumes picolibc. It hard-codes `HAVE_S_ISREG`,
  `HAVE_LINK`, `HAVE_SETENV`, `HAVE_STRERROR_L`, `HAVE_ALIGNED_ALLOC` and
  `HAVE_AT_QUICK_EXIT`, among others, and uses `config/os/picolibc`, which
  needs picolibc's `__CTYPE_*` masks and `_ctype_wide` table.
- Neither: `crossconfig.m4` matches on the host. `arm*-*-eabi` has no
  case, so configure stops with "No support for this host/target
  combination."

None of the three fits rtoskit-libc as it stands. `config/os/generic`, which
needs only the ISO `<ctype.h>` functions (its `classic_table()` returns
null, and `toupper`/`tolower` call the C functions), is the right OS layer.
The toolchain therefore needs a small libstdc++ patch: a configure branch
(`--with-rtoskit-libc`, or an `arm*-*-eabi*` case in `crossconfig.m4`) that
selects `os/generic` and declares exactly what D128 provides. This belongs
to sgstreet/rtoskit-gcc#7 (Q39). The library side is unaffected.

## Consequences for D128

- `fileno` and `lseek` are needed only for the default `stdio` backend.
  `fdopen` is needed in both modes. D128's list stands, because the
  recommended backend is `stdio`.
- `read` and `write` move from "declared only" to "defined by the library,
  replaceable by the port" (D130).
- `isatty` is not used by libstdc++ or by pi-pico's product code. It stays
  optional, needs a seam addition to answer truthfully, and is deferred
  until a consumer asks.
- `getentropy`: libstdc++'s `random_device` uses it when configure finds it.
  Without it, `random_device` has no source on this target. P7.03 decides
  whether a port supplies an entropy seam call; nothing in pi-pico needs it.

## Edge cases

- `fdopen` on a handle the port does not know: the stream is made, and the
  first read or write fails with the port's `EBADF`. POSIX allows this,
  since the function need not validate the descriptor.
- `fdopen(fd, "w")` on a read-only handle: POSIX says the mode must be
  allowed by the descriptor's access mode. The library cannot ask the port
  for a handle's mode, so the error comes from the first `__rtos_write`.
- `fileno` on a string stream (`sscanf`'s internal `FILE`): there is none,
  since string streams never leave the library.
- `fileno` on a closed stream: undefined behaviour, as in POSIX.
- `lseek` on 0, 1 or 2 where the port cannot seek: `__rtos_seek`'s
  `-ESPIPE` becomes `errno` `ESPIPE`.
- `off_t`: P7.02 makes it 64-bit (`long long`) on every architecture, to
  match `__rtos_seek`; D15 requires the same type on every core of a
  triple.

## Test vectors (for P7.02 and P7.03)

- `fileno(stdin) == 0`, `fileno(stdout) == 1`, `fileno(stderr) == 2`.
- `fdopen(fileno(fopen(path, "w+")), "r+")`: writes through one stream are
  read back through the other after `fflush`.
- `lseek(fd, 0, SEEK_END)` equals the length written.
- `lseek(fd, -1, SEEK_SET)` returns -1 with `EINVAL`.
- `open` with `O_CREAT | O_EXCL` on an existing path returns -1 with
  `EEXIST`. `read` and `write` return byte counts, with -1 and `errno` on
  error.
- On `port/none`:
  - `open` returns -1 with `ENOSYS`;
  - `write(1, …)` reaches the console;
  - `read(0, …)` returns 0 (end of file), because its `__rtos_read` has no
    input;
  - `lseek(1, 0, SEEK_CUR)` returns -1 with `ESPIPE`.

## P7.03: the library functions

### Sources reviewed

- POSIX.1-2024, `functions/open.html`, `read.html`, `write.html`,
  `close.html`, `lseek.html`, `fileno.html`, `fdopen.html`, `strnlen.html`,
  `strcasecmp.html`, `reallocarray.html`, `clock_getres.html` (which also
  covers `clock_gettime`), `nanosleep.html`, `sched_yield.html`, `getentropy.html`,
  and `basedefs/V1_chap02.html` §2.2 (the feature test macro
  `_POSIX_C_SOURCE` and the namespace it opens).
- glibc's `<features.h>` (<https://sourceware.org/git/?p=glibc.git;a=blob;f=include/features.h>),
  newlib's `<sys/features.h>` and picolibc's copy of it, read for how each
  decides which non-ISO names an ISO header shows: all three show them by
  default, and hide them under `__STRICT_ANSI__` (GCC's `-std=c23`) unless a
  feature test macro asks. g++ always defines `_GNU_SOURCE`, so C++ sees them.
- musl (<https://git.musl-libc.org/cgit/musl/tree/src/string/strnlen.c>,
  `src/string/strcasecmp.c`), FreeBSD (`lib/libc/string/strcasecmp.c`) and
  OpenBSD (`lib/libc/stdlib/reallocarray.c`, where the function started),
  read for structure only: `strnlen` over `memchr`; `strcasecmp` folding both
  bytes to lower case as `unsigned char`; `reallocarray` as an overflow
  check before `realloc`.
- glibc's `sincos` (`sysdeps/ieee754/dbl-64/s_sincos.c`): a shared
  argument reduction, which saves one reduction per call at the cost of a
  third trigonometric kernel.

### Choices

- **Visibility (D132).** fileno, fdopen, strnlen, reallocarray, sincos,
  sincosf, clock_gettime, clock_getres, nanosleep, `clockid_t` and the
  `CLOCK_*` clocks are not ISO names, so a strictly conforming program may
  define them (C23 7.1.3). `<__cdefs.h>` defines `__LIBC_POSIX_VISIBLE` as 1
  unless `__STRICT_ANSI__` is defined without `_POSIX_C_SOURCE`,
  `_XOPEN_SOURCE`, `_GNU_SOURCE` or `_DEFAULT_SOURCE`, and the ISO headers
  declare the names under `#if __LIBC_POSIX_VISIBLE`. The POSIX-only headers
  (`<unistd.h>`, `<fcntl.h>`, `<strings.h>`, `<sched.h>`) need no guard.
  The library's own files define `_POSIX_C_SOURCE` (or `_GNU_SOURCE` for
  sincos) before their includes.
- **open.** `O_ACCMODE` selects `__RTOS_OPEN_READ`, `WRITE` or both, and the
  value 3 fails with `EINVAL`. `O_APPEND`, `O_CREAT`, `O_TRUNC` and `O_EXCL`
  map one to one. The seam has no file modes, so the variadic mode is never
  read. `O_NONBLOCK` has no effect on the files the seam opens, and unknown
  bits are ignored, as Linux does.
- **read, write, close, lseek.** One seam call each; a negative return
  becomes `errno` and -1.
- **fileno** returns the handle under the stream's lock. **fdopen** parses
  the mode with `__libc_file_mode` (so `"w"` and `"x"` add nothing to an
  open file, as POSIX says) and calls `__libc_file_new`. A negative
  descriptor fails with `EBADF`. On failure POSIX leaves the descriptor with
  the caller, but `__libc_file_new` closed the handle when it failed; now it
  leaves the handle open, and fopen and tmpfile close it themselves.
- **strnlen** is `memchr` for the terminator within `maxlen`, so it reads
  no byte past `maxlen` and inherits the string kernels' `memchr`.
- **strcasecmp, strncasecmp** fold `'A'`–`'Z'` with `__libc_isupper` (the
  only locale is "C") and compare as `unsigned char`, in a new
  `libc/strings` module.
- **reallocarray** lives beside `calloc` in `libc/malloc`, the same
  `ckd_mul` check, then `realloc`. On overflow the old block is kept.
- **sincos, sincosf** call `sin` and `cos` (or `sinf` and `cosf`). Each
  result is exactly the single function's, on every libm layer of every
  architecture, and the files add no kernel. A shared reduction would save
  time on large arguments only, and would need one more kernel per layer;
  it can come later as a speed-flavour file (D77) if a benchmark asks.
- **clock_gettime, clock_getres** pass the clock to the seam as a time base:
  `CLOCK_REALTIME` is `TIME_UTC` (1), `CLOCK_MONOTONIC` is `TIME_MONOTONIC`
  (2), and `CLOCK_PROCESS_CPUTIME_ID` and `CLOCK_THREAD_CPUTIME_ID` are
  `TIME_ACTIVE` and `TIME_THREAD_ACTIVE`. An unknown clock is the port's
  `EINVAL`.
- **nanosleep** is `__rtos_thread_sleep`: `EINVAL` for `tv_nsec` outside
  [0, 10⁹), and `EINTR` with the time left in `rmtp`.
- **sched_yield** calls `__rtos_thread_yield` and returns 0.
- **getentropy** is not provided: no port has an entropy source, and the
  seam would need a new call. libstdc++'s `random_device` then has no
  default source, which configure detects. **isatty** stays deferred.

### Edge cases and tests (`test/unit/posix`, `test/unit/none`, `test/unit/api`)

- open/write/lseek/read/close round trip; `lseek` past the end and
  `SEEK_END`; `lseek(fd, -1, SEEK_SET)` gives `EINVAL`; I/O on a closed
  descriptor gives `EBADF`; `O_EXCL` on an existing file gives `EEXIST`, a
  missing file `ENOENT`, `O_ACCMODE` `EINVAL`; `O_APPEND` writes at the end;
  a write to an `O_RDONLY` descriptor fails; `O_TRUNC` empties the file.
- `fileno` of the standard streams is 0, 1, 2; two streams on one
  descriptor (`fopen` then `fdopen(fileno(…))`) see each other's writes after
  `fflush`; closing the `fdopen` stream closes the descriptor; `fdopen`'s
  `"w"` does not truncate; `fdopen(-1, …)` gives `EBADF`;
  `regression_fdopen_failure_keeps_descriptor`: after a bad mode the
  descriptor still writes.
- `strnlen` with `maxlen` 0, below, at and above the length, `SIZE_MAX`, and
  an unterminated array; `strcasecmp` and `strncasecmp` on case pairs,
  prefixes, the characters between `'Z'` and `'a'` (`'['` sorts below
  `'a'` because both sides fold to lower case), bytes above 127, and `n`
  of 0 and `SIZE_MAX`.
- `reallocarray`: grow keeps the contents; both operand orders of a
  wrapping product fail with `ENOMEM` and keep the block.
- `sincos`, `sincosf`: bit-identical to `sin`/`cos` at ±0, the smallest
  subnormal, 0.5, ±1, π, 10²², the largest power of two, ±∞ and NaN.
- Clocks: `CLOCK_REALTIME` agrees with `timespec_get(TIME_UTC)`;
  resolutions are positive; unknown clocks give `EINVAL`; a 2 ms
  `nanosleep` lasts at least 2 ms on `CLOCK_MONOTONIC`; `tv_nsec` of 10⁹ and
  -1 give `EINVAL`; `sched_yield` returns 0.
- `port/none`: `open` gives `ENOSYS`, `write(0, …)` `EBADF`, `read(0, …)` 0,
  `lseek(1, …)` `ESPIPE`, `close(5)` `EBADF`; the clocks and `nanosleep`
  give `ENOSYS`; `nanosleep` with a bad `tv_nsec` still gives `EINVAL`.
- `test/unit/api`: strict C23 leaves every one of the names free (the test
  defines objects with them), and under `_POSIX_C_SOURCE` each has the
  POSIX type.

## P7.04: what `std::stacktrace` needs

### Sources reviewed

- GCC 16 release branch (`releases/gcc-16`), read for structure only:
  - `libstdc++-v3/acinclude.m4` `GLIBCXX_ENABLE_BACKTRACE`
    (<https://github.com/gcc-mirror/gcc/blob/releases/gcc-16/libstdc++-v3/acinclude.m4>);
  - `libstdc++-v3/src/libbacktrace/Makefile.am`;
  - `libbacktrace/fileline.c`, `posix.c`, `elf.c`, `read.c`, `alloc.c`
    (<https://github.com/gcc-mirror/gcc/tree/releases/gcc-16/libbacktrace>).
- The toolchain in `local/` (arm-none-eabi GCC 16.2, built for newlib): the
  members and undefined symbols of `libstdc++exp.a`.

### How libstdc++ builds `<stacktrace>`

- `--enable-libstdcxx-backtrace` defaults to `auto`, which follows
  `is_hosted` (off for freestanding builds and AVR). When on, libbacktrace is
  compiled into `libstdc++exp.a` with its symbols renamed `__glibcxx_backtrace_*`:
  - always `atomic`, `backtrace`, `dwarf`, `fileline`, `posix`, `sort`,
    `simple`, `state` and `cp-demangle`;
  - a format file, `elf` (from the compiler's object format, ELF32 here);
  - a view file and an allocator. Without `<sys/mman.h>` they are `read.c`
    and `alloc.c` (malloc). With it, `mmapio.c` and, when `MAP_ANONYMOUS`
    exists, `mmap.c`.
- `HAVE_FCNTL` is forced on for target builds other than mingw, so `posix.c`
  calls `fcntl(fd, F_SETFD, FD_CLOEXEC)` after `open` (with `O_CLOEXEC` when
  defined). `getexecname` is assumed only on Solaris.
- `std::stacktrace::current()` (`__stacktrace_impl::_S_current`) creates a
  libbacktrace state and calls `backtrace_simple`, which is
  `_Unwind_Backtrace` with a callback storing each frame's PC. Capture
  therefore needs only libgcc's unwinder and the EHABI tables.
  `to_string()` and the entries' `description()`/`source_file()` call
  `backtrace_pcinfo`/`backtrace_syminfo`, which open the executable. Both
  paths are in one object (`stacktrace.o`), so the whole of libbacktrace is
  linked whenever `std::stacktrace` is used.

### The C library names it needs

The toolchain's `libstdc++exp.a` (configured for newlib, which has
`<sys/mman.h>`) needs these undefined names:

- `open`, `close`, `fcntl`, `fstat`, `getpid`;
- `mmap`, `munmap`, `getpagesize`;
- `errno` (as `__thread`, which matches D131);
- `malloc`, `realloc`, `free`, `memcpy`, `memcmp`, `memset`, `strcmp`,
  `strncmp`, `strlen`, `strnlen`, `strrchr`, `bsearch`, `snprintf`,
  `sprintf`;
- from libgcc and libsupc++: `_Unwind_Backtrace`, `_Unwind_VRS_Get` and
  `__cxa_demangle`.

Configured against this library there is no `<sys/mman.h>`, so `mmap`,
`munmap` and `getpagesize` drop out and `read` and `lseek` come in. The
library already had everything except `fstat`, `fcntl` and `getpid`, which
P7.04 adds (D134).

**Compile check.** The GCC 16 sources of the eleven files libstdc++ would
use, with a `config.h` holding what its configure finds for this library
(`HAVE_FCNTL`, `HAVE_DECL_STRNLEN`, ELF32, no `<sys/mman.h>`), compile
against the library's headers (`-nostdinc`, `-D_GNU_SOURCE`, GCC's default C
mode). Linking and running them was not done in this session (running
fetched code needs the user's permission); P7.05's libstdc++ build does it.

**C mode.** In `-std=gnu11` the same files fail. `<stddef.h>` uses
`nullptr` unconditionally, as do `<assert.h>`, `<inttypes.h>`,
`<stdatomic.h>` and `<stdbit.h>`. libstdc++ compiles libbacktrace in the
compiler's default mode (`gnu23` for GCC 16), so this does not affect it,
but pi-pico builds with `-std=gnu11` (Q40). Resolved by D135: the headers stay C23-only, and pi-pico moves to `-std=gnu23`.

### Without an executable

`fileline_initialize` tries, in order:

1. the file name given to `backtrace_create_state`;
2. `getexecname`, `_pgmptr`;
3. `/proc/self/exe`, `/proc/curproc/file`, `/proc/<getpid()>/object/a.out`;
4. the BSD `sysctl` paths, macOS, Windows and HP-UX.

A file that does not exist moves it to the next candidate. When none opens,
it reports "libbacktrace could not find executable to open" once through
the error callback, marks the state failed, and every later `pcinfo` or
`syminfo` returns at once. libstdc++ then gives entries with an empty
description and file. On `port/none`, `open` fails with `ENOSYS`, which is
not `ENOENT`, so the first attempt reports that error and stops; the result
is the same. Capture never touches files.

### `fstat`, `fcntl`, `getpid` over the seam

- **`fstat`.** The seam knows a handle's offset and nothing else. A seek to
  the current position tells a seekable handle from a stream: `ESPIPE`
  becomes a character device of size 0, and another error is returned.
  Then a seek to the end gives the size, and a seek back restores the
  offset. `st_nlink` is 1 and the other fields are 0. A pipe on Linux is
  reported as a character device, not a FIFO; nothing in the subset tells
  them apart.
- **`fcntl`.** Only `F_GETFD` and `F_SETFD`, which libbacktrace uses.
  `FD_CLOEXEC` has no effect, because nothing executes another program, so
  `F_GETFD` returns 0. A zero-length seek validates the handle, and only
  `EBADF` from it is an error, since a console may not seek. Other commands
  give `EINVAL`.
- **`getpid`.** The program is the only process: 1.

### Unwind tables

- **Without `-funwind-tables`,** C functions get no EHABI entry. The linker
  marks code without one `EXIDX_CANTUNWIND`, so a walk that reaches a
  library frame (`qsort` calling a comparison) stops there cleanly.
- **With `LIBC_UNWIND_TABLES=1`,** every library function has an entry, and
  the walk continues to `main`.
- **What the option costs:**
  - The tables: 8 bytes per function in `.ARM.exidx`, plus `.ARM.extab`
    entries for frames the compact model cannot describe. That is about
    40 KB over `libc.a`. In the debug test images it adds 1.4 KB (`smoke`)
    and 4.4 KB (`stdio`).
  - The unwinder: each entry references libgcc's personality routine
    `__aeabi_unwind_cpp_pr0`, which brings in the unwinder (about 3.4 KB)
    and `abort`.

### Tests

- `test/unit/unwind` (Arm):
  - own frames;
  - through `qsort` in both builds;
  - `__exidx_start` < `__exidx_end`.
- `test/unit/posix`:
  - `fstat` on a regular file: size, offset kept;
  - `fcntl` flags and errors;
  - `fstat`/`fcntl` on closed and negative descriptors;
  - `O_CLOEXEC` with `open`;
  - `getpid`.
- `test/unit/none`:
  - `fstat(1)` is a character device;
  - `fstat(-1)` gives `EBADF`;
  - `fcntl` and `getpid`.
- `test/unit/api`: the new macros, types and prototypes; `<sys/stat.h>` in
  the strict standalone compile.
- `test/unit/cxx`: `<sys/stat.h>` as C++17 and `gnu++98`.
