# Research: conformance closure for phase 1

Step: P1.17. Decision: PLAN D50.

## Sources reviewed

- The standard text: C23 Annex B (the library summary, `docs/standard/annex-b.pdf`), Annex J.3 and J.4 (implementation-defined and locale-specific behaviour), 4p6 (freestanding implementations), and 6.10.10.3 (the conditional feature macros).
- libc-test `src/api/*.c` (musl's header tests): each declaration is checked by taking a function's address into a pointer of the expected type, compiled with `-pedantic-errors`. They cover only part of C23 and mix in POSIX names.
- glibc's `include/stdc-predef.h`: the hosted GCC driver pre-includes it, and it defines `__STDC_IEC_559__`, `__STDC_IEC_60559_BFP__`, `__STDC_IEC_559_COMPLEX__` and `__STDC_ISO_10646__`. GCC skips the pre-include under `-nostdinc` and `-ffreestanding`.
- musl and picolibc, for documentation of implementation-defined behaviour: musl documents its behaviour piecemeal; picolibc does not document J.3 at all.

## Approaches

| Check | Options | Chosen |
|---|---|---|
| Prototypes | (a) Function pointers of the expected type, as libc-test does. (b) Redeclare each function with its Annex B prototype. | (b). A redeclaration with an incompatible type is a constraint violation, so a mismatch fails to compile. The function name is parenthesised, so a masking macro does not expand. |
| Macros, types, constants | `#ifndef X` alone misses enum constants, typedefs and objects. | `#ifndef X` followed by `static_assert(sizeof(X) > 0)`. This accepts a macro, a type, a constant or an object of the name. `FILE` may be incomplete (C23 6.2.5p1), so it is checked through a pointer. |
| Pseudo-prototypes (`real-floating`, generic placeholders, `QChar`, atomic `A`/`C`/`M`) | Cannot be redeclared. | An existence check: a macro, or a declared function. |
| Freestanding | (a) Compile-only. (b) Link without the port. | (b). The freestanding test links `libc.a` without the port and without startup code, so any dependency on the RTOS seam (errno through TLS, locks, I/O) fails the build. A second link through the port runs the checks. |

## Scope exclusions in the audit

The audit leaves these out:
- `<uchar.h>`, `<wchar.h>` and `<wctype.h>`;
- decimal floating point (DFP);
- Annex K (`_s` functions, `errno_t`, `rsize_t`);
- the macros that are defined only when supported (the `FE_*` exceptions and directions, `FP_FAST_*`, `__STDC_NO_*`, `imaginary`).

These exclusions match PLAN §1, D44 and D49.

## Findings

The audit generated these checks:
- 689 identifiers;
- 642 prototypes;
- 73 generic names across 28 headers.

It found two discrepancies:
- `BOOL_MAX` (C23 5.2.5.3.2) was missing from `<limits.h>`;
- `CR_DECIMAL_DIG` (Annex F.5) was missing from `<float.h>`. It is defined under `__STDC_WANT_IEC_60559_EXT__` as 800. The conversions are correctly rounded for any length (D38, D39).

Fault injection confirmed that the checks bite: removing a macro, changing a prototype's parameter type, or adding a seam call to a freestanding string function each fails the build.

## Replacing libc-test's `api/*` tests (issue #93, D122)

libc-test's 14 running `api/*` tests are compile-time checks:
- every type exists;
- every macro that must be an integer constant expression works as a `case`
  label;
- a constant has the size of its type;
- every function has its exact prototype;
- each header compiles with `-pedantic-errors`.

For what each test checks beyond existence, the tests were read only to list
it. Nothing was copied: every in-tree check is written from N3220 clause 7.

| libc-test test | What it checked beyond existence | In-tree replacement |
|---|---|---|
| `api/assert` | `assert` is a macro with and without `NDEBUG` | `test/unit/api`: `NDEBUG` toggled in `ndebug.c`, and the argument evaluated or not |
| `api/complex` | the types and `I`, the functions' prototypes | `annexb` (prototypes); `api`: the types of `I`, `_Complex_I` and `CMPLX*`; I² = −1 |
| `api/errno` | `EDOM`, `EILSEQ` and `ERANGE` are constant expressions | `headers` (positive, distinct); `api`: `case` labels, and `errno` an `int` lvalue |
| `api/fenv` | `fenv_t`, `fexcept_t`, the `FE_*` macros | `annexb`; `api`: the defined exception macros OR to distinct values and to `FE_ALL_EXCEPT`, the rounding macros are distinct and non-negative, and `FE_DFL_ENV` and `FE_DFL_MODE` have their types |
| `api/float` | the characteristics are constant expressions of their types | `headers` (`FLT_RADIX` and the sizes); `api`: every integer characteristic in `#if` with its IEEE 754 binary32 and binary64 value, every floating characteristic of its own type and value |
| `api/iso646` | the eleven macros exist | `annexb`; `api`: each one evaluated as its operator |
| `api/math` | `float_t`, `double_t`, `HUGE_VAL*`, `INFINITY`, `NAN`, `FP_*` constants, `MATH_ERR*`, prototypes | `annexb` (prototypes); `api`: the types, distinct classification and rounding-direction values, `FP_ILOGB*`/`FP_LLOGB*` values, `math_errhandling` |
| `api/setjmp` | `jmp_buf`, `setjmp`, `longjmp` | `annexb`; `api`: `longjmp(env, 0)` makes `setjmp` return 1, and another value comes back unchanged |
| `api/signal` | `sig_atomic_t`, `SIG_*`, the six signal numbers | `annexb`; `api`: the handler macros have the handler type, the numbers are positive and distinct, and `signal` returns the previous handler |
| `api/stdarg` | `va_list` and the four macros | `annexb`; `api`: a variadic sum with the C23 one-argument `va_start` and an independent `va_copy` |
| `api/stdbool` | `bool`, `true`, `false` | `headers` (`__bool_true_false_are_defined`); C23 keywords |
| `api/stddef` | the types, `NULL`, `offsetof` | `annexb`; `api`: `offsetof` is a constant of type `size_t`, `max_align_t` alignment, `nullptr_t` |
| `api/stdint` | every type, every limit a constant expression | `headers` (every limit in `#if`, exact values); `annexb` |
| `api/tgmath` | each generic macro accepts a double argument | `annexb` (the 73 names); `api`: the result type of every macro for float, double, long double, integer and complex arguments, mixed two- and three-argument forms, and the narrowing `f*`/`d*` macros |

libc-test compiled these tests with `-pedantic-errors`. `test/unit/api`
replaces that gate: `api.mk` compiles every public header on its own, with
`-std=c23 -pedantic-errors -Werror`, on every architecture in `ARCHS`. A
header that relies on an extension without its reserved spelling then fails
the build.
