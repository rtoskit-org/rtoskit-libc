/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * float_accuracy.h
 *
 * Created on: Nov 10, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _FLOAT_ACCURACY_H
#define _FLOAT_ACCURACY_H

/*
 * The accuracy the build type promises a float function (expect.c in debug, release and speed, fast/expect.c in
 * fast, D162, D164, selected by subdir.mk): the error below which one result of name passes, against exact, and
 * whether a function's worst error and its count of results not correctly rounded pass
 */
double float_accuracy_tolerance(const char *name, double exact);
bool float_accuracy_within(const char *name, double worst, long misrounded, long count);

#endif /* _FLOAT_ACCURACY_H */
