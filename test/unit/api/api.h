/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * api.h
 *
 * Created on: Oct 30, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _API_H
#define _API_H

/* How often counted() ran: api.c's assert calls it with NDEBUG undefined, ndebug.c's with NDEBUG defined */
extern int assert_count;
bool counted(void);
void ndebug_assert(void);

#endif /* _API_H */
