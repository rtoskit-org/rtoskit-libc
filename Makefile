.DEFAULT_GOAL := all
ifeq (${MAKECMDGOALS},)
MAKECMDGOALS := all
endif

# The architectures (ARCH_CROSS toolchain files under tools/makefiles/, PLAN D24, D106)
ARCHS ?= x86_64 armv6-m armv6s-m armv7-m armv7e-m armv7e-m-dp armv8-m.main
BUILDS ?= debug release speed

export ARCH_CROSS ?= all
export BUILD_TYPE ?= all
export PROJECT_ROOT ?= ${CURDIR}
export OUTPUT_ROOT ?= ${PROJECT_ROOT}/build
export TOOLS_ROOT ?= ${PROJECT_ROOT}/tools
export PREFIX ?= ${PROJECT_ROOT}/images

ifeq (${V},)
SILENT=--silent
endif

MAKEFLAGS += ${SILENT}

# GNU make 4.4 or newer (PLAN D117): tools/makefiles/project.mk relies on .NOTINTERMEDIATE, which an older make
# ignores without a word, and then a deleted archive is not rebuilt while what was built from it is newer
ifeq ($(filter notintermediate,${.FEATURES}),)
$(error GNU make 4.4 or newer is required (this is ${MAKE_VERSION}); see "Build requirements" in README.md)
endif

# An ARCH_CROSS without a toolchain file (one a parent build exported, say) stops here, not deep in a compile
ifneq ($(filter-out all,${ARCH_CROSS}),)
ifeq ($(wildcard ${TOOLS_ROOT}/makefiles/${ARCH_CROSS}.mk),)
$(error ARCH_CROSS='${ARCH_CROSS}' has no toolchain file ${TOOLS_ROOT}/makefiles/${ARCH_CROSS}.mk; use one of: ${ARCHS})
endif
endif

ifeq (${ARCH_CROSS},all)

