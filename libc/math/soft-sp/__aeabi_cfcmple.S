/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __aeabi_cfcmple.S
 *
 * Created on: Oct 25, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

/*
 * The ordering of a and b in the flags (RTABI 4.1.2; PLAN D109): Z for
 * equal, C clear for less, C set and Z clear for unordered; r0-r3 kept.
 * The compare core changes only ip and the flags, so this is a tail call.
 */
	.syntax unified
	.thumb
	.section .text.__aeabi_cfcmple,"ax",%progbits
	.global __aeabi_cfcmple
	.type __aeabi_cfcmple, %function
__aeabi_cfcmple:
	b __libc_softfloat_compare
	.size __aeabi_cfcmple, . - __aeabi_cfcmple

	.section .note.GNU-stack, "", %progbits
