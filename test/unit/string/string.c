/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * string.c
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * <string.h> (P1.03): every alignment and length around the word-at-a-time
 * paths, overlapping moves, the Two-Way strstr against a naive search, the
 * C23 additions and the const-preserving generic macros. Vectors from
 * docs/research/string-ctype-stdbit.md.
 */

static int failures;

static void fail(int line, const char *what)
{
	char message[256];

	snprintf(message, sizeof(message), "fail: line %d: %s\n", line, what);
	fputs(message, stdout);
	++failures;
}

#define CHECK(condition) ((condition) ? (void)0 : fail(__LINE__, #condition))

static unsigned char buffer[256];
static unsigned char other[256];

static void fill(unsigned char *p, size_t n, unsigned int seed)
{
	for (size_t i = 0; i < n; ++i)
		p[i] = (unsigned char)(seed + i * 7 + 1) | 1;
}

static void test_strlen_strchr_memchr(void)
{
	for (size_t align = 0; align < 16; ++align) {
		for (size_t length = 0; length < 64; ++length) {
			char *s = (char *)buffer + align;

			/* Bytes 0x80 and 0xff around the terminator stress the zero-byte test */
			memset(buffer, 0xff, sizeof(buffer));
			for (size_t i = 0; i < length; ++i)
				s[i] = (i & 1) ? (char)0x80 : (char)(0x41 + i % 26);
			s[length] = '\0';

			CHECK(strlen(s) == length);
			CHECK(strchr(s, '\0') == s + length);
			CHECK(memchr(s, '\0', length + 1) == s + length);
			CHECK(memchr(s, '\0', length) == nullptr);
			CHECK(strchr(s, 0x7f) == nullptr);

			for (size_t i = 0; i < length; ++i) {
				int c = (unsigned char)s[i];
				char *first = s;

				while (*first != (char)c)
					++first;
				CHECK(strchr(s, c) == first);
				CHECK(memchr(s, c, length) == first);
				/* The int is converted to unsigned char (C23 7.26.5.2p2) */
				CHECK(memchr(s, c + 256, length) == first);
				CHECK(strrchr(s, c) != nullptr && *strrchr(s, c) == (char)c);
			}
		}
	}

	CHECK(strrchr("abcabc", 'b') == (const char *)"abcabc" + 4 || strcmp(strrchr("abcabc", 'b'), "bc") == 0);
	CHECK(strrchr("abc", '\0') != nullptr && *strrchr("abc", '\0') == '\0');
	CHECK(strrchr("abc", 'x') == nullptr);
}

static void test_copies(void)
{
	for (size_t dst_align = 0; dst_align < 8; ++dst_align) {
		for (size_t src_align = 0; src_align < 8; ++src_align) {
			for (size_t n = 0; n < 48; ++n) {
				fill(other, sizeof(other), (unsigned int)(n + src_align));
				memset(buffer, 0, sizeof(buffer));
				CHECK(memcpy(buffer + dst_align, other + src_align, n) == buffer + dst_align);
				CHECK(memcmp(buffer + dst_align, other + src_align, n) == 0);
				CHECK(dst_align == 0 || buffer[dst_align - 1] == 0);
				CHECK(buffer[dst_align + n] == 0);
			}
		}
	}

	/* memmove: every overlap in both directions */
	for (size_t from = 0; from < 24; ++from) {
		for (size_t to = 0; to < 24; ++to) {
			for (size_t n = 0; n < 40; ++n) {
				unsigned char want[128];

				fill(buffer, 128, (unsigned int)n);
				memcpy(want, buffer, 128);
				for (size_t i = 0; i < n; ++i)
					want[to + i] = buffer[from + i];
				CHECK(memmove(buffer + to, buffer + from, n) == buffer + to);
				CHECK(memcmp(buffer, want, 128) == 0);
			}
		}
	}

	for (size_t align = 0; align < 8; ++align) {
		for (size_t n = 0; n < 48; ++n) {
			memset(buffer, 0x11, sizeof(buffer));
			CHECK(memset(buffer + align, 0x1a5, n) == buffer + align);
			for (size_t i = 0; i < 64; ++i)
				CHECK(buffer[i] == ((i >= align && i < align + n) ? 0xa5 : 0x11));
		}
	}

	memset(buffer, 0x11, 16);
	CHECK(memset_explicit(buffer + 1, 0, 8) == buffer + 1);
	CHECK(buffer[0] == 0x11 && buffer[1] == 0 && buffer[8] == 0 && buffer[9] == 0x11);
}

