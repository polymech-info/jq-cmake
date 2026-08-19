/*
 * Win32 implementations of the pthread TLS-key functions declared in
 * jv_thread.h but left undefined for non-MinGW Win32 builds.
 *
 * pthread_key_t = unsigned long (same width as DWORD on Win32).
 *
 * Destructor support is intentionally omitted: jv_dtoa_tsd.c registers its
 * own atexit() cleanup (jv_tsd_dtoa_ctx_fini), which is sufficient for
 * embedded, non-multi-threaded use.
 */

#if defined(_WIN32) && !defined(__MINGW32__)

#include "jq_pthread_win32.h"  /* brings in windows.h + pthread_once_t */

/* pthread_key_t is unsigned long; DWORD is unsigned long on Win32. */

int pthread_key_create(unsigned long *key, void (*destructor)(void *)) {
    DWORD slot = TlsAlloc();
    if (slot == TLS_OUT_OF_INDEXES)
        return EAGAIN;
    *key = (unsigned long)slot;
    (void)destructor;  /* see comment above */
    return 0;
}

int pthread_setspecific(unsigned long key, const void *value) {
    return TlsSetValue((DWORD)key, (LPVOID)(uintptr_t)value) ? 0 : EINVAL;
}

void *pthread_getspecific(unsigned long key) {
    return TlsGetValue((DWORD)key);
}

#endif /* _WIN32 && !__MINGW32__ */
