#
# Copyright (C) 2017 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# project.mk
#
# Created on: Mar 16, 2017
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

include ${PROJECT_ROOT}/tools/makefiles/common.mk

ifneq ($(wildcard ${SOURCE_DIR}/subdir.mk),)
include ${SOURCE_DIR}/subdir.mk
else
SRC += $(wildcard ${SOURCE_DIR}/*.cpp)
SRC += $(wildcard ${SOURCE_DIR}/*.c)
SRC += $(wildcard ${SOURCE_DIR}/*.S)
SRC += $(wildcard ${SOURCE_DIR}/*.s)
endif

OBJ := $(patsubst %.c,%.o,$(filter %.c,${SRC})) $(patsubst %.cpp,%.o,$(filter %.cpp,${SRC})) $(patsubst %.s,%.o,$(filter %.s,${SRC})) $(patsubst %.S,%.o,$(filter %.S,${SRC}))
OBJ := $(subst ${PROJECT_ROOT},${BUILD_ROOT},${OBJ})
# Each module records the objects its current SRC builds (PLAN D117), and a parent archives or links those,
# not every *.o left in a child's build directory: an object whose source a variant now shadows would stay.
# A child without the record (a tree built before it existed) falls back to its *.o files.
$(file >${CURDIR}/.objects,${OBJ})
module_objects = $(strip $(if $(wildcard $(1)/.objects),$(file <$(1)/.objects),$(wildcard $(1)/*.o)))
TARGET_OBJ := $(foreach dir, $(addprefix ${BUILD_ROOT}/, ${TARGET_OBJ_LIBS}), $(call module_objects,$(dir))) $(addprefix ${BUILD_ROOT}/, ${TARGET_OBJS}) ${EXTRA_OBJS}

#$(info SUBDIRS=${SUBDIRS})
#$(info SOURCE_DIR=${SOURCE_DIR})
#$(info BUILD_ROOT=${BUILD_ROOT})
#$(info SRC=${SRC})
#$(info OBJ=${OBJ})
#$(info TARGET_OBJS=${TARGET_OBJS})
#$(info TARGET_OBJ_LIBS=${TARGET_OBJ_LIBS})
#$(info TARGET_OBJ=${TARGET_OBJ})
#$(info EXTRA_DEPS=${EXTRA_DEPS})
#$(info EXTRA_ELF_DEPS=${EXTRA_ELF_DEPS})
#$(info EXTRA_TARGETS=${EXTRA_TARGETS})
#$(info TARGET=${TARGET})
#$(info ALL-TARGET=$(addprefix ${CURDIR}/,${TARGET}))
#$(info CURDIR=${CURDIR})
#$(info MAKECMDGOALS=${MAKECMDGOALS})
#$(info EXTRA_CLEAN=${EXTRA_CLEAN})

LISTING_STYLE ?= -adhlns


all: ${EXTRA_TARGETS} ${OBJ}

clean:
	@echo "CLEANING ${CURDIR} ${EXTRA_CLEAN}"
	${RM} ${CURDIR}/*.bin ${CURDIR}/*.elf ${CURDIR}/*.map ${CURDIR}/*.smap ${CURDIR}/*.dis ${OBJ} ${OBJ:%.o=%.d} ${OBJ:%.o=%.dis} ${OBJ:%.o=%.o.lst} ${CURDIR}/*.su ${CURDIR}/*.ci ${CURDIR}/*.a ${EXTRA_CLEAN}

distclean:

install:

${EXTRA_DEPS} ${EXTRA_LIB_DEPS} ${EXTRA_ELF_DEPS} ${TARGET_OBJ}:

${CURDIR}/%.a: ${OBJ} ${TARGET_OBJ} ${EXTRA_LIB_DEPS} ${EXTRA_DEPS}
	@echo "ARCHIVING $@"
	${RM} $@
	$(AR) ${ARFLAGS} $@ ${OBJ} ${TARGET_OBJ}

${CURDIR}/%.so: ${OBJ} ${TARGET_OBJ} ${EXTRA_LIB_DEPS} ${EXTRA_DEPS}
	@echo "LINKING $@"
	$(LD) --shared -Wl,-soname,$@ ${LOADLIBES} ${LDFLAGS} -Wl,--cref -Wl,-Map,"$(basename ${@}).map" -o $@ ${OBJ} ${LDLIBS}
	$(NM) -n $@ | grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)' > ${@:%.so=%.smap}

${CURDIR}/%.elf: ${OBJ} ${TARGET_OBJ} ${EXTRA_ELF_DEPS} ${EXTRA_DEPS}
	@echo "LINKING $@"
	$(LD) ${LDFLAGS} -Wl,--cref -Wl,-Map,"$(basename ${@}).map" ${LOADLIBES} -o $@ ${OBJ} ${TARGET_OBJ} ${LDLIBS}
	${TOOLS_ROOT}/scripts/trim-map -e $(basename ${@}).map
	$(OBJDUMP) -S $@ > ${@:%.elf=%.dis}
	$(NM) -n $@ | grep -v '\( [aNUw] \)\|\(__crc_\)\|\( \$[adt]\)' > ${@:%.elf=%.smap}
	@echo "SIZE $@"
	$(SIZE) -B $@

${CURDIR}/%.bin: ${CURDIR}/%.elf
	@echo "GENERATING $@"
#	$(OBJCOPY) --gap=0xff -O binary $< $@
	$(OBJCOPY) -O binary $< $@

${CURDIR}/%.o: ${SOURCE_DIR}/%.c ${EXTRA_OBJ_DEPS}
	@echo "COMPILING $<"
	$(CC) ${CPPFLAGS} ${CFLAGS} -Wa,${LISTING_STYLE}="$@.lst" -MMD -MP -c -o $@ $<

${CURDIR}/%.o: ${SOURCE_DIR}/%.cpp ${EXTRA_OBJ_DEPS}
	@echo "COMPILING $<"
	$(CXX) ${CPPFLAGS} ${CXXFLAGS} -Wa,${LISTING_STYLE}="$@.lst" -MMD -MP -c -o $@ $<

${CURDIR}/%.o: ${SOURCE_DIR}/%.s ${EXTRA_OBJ_DEPS}
	@echo "ASSEMBLING $<"
	$(AS) ${ASFLAGS} ${LISTING_STYLE}="$@.lst" -o $@ $<

${CURDIR}/%.o: ${SOURCE_DIR}/%.S ${EXTRA_OBJ_DEPS}
	@echo "ASSEMBLING $<"
	$(CC) ${CPPFLAGS} -x assembler-with-cpp ${ASFLAGS} -Wa,${LISTING_STYLE}="$@.lst" -c -o $@ $<

ifneq (${MAKECMDGOALS},clean)
-include ${OBJ:.o=.d}
endif

