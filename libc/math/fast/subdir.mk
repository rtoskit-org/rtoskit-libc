#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Nov 10, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The generic fast files, less those any layer of the core provides, plain, in speed/ or in fast/ (PLAN D162, D168:
# a layer's file is the core's own, and beats a generic fast one)
LIBC_MATH_FAST_LAYERED := $(call libc_math_layer_stems,${LIBC_MATH_LAYERS})
SRC := $(filter-out $(foreach stem,${LIBC_MATH_FAST_LAYERED},${SOURCE_DIR}/${stem}.c ${SOURCE_DIR}/${stem}.S),$(wildcard ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.S))
# The 2^(j/32) table only where a user of __libc_fast_exp2 in this directory stays
SRC := $(if $(filter %/expf.c %/exp2f.c %/exp10f.c %/expm1f.c %/exp2m1f.c %/exp10m1f.c %/powf.c %/sinhf.c %/coshf.c %/tanhf.c,${SRC}),${SRC},$(filter-out %/__libc_fast_exp2_table.c,${SRC}))
# The atan(j/8) table only where an inverse trigonometric function of this directory stays
SRC := $(if $(filter %/atanf.c %/atan2f.c %/asinf.c %/acosf.c %/atanpif.c %/atan2pif.c %/asinpif.c %/acospif.c,${SRC}),${SRC},$(filter-out %/__libc_fast_atan_table.c,${SRC}))
