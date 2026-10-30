#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 30, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# C++ is tested on the Arm architectures only; the x86_64 development build has no C++ runtime to link (PLAN D121)
ifeq (${ARCH_FAMILY},arm)
SRC += ${SOURCE_DIR}/cxx.cpp ${SOURCE_DIR}/headers17.cpp
endif
