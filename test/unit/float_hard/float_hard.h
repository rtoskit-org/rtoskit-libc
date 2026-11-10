/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * float_hard.h
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FLOAT_HARD_H
#define _FLOAT_HARD_H

/*
 * Whether got is acceptable for a hard case of name: want is the correctly rounded result in the mode (nearest or
 * a directed one), result the one to nearest and direction the side of it on which the exact value lies. expect.c
 * (debug, release, speed) wants the bits of want; fast/expect.c the fast contract (D162, D163), selected by subdir.mk.
 */
bool float_hard_accept(const char *name, bool nearest, float got, float want, float result, int direction);

#endif /* _FLOAT_HARD_H */