# Stop at the first architecture that fails instead of reporting only the last one
all:
	@for arch in ${ARCHS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile ARCH_CROSS=$$arch all || exit $$?; \
	done
.PHONY: all

clean:
	@for arch in ${ARCHS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile ARCH_CROSS=$$arch clean || exit $$?; \
	done
.PHONY: clean

check:
	@for arch in ${ARCHS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile ARCH_CROSS=$$arch check || exit $$?; \
	done
.PHONY: check

library:
	@for arch in ${ARCHS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile ARCH_CROSS=$$arch library || exit $$?; \
	done
.PHONY: library

bench:
	@for arch in ${ARCHS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile ARCH_CROSS=$$arch bench || exit $$?; \
	done
.PHONY: bench

aor:
	@for arch in ${ARCHS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile ARCH_CROSS=$$arch aor || exit $$?; \
	done
.PHONY: aor

else ifeq (${BUILD_TYPE},all)

export INSTALL_ROOT := ${PREFIX}$(if ${ARCH_CROSS},/${ARCH_CROSS},)
export BUILD_ROOT := ${OUTPUT_ROOT}$(if ${ARCH_CROSS},/${ARCH_CROSS},)

all:
	@for build_type in ${BUILDS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile BUILD_TYPE=$$build_type all || exit $$?; \
	done
.PHONY: all

clean:
	@for build_type in ${BUILDS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile BUILD_TYPE=$$build_type clean || exit $$?; \
	done
.PHONY: clean

check:
	@for build_type in ${BUILDS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile BUILD_TYPE=$$build_type check || exit $$?; \
	done
.PHONY: check

library:
	@for build_type in ${BUILDS}; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile BUILD_TYPE=$$build_type library || exit $$?; \
	done
.PHONY: library

# Benchmarks measure the two flavours: release (size) and speed (PLAN D73)
bench:
	@for build_type in release speed; do \
		${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile BUILD_TYPE=$$build_type bench || exit $$?; \
	done
.PHONY: bench

# The AOR archives do not depend on the build type; the smoke test links the release build (PLAN D92)
aor:
	@${MAKE} --no-print-directory -f ${PROJECT_ROOT}/Makefile BUILD_TYPE=release aor
.PHONY: aor

else

export INSTALL_ROOT := ${PREFIX}$(if ${ARCH_CROSS},/${ARCH_CROSS},)$(if ${BUILD_TYPE},/${BUILD_TYPE},)
export BUILD_ROOT := ${OUTPUT_ROOT}$(if ${ARCH_CROSS},/${ARCH_CROSS},)$(if ${BUILD_TYPE},/${BUILD_TYPE},)

# Concurrent "install -D" calls can race creating the same parent directory,
# so create the install root once, serially, for goals that install anything.
NO_INSTALL_ROOT_GOALS := clean distclean realclean info help setup
ifneq ($(filter-out ${NO_INSTALL_ROOT_GOALS},$(or ${MAKECMDGOALS},all)),)
$(if $(shell mkdir -p '${INSTALL_ROOT}' 2>&1),$(error cannot create ${INSTALL_ROOT}))
endif

include ${TOOLS_ROOT}/makefiles/setup.mk
include ${TOOLS_ROOT}/makefiles/tree.mk

# Dependency ordering only active for full builds (all/clean/install)
# Individual module targets (make libc, make test/unit/smoke) are dep-free
ifneq ($(filter-out all target clean install check bench aor library,${MAKECMDGOALS}),)
target:
else
# The cross toolchain is downloaded into local/ on first use (tools/makefiles/setup.mk)
libc port: setup-tools
example test: libc port
target: example test
endif

# The library alone, for a parent project (docs/configuration.md, "Building from another project", PLAN D105):
# the headers, the libc*.a archives and the port archives, installed into ${INSTALL_ROOT}; no tests or examples
library: libc port
.PHONY: library

# Build, then run every unit test (a test directory may provide <name>.run to drive its binary).
# Last, no program may link a libgcc object that defines one of the library's own symbols (PLAN D102)
check: target
	${PROJECT_ROOT}/test/unit/run-unit ${BUILD_ROOT}/test/unit
	${PROJECT_ROOT}/tools/scripts/libgcc-check --nm ${NM} --libgcc $(shell ${CC} ${CROSS_FLAGS} -print-libgcc-file-name) ${BUILD_ROOT}
.PHONY: check

# Build, then run the test/bench programs and compare them with docs/perf/baseline-<arch>[-speed].json
# (tools/scripts/bench-runner, PLAN D72); BENCH_UPDATE=1 records the results as the new baseline
bench: target setup-bench
	cd ${PROJECT_ROOT} && tools/scripts/bench-runner --build-root ${OUTPUT_ROOT} --build-type ${BUILD_TYPE} run --arch ${ARCH_CROSS}
	cd ${PROJECT_ROOT} && $(if ${BENCH_UPDATE},tools/scripts/bench-runner --build-root ${OUTPUT_ROOT} --build-type ${BUILD_TYPE} report --arch ${ARCH_CROSS},tools/scripts/bench-runner --build-root ${OUTPUT_ROOT} --build-type ${BUILD_TYPE} diff --arch ${ARCH_CROSS})
.PHONY: bench

# Build Arm Optimized Routines for the architecture into build/<arch>/aor/ and run its smoke test
# against this build (tools/scripts/aor-build, Phase 4, PLAN D92); measurement only, never part of libc.a
aor: target setup-aor
	cd ${PROJECT_ROOT} && tools/scripts/aor-build --build-root ${OUTPUT_ROOT} build --arch ${ARCH_CROSS}
	cd ${PROJECT_ROOT} && tools/scripts/aor-build --build-root ${OUTPUT_ROOT} smoke --arch ${ARCH_CROSS} --build-type ${BUILD_TYPE}
.PHONY: aor

endif

db:
	bear --append -- ${MAKE} all

distclean:
	@echo "DISTCLEAN ${PREFIX} ${OUTPUT_ROOT} ${PROJECT_ROOT}/compile_commands.json"
	-${RM} -r ${PREFIX} ${OUTPUT_ROOT} ${PROJECT_ROOT}/compile_commands.json

realclean: distclean
	@echo "REALCLEAN ${PROJECT_ROOT}/tmp ${PROJECT_ROOT}/local"
	-${RM} -r ${PROJECT_ROOT}/tmp ${PROJECT_ROOT}/local

info:
	@echo "ARCH_CROSS=${ARCH_CROSS}"
	@echo "ARCH_FAMILY=${ARCH_FAMILY}"
	@echo "LIBC_FPU=${LIBC_FPU}"
	@echo "RTOS_PORT=${RTOS_PORT}"
	@echo "CROSS_COMPILE=${CROSS_COMPILE}"
	@echo "BUILD_TYPE=${BUILD_TYPE}"
	@echo "PROJECT_ROOT=${PROJECT_ROOT}"
	@echo "OUTPUT_ROOT=${OUTPUT_ROOT}"
	@echo "TOOLS_ROOT=${TOOLS_ROOT}"
	@echo "PREFIX=${PREFIX}"
	@echo "BUILD_ROOT=${BUILD_ROOT}"
	@echo "INSTALL_ROOT=${INSTALL_ROOT}"
.PHONY: info

help:
	@echo "Targets:"
	@echo "  all                    Build everything (full ordered build)"
	@echo "  clean                  Clean all build artifacts"
	@echo "  install                Install all build artifacts"
	@echo "  check                  Build and run the tests"
	@echo "  library                Build and install only the headers, libc*.a and the port archives"
	@echo "  bench                  Build release and speed, run the benchmarks against docs/perf"
	@echo "  setup-bench            Build the instruction-counting qemu for bench into local/"
	@echo "  aor                    Fetch and build Arm Optimized Routines for comparison, run its smoke test"
	@echo "  db                     Generate compilation database (bear)"
	@echo "  setup                  Download and install host build tools"
	@echo "  distclean              Delete all build artifacts and output directories"
	@echo "  realclean              Delete all build artifacts, directories and downloaded tools"
	@echo "  info                   Display build configuration"
	@echo ""
	@echo "Direct module targets (dep-free, assumes prior full build; set ARCH_CROSS and BUILD_TYPE):"
	@echo "  <module>               Build a top-level module, e.g. libc, port, test"
	@echo "  <module>/<submodule>   Build a nested module, e.g. libc/string, port/linux"
	@echo "  test/unit/<name>       Build a single unit test, e.g. test/unit/smoke"
	@echo ""
	@echo "Configuration variables:"
	@echo "  ARCH_CROSS=all         Which architecture(s): all (${ARCHS}), or a single one"
	@echo "  ARCHS=\"...\"            The architectures ARCH_CROSS=all loops over"
	@echo "  BUILD_TYPE=all         Build variant: debug, release, speed (the speed flavour), or all (${BUILDS})"
	@echo "  BUILDS=\"...\"           The build types BUILD_TYPE=all loops over"
	@echo "  LIBC_MALLOC=nano       The allocator inside libc.a: nano, tlsf or tiny (make clean after changing it)"
	@echo "  HOST_CC=<gcc>          The host GCC 14 or newer for x86_64 (found by tools/scripts/host-cc)"
	@echo "  CROSS_COMPILE=<prefix> Your own arm-none-eabi toolchain (GCC 14 or newer) instead of the download"
	@echo "  OUTPUT_ROOT=<path>     Where the build trees go (default build/)"
	@echo "  PREFIX=<path>          Where the installs go (default images/)"
	@echo "  V=1                    Verbose output (show full compiler commands)"
	@echo "  BENCH_UPDATE=1         With bench: record the results as the architecture's benchmark baseline"
	@echo "  PROJECT_ROOT=<path>    Override project root (required in worktrees)"
.PHONY: help

${V}.SILENT:
