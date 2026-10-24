GITHUB_ACCOUNT ?= sgstreet

# arm-none-eabi GCC 16.2 (red-rocket-computing/cross-dev release v0.3.0). The target is the
# versioned driver, so changing the version installs the new toolchain on the next build.
COMPILER_VERSION ?= v0.3.0
COMPILER_GCC_VERSION := 16.2.0
COMPILER_ARCHIVE := gcc-arm-none-eabi-$(subst .,-,${COMPILER_GCC_VERSION}).tar.xz
COMPILER_PATH := ${PROJECT_ROOT}/tmp
COMPILER_URL := https://github.com/red-rocket-computing/cross-dev/releases/download/${COMPILER_VERSION}/${COMPILER_ARCHIVE}
COMPILER_TARGET := ${PROJECT_ROOT}/local/bin/arm-none-eabi-gcc-${COMPILER_GCC_VERSION}

# The benchmark qemu (P3.01, PLAN D72): qemu-arm and qemu-x86_64 built with TCG plugin support,
# which the distribution packages leave out, and the instruction counting plugin. Only
# "make bench" needs them ("make setup-bench"); building qemu needs ninja, python3 and the
# glib development files. Installed after the compiler, whose install empties local/.
QEMU_VERSION := 8.2.2
QEMU_URL := https://download.qemu.org/qemu-${QEMU_VERSION}.tar.xz
QEMU_SOURCE := ${PROJECT_ROOT}/tmp/qemu-${QEMU_VERSION}
QEMU_ROOT := ${PROJECT_ROOT}/local/qemu-${QEMU_VERSION}
QEMU_TARGET := ${QEMU_ROOT}/bin/qemu-arm
BENCH_PLUGIN := ${QEMU_ROOT}/lib/bench-roi.so

# Arm Optimized Routines (Phase 4, PLAN D91, D92): fetched only to measure rtoskit-libc against it,
# built by tools/scripts/aor-build into build/<board>/aor/ and never into libc.a ("make setup-aor").
# A shallow fetch of the pinned commit; the commit ID is the content check.
AOR_URL := https://github.com/ARM-software/optimized-routines.git
AOR_REVISION := f67e7daa5457646fad9f75580263dba654c988aa
AOR_ROOT := ${PROJECT_ROOT}/local/aor-${AOR_REVISION}
AOR_TARGET := ${AOR_ROOT}/.fetched

SETUP_TARGETS := ${COMPILER_TARGET}
SETUP_PATHS := ${COMPILER_PATH}

# A compiler handed in from outside (a parent build passing CROSS_COMPILE, PLAN D105) is used as it is:
# nothing is downloaded, but it must exist and be GCC 14 or newer (D9)
ifneq ($(filter command line environment,$(origin CROSS_COMPILE)),)
ifeq ($(wildcard ${CROSS_COMPILE}gcc),)
$(error CROSS_COMPILE=${CROSS_COMPILE}: ${CROSS_COMPILE}gcc does not exist)
endif
COMPILER_MAJOR := $(firstword $(subst ., ,$(shell ${CROSS_COMPILE}gcc -dumpversion)))
ifneq ($(shell [ '${COMPILER_MAJOR}' -ge 14 ] 2>/dev/null && echo ok),ok)
$(error CROSS_COMPILE=${CROSS_COMPILE}: GCC ${COMPILER_MAJOR} is older than the GCC 14 the library needs (PLAN D9))
endif
SETUP_TARGETS := $(filter-out ${COMPILER_TARGET},${SETUP_TARGETS})
endif

include ${PROJECT_ROOT}/tools/makefiles/common.mk

${COMPILER_TARGET}:
	@echo "DOWNLOADING ${COMPILER_URL}"
	[ -d ${COMPILER_PATH} ] || mkdir -p ${COMPILER_PATH}
	cd ${COMPILER_PATH} && wget -nv ${COMPILER_URL}
	@echo "INSTALLING ${@}"
	$(RM) -r ${PROJECT_ROOT}/local
	mkdir -p ${PROJECT_ROOT}/local
	tar -C ${PROJECT_ROOT}/local --strip-components=1 -xf ${COMPILER_PATH}/${COMPILER_ARCHIVE}
	$(RM) -rf ${COMPILER_PATH}/${COMPILER_ARCHIVE}
	
${QEMU_TARGET}: | ${COMPILER_TARGET}
	@echo "BUILDING qemu ${QEMU_VERSION} with plugins"
	mkdir -p ${PROJECT_ROOT}/tmp
	cd ${PROJECT_ROOT}/tmp && wget -nv ${QEMU_URL}
	tar -C ${PROJECT_ROOT}/tmp -xf ${PROJECT_ROOT}/tmp/qemu-${QEMU_VERSION}.tar.xz
	cd ${QEMU_SOURCE} && ./configure --target-list=arm-linux-user,x86_64-linux-user --enable-plugins --disable-docs --disable-tools --disable-system > configure.log
	ninja -C ${QEMU_SOURCE}/build qemu-arm qemu-x86_64
	install -m 755 -D ${QEMU_SOURCE}/build/qemu-x86_64 ${QEMU_ROOT}/bin/qemu-x86_64
	install -m 644 -D ${QEMU_SOURCE}/include/qemu/qemu-plugin.h ${QEMU_ROOT}/include/qemu-plugin.h
	install -m 755 -D ${QEMU_SOURCE}/build/qemu-arm ${@}
	$(RM) -r ${QEMU_SOURCE} ${PROJECT_ROOT}/tmp/qemu-${QEMU_VERSION}.tar.xz

${BENCH_PLUGIN}: ${PROJECT_ROOT}/tools/qemu-plugins/bench-roi.c ${QEMU_TARGET}
	@echo "BUILDING ${@}"
	mkdir -p $(dir ${@})
	cc -std=gnu2x -O2 -Wall -Werror -shared -fPIC $(shell pkg-config --cflags glib-2.0) -I${QEMU_ROOT}/include -o ${@} ${<}

setup-bench: ${QEMU_TARGET} ${BENCH_PLUGIN}
	@:
.PHONY: setup-bench

${AOR_TARGET}: | ${COMPILER_TARGET}
	@echo "FETCHING ${AOR_URL} ${AOR_REVISION}"
	$(RM) -r ${AOR_ROOT}
	git init -q ${AOR_ROOT}
	git -C ${AOR_ROOT} fetch -q --depth 1 ${AOR_URL} ${AOR_REVISION}
	git -C ${AOR_ROOT} checkout -q FETCH_HEAD
	test "$$(git -C ${AOR_ROOT} rev-parse HEAD)" = ${AOR_REVISION}
	touch ${@}

setup-aor: ${AOR_TARGET}
	@:
.PHONY: setup-aor

setup-realclean:
	@echo "REMOVING ${SETUP_PATHS}"
	-$(RM) -rf ${SETUP_PATHS} ${PROJECT_ROOT}/local
.PHONY: setup-realclean

setup-tools: ${SETUP_TARGETS}
	@:
.PHONY: setup-tools

setup: setup-tools
	@:
.PHONY: setup