static void test_memccpy(void)
{
	char dst[16];

	memset(dst, 'x', sizeof(dst));
	CHECK(memccpy(dst, "hello", 'l', 5) == dst + 3);
	CHECK(memcmp(dst, "helx", 4) == 0);

	memset(dst, 'x', sizeof(dst));
	CHECK(memccpy(dst, "hello", 'z', 5) == nullptr);
	CHECK(memcmp(dst, "hellox", 6) == 0);

	/* c is converted to unsigned char; n bounds the copy */
	CHECK(memccpy(dst, "a\xff"
					   "b",
				  0x1ff, 3) == dst + 2);
	CHECK(memccpy(dst, "abc", 'c', 2) == nullptr);
	CHECK(memccpy(dst, "abc", 'a', 0) == nullptr);
}

static void test_string_copies(void)
{
	char dst[16];

	CHECK(strcpy(dst, "abc") == dst && strcmp(dst, "abc") == 0);
	CHECK(strcat(dst, "def") == dst && strcmp(dst, "abcdef") == 0);
	CHECK(strncat(dst, "ghijk", 2) == dst && strcmp(dst, "abcdefgh") == 0);
	CHECK(strncat(dst, "", 5) == dst && strcmp(dst, "abcdefgh") == 0);

	/* strncpy pads with zeros, and does not terminate when s2 is too long */
	memset(dst, 'x', sizeof(dst));
	CHECK(strncpy(dst, "ab", 5) == dst && memcmp(dst, "ab\0\0\0x", 6) == 0);
	memset(dst, 'x', sizeof(dst));
	CHECK(strncpy(dst, "abcdef", 3) == dst && memcmp(dst, "abcx", 4) == 0);

	/* An unterminated source array is fine within n */
	char unterminated[3] = { 'u', 'v', 'w' };

	memset(dst, 'x', sizeof(dst));
	strncpy(dst, unterminated, 3);
	CHECK(memcmp(dst, "uvwx", 4) == 0);
	strcpy(dst, "a");
	strncat(dst, unterminated, 3);
	CHECK(strcmp(dst, "auvw") == 0);

	memset(dst, 'x', sizeof(dst));
	CHECK(strxfrm(dst, "abc", 4) == 3 && strcmp(dst, "abc") == 0);
	memset(dst, 'x', sizeof(dst));
	CHECK(strxfrm(dst, "abc", 3) == 3 && dst[0] == 'x');
	CHECK(strxfrm(nullptr, "abc", 0) == 3);
}

static void test_compares(void)
{
	CHECK(memcmp("abc", "abd", 3) < 0);
	CHECK(memcmp("abd", "abc", 3) > 0);
	CHECK(memcmp("abc", "abd", 2) == 0);
	CHECK(memcmp("\x80", "\x7f", 1) > 0);
	CHECK(memcmp("a", "b", 0) == 0);
	CHECK(strcmp("abc", "abc") == 0);
	CHECK(strcmp("abc", "abcd") < 0);
	CHECK(strcmp("abcd", "abc") > 0);
	CHECK(strcmp("\xff", "a") > 0);
	CHECK(strcmp("", "") == 0);
	CHECK(strncmp("abcx", "abcy", 3) == 0);
	CHECK(strncmp("abcx", "abcy", 4) < 0);
	CHECK(strncmp("ab", "abc", 5) < 0);
	CHECK(strncmp("abc", "xyz", 0) == 0);
	CHECK(strcoll("abc", "abd") < 0);
}

static void test_spans(void)
{
	CHECK(strspn("aabbcd", "ab") == 4);
	CHECK(strspn("aabbcd", "") == 0);
	CHECK(strspn("", "ab") == 0);
	CHECK(strspn("\xff\xfe", "\xfe\xff") == 2);
	CHECK(strcspn("abcd", "dc") == 2);
	CHECK(strcspn("abcd", "d") == 3);
	CHECK(strcspn("abcd", "") == 4);
	CHECK(strcspn("abcd", "xyz") == 4);
	CHECK(strpbrk("abcd", "dc") != nullptr && *strpbrk("abcd", "dc") == 'c');
	CHECK(strpbrk("abcd", "xyz") == nullptr);
}

static void test_strtok(void)
{
	char text[] = ",,a,,bc;d,";
	char only[] = ";;;";

	CHECK(strcmp(strtok(text, ",;"), "a") == 0);
	CHECK(strcmp(strtok(nullptr, ",;"), "bc") == 0);
	CHECK(strcmp(strtok(nullptr, ","), "d") == 0);
	CHECK(strtok(nullptr, ",") == nullptr);
	CHECK(strtok(nullptr, ",") == nullptr);
	CHECK(strtok(only, ";") == nullptr);
}

static const char *naive_strstr(const char *h, const char *n)
{
	size_t m = strlen(n);

	while (true) {
		if (strncmp(h, n, m) == 0)
			return h;
		if (*h == '\0')
			return nullptr;
		++h;
	}
}

