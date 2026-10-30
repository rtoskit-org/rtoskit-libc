/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * __dso_handle.c
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include "internal/cxa.h"

/*
 * Itanium C++ ABI 3.3.5.1: the compiler passes &__dso_handle to __cxa_atexit
 * to name the image a static object belongs to. A statically linked program
 * is one image. GCC's crtbegin.o defines it too; this member is linked only
 * when nothing else does (PLAN D121).
 */
void *__dso_handle = &__dso_handle;
