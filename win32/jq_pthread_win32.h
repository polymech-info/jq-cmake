/*
 * Win32/MSVC pthread compatibility shim for libjq.
 *
 * jv_thread.h covers pthread_mutex_t for Win32 but omits pthread_once_t
 * and the TLS-key API (pthread_key_*).  This header fills those gaps.
 *
 * Included before every jq .c translation unit via CMake's /FI flag when
 * building with MSVC on Windows.  Must be self-contained.
 *
 * pthread_once  -> INIT_ONCE / InitOnceExecuteOnce  (Vista+)
 * pthread_key_* -> TlsAlloc / TlsSetValue / TlsGetValue (TLS slots)
 *
 * Note: TlsAlloc does not support per-thread destructors natively.
 * jv_dtoa_tsd.c registers its own atexit() handler (jv_tsd_dtoa_ctx_fini)
 * for cleanup, so omitting destructor dispatch is safe for embedded use.
 */
#pragma once

#if defined(_WIN32) && !defined(__MINGW32__)

/* ── __attribute__ ───────────────────────────────────────────────────────── *
 * MSVC does not understand GCC/Clang __attribute__((...)).
 * jv_alloc.h uses __attribute__((warn_unused_result)); define it away.
 * Guard against redefining if some other header already did.
 */
#ifndef __attribute__
#  define __attribute__(x)   /* nothing */
#endif

/* ── MIN / MAX ───────────────────────────────────────────────────────────── *
 * util.h defines MIN/MAX using GCC's __typeof__ + statement expressions.
 * Pre-define them here (before util.h is ever included) so util.h's
 * #ifndef guards keep the MSVC-safe versions.
 * These evaluate arguments twice — safe for jq's non-side-effecting uses.
 */
#ifndef MIN
#  define MIN(a, b)  ((a) < (b) ? (a) : (b))
#endif
#ifndef MAX
#  define MAX(a, b)  ((a) > (b) ? (a) : (b))
#endif

/* ── fopen (UTF-8 override) ─────────────────────────────────────────────── *
 * util.c defines a Win32 UTF-8-aware fopen() that conflicts with MSVC's CRT.
 * Rename the definition to avoid LNK2005; jq's internal fopen() calls still
 * resolve to this override, preserving UTF-8 path behaviour.
 */
#include <stdio.h>
#define fopen  jq_fopen_utf8_impl

/* ── alloca ─────────────────────────────────────────────────────────────── *
 * util.c uses alloca(); MSVC spells it _alloca.
 */
#ifndef alloca
#  include <malloc.h>
#  define alloca  _alloca
#endif

/* ── fstat / fileno (POSIX → MSVC underscore variants) ─────────────────── */
#ifndef fstat
#  include <sys/stat.h>
#  define fstat(fd, sb)   _fstat((fd), (struct _stat *)(sb))
#endif

/* ── S_ISDIR ────────────────────────────────────────────────────────────── *
 * POSIX macro — not in MSVC's sys/stat.h.
 */
#ifndef S_ISDIR
#  ifndef _S_IFDIR
#    include <sys/stat.h>
#  endif
#  define S_ISDIR(m)  (((m) & _S_IFMT) == _S_IFDIR)
#endif

/* ── scalb ──────────────────────────────────────────────────────────────── *
 * BSD math function — not in MSVC.  scalbn() is C99-standard and equivalent.
 */
#ifndef scalb
#  include <math.h>
#  define scalb(x, e)  scalbn((x), (int)(e))
#endif

#ifndef WIN32
#  define WIN32 1   /* jv_thread.h gates its Win32 block on WIN32 */
#endif

#ifndef _WIN32_WINNT
#  define _WIN32_WINNT 0x0600   /* Vista required for InitOnceExecuteOnce */
#endif

#include <windows.h>
#include <errno.h>

/* ── pthread_once ────────────────────────────────────────────────────────── */

typedef INIT_ONCE pthread_once_t;

#define PTHREAD_ONCE_INIT  INIT_ONCE_STATIC_INIT

static BOOL CALLBACK _jq_once_trampoline(PINIT_ONCE io, PVOID param, PVOID *ctx) {
    (void)io; (void)ctx;
    ((void(*)(void))(param))();
    return TRUE;
}

static __inline int pthread_once(pthread_once_t *once, void (*fn)(void)) {
    InitOnceExecuteOnce(once, _jq_once_trampoline, (PVOID)(void *)fn, NULL);
    return 0;
}

/* ── pthread_key_* (TLS slots) ───────────────────────────────────────────── */
/*
 * pthread_key_t is declared as `unsigned long` in jv_thread.h.
 * Win32 DWORD is `unsigned long`, so no cast issues.
 * The three functions are declared in jv_thread.h; we provide the bodies
 * in jq_pthread_win32.c (linked into the libjq static library).
 */

#endif /* _WIN32 && !__MINGW32__ */
