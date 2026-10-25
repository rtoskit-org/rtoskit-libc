# Toolchain

Recorded in P1.01 (2026-10-18), Ubuntu 24.04 cloud container; the Arm toolchain was added in P1.06 (2026-10-19).

| Tool | Version | Source | Used for |
|---|---|---|---|
| GCC | 14.2.0 (`gcc-14`) | apt `gcc-14` | x86_64 libc and tests (`-std=c23`; GCC 13 rejects it, see PLAN D9). Any GCC 14 or newer works: `tools/scripts/host-cc` picks plain `gcc` when it is new enough, else the newest `gcc-NN`, and `HOST_CC` overrides (D107). |
| binutils | 2.42 | apt (default) | x86_64 `as`, `ld`, `ar` |
| qemu-user | 8.2.2 | apt `qemu-user` | runs ARM test binaries from P1.06 |
| clang-format | 18.1.3 | apt `clang-format` | formatting with `tools/clang-format` |
| arm-none-eabi GCC | 16.2.0 (crosstool-NG 1.29.0, binutils with it) | `make setup` downloads red-rocket-computing/cross-dev release v0.3.0 into `local/` (`tools/makefiles/setup.mk`); the first `make` does it through the `libc port: setup-tools` dependency | the six Arm architectures (D9, D36, D37, D108). Multilibs used: `thumb/v6-m/nofp` (armv6-m and armv6s-m), `thumb/v7-m/nofp` (armv7-m), `thumb/v7e-m+fp/hard`, `thumb/v7e-m+dp/hard`, `thumb/v8-m.main+fp/hard`. The apt `gcc-arm-none-eabi` (13.2) is too old for `-std=c23`. |

Cloud sessions install the apt packages through `.claude/hooks/session-start.sh`; the Arm toolchain comes from `make setup` (about 180 MB, unpacked into `local/`, which `make realclean` removes).

## Notes

- The headers rely on GCC's predefined macros (`__INT64_C(c)`,
  `__SIG_ATOMIC_MIN__`, `__WCHAR_MIN__`, `__WINT_MIN__`, ...). Clang 18 lacks
  some of them (it has `__INT64_C_SUFFIX__` instead). Verify the Clang version
  that provides them before enabling Clang builds (D9).
- `ARCH_CROSS=x86_64` uses `tools/makefiles/x86_64.mk`. The Arm toolchain
  files (`armv6-m.mk`, `armv6s-m.mk`, `armv7-m.mk`, `armv7e-m.mk`, `armv7e-m-dp.mk`, `armv8-m.main.mk`) follow
  pi-pico's toolchain-file pattern (`CROSS_COMPILE`, `CROSS_FLAGS`) but are named after
  the architecture and use `-march`, not `-mcpu` (D24); they share `arm-none-eabi.mk`
  and export `ARCH_FAMILY` and `LIBC_FPU` (D34). `armv6-m` is plain Armv6-M (no `SVC`);
  `armv6s-m` adds the OS extension (D37). `armv7-m` is Armv7-M without an FPU or the DSP
  extension (Cortex-M3, D108).
- On the Arm architectures, `tools/makefiles/port-linux-arm.mk` runs the tests with
  `qemu-arm -cpu max` (D36) and links them with `port/linux/arm/headers.ld` so the ELF
  headers are loaded (D106).
- GCC 15.2 did not implement `_BitInt` for 32-bit Arm; GCC 16.2 does
  (`__BITINT_MAXWIDTH__` is 65535), which is one reason for the 16.2 toolchain (D37).
- The arm-none-eabi objects have no `.note.GNU-stack`; the link passes
  `-z noexecstack` instead of letting ld warn about an executable stack.
- `__builtin_flt_rounds` does not exist in GCC 14, so `FLT_ROUNDS` calls the
  library function `__flt_rounds()`.
- Ubuntu's GCC enables PIE, `-fstack-protector-strong` and CET by default.
  `tools/makefiles/x86_64.mk` turns off PIE and the stack protector for the
  static, libc-less link. It also sets `LISTING_STYLE := -adlns` (as
  `release.mk` does for host builds), because the native assembler cannot
  interleave source in listings of optimised code.

## Formatting check

```sh
clang-format --style=file:tools/clang-format --dry-run $(find libc port include test/unit test/libc-test/shim example -name '*.[ch]')
```
