#
# Copyright (C) 2017 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# commom.mk
#
# Created on: Mar 16, 2017
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The architecture fixes the toolchain, the flags and the library variants (PLAN D24, D106)
-include ${PROJECT_ROOT}/tools/makefiles/${ARCH_CROSS}.mk

# Programs the build links (tests, examples, size and benchmark programs) bind to the Linux test port;
# libc.a and every port archive are built regardless (PLAN D106)
export RTOS_PORT := linux
-include ${PROJECT_ROOT}/tools/makefiles/port-${RTOS_PORT}-${ARCH_FAMILY}.mk

# The allocator built into libc.a (PLAN D32, D54)
export LIBC_MALLOC ?= nano

-include ${PROJECT_ROOT}/tools/makefiles/${CHIP_FAMILY}.mk
-include ${PROJECT_ROOT}/tools/makefiles/${CHIP_TYPE}.mk
-include ${PROJECT_ROOT}/tools/makefiles/${BUILD_TYPE}.mk

