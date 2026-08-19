/*
 * Minimal sys/time.h stub for Win32/MSVC.
 * jq only uses gettimeofday() behind #ifdef HAVE_GETTIMEOFDAY (which we do
 * not define), but it always includes this header unconditionally in builtin.c.
 * We just need the types so the translation unit parses cleanly.
 */
#pragma once

#ifndef _WIN32_SYS_TIME_H_STUB
#define _WIN32_SYS_TIME_H_STUB

#include <time.h>

/* timeval is already defined by winsock.h/winsock2.h when windows.h is
   included.  Only define the stub if neither winsock header has been pulled. */
#if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_) && !defined(_TIMEVAL_DEFINED)
#define _TIMEVAL_DEFINED
struct timeval {
    long tv_sec;
    long tv_usec;
};
#endif

struct timezone {
    int tz_minuteswest;
    int tz_dsttime;
};

#endif /* _WIN32_SYS_TIME_H_STUB */
