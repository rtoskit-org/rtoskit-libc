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

# Arm EHABI unwinding (.ARM.exidx, libgcc's unwind-arm) only; x86_64's library is built without unwind tables (D134)
ifeq (${ARCH_FAMILY},arm)
SRC += ${SOURCE_DIR}/unwind.c
endif
