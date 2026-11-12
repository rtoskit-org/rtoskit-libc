#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# subdir.mk
#
# Created on: Nov 12, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The layer's fast files (PLAN D168, D188); dp-fixed is the last layer of every core that has it, so nothing later shadows them
SRC := $(wildcard ${SOURCE_DIR}/*.c ${SOURCE_DIR}/*.S)
