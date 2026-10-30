#
# Copyright (C) 2026 Red Rocket Computing, LLC
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# port-linux-arm.mk
#
# Created on: Oct 24, 2026
#     Author: Stephen Street (stephen@redrocketcomputing.com)
#

# The Linux test port on the Arm cores (PLAN D106): the programs the build links (unit tests,
# examples, size and benchmark programs) are Linux processes run under qemu-user

# qemu-arm runs the M-profile code on its A-profile "max" CPU; see docs/research/arm-bring-up.md
export TARGET_RUNNER := qemu-arm -cpu max

# Room for the ELF and program headers in the first loaded segment, so the port finds PT_TLS
LDFLAGS += -Wl,-T,${PROJECT_ROOT}/port/linux/arm/headers.ld
