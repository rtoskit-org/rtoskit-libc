/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * scan.h
 *
 * Created on: Oct 19, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#ifndef _INTERNAL_SCAN_H
#define _INTERNAL_SCAN_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "internal/file.h"
#include "internal/length.h"

/*
 * The input of the scanf engine (docs/research/scanf.md): a string, or a
 * stream whose lock the caller holds. One character can be pushed back. For
 * a stream it goes back into the read window it came from, so the stream's
 * own pushback slot stays free for the caller's ungetc (test/unit/conformance);
 * only a character that came from that slot returns there.
 */
struct __libc_scan_input {
	FILE *stream;
	const unsigned char *string;
	size_t count;
	bool from_slot;
};

static inline int __libc_scan_get(struct __libc_scan_input *in)
{
	int c;

	if (in->stream != nullptr) {
		in->from_slot = in->stream->flags & __LIBC_FILE_UNGOT;
		c = __libc_file_getc(in->stream);
	} else {
		c = *in->string;
		if (c == '\0')
			return EOF;
		++in->string;
	}
	if (c != EOF)
		++in->count;

	return c;
}

static inline void __libc_scan_unget(struct __libc_scan_input *in, int c)
{
	if (c == EOF)
		return;

	--in->count;
	if (in->stream != nullptr) {
		if (in->from_slot || !(in->stream->flags & __LIBC_FILE_READING) || in->stream->pos == in->stream->buffer) {
			in->stream->ungot = c;
			in->stream->flags |= __LIBC_FILE_UNGOT;
		} else {
			--in->stream->pos;
		}
	} else {
		--in->string;
	}
}

/*
 * Stores an integer conversion's value through the pointer type the length
 * modifier names; out-of-range values wrap as the conversion does. The
 * minimal flavour replaces it and __libc_scan_count, refusing what it leaves
 * out with errno EINVAL (libc-minimal-printf.a, PLAN D53).
 */
bool __libc_scan_store(va_list *ap, enum __libc_length length, uintmax_t value);

/* n (C23 7.23.6.2p12): stores the count of characters read */
bool __libc_scan_count(va_list *ap, enum __libc_length length, size_t count);

/* C23 7.23.6.2: the number of items assigned, or EOF for an input failure before the first conversion */
int __libc_scan(struct __libc_scan_input *in, const char *format, va_list ap);

/*
 * One a/A/e/E/f/F/g/G conversion of at most width characters. length is 0
 * (float), 'l' (double) or 'L' (long double); target null means suppressed.
 * Returns 1 for a conversion, 0 for a matching failure and EOF for an input
 * failure. A separate symbol so a no-float flavour can replace it (PLAN D13).
 */
int __libc_scan_float(struct __libc_scan_input *in, size_t width, char length, void *target);

#endif /* _INTERNAL_SCAN_H */
