#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Oct 25, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The helpers exist on the Arm cores only (PLAN D109)
SRC := ${SOURCE_DIR}/softsp.c ${SOURCE_DIR}/$(if $(filter arm,${ARCH_FAMILY}),operations.c,none.c)
