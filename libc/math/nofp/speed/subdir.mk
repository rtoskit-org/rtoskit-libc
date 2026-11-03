#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Nov 3, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The out-of-line 16-bit product helper only where fixed-wide.h declares it (Armv6-M); armv7-m multiplies inline (P9.01)
SRC := $(filter-out $(if $(filter armv6-m armv6s-m,${ARCH_CROSS}),,${SOURCE_DIR}/__libc_fixed_wide.c),$(wildcard ${SOURCE_DIR}/*.c))
