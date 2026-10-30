/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * stdnoreturn.h
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _STDNORETURN_H
#define _STDNORETURN_H

/* C++ has [[noreturn]] and no noreturn macro */
#ifndef __cplusplus
/* C23 7.25: obsolescent, prefer the [[noreturn]] attribute */
#define noreturn _Noreturn

#endif /* __cplusplus */

#endif /* _STDNORETURN_H */
