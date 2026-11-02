# Research: the minimal POSIX subset

Issue #106. Plan: PLAN D128 (the subset), phase 7. Decisions from this note:
D129 (descriptors are seam handles; answers Q37), D130 (the library defines
`open`, `close`, `read` and `write`; answers Q38). Open: Q39 (how a hosted
libstdc++ is configured for this library).

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
