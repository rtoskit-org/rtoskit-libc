#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# libc-test.mk
#
# Created on: Oct 18, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# Builds one executable per libc-test test of suite ${LIBC_TEST_SUITE} whose
# status in test/libc-test/manifest.txt is "run". Included by the suite modules
# under test/libc-test after project.mk. ${LIBC_TEST_EXTRA} names extra sources
# of the suite linked into every test (api/main.c); ${LIBC_TEST_COMMON} names
# sources of libc-test's src/common linked the same way (math/mtest.c);
# ${LIBC_TEST_INCLUDE} names shim headers forced into every test of the suite.

LIBC_TEST_ROOT := ${PROJECT_ROOT}/third-party/libc-test
LIBC_TEST_MANIFEST := ${PROJECT_ROOT}/test/libc-test/manifest.txt
LIBC_TEST_SOURCE := ${LIBC_TEST_ROOT}/src/${LIBC_TEST_SUITE}

LIBC_TEST_NAMES := $(shell awk -v suite=${LIBC_TEST_SUITE}/ '$$1 !~ /^#/ && $$2 == "run" && index($$1, suite) == 1 { print substr($$1, length(suite) + 1) }' ${LIBC_TEST_MANIFEST})
LIBC_TEST_OBJ := $(addprefix ${CURDIR}/,$(addsuffix .o,${LIBC_TEST_NAMES}))
LIBC_TEST_EXTRA_OBJ := $(addprefix ${CURDIR}/,$(addsuffix .o,${LIBC_TEST_EXTRA})) $(addprefix ${CURDIR}/,$(addsuffix .o,${LIBC_TEST_COMMON}))
LIBC_TEST_ELF := $(LIBC_TEST_OBJ:.o=.elf)
LIBC_TEST_SHIM_OBJ := $(patsubst ${PROJECT_ROOT}/%.c,${BUILD_ROOT}/%.o,$(wildcard ${PROJECT_ROOT}/test/libc-test/shim/*.c))
LIBC_TEST_PORT_OBJ := $(foreach dir,port/${RTOS_PORT} port/${RTOS_PORT}/${ARCH_FAMILY},$(call module_objects,${BUILD_ROOT}/${dir}))

# libc-test's own warning set (config.mak.def), without -Werror for the test code; the api tests add theirs
# M_PI is an XSI extension functional/snprintf takes from <math.h> under _XOPEN_SOURCE; ISO C's <math.h> has no such name
LIBC_TEST_CPPFLAGS := ${CPPFLAGS} -I${PROJECT_ROOT}/test/libc-test/shim/include -DM_PI=3.14159265358979323846
# Headers a suite forces into every test (${LIBC_TEST_INCLUDE}, found in the shim), e.g. math's signgam.h
LIBC_TEST_CPPFLAGS += $(addprefix -include ,${LIBC_TEST_INCLUDE})
ifneq (${LIBC_TEST_COMMON},)
# After the shim, so its test.h replaces src/common's; mtest.h is found here
LIBC_TEST_CPPFLAGS += -I${LIBC_TEST_ROOT}/src/common
endif
LIBC_TEST_CFLAGS := $(filter-out -Werror -Wmissing-declarations -fstack-usage -fcallgraph-info=su,${CFLAGS}) -fno-builtin -frounding-math -Wno-unused-function -Wno-missing-braces -Wno-unused -Wno-overflow -Wno-unknown-pragmas -Werror=implicit-function-declaration -Werror=implicit-int -Werror=pointer-sign -Werror=pointer-arith
ifeq (${LIBC_TEST_SUITE},api)
LIBC_TEST_CFLAGS += -pedantic-errors -Werror -Wno-unused
endif

EXTRA_CLEAN += ${LIBC_TEST_OBJ} ${LIBC_TEST_EXTRA_OBJ} $(LIBC_TEST_OBJ:.o=.d) $(LIBC_TEST_EXTRA_OBJ:.o=.d) $(LIBC_TEST_OBJ:.o=.o.lst) $(LIBC_TEST_EXTRA_OBJ:.o=.o.lst)

# A link that fails must not leave an empty executable that looks up to date
.DELETE_ON_ERROR:

all: ${LIBC_TEST_ELF}

${LIBC_TEST_OBJ} $(addprefix ${CURDIR}/,$(addsuffix .o,${LIBC_TEST_EXTRA})): ${CURDIR}/%.o: ${LIBC_TEST_SOURCE}/%.c
	@echo "COMPILING $<"
	$(CC) ${LIBC_TEST_CPPFLAGS} ${LIBC_TEST_CFLAGS} -Wa,${LISTING_STYLE}="$@.lst" -MMD -MP -c -o $@ $<

$(addprefix ${CURDIR}/,$(addsuffix .o,${LIBC_TEST_COMMON})): ${CURDIR}/%.o: ${LIBC_TEST_ROOT}/src/common/%.c
	@echo "COMPILING $<"
	$(CC) ${LIBC_TEST_CPPFLAGS} ${LIBC_TEST_CFLAGS} -Wa,${LISTING_STYLE}="$@.lst" -MMD -MP -c -o $@ $<

${LIBC_TEST_ELF}: ${CURDIR}/%.elf: ${CURDIR}/%.o ${LIBC_TEST_EXTRA_OBJ} ${LIBC_TEST_SHIM_OBJ} ${LIBC_TEST_PORT_OBJ} ${EXTRA_ELF_DEPS}
	@echo "LINKING $@"
	$(LD) ${LDFLAGS} -Wl,-Map,"$(basename ${@}).map" ${LOADLIBES} -o $@ $< ${LIBC_TEST_EXTRA_OBJ} ${LIBC_TEST_SHIM_OBJ} ${LIBC_TEST_PORT_OBJ} ${LDLIBS}

ifneq (${MAKECMDGOALS},clean)
-include $(LIBC_TEST_OBJ:.o=.d) $(LIBC_TEST_EXTRA_OBJ:.o=.d)
endif
