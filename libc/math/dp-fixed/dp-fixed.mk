#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# dp-fixed.mk
#
# Created on: Nov 3, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# A layer whose files are all in its speed/ directory (PLAN D77): nothing to build here
ifeq ($(findstring ${BUILD_ROOT},${CURDIR}),)
include ${PROJECT_ROOT}/tools/makefiles/target.mk
else

include ${PROJECT_ROOT}/tools/makefiles/project.mk

endif
