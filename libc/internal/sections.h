/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * sections.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_SECTIONS_H
#define _INTERNAL_SECTIONS_H

/* Constructor tables delimited by the linker (provided by the default GNU ld and lld scripts) */
extern void (*const __init_array_start[])(void);
extern void (*const __init_array_end[])(void);

/* Destructor table, run in reverse by exit */
extern void (*const __fini_array_start[])(void);
extern void (*const __fini_array_end[])(void);

#endif /* _INTERNAL_SECTIONS_H */
