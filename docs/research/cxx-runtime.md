# Research: rtoskit-libc under C++

Issue #98. Decisions: PLAN D120 (C++-clean headers), D121 (the C++ ABI hooks
the C library owns). Resolves the C-library side of Q8.

## What C++ needs from a C library

A C++ program built with `g++` and libstdc++ reaches the C library in three
ways:

1. **Through the C headers.** libstdc++'s `<cstdio>`, `<cstring>`, `<cmath>`
   and the others `#include` the C library's headers. Those headers must parse
   as C++ and give their functions C linkage.
2. **Through the C++ ABI's termination hooks.** The compiler registers the
   destructor of every static object with `__cxa_atexit` (on Arm, through
   libsupc++'s `__aeabi_atexit`), naming the image with `&__dso_handle`. It
   registers the destructor of a `thread_local` object with
   `__cxa_thread_atexit`. Both normally come from the C library.
3. **Through ordinary C functions** that libsupc++ and the unwinder call:
   `malloc`, `free`, `abort`, `memcpy`, `strcmp` and so on.

## Sources reviewed

- **Itanium C++ ABI**, 3.3.5 "DSO Object Destruction API"
  (<https://itanium-cxx-abi.github.io/cxx-abi/abi.html#dso-dtor>):
  - `__cxa_atexit(f, p, d)` registers `f(p)` for the image `d`;
  - `__cxa_finalize(d)` runs that image's registrations, newest first, each
    once; with a null `d` it runs every termination function, `atexit`'s
    included;
  - `__dso_handle` is the image's address.
- **Arm C++ ABI** (CPPABI32,
  <https://github.com/ARM-software/abi-aa/blob/main/cppabi32/cppabi32.rst>):
  - `__aeabi_atexit(object, destructor, dso)` is `__cxa_atexit` with the first
    two arguments swapped;
  - libsupc++ provides it (`atexit_arm.o`), calling `__cxa_atexit`.
- **C++ standard**:
  - **[basic.start.term]:** at exit, `thread_local` objects of the exiting
    thread are destroyed first; then static objects and `atexit` functions run
    in the reverse order of their registration, interleaved.
  - **[support.c.headers]:** `<iso646.h>` and `<stdnoreturn.h>` define nothing
    in C++, `<stdatomic.h>` maps onto `std::atomic` (C++23), `<tgmath.h>`
    includes `<cmath>` and `<complex>`.
- **libstdc++'s `atexit_thread.cc`**
  (<https://github.com/gcc-mirror/gcc/blob/master/libstdc%2B%2B-v3/libsupc%2B%2B/atexit_thread.cc>):
  - when configure finds `__cxa_thread_atexit` in the C library
    (`_GLIBCXX_HAVE___CXA_THREAD_ATEXIT`), libstdc++ defines none of its own;
  - with only `__cxa_thread_atexit_impl` it forwards to that;
  - otherwise it keeps its own list and runs it from `atexit` or a gthread key.
- **musl** `src/exit/atexit.c`
  (<https://git.musl-libc.org/cgit/musl/tree/src/exit/atexit.c>): a single list
  of `{function, argument}` slots shared by `atexit` and `__cxa_atexit`, in
  blocks of 32, the first static and the rest from `calloc`.
- **newlib** `libc/stdlib/__atexit.c`
  (<https://sourceware.org/git/?p=newlib-cygwin.git;a=blob;f=newlib/libc/stdlib/__atexit.c>):
  - one table of 32 entries holding the function, its argument, the DSO
    handle and a bit saying which kind of call each entry needs;
  - more blocks with `malloc` unless `_ATEXIT_DYNAMIC_ALLOC` is off.
- **glibc** `stdlib/cxa_thread_atexit_impl.c`: a per-thread list of
  `{func, obj, dso}` nodes from `calloc`, run from `__call_tls_dtors` at thread
  exit and from `exit` before the `atexit` functions.
- **The toolchain's libsupc++** (crosstool-NG GCC 16.2,
  `--enable-threads=no`, built against picolibc), read with `nm`. Its
  undefined C symbols are:
  - the exit and memory calls: `__cxa_atexit`, `atexit`, `abort`, `malloc`,
    `free`, `realloc`, `memalign`;
  - the string calls: `memcpy`, `memcmp`, `memset`, `strcmp`, `strncmp`,
    `strlen`, `strcpy`, `strchr`, `strtoul`;
  - the rest: `getenv`, `sprintf`, `fputs`, `fputc` and the object `stderr`.
  The only sources of `memalign` and `stderr` are aligned `operator new`
  (`new_opa.o`) and the verbose terminate handler (`vterminate.o`). The
  toolchain is configured with `--disable-libstdcxx-verbose`, so neither is
  linked by default.

## Trade-offs

### Headers

- **`extern "C"`:** CLAUDE.md already allows `__cplusplus` guards for it. One
  piece, `<__cdefs.h>`, defines `__LIBC_BEGIN_DECLS`/`__LIBC_END_DECLS`, so
  each header carries no guard of its own.
- **`restrict` and `typeof`:** C23 keywords that C++ lacks. GCC and Clang
  accept `__restrict` and `__typeof__` in both languages, so the headers use
  those spellings and need no conditional (the floor is GCC or Clang, D9).
- **`NULL`:** `((void *)0)` does not convert to other pointer types in C++.
  `__LIBC_NULL` is `nullptr` in C++.
- **What C++ does not have:**
  - `wchar_t` is a C++ keyword, so the `typedef` is C-only;
  - `_Atomic`, `_Generic` and `__builtin_complex` do not exist in C++, so
    `<stdatomic.h>` and `<tgmath.h>` have C-only bodies; the C++ standard
    library's own versions of those headers precede ours on a C++ include path;
  - the `_Generic` forms of `memchr`, `strchr`, `strpbrk`, `strrchr`, `strstr`
    and `bsearch` are C-only: libstdc++'s `<cstring>` and `<cstdlib>` supply
    the `const` overloads;
  - the `<inttypes.h>` self-check is C-only;
  - `<iso646.h>`, `<stdnoreturn.h>` and the `complex` and `I` macros of
    `<complex.h>` define nothing in C++, as the C++ standard says.
- **The `rename` parameter:** `new` is a C++ keyword. It is renamed
  `new_name`.

### Static destructors

- **One list or two:** C++ requires static destructors and `atexit` functions
  to run in one reverse order. musl and newlib keep one list whose entries
  carry an argument. Here that would grow every C program that uses `atexit`
  by 4 bytes a slot, 128 bytes in all.
- **The chosen shape:**
  - `atexit` keeps its table of 32 function pointers.
  - `__cxa_atexit` keeps its own `{func, arg, dso, level}` entries, in blocks
    of 32: the first static, the rest from `calloc`.
  - A run of consecutive `__cxa_atexit` registrations puts one internal
    function in the `atexit` table, and each entry records that slot as its
    `level`.
  - When the exit runner pops the slot, the internal function runs the newest
    entries of its level, newest first. Those are then exactly the entries
    registered while that slot was the newest.
  - So the two lists interleave correctly, a C program pays nothing, and a C++
    program with any number of static objects takes one `atexit` slot per run
    of registrations.
- **`__cxa_finalize(d)`** marks each entry it runs as a hole, which the exit
  path skips. `__cxa_finalize(nullptr)` runs the whole `atexit` table, as the
  ABI says.
- **`__dso_handle`** is a member of its own. GCC's `crtbegin.o` defines it
  too; the archive member is used only when nothing else defines it.

### `thread_local` destructors

- **`__cxa_thread_atexit` in the C library.** libstdc++ configured against
  rtoskit-libc then finds it and defines none of its own. The list is a
  `thread_local` pointer to `malloc`'d nodes.
- **When the list runs:**
  - `thrd_exit` runs it before the `tss` destructors, as glibc does;
  - the `atexit` runner runs it first, so the main thread's `thread_local`
    objects die before any static object.
  - Both calls are weak, so a C program that never uses C++ `thread_local`
    links neither.
- **Linking the runner:** `__cxa_thread_atexit` takes a strong reference to
  the `atexit` table, so `exit` runs the list even in a program that
  registers nothing else.
- **The current toolchain.** Its libsupc++ was configured against picolibc
  and defines its own `__cxa_thread_atexit`. A program linked the usual way
  (libsupc++ before libc) takes that one. The unit test lists `libc.a` first
  to test ours.

### Threads model

The test toolchain uses `--enable-threads=no` (the single gthreads model).
Static local initialisation guards and `std::mutex` do no locking. A
`gthr-rtoskit.h` over `<threads.h>` belongs to the rtoskit-gcc toolchain (Q8
part 2), not to this library.

## Edge cases

- A destructor that registers another one: the `atexit` runner and
  `__cxa_finalize` take an entry out before calling it, so the new
  registration runs next.
- `__cxa_finalize` twice on the same image: the second call finds only
  holes.
- More than 32 registrations: a second block is allocated, and it is freed
  when it empties.
- The `atexit` table is full when a new run of registrations starts:
  `__cxa_atexit` returns nonzero, as `atexit` does.
- A thread that uses no `thread_local` object with a destructor runs an empty
  list.

## Test vectors

`test/unit/cxx` runs on every Arm architecture in every build type:
- **`cxx.cpp`**, compiled as C++23:
  - static constructors in declaration order;
  - exit order `t` (the main thread's `thread_local`), then `d`, `x`
    (an `atexit` handler), 70 `__cxa_atexit` registrations (more than one
    block, newest first), `q`, `b`, `a`, and last the check registered before
    any object;
  - `__cxa_finalize` on a fake image, twice;
  - a `thread_local` destructor at `thrd_exit`;
  - exceptions through four frames with destructors, and a rethrow;
  - `dynamic_cast`, virtual calls, `new[]` and `delete[]`.
- **`headers17.cpp`**, compiled as C++17: includes every public header and
  calls a function from several of them, which links only if they have C
  linkage.