static void test_strstr(void)
{
	char haystack[16];
	char needle[12];

	CHECK(strstr("abc", "") != nullptr && strcmp(strstr("abc", ""), "abc") == 0);
	CHECK(strstr("abc", "abcd") == nullptr);
	CHECK(strstr("", "a") == nullptr);
	CHECK(strstr("aaaaaab", "aaab") != nullptr && strcmp(strstr("aaaaaab", "aaab"), "aaab") == 0);
	CHECK(strstr("abababc", "ababc") != nullptr && strcmp(strstr("abababc", "ababc"), "ababc") == 0);

	/* Every haystack up to 12 letters and needle up to 6 over {a, b} */
	for (unsigned int hl = 0; hl <= 12; ++hl) {
		for (unsigned int hbits = 0; hbits < (1U << hl); hbits += hl > 10 ? 3 : 1) {
			for (unsigned int i = 0; i < hl; ++i)
				haystack[i] = (hbits >> i) & 1 ? 'b' : 'a';
			haystack[hl] = '\0';
			for (unsigned int nl = 1; nl <= 6; ++nl) {
				for (unsigned int nbits = 0; nbits < (1U << nl); ++nbits) {
					for (unsigned int i = 0; i < nl; ++i)
						needle[i] = (nbits >> i) & 1 ? 'b' : 'a';
					needle[nl] = '\0';
					if (strstr(haystack, needle) != naive_strstr(haystack, needle)) {
						fail(__LINE__, "strstr differs from the naive search");
						return;
					}
				}
			}
		}
	}

	/* Longer, periodic and three-letter needles on a pseudo-random haystack */
	static char big[2048];
	unsigned int state = 12345;

	for (size_t i = 0; i < sizeof(big) - 1; ++i) {
		state = state * 1103515245 + 12345;
		big[i] = "abc"[(state >> 16) % 3];
	}
	for (size_t start = 0; start + 40 < sizeof(big); start += 37) {
		for (size_t len = 2; len < 40; len += 3) {
			char probe[48];

			memcpy(probe, big + start, len);
			probe[len] = '\0';
			CHECK(strstr(big, probe) == naive_strstr(big, probe));
			probe[len - 1] = 'd';
			CHECK(strstr(big, probe) == nullptr);
		}
	}
}

static void test_generic(void)
{
	const char *cs = "abc";
	char s[] = "abc";
	const void *cv = cs;

	/* C23 7.26.5.1: const in, const out */
	static_assert(_Generic(strchr(cs, 'a'), const char *: 1, default: 0));
	static_assert(_Generic(strchr(s, 'a'), char *: 1, default: 0));
	static_assert(_Generic(strrchr(cs, 'a'), const char *: 1, default: 0));
	static_assert(_Generic(strpbrk(cs, "a"), const char *: 1, default: 0));
	static_assert(_Generic(strstr(cs, "a"), const char *: 1, default: 0));
	static_assert(_Generic(strstr(s, "a"), char *: 1, default: 0));
	static_assert(_Generic(memchr(cv, 'a', 3), const void *: 1, default: 0));
	static_assert(_Generic(memchr(s, 'a', 3), void *: 1, default: 0));

	/* The function itself stays addressable */
	char *(*function)(const char *, int) = strchr;

	CHECK(function(s, 'b') == s + 1);
}

