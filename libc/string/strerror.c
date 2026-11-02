/*
 * Copyright (C) 2026 Red Rocket Computing, LLC
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * strerror.c
 *
 * Created on: Oct 18, 2026
 *     Author: Stephen Street (stephen@redrocketcomputing.com)
 */

#include <errno.h>
#include <string.h>

/*
 * C23 7.26.6.3. The messages for the sparse Linux numbering (D20) are one
 * array of records: the error number plus one, the message and its null
 * character; the array's own terminator ends it. A byte per message is all
 * the index costs, where a switch compiled to a table of pointers (PLAN D54).
 * Every number <errno.h> defines has a message.
 */
static const char messages[] =
	"\001"
	"No error\0" /* 0 */
	"\002"
	"Operation not permitted\0" /* EPERM */
	"\003"
	"No such file or directory\0" /* ENOENT */
	"\004"
	"No such process\0" /* ESRCH */
	"\005"
	"Interrupted system call\0" /* EINTR */
	"\006"
	"Input/output error\0" /* EIO */
	"\007"
	"No such device or address\0" /* ENXIO */
	"\010"
	"Argument list too long\0" /* E2BIG */
	"\011"
	"Exec format error\0" /* ENOEXEC */
	"\012"
	"Bad file descriptor\0" /* EBADF */
	"\013"
	"No child processes\0" /* ECHILD */
	"\014"
	"Resource temporarily unavailable\0" /* EAGAIN */
	"\015"
	"Cannot allocate memory\0" /* ENOMEM */
	"\016"
	"Permission denied\0" /* EACCES */
	"\017"
	"Bad address\0" /* EFAULT */
	"\021"
	"Device or resource busy\0" /* EBUSY */
	"\022"
	"File exists\0" /* EEXIST */
	"\023"
	"Invalid cross-device link\0" /* EXDEV */
	"\024"
	"No such device\0" /* ENODEV */
	"\025"
	"Not a directory\0" /* ENOTDIR */
	"\026"
	"Is a directory\0" /* EISDIR */
	"\027"
	"Invalid argument\0" /* EINVAL */
	"\030"
	"Too many open files in system\0" /* ENFILE */
	"\031"
	"Too many open files\0" /* EMFILE */
	"\032"
	"Inappropriate ioctl for device\0" /* ENOTTY */
	"\033"
	"Text file busy\0" /* ETXTBSY */
	"\034"
	"File too large\0" /* EFBIG */
	"\035"
	"No space left on device\0" /* ENOSPC */
	"\036"
	"Illegal seek\0" /* ESPIPE */
	"\037"
	"Read-only file system\0" /* EROFS */
	"\040"
	"Too many links\0" /* EMLINK */
	"\041"
	"Broken pipe\0" /* EPIPE */
	"\042"
	"Numerical argument out of domain\0" /* EDOM */
	"\043"
	"Numerical result out of range\0" /* ERANGE */
	"\044"
	"Resource deadlock avoided\0" /* EDEADLK */
	"\045"
	"File name too long\0" /* ENAMETOOLONG */
	"\046"
	"No locks available\0" /* ENOLCK */
	"\047"
	"Function not implemented\0" /* ENOSYS */
	"\050"
	"Directory not empty\0" /* ENOTEMPTY */
	"\051"
	"Too many levels of symbolic links\0" /* ELOOP */
	"\053"
	"No message of desired type\0" /* ENOMSG */
	"\054"
	"Identifier removed\0" /* EIDRM */
	"\075"
	"Device not a stream\0" /* ENOSTR */
	"\076"
	"No data available\0" /* ENODATA */
	"\077"
	"Timer expired\0" /* ETIME */
	"\100"
	"Out of streams resources\0" /* ENOSR */
	"\104"
	"Link has been severed\0" /* ENOLINK */
	"\110"
	"Protocol error\0" /* EPROTO */
	"\111"
	"Multihop attempted\0" /* EMULTIHOP */
	"\113"
	"Bad message\0" /* EBADMSG */
	"\114"
	"Value too large for defined data type\0" /* EOVERFLOW */
	"\115"
	"Name not unique on network\0" /* ENOTUNIQ */
	"\116"
	"File descriptor in bad state\0" /* EBADFD */
	"\125"
	"Invalid or incomplete multibyte or wide character\0" /* EILSEQ */
	"\131"
	"Socket operation on non-socket\0" /* ENOTSOCK */
	"\132"
	"Destination address required\0" /* EDESTADDRREQ */
	"\133"
	"Message too long\0" /* EMSGSIZE */
	"\134"
	"Protocol wrong type for socket\0" /* EPROTOTYPE */
	"\135"
	"Protocol not available\0" /* ENOPROTOOPT */
	"\136"
	"Protocol not supported\0" /* EPROTONOSUPPORT */
	"\137"
	"Socket type not supported\0" /* ESOCKTNOSUPPORT */
	"\140"
	"Operation not supported\0" /* EOPNOTSUPP */
	"\142"
	"Address family not supported by protocol\0" /* EAFNOSUPPORT */
	"\143"
	"Address already in use\0" /* EADDRINUSE */
	"\144"
	"Cannot assign requested address\0" /* EADDRNOTAVAIL */
	"\145"
	"Network is down\0" /* ENETDOWN */
	"\146"
	"Network is unreachable\0" /* ENETUNREACH */
	"\147"
	"Network dropped connection on reset\0" /* ENETRESET */
	"\150"
	"Software caused connection abort\0" /* ECONNABORTED */
	"\151"
	"Connection reset by peer\0" /* ECONNRESET */
	"\152"
	"No buffer space available\0" /* ENOBUFS */
	"\153"
	"Transport endpoint is already connected\0" /* EISCONN */
	"\154"
	"Transport endpoint is not connected\0" /* ENOTCONN */
	"\155"
	"Cannot send after transport endpoint shutdown\0" /* ESHUTDOWN */
	"\157"
	"Connection timed out\0" /* ETIMEDOUT */
	"\160"
	"Connection refused\0" /* ECONNREFUSED */
	"\161"
	"Host is down\0" /* EHOSTDOWN */
	"\162"
	"No route to host\0" /* EHOSTUNREACH */
	"\163"
	"Operation already in progress\0" /* EALREADY */
	"\164"
	"Operation now in progress\0" /* EINPROGRESS */
	"\165"
	"Stale file handle\0" /* ESTALE */
	"\173"
	"Disk quota exceeded\0" /* EDQUOT */
	"\176"
	"Operation canceled\0" /* ECANCELED */
	"\203"
	"Owner died\0" /* EOWNERDEAD */
	"\204"
	"State not recoverable\0" /* ENOTRECOVERABLE */
	"\311"
	"Inappropriate file type or format\0" /* EFTYPE */;

char *strerror(int errnum)
{
	const unsigned char *p = (const unsigned char *)messages;

	while (*p != 0) {
		if (*p - 1 == errnum)
			return (char *)p + 1;
		while (*++p != 0);
		++p;
	}

	return (char *)"Unknown error";
}
