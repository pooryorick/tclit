/*
 * Copyright © 1991-1994 The Regents of the University of California.
 * Copyright © 1994-1996 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

/*
 * You may distribute and/or modify this program under the terms of the GNU
 * Affero General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.

 * See the file "COPYING" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
*/

/*
 * tclPosixStr.c --
 *
 *	This file contains procedures that generate strings corresponding to
 *	various POSIX-related codes, such as errno and signals.
 */

#include "tclInt.h"

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ErrnoId --
 *
 *	Return a textual identifier for the current errno value.
 *
 * Results:
 *	This procedure returns a machine-readable textual identifier that
 *	corresponds to the current errno value (e.g. "EPERM"). The identifier
 *	is the same as the #define name in errno.h.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

const char *
Tcl_ErrnoId(void)
{
    switch (errno) {
#if defined(E2BIG) && (!defined(EOVERFLOW) || (E2BIG != EOVERFLOW))
    case E2BIG: return "E2BIG";
#endif
#ifdef EACCES
    case EACCES: return "EACCES";
#endif
#ifdef EADDRINUSE
    case EADDRINUSE: return "EADDRINUSE";
#endif
#ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL: return "EADDRNOTAVAIL";
#endif
#ifdef EADV
    case EADV: return "EADV";
#endif
#ifdef EAFNOSUPPORT
    case EAFNOSUPPORT: return "EAFNOSUPPORT";
#endif
#ifdef EAGAIN
    case EAGAIN: return "EAGAIN";
#endif
#ifdef EALIGN
    case EALIGN: return "EALIGN";
#endif
#if defined(EALREADY) && (!defined(EBUSY) || (EALREADY != EBUSY))
    case EALREADY: return "EALREADY";
#endif
#ifdef EBADCAT
    case EBADCAT: return "EBADCAT";
#endif
#ifdef EBADE
    case EBADE: return "EBADE";
#endif
#ifdef EBADF
    case EBADF: return "EBADF";
#endif
#ifdef EBADFD
    case EBADFD: return "EBADFD";
#endif
#ifdef EBADMSG
    case EBADMSG: return "EBADMSG";
#endif
#ifdef EBADR
    case EBADR: return "EBADR";
#endif
#ifdef EBADRPC
    case EBADRPC: return "EBADRPC";
#endif
#ifdef EBADRQC
    case EBADRQC: return "EBADRQC";
#endif
#ifdef EBADSLT
    case EBADSLT: return "EBADSLT";
#endif
#ifdef EBFONT
    case EBFONT: return "EBFONT";
#endif
#ifdef EBUSY
    case EBUSY: return "EBUSY";
#endif
#ifdef ECANCELED
    case ECANCELED: return "ECANCELED";
#endif
#ifdef ECASECLASH
    case ECASECLASH: return "ECASECLASH";
#endif
#ifdef ECHILD
    case ECHILD: return "ECHILD";
#endif
#ifdef ECHRNG
    case ECHRNG: return "ECHRNG";
#endif
#ifdef ECOMM
    case ECOMM: return "ECOMM";
#endif
#ifdef ECONNABORTED
    case ECONNABORTED: return "ECONNABORTED";
#endif
#ifdef ECONNREFUSED
    case ECONNREFUSED: return "ECONNREFUSED";
#endif
#ifdef ECONNRESET
    case ECONNRESET: return "ECONNRESET";
#endif
#if defined(EDEADLK) && (!defined(EWOULDBLOCK) || (EDEADLK != EWOULDBLOCK))
    case EDEADLK: return "EDEADLK";
#endif
#if defined(EDEADLOCK) && (!defined(EDEADLK) || (EDEADLOCK != EDEADLK))
    case EDEADLOCK: return "EDEADLOCK";
#endif
#ifdef EDESTADDRREQ
    case EDESTADDRREQ: return "EDESTADDRREQ";
#endif
#ifdef EDIRTY
    case EDIRTY: return "EDIRTY";
#endif
#ifdef EDOM
    case EDOM: return "EDOM";
#endif
#ifdef EDOTDOT
    case EDOTDOT: return "EDOTDOT";
#endif
#ifdef EDQUOT
    case EDQUOT: return "EDQUOT";
#endif
#ifdef EDUPPKG
    case EDUPPKG: return "EDUPPKG";
#endif
#ifdef EEXIST
    case EEXIST: return "EEXIST";
#endif
#ifdef EFAIL
    case EFAIL: return "EFAIL";
#endif
#ifdef EFAULT
    case EFAULT: return "EFAULT";
#endif
#ifdef EFBIG
    case EFBIG: return "EFBIG";
#endif
#ifdef EFTYPE
    case EFTYPE: return "EFTYPE";
#endif
#ifdef EHOSTDOWN
    case EHOSTDOWN: return "EHOSTDOWN";
#endif
#ifdef EHOSTUNREACH
    case EHOSTUNREACH: return "EHOSTUNREACH";
#endif
#if defined(EIDRM) && (!defined(EINPROGRESS) || (EIDRM != EINPROGRESS))
    case EIDRM: return "EIDRM";
#endif
#ifdef EINIT
    case EINIT: return "EINIT";
#endif
#ifdef EILSEQ
    case EILSEQ: return "EILSEQ";
#endif
#ifdef EINPROG
    case EINPROG: return "EINPROG";
#endif
#ifdef EINPROGRESS
    case EINPROGRESS: return "EINPROGRESS";
#endif
#ifdef EINTR
    case EINTR: return "EINTR";
#endif
#ifdef EINVAL
    case EINVAL: return "EINVAL";
#endif
#ifdef EIO
    case EIO: return "EIO";
#endif
#ifdef EISCONN
    case EISCONN: return "EISCONN";
#endif
#ifdef EISDIR
    case EISDIR: return "EISDIR";
#endif
#ifdef EISNAM
    case EISNAM: return "EISNAM";
#endif
#ifdef EL2HLT
    case EL2HLT: return "EL2HLT";
#endif
#ifdef EL2NSYNC
    case EL2NSYNC: return "EL2NSYNC";
#endif
#ifdef EL3HLT
    case EL3HLT: return "EL3HLT";
#endif
#ifdef EL3RST
    case EL3RST: return "EL3RST";
#endif
#ifdef ELBIN
    case ELBIN: return "ELBIN";
#endif
#ifdef ELIBACC
    case ELIBACC: return "ELIBACC";
#endif
#ifdef ELIBBAD
    case ELIBBAD: return "ELIBBAD";
#endif
#ifdef ELIBEXEC
    case ELIBEXEC: return "ELIBEXEC";
#endif
#if defined(ELIBMAX) && (!defined(ECANCELED) || (ELIBMAX != ECANCELED))
    case ELIBMAX: return "ELIBMAX";
#endif
#ifdef ELIBSCN
    case ELIBSCN: return "ELIBSCN";
#endif
#ifdef ELNRNG
    case ELNRNG: return "ELNRNG";
#endif
#if defined(ELOOP) && (!defined(ENOENT) || (ELOOP != ENOENT))
    case ELOOP: return "ELOOP";
#endif
#ifdef EMEDIUMTYPE
    case EMEDIUMTYPE: return "EMEDIUMTYPE";
#endif
#ifdef EMFILE
    case EMFILE: return "EMFILE";
#endif
#ifdef EMLINK
    case EMLINK: return "EMLINK";
#endif
#ifdef EMSGSIZE
    case EMSGSIZE: return "EMSGSIZE";
#endif
#ifdef EMULTIHOP
    case EMULTIHOP: return "EMULTIHOP";
#endif
#ifdef ENAMETOOLONG
    case ENAMETOOLONG: return "ENAMETOOLONG";
#endif
#ifdef ENAVAIL
    case ENAVAIL: return "ENAVAIL";
#endif
#ifdef ENETDOWN
    case ENETDOWN: return "ENETDOWN";
#endif
#ifdef ENETRESET
    case ENETRESET: return "ENETRESET";
#endif
#ifdef ENETUNREACH
    case ENETUNREACH: return "ENETUNREACH";
#endif
#ifdef ENFILE
    case ENFILE: return "ENFILE";
#endif
#ifdef ENMFILE
    case ENMFILE: return "ENMFILE";
#endif
#ifdef ENOANO
    case ENOANO: return "ENOANO";
#endif
#if defined(ENOBUFS) && (!defined(ENOSR) || (ENOBUFS != ENOSR))
    case ENOBUFS: return "ENOBUFS";
#endif
#ifdef ENOCSI
    case ENOCSI: return "ENOCSI";
#endif
#if defined(ENODATA) && (!defined(ECONNREFUSED) || (ENODATA != ECONNREFUSED))
    case ENODATA: return "ENODATA";
#endif
#ifdef ENODEV
    case ENODEV: return "ENODEV";
#endif
#ifdef ENOENT
    case ENOENT: return "ENOENT";
#endif
#ifdef ENOEXEC
    case ENOEXEC: return "ENOEXEC";
#endif
#ifdef ENOLCK
    case ENOLCK: return "ENOLCK";
#endif
#ifdef ENOLINK
    case ENOLINK: return "ENOLINK";
#endif
#ifdef ENOMEM
    case ENOMEM: return "ENOMEM";
#endif
#ifdef ENOMEDIUM
    case ENOMEDIUM: return "ENOMEDIUM";
#endif
#ifdef ENOMSG
    case ENOMSG: return "ENOMSG";
#endif
#ifdef ENONET
    case ENONET: return "ENONET";
#endif
#ifdef ENOPKG
    case ENOPKG: return "ENOPKG";
#endif
#ifdef ENOPROTOOPT
    case ENOPROTOOPT: return "ENOPROTOOPT";
#endif
#ifdef ENOSHARE
    case ENOSHARE: return "ENOSHARE";
#endif
#ifdef ENOSPC
    case ENOSPC: return "ENOSPC";
#endif
#if defined(ENOSR) && (!defined(ENAMETOOLONG) || (ENAMETOOLONG != ENOSR))
    case ENOSR: return "ENOSR";
#endif
#if defined(ENOSTR) && (!defined(ENOTTY) || (ENOTTY != ENOSTR))
    case ENOSTR: return "ENOSTR";
#endif
#ifdef ENOSYM
    case ENOSYM: return "ENOSYM";
#endif
#ifdef ENOSYS
    case ENOSYS: return "ENOSYS";
#endif
#ifdef ENOTBLK
    case ENOTBLK: return "ENOTBLK";
#endif
#ifdef ENOTCONN
    case ENOTCONN: return "ENOTCONN";
#endif
#ifdef ENOTRECOVERABLE
    case ENOTRECOVERABLE: return "ENOTRECOVERABLE";
#endif
#ifdef ENOTDIR
    case ENOTDIR: return "ENOTDIR";
#endif
#if defined(ENOTEMPTY) && (!defined(EEXIST) || (ENOTEMPTY != EEXIST))
    case ENOTEMPTY: return "ENOTEMPTY";
#endif
#ifdef ENOTNAM
    case ENOTNAM: return "ENOTNAM";
#endif
#ifdef ENOTSOCK
    case ENOTSOCK: return "ENOTSOCK";
#endif
#ifdef ENOTSUP
    case ENOTSUP: return "ENOTSUP";
#endif
#ifdef ENOTTY
    case ENOTTY: return "ENOTTY";
#endif
#ifdef ENOTUNIQ
    case ENOTUNIQ: return "ENOTUNIQ";
#endif
#ifdef ENWAIT
    case ENWAIT: return "ENWAIT";
#endif
#ifdef ENXIO
    case ENXIO: return "ENXIO";
#endif
#if defined(EOPNOTSUPP) &&  (!defined(ENOTSUP) || (ENOTSUP != EOPNOTSUPP))
    case EOPNOTSUPP: return "EOPNOTSUPP";
#endif
#ifdef EOTHER
    case EOTHER: return "EOTHER";
#endif
#if defined(EOVERFLOW) && (!defined(EFBIG) || (EOVERFLOW != EFBIG)) && (!defined(EINVAL) || (EOVERFLOW != EINVAL))
    case EOVERFLOW: return "EOVERFLOW";
#endif
#ifdef EOWNERDEAD
    case EOWNERDEAD: return "EOWNERDEAD";
#endif
#ifdef EPERM
    case EPERM: return "EPERM";
#endif
#if defined(EPFNOSUPPORT) && (!defined(ENOLCK) || (ENOLCK != EPFNOSUPPORT))
    case EPFNOSUPPORT: return "EPFNOSUPPORT";
#endif
#ifdef EPIPE
    case EPIPE: return "EPIPE";
#endif
#ifdef EPROCLIM
    case EPROCLIM: return "EPROCLIM";
#endif
#ifdef EPROCUNAVAIL
    case EPROCUNAVAIL: return "EPROCUNAVAIL";
#endif
#ifdef EPROGMISMATCH
    case EPROGMISMATCH: return "EPROGMISMATCH";
#endif
#ifdef EPROGUNAVAIL
    case EPROGUNAVAIL: return "EPROGUNAVAIL";
#endif
#ifdef EPROTO
    case EPROTO: return "EPROTO";
#endif
#ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT: return "EPROTONOSUPPORT";
#endif
#ifdef EPROTOTYPE
    case EPROTOTYPE: return "EPROTOTYPE";
#endif
#ifdef ERANGE
    case ERANGE: return "ERANGE";
#endif
#if defined(EREFUSED) && (!defined(ECONNREFUSED) || (EREFUSED != ECONNREFUSED))
    case EREFUSED: return "EREFUSED";
#endif
#ifdef EREMCHG
    case EREMCHG: return "EREMCHG";
#endif
#ifdef EREMDEV
    case EREMDEV: return "EREMDEV";
#endif
#ifdef EREMOTE
    case EREMOTE: return "EREMOTE";
#endif
#ifdef EREMOTEIO
    case EREMOTEIO: return "EREMOTEIO";
#endif
#ifdef EREMOTERELEASE
    case EREMOTERELEASE: return "EREMOTERELEASE";
#endif
#ifdef ERESTART
    case ERESTART: return "ERESTART";
#endif
#ifdef EROFS
    case EROFS: return "EROFS";
#endif
#ifdef ERPCMISMATCH
    case ERPCMISMATCH: return "ERPCMISMATCH";
#endif
#ifdef ERREMOTE
    case ERREMOTE: return "ERREMOTE";
#endif
#ifdef ESHUTDOWN
    case ESHUTDOWN: return "ESHUTDOWN";
#endif
#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT: return "ESOCKTNOSUPPORT";
#endif
#ifdef ESPIPE
    case ESPIPE: return "ESPIPE";
#endif
#ifdef ESRCH
    case ESRCH: return "ESRCH";
#endif
#ifdef ESRMNT
    case ESRMNT: return "ESRMNT";
#endif
#ifdef ESTALE
    case ESTALE: return "ESTALE";
#endif
#ifdef ESUCCESS
    case ESUCCESS: return "ESUCCESS";
#endif
#if defined(ETIME) && (!defined(ELOOP) || (ETIME != ELOOP))
    case ETIME: return "ETIME";
#endif
#if defined(ETIMEDOUT) && (!defined(ENOSTR) || (ETIMEDOUT != ENOSTR))
    case ETIMEDOUT: return "ETIMEDOUT";
#endif
#ifdef ETOOMANYREFS
    case ETOOMANYREFS: return "ETOOMANYREFS";
#endif
#ifdef ETXTBSY
    case ETXTBSY: return "ETXTBSY";
#endif
#ifdef EUCLEAN
    case EUCLEAN: return "EUCLEAN";
#endif
#ifdef EUNATCH
    case EUNATCH: return "EUNATCH";
#endif
#ifdef EUSERS
    case EUSERS: return "EUSERS";
#endif
#ifdef EVERSION
    case EVERSION: return "EVERSION";
#endif
#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || (EWOULDBLOCK != EAGAIN))
    case EWOULDBLOCK: return "EWOULDBLOCK";
#endif
#ifdef EXDEV
    case EXDEV: return "EXDEV";
#endif
#ifdef EXFULL
    case EXFULL: return "EXFULL";
#endif
    }
    return "unknown error";
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_ErrnoMsg --
 *
 *	Return a human-readable message corresponding to a given errno value.
 *
 * Results:
 *	The return value is the standard POSIX error message for errno.  This
 *	procedure is used instead of strerror because strerror returns
 *	slightly different values on different machines (e.g. different
 *	capitalizations), which cause problems for things such as regression
 *	tests. This procedure provides messages for most standard errors, then
 *	it calls strerror for things it doesn't understand.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

const char *
Tcl_ErrnoMsg(
    int err)			/* Error number (such as in errno variable). */
{
    switch (err) {
#if defined(E2BIG) && (!defined(EOVERFLOW) || (E2BIG != EOVERFLOW))
    case E2BIG: return "argument list too long";
#endif
#ifdef EACCES
    case EACCES: return "permission denied";
#endif
#ifdef EADDRINUSE
    case EADDRINUSE: return "address already in use";
#endif
#ifdef EADDRNOTAVAIL
    case EADDRNOTAVAIL: return "cannot assign requested address";
#endif
#ifdef EADV
    case EADV: return "advertise error";
#endif
#ifdef EAFNOSUPPORT
    case EAFNOSUPPORT: return "address family not supported by protocol";
#endif
#ifdef EAGAIN
    case EAGAIN: return "resource temporarily unavailable";
#endif
#ifdef EALIGN
    case EALIGN: return "alignment error";
#endif
#if defined(EALREADY) && (!defined(EBUSY) || (EALREADY != EBUSY))
    case EALREADY: return "operation already in progress";
#endif
#ifdef EBADCAT
    case EBADCAT: return "bad message catalogue format";
#endif
#ifdef EBADE
    case EBADE: return "invalid exchange";
#endif
#ifdef EBADF
    case EBADF: return "bad file descriptor";
#endif
#ifdef EBADFD
    case EBADFD: return "file descriptor in bad state";
#endif
#ifdef EBADMSG
    case EBADMSG: return "bad message";
#endif
#ifdef EBADR
    case EBADR: return "invalid request descriptor";
#endif
#ifdef EBADRPC
    case EBADRPC: return "RPC structure is bad";
#endif
#ifdef EBADRQC
    case EBADRQC: return "invalid request code";
#endif
#ifdef EBADSLT
    case EBADSLT: return "invalid slot";
#endif
#ifdef EBFONT
    case EBFONT: return "bad font file format";
#endif
#ifdef EBUSY
    case EBUSY: return "device or resource busy";
#endif
#ifdef ECANCELED
    case ECANCELED: return "operation canceled";
#endif
#ifdef ECASECLASH
    case ECASECLASH: return "filename exists with different case";
#endif
#ifdef ECHILD
    case ECHILD: return "no child processes";
#endif
#ifdef ECHRNG
    case ECHRNG: return "channel number out of range";
#endif
#ifdef ECOMM
    case ECOMM: return "communication error on send";
#endif
#ifdef ECONNABORTED
    case ECONNABORTED: return "software caused connection abort";
#endif
#ifdef ECONNREFUSED
    case ECONNREFUSED: return "connection refused";
#endif
#ifdef ECONNRESET
    case ECONNRESET: return "connection reset by peer";
#endif
#if defined(EDEADLK) && (!defined(EWOULDBLOCK) || (EDEADLK != EWOULDBLOCK))
    case EDEADLK: return "resource deadlock avoided";
#endif
#if defined(EDEADLOCK) && (!defined(EDEADLK) || (EDEADLOCK != EDEADLK))
    case EDEADLOCK: return "resource deadlock avoided";
#endif
#ifdef EDESTADDRREQ
    case EDESTADDRREQ: return "destination address required";
#endif
#ifdef EDIRTY
    case EDIRTY: return "mounting a dirty fs w/o force";
#endif
#ifdef EDOM
    case EDOM: return "numerical argument out of domain";
#endif
#ifdef EDOTDOT
    case EDOTDOT: return "cross mount point";
#endif
#ifdef EDQUOT
    case EDQUOT: return "disk quota exceeded";
#endif
#ifdef EDUPPKG
    case EDUPPKG: return "duplicate package name";
#endif
#ifdef EEXIST
    case EEXIST: return "file exists";
#endif
#ifdef EFAIL
    case EFAIL: return "cannot start operation";
#endif
#ifdef EFAULT
    case EFAULT: return "bad address";
#endif
#ifdef EFBIG
    case EFBIG: return "file too large";
#endif
#ifdef EFTYPE
    case EFTYPE: return "inappropriate file type or format";
#endif
#ifdef EHOSTDOWN
    case EHOSTDOWN: return "host is down";
#endif
#ifdef EHOSTUNREACH
    case EHOSTUNREACH: return "no route to host";
#endif
#if defined(EIDRM) && (!defined(EINPROGRESS) || (EIDRM != EINPROGRESS))
    case EIDRM: return "identifier removed";
#endif
#ifdef EINIT
    case EINIT: return "initialization error";
#endif
#ifdef EILSEQ
    case EILSEQ: return "invalid or incomplete multibyte or wide character";
#endif
#ifdef EINPROG
    case EINPROG: return "asynchronous operation in progress";
#endif
#ifdef EINPROGRESS
    case EINPROGRESS: return "operation now in progress";
#endif
#ifdef EINTR
    case EINTR: return "interrupted system call";
#endif
#ifdef EINVAL
    case EINVAL: return "invalid argument";
#endif
#ifdef EIO
    case EIO: return "input/output error";
#endif
#ifdef EISCONN
    case EISCONN: return "transport endpoint is already connected";
#endif
#ifdef EISDIR
    case EISDIR: return "is a directory";
#endif
#ifdef EISNAM
    case EISNAM: return "is a named type file";
#endif
#ifdef EL2HLT
    case EL2HLT: return "level 2 halted";
#endif
#ifdef EL2NSYNC
    case EL2NSYNC: return "level 2 not synchronized";
#endif
#ifdef EL3HLT
    case EL3HLT: return "level 3 halted";
#endif
#ifdef EL3RST
    case EL3RST: return "level 3 reset";
#endif
#ifdef ELBIN
    case ELBIN: return "inode is remote";
#endif
#ifdef ELIBACC
    case ELIBACC: return "can not access a needed shared library";
#endif
#ifdef ELIBBAD
    case ELIBBAD: return "accessing a corrupted shared library";
#endif
#ifdef ELIBEXEC
    case ELIBEXEC: return "cannot exec a shared library directly";
#endif
#if defined(ELIBMAX) && (!defined(ECANCELED) || (ELIBMAX != ECANCELED))
    case ELIBMAX: return
	    "attempting to link in too many shared libraries";
#endif
#ifdef ELIBSCN
    case ELIBSCN: return ".lib section in a.out corrupted";
#endif
#ifdef ELNRNG
    case ELNRNG: return "link number out of range";
#endif
#if defined(ELOOP) && (!defined(ENOENT) || (ELOOP != ENOENT))
    case ELOOP: return "too many levels of symbolic links";
#endif
#ifdef EMEDIUMTYPE
    case EMEDIUMTYPE: return "wrong medium type";
#endif
#ifdef EMFILE
    case EMFILE: return "too many open files";
#endif
#ifdef EMLINK
    case EMLINK: return "too many links";
#endif
#ifdef EMSGSIZE
    case EMSGSIZE: return "message too long";
#endif
#ifdef EMULTIHOP
    case EMULTIHOP: return "multihop attempted";
#endif
#ifdef ENAMETOOLONG
    case ENAMETOOLONG: return "file name too long";
#endif
#ifdef ENAVAIL
    case ENAVAIL: return "not available";
#endif
#ifdef ENETDOWN
    case ENETDOWN: return "network is down";
#endif
#ifdef ENETRESET
    case ENETRESET: return "network dropped connection on reset";
#endif
#ifdef ENETUNREACH
    case ENETUNREACH: return "network is unreachable";
#endif
#ifdef ENFILE
    case ENFILE: return "too many open files in system";
#endif
#ifdef ENMFILE
    case ENMFILE: return "no more files";
#endif
#ifdef ENOANO
    case ENOANO: return "no anode";
#endif
#if defined(ENOBUFS) && (!defined(ENOSR) || (ENOBUFS != ENOSR))
    case ENOBUFS: return "no buffer space available";
#endif
#ifdef ENOCSI
    case ENOCSI: return "no CSI structure available";
#endif
#if defined(ENODATA) && (!defined(ECONNREFUSED) || (ENODATA != ECONNREFUSED))
    case ENODATA: return "no data available";
#endif
#ifdef ENODEV
    case ENODEV: return "no such device";
#endif
#ifdef ENOENT
    case ENOENT: return "no such file or directory";
#endif
#ifdef ENOEXEC
    case ENOEXEC: return "exec format error";
#endif
#ifdef ENOLCK
    case ENOLCK: return "no locks available";
#endif
#ifdef ENOLINK
    case ENOLINK: return "link has been severed";
#endif
#ifdef ENOMEM
    case ENOMEM: return "cannot allocate memory";
#endif
#ifdef ENOMEDIUM
    case ENOMEDIUM: return "no medium found";
#endif
#ifdef ENOMSG
    case ENOMSG: return "no message of desired type";
#endif
#ifdef ENONET
    case ENONET: return "machine is not on the network";
#endif
#ifdef ENOPKG
    case ENOPKG: return "package not installed";
#endif
#ifdef ENOPROTOOPT
    case ENOPROTOOPT: return "protocol not available";
#endif
#ifdef ENOSHARE
    case ENOSHARE: return "no such host or network path";
#endif
#ifdef ENOSPC
    case ENOSPC: return "no space left on device";
#endif
#if defined(ENOSR) && (!defined(ENAMETOOLONG) || (ENAMETOOLONG != ENOSR))
    case ENOSR: return "out of streams resources";
#endif
#if defined(ENOSTR) && (!defined(ENOTTY) || (ENOTTY != ENOSTR))
    case ENOSTR: return "device not a stream";
#endif
#ifdef ENOSYM
    case ENOSYM: return "unresolved symbol name";
#endif
#ifdef ENOSYS
    case ENOSYS: return "function not implemented";
#endif
#ifdef ENOTBLK
    case ENOTBLK: return "block device required";
#endif
#ifdef ENOTCONN
    case ENOTCONN: return "transport endpoint is not connected";
#endif
#ifdef ENOTDIR
    case ENOTDIR: return "not a directory";
#endif
#if defined(ENOTEMPTY) && (!defined(EEXIST) || (ENOTEMPTY != EEXIST))
    case ENOTEMPTY: return "directory not empty";
#endif
#ifdef ENOTNAM
    case ENOTNAM: return "not a name file";
#endif
#ifdef ENOTRECOVERABLE
    case ENOTRECOVERABLE: return "state not recoverable";
#endif
#ifdef ENOTSOCK
    case ENOTSOCK: return "socket operation on non-socket";
#endif
#ifdef ENOTSUP
    case ENOTSUP: return "operation not supported";
#endif
#ifdef ENOTTY
    case ENOTTY: return "inappropriate ioctl for device";
#endif
#ifdef ENOTUNIQ
    case ENOTUNIQ: return "name not unique on network";
#endif
#ifdef ENWAIT
    case ENWAIT: return "No waiting processes";
#endif
#ifdef ENXIO
    case ENXIO: return "no such device or address";
#endif
#if defined(EOPNOTSUPP) &&  (!defined(ENOTSUP) || (ENOTSUP != EOPNOTSUPP))
    case EOPNOTSUPP: return "operation not supported on socket";
#endif
#ifdef EOTHER
    case EOTHER: return "other error";
#endif
#if defined(EOVERFLOW) && (!defined(EFBIG) || (EOVERFLOW != EFBIG)) && (!defined(EINVAL) || (EOVERFLOW != EINVAL))
    case EOVERFLOW: return "value too large for defined data type";
#endif
#ifdef EOWNERDEAD
    case EOWNERDEAD: return "owner died";
#endif
#ifdef EPERM
    case EPERM: return "operation not permitted";
#endif
#if defined(EPFNOSUPPORT) && (!defined(ENOLCK) || (ENOLCK != EPFNOSUPPORT))
    case EPFNOSUPPORT: return "protocol family not supported";
#endif
#ifdef EPIPE
    case EPIPE: return "broken pipe";
#endif
#ifdef EPROCLIM
    case EPROCLIM: return "too many processes";
#endif
#ifdef EPROCUNAVAIL
    case EPROCUNAVAIL: return "bad procedure for program";
#endif
#ifdef EPROGMISMATCH
    case EPROGMISMATCH: return "program version wrong";
#endif
#ifdef EPROGUNAVAIL
    case EPROGUNAVAIL: return "RPC program not available";
#endif
#ifdef EPROTO
    case EPROTO: return "protocol error";
#endif
#ifdef EPROTONOSUPPORT
    case EPROTONOSUPPORT: return "protocol not supported";
#endif
#ifdef EPROTOTYPE
    case EPROTOTYPE: return "protocol wrong type for socket";
#endif
#ifdef ERANGE
    case ERANGE: return "numerical result out of range";
#endif
#if defined(EREFUSED) && (!defined(ECONNREFUSED) || (EREFUSED != ECONNREFUSED))
    case EREFUSED: return "connection refused";
#endif
#ifdef EREMCHG
    case EREMCHG: return "remote address changed";
#endif
#ifdef EREMDEV
    case EREMDEV: return "remote device";
#endif
#ifdef EREMOTE
    case EREMOTE: return "object is remote";
#endif
#ifdef EREMOTEIO
    case EREMOTEIO: return "remote I/O error";
#endif
#ifdef EREMOTERELEASE
    case EREMOTERELEASE: return "remote peer released connection";
#endif
#ifdef ERESTART
    case ERESTART: return "interrupted system call should be restarted";
#endif
#ifdef EROFS
    case EROFS: return "read-only file system";
#endif
#ifdef ERPCMISMATCH
    case ERPCMISMATCH: return "RPC version is wrong";
#endif
#ifdef ERREMOTE
    case ERREMOTE: return "object is remote";
#endif
#ifdef ESHUTDOWN
    case ESHUTDOWN: return "cannot send after transport endpoint shutdown";
#endif
#ifdef ESOCKTNOSUPPORT
    case ESOCKTNOSUPPORT: return "socket type not supported";
#endif
#ifdef ESPIPE
    case ESPIPE: return "illegal seek";
#endif
#ifdef ESRCH
    case ESRCH: return "no such process";
#endif
#ifdef ESRMNT
    case ESRMNT: return "srmount error";
#endif
#ifdef ESTALE
    case ESTALE: return "stale file handle";
#endif
#ifdef ESTRPIPE
    case ESTRPIPE: return "streams pipe error";
#endif
#ifdef ESUCCESS
    case ESUCCESS: return "success";
#endif
#if defined(ETIME) && (!defined(ELOOP) || (ETIME != ELOOP))
    case ETIME: return "timer expired";
#endif
#if defined(ETIMEDOUT) && (!defined(ENOSTR) || (ETIMEDOUT != ENOSTR))
    case ETIMEDOUT: return "connection timed out";
#endif
#ifdef ETOOMANYREFS
    case ETOOMANYREFS: return "too many references: cannot splice";
#endif
#ifdef ETXTBSY
    case ETXTBSY: return "text file busy";
#endif
#ifdef EUCLEAN
    case EUCLEAN: return "structure needs cleaning";
#endif
#ifdef EUNATCH
    case EUNATCH: return "protocol driver not attached";
#endif
#ifdef EUSERS
    case EUSERS: return "too many users";
#endif
#ifdef EVERSION
    case EVERSION: return "version mismatch";
#endif
#if defined(EWOULDBLOCK) && (!defined(EAGAIN) || (EWOULDBLOCK != EAGAIN))
    case EWOULDBLOCK: return "operation would block";
#endif
#ifdef EXDEV
    case EXDEV: return "invalid cross-device link";
#endif
#ifdef EXFULL
    case EXFULL: return "exchange full";
#endif
    default:
#ifdef NO_STRERROR
	return "unknown POSIX error";
#else
	return strerror(err);
#endif
    }
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SignalId --
 *
 *	Return a textual identifier for a signal number.
 *
 * Results:
 *	This procedure returns a machine-readable textual identifier that
 *	corresponds to sig.  The identifier is the same as the #define name in
 *	signal.h.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

const char *
Tcl_SignalId(
    int sig)			/* Number of signal. */
{
    switch (sig) {
#ifdef SIGABRT
    case SIGABRT: return "SIGABRT";
#endif
#ifdef SIGALRM
    case SIGALRM: return "SIGALRM";
#endif
#ifdef SIGBUS
    case SIGBUS: return "SIGBUS";
#endif
#ifdef SIGCHLD
    case SIGCHLD: return "SIGCHLD";
#endif
#if defined(SIGCLD) && (!defined(SIGCHLD) || (SIGCLD != SIGCHLD))
    case SIGCLD: return "SIGCLD";
#endif
#ifdef SIGCONT
    case SIGCONT: return "SIGCONT";
#endif
#if defined(SIGEMT) && (!defined(SIGXCPU) || (SIGEMT != SIGXCPU))
    case SIGEMT: return "SIGEMT";
#endif
#ifdef SIGFPE
    case SIGFPE: return "SIGFPE";
#endif
#ifdef SIGHUP
    case SIGHUP: return "SIGHUP";
#endif
#ifdef SIGILL
    case SIGILL: return "SIGILL";
#endif
#ifdef SIGINT
    case SIGINT: return "SIGINT";
#endif
#ifdef SIGIO
    case SIGIO: return "SIGIO";
#endif
#if defined(SIGIOT) && (!defined(SIGABRT) || (SIGIOT != SIGABRT))
    case SIGIOT: return "SIGIOT";
#endif
#ifdef SIGKILL
    case SIGKILL: return "SIGKILL";
#endif
#if defined(SIGLOST) && (!defined(SIGIOT) || (SIGLOST != SIGIOT)) && (!defined(SIGURG) || (SIGLOST != SIGURG)) && (!defined(SIGPROF) || (SIGLOST != SIGPROF)) && (!defined(SIGIO) || (SIGLOST != SIGIO))
    case SIGLOST: return "SIGLOST";
#endif
#ifdef SIGPIPE
    case SIGPIPE: return "SIGPIPE";
#endif
#if defined(SIGPOLL) && (!defined(SIGIO) || (SIGPOLL != SIGIO))
    case SIGPOLL: return "SIGPOLL";
#endif
#ifdef SIGPROF
    case SIGPROF: return "SIGPROF";
#endif
#if defined(SIGPWR) && (!defined(SIGXFSZ) || (SIGPWR != SIGXFSZ)) && (!defined(SIGLOST) || (SIGPWR != SIGLOST))
    case SIGPWR: return "SIGPWR";
#endif
#ifdef SIGQUIT
    case SIGQUIT: return "SIGQUIT";
#endif
#if defined(SIGSEGV) && (!defined(SIGBUS) || (SIGSEGV != SIGBUS))
    case SIGSEGV: return "SIGSEGV";
#endif
#ifdef SIGSTOP
    case SIGSTOP: return "SIGSTOP";
#endif
#ifdef SIGSYS
    case SIGSYS: return "SIGSYS";
#endif
#ifdef SIGTERM
    case SIGTERM: return "SIGTERM";
#endif
#ifdef SIGTRAP
    case SIGTRAP: return "SIGTRAP";
#endif
#ifdef SIGTSTP
    case SIGTSTP: return "SIGTSTP";
#endif
#ifdef SIGTTIN
    case SIGTTIN: return "SIGTTIN";
#endif
#ifdef SIGTTOU
    case SIGTTOU: return "SIGTTOU";
#endif
#if defined(SIGURG) && (!defined(SIGIO) || (SIGURG != SIGIO))
    case SIGURG: return "SIGURG";
#endif
#if defined(SIGUSR1) && (!defined(SIGIO) || (SIGUSR1 != SIGIO))
    case SIGUSR1: return "SIGUSR1";
#endif
#if defined(SIGUSR2) && (!defined(SIGURG) || (SIGUSR2 != SIGURG))
    case SIGUSR2: return "SIGUSR2";
#endif
#ifdef SIGVTALRM
    case SIGVTALRM: return "SIGVTALRM";
#endif
#ifdef SIGWINCH
    case SIGWINCH: return "SIGWINCH";
#endif
#ifdef SIGXCPU
    case SIGXCPU: return "SIGXCPU";
#endif
#ifdef SIGXFSZ
    case SIGXFSZ: return "SIGXFSZ";
#endif
#if defined(SIGINFO) && (!defined(SIGPWR) || (SIGINFO != SIGPWR))
    case SIGINFO: return "SIGINFO";
#endif
    }
    return "unknown signal";
}

/*
 *----------------------------------------------------------------------
 *
 * Tcl_SignalMsg --
 *
 *	Return a human-readable message describing a signal.
 *
 * Results:
 *	This procedure returns a string describing sig that should make sense
 *	to a human.  It may not be easy for a machine to parse.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

const char *
Tcl_SignalMsg(
    int sig)			/* Number of signal. */
{
    switch (sig) {
#ifdef SIGABRT
    case SIGABRT: return "SIGABRT";
#endif
#ifdef SIGALRM
    case SIGALRM: return "alarm clock";
#endif
#ifdef SIGBUS
    case SIGBUS: return "bus error";
#endif
#ifdef SIGCHLD
    case SIGCHLD: return "child status changed";
#endif
#if defined(SIGCLD) && (!defined(SIGCHLD) || (SIGCLD != SIGCHLD))
    case SIGCLD: return "child status changed";
#endif
#ifdef SIGCONT
    case SIGCONT: return "continue after stop";
#endif
#if defined(SIGEMT) && (!defined(SIGXCPU) || (SIGEMT != SIGXCPU))
    case SIGEMT: return "EMT instruction";
#endif
#ifdef SIGFPE
    case SIGFPE: return "floating-point exception";
#endif
#ifdef SIGHUP
    case SIGHUP: return "hangup";
#endif
#ifdef SIGILL
    case SIGILL: return "illegal instruction";
#endif
#ifdef SIGINT
    case SIGINT: return "interrupt";
#endif
#ifdef SIGIO
    case SIGIO: return "input/output possible on file";
#endif
#if defined(SIGIOT) && (!defined(SIGABRT) || (SIGABRT != SIGIOT))
    case SIGIOT: return "IOT instruction";
#endif
#ifdef SIGKILL
    case SIGKILL: return "kill signal";
#endif
#if defined(SIGLOST) && (!defined(SIGIOT) || (SIGLOST != SIGIOT)) && (!defined(SIGURG) || (SIGLOST != SIGURG)) && (!defined(SIGPROF) || (SIGLOST != SIGPROF)) && (!defined(SIGIO) || (SIGLOST != SIGIO))
    case SIGLOST: return "resource lost";
#endif
#ifdef SIGPIPE
    case SIGPIPE: return "write on pipe with no readers";
#endif
#if defined(SIGPOLL) && (!defined(SIGIO) || (SIGPOLL != SIGIO))
    case SIGPOLL: return "input/output possible on file";
#endif
#ifdef SIGPROF
    case SIGPROF: return "profiling alarm";
#endif
#if defined(SIGPWR) && (!defined(SIGXFSZ) || (SIGPWR != SIGXFSZ)) && (!defined(SIGLOST) || (SIGPWR != SIGLOST))
    case SIGPWR: return "power-fail restart";
#endif
#ifdef SIGQUIT
    case SIGQUIT: return "quit signal";
#endif
#if defined(SIGSEGV) && (!defined(SIGBUS) || (SIGSEGV != SIGBUS))
    case SIGSEGV: return "segmentation violation";
#endif
#ifdef SIGSTOP
    case SIGSTOP: return "stop";
#endif
#ifdef SIGSYS
    case SIGSYS: return "bad argument to system call";
#endif
#ifdef SIGTERM
    case SIGTERM: return "software termination signal";
#endif
#ifdef SIGTRAP
    case SIGTRAP: return "trace trap";
#endif
#ifdef SIGTSTP
    case SIGTSTP: return "stop signal from tty";
#endif
#ifdef SIGTTIN
    case SIGTTIN: return "background tty read";
#endif
#ifdef SIGTTOU
    case SIGTTOU: return "background tty write";
#endif
#if defined(SIGURG) && (!defined(SIGIO) || (SIGURG != SIGIO))
    case SIGURG: return "urgent I/O condition";
#endif
#if defined(SIGUSR1) && (!defined(SIGIO) || (SIGUSR1 != SIGIO))
    case SIGUSR1: return "user-defined signal 1";
#endif
#if defined(SIGUSR2) && (!defined(SIGURG) || (SIGUSR2 != SIGURG))
    case SIGUSR2: return "user-defined signal 2";
#endif
#ifdef SIGVTALRM
    case SIGVTALRM: return "virtual time alarm";
#endif
#ifdef SIGWINCH
    case SIGWINCH: return "window changed";
#endif
#ifdef SIGXCPU
    case SIGXCPU: return "exceeded CPU time limit";
#endif
#ifdef SIGXFSZ
    case SIGXFSZ: return "exceeded file size limit";
#endif
#if defined(SIGINFO) && (!defined(SIGPWR) || (SIGINFO != SIGPWR))
    case SIGINFO: return "information request";
#endif
    }
    return "unknown signal";
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * End:
 */