static void test_misc(void)
{
	alignas(16) static char aligned[32];

	CHECK(strcmp(strerror(0), "No error") == 0);
	CHECK(strcmp(strerror(EDOM), "Numerical argument out of domain") == 0);
	CHECK(strcmp(strerror(ERANGE), "Numerical result out of range") == 0);
	CHECK(strcmp(strerror(-1), "Unknown error") == 0);
	CHECK(strerror(EILSEQ) != nullptr);
	CHECK(strcmp(strerror(256), "Unknown error") == 0 && strcmp(strerror(INT_MIN), "Unknown error") == 0);

	/* Every number <errno.h> defines, with its exact message (the table is compacted in P2.04, PLAN D54) */
	static const struct {
		int number;
		const char *text;
	} messages[] = {
		{ EPERM, "Operation not permitted" },
		{ ENOENT, "No such file or directory" },
		{ ESRCH, "No such process" },
		{ EINTR, "Interrupted system call" },
		{ EIO, "Input/output error" },
		{ ENXIO, "No such device or address" },
		{ E2BIG, "Argument list too long" },
		{ ENOEXEC, "Exec format error" },
		{ EBADF, "Bad file descriptor" },
		{ ECHILD, "No child processes" },
		{ EAGAIN, "Resource temporarily unavailable" },
		{ ENOMEM, "Cannot allocate memory" },
		{ EACCES, "Permission denied" },
		{ EFAULT, "Bad address" },
		{ EBUSY, "Device or resource busy" },
		{ EEXIST, "File exists" },
		{ EXDEV, "Invalid cross-device link" },
		{ ENODEV, "No such device" },
		{ ENOTDIR, "Not a directory" },
		{ EISDIR, "Is a directory" },
		{ EINVAL, "Invalid argument" },
		{ ENFILE, "Too many open files in system" },
		{ EMFILE, "Too many open files" },
		{ ENOTTY, "Inappropriate ioctl for device" },
		{ ETXTBSY, "Text file busy" },
		{ EFBIG, "File too large" },
		{ ENOSPC, "No space left on device" },
		{ ESPIPE, "Illegal seek" },
		{ EROFS, "Read-only file system" },
		{ EMLINK, "Too many links" },
		{ EPIPE, "Broken pipe" },
		{ EDOM, "Numerical argument out of domain" },
		{ ERANGE, "Numerical result out of range" },
		{ EDEADLK, "Resource deadlock avoided" },
		{ ENAMETOOLONG, "File name too long" },
		{ ENOLCK, "No locks available" },
		{ ENOSYS, "Function not implemented" },
		{ ENOTEMPTY, "Directory not empty" },
		{ ELOOP, "Too many levels of symbolic links" },
		{ ENOMSG, "No message of desired type" },
		{ EIDRM, "Identifier removed" },
		{ ENOSTR, "Device not a stream" },
		{ ENODATA, "No data available" },
		{ ETIME, "Timer expired" },
		{ ENOSR, "Out of streams resources" },
		{ ENOLINK, "Link has been severed" },
		{ EPROTO, "Protocol error" },
		{ EMULTIHOP, "Multihop attempted" },
		{ EBADMSG, "Bad message" },
		{ EOVERFLOW, "Value too large for defined data type" },
		{ EILSEQ, "Invalid or incomplete multibyte or wide character" },
		{ ENOTSOCK, "Socket operation on non-socket" },
		{ EDESTADDRREQ, "Destination address required" },
		{ EMSGSIZE, "Message too long" },
		{ EPROTOTYPE, "Protocol wrong type for socket" },
		{ ENOPROTOOPT, "Protocol not available" },
		{ EPROTONOSUPPORT, "Protocol not supported" },
		{ EOPNOTSUPP, "Operation not supported" },
		{ EAFNOSUPPORT, "Address family not supported by protocol" },
		{ EADDRINUSE, "Address already in use" },
		{ EADDRNOTAVAIL, "Cannot assign requested address" },
		{ ENETDOWN, "Network is down" },
		{ ENETUNREACH, "Network is unreachable" },
		{ ENETRESET, "Network dropped connection on reset" },
		{ ECONNABORTED, "Software caused connection abort" },
		{ ECONNRESET, "Connection reset by peer" },
		{ ENOBUFS, "No buffer space available" },
		{ EISCONN, "Transport endpoint is already connected" },
		{ ENOTCONN, "Transport endpoint is not connected" },
		{ ETIMEDOUT, "Connection timed out" },
		{ ECONNREFUSED, "Connection refused" },
		{ EHOSTUNREACH, "No route to host" },
		{ EALREADY, "Operation already in progress" },
		{ EINPROGRESS, "Operation now in progress" },
		{ ESTALE, "Stale file handle" },
		{ EDQUOT, "Disk quota exceeded" },
		{ ECANCELED, "Operation canceled" },
		{ EOWNERDEAD, "Owner died" },
		{ ENOTRECOVERABLE, "State not recoverable" },
		/* Beyond POSIX (PLAN D128) */
		{ ENOTUNIQ, "Name not unique on network" },
		{ EBADFD, "File descriptor in bad state" },
		{ ESOCKTNOSUPPORT, "Socket type not supported" },
		{ ESHUTDOWN, "Cannot send after transport endpoint shutdown" },
		{ EHOSTDOWN, "Host is down" },
		{ EFTYPE, "Inappropriate file type or format" },
	};

	for (size_t i = 0; i < sizeof(messages) / sizeof(messages[0]); ++i)
		CHECK(strcmp(strerror(messages[i].number), messages[i].text) == 0);

	CHECK(memalignment(nullptr) == 0);
	CHECK(memalignment(aligned) >= 16);
	CHECK(memalignment(aligned + 1) == 1);
	CHECK(memalignment(aligned + 2) == 2);
	CHECK(memalignment(aligned + 4) == 4);
	CHECK(memalignment(aligned + 12) == 4);
	CHECK(memalignment((void *)0x1000) == 0x1000);
}

int main(void)
{
	test_strlen_strchr_memchr();
	test_copies();
	test_memccpy();
	test_string_copies();
	test_compares();
	test_spans();
	test_strtok();
	test_strstr();
	test_generic();
	test_misc();

	if (failures == 0)
		puts("pass: string");

	return failures == 0 ? 0 : 1;
}
