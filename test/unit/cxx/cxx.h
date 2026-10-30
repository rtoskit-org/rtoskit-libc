/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * cxx.h
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _CXX_H
#define _CXX_H

/* The C++ ABI hooks the test calls directly (libstdc++'s <cxxabi.h> declares them) */
extern "C" {
int __cxa_atexit(void (*func)(void *), void *arg, void *dso);
void __cxa_finalize(void *dso);
}

int headers17(void);

#endif /* _CXX_H */
