/*
 * libgen.h stub for Win32/MSVC.
 * Provides dirname() and basename() compatible with jq's linker.c.
 * linker.c uses WIN32 shlwapi path functions when WIN32 is defined,
 * so these stubs only need to compile (they will not be called).
 */
#pragma once

#ifndef _WIN32_LIBGEN_H_STUB
#define _WIN32_LIBGEN_H_STUB

#include <string.h>
#include <stdlib.h>

/* POSIX: modify the string in place, return pointer into it.
   These simple implementations cover jq's usage patterns.
   Note: POSIX specifies these may modify their argument. */

static __inline char *dirname(char *path) {
    if (!path || !*path) return (char *)".";
    char *last_sep = NULL;
    char *p = path;
    while (*p) {
        if (*p == '/' || *p == '\\') last_sep = p;
        p++;
    }
    if (!last_sep) return (char *)".";
    if (last_sep == path) { last_sep[1] = '\0'; return path; }
    *last_sep = '\0';
    return path;
}

static __inline char *basename(char *path) {
    if (!path || !*path) return (char *)".";
    char *last_sep = path;
    char *p = path;
    while (*p) {
        if (*p == '/' || *p == '\\') last_sep = p + 1;
        p++;
    }
    return last_sep;
}

#endif /* _WIN32_LIBGEN_H_STUB */
