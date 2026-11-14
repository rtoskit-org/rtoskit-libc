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
SRC := $(if $(filter %/expf.c %/exp2f.c %/exp10f.c %/expm1f.c %/exp2m1f.c %/exp10m1f.c %/powf.c %/sinhf.c %/coshf.c %/tanhf.c %/erff.c %/erfcf.c,${SRC}),${SRC},$(filter-out %/__libc_fast_exp2_table.c,${SRC}))
# The float logarithms' 16-step table only where logf, log2f, log10f or powf of this directory stays (P11.06b)
SRC := $(if $(filter %/logf.c %/log2f.c %/log10f.c %/powf.c,${SRC}),${SRC},$(filter-out %/__libc_fast_logf_table.c,${SRC}))
# The atan(j/8) table only where an inverse trigonometric function of this directory stays
SRC := $(if $(filter %/atanf.c %/atan2f.c %/asinf.c %/acosf.c %/atanpif.c %/atan2pif.c %/asinpif.c %/acospif.c,${SRC}),${SRC},$(filter-out %/__libc_fast_atan_table.c,${SRC}))
# The erfc table only where erff or erfcf of this directory stays
SRC := $(if $(filter %/erff.c %/erfcf.c,${SRC}),${SRC},$(filter-out %/__libc_fast_erfc_table.c,${SRC}))
SRC := $(if $(filter %/exp.c %/exp2.c %/exp10.c %/expm1.c %/pow.c %/sinh.c %/cosh.c %/tanh.c,${SRC}),${SRC},$(filter-out %/__libc_fast_exp_pairs.c,${SRC}))
# The log tables only where a logarithm, pow or an inverse hyperbolic function of this directory stays (the special values
# stay: dp-fixed/fast calls them too)
SRC := $(if $(filter %/log.c %/log10.c %/log1p.c %/pow.c %/asinh.c %/acosh.c %/atanh.c,${SRC}),${SRC},$(filter-out %/__libc_fast_log_table.c,${SRC}))
SRC := $(if $(filter %/log2.c,${SRC}),${SRC},$(filter-out %/__libc_fast_log2_table.c,${SRC}))
# The trigonometric table only where a double sin, cos, sincos or tan of this directory stays, the slow path also where a
# float one does
SRC := $(if $(filter %/sin.c %/cos.c %/sincos.c %/tan.c,${SRC}),${SRC},$(filter-out %/__libc_fast_sin_table.c,${SRC}))
SRC := $(if $(filter %/sin.c %/cos.c %/sincos.c %/tan.c %/sinf.c %/cosf.c %/sincosf.c %/tanf.c,${SRC}),${SRC},$(filter-out %/__libc_fast_trig_slow.c,${SRC}))
# The atan(j/16) pairs only where a double atan or atan2 of this directory stays (the atan2 slow path stays: dp-fixed/fast
# calls it too)
SRC := $(if $(filter %/atan.c %/atan2.c,${SRC}),${SRC},$(filter-out %/__libc_fast_atan_pairs.c,${SRC}))
