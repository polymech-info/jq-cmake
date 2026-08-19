/*
 * Minimal unistd.h stub for Win32/MSVC.
 * Maps the POSIX names used by jq to their MSVC equivalents.
 */
#pragma once

#ifndef _WIN32_UNISTD_H_STUB
#define _WIN32_UNISTD_H_STUB

#include <io.h>        /* _isatty, _fileno, _close, _read, _write */
#include <process.h>   /* _getpid */
#include <direct.h>    /* _getcwd */
#include <stdlib.h>    /* _putenv */

/* Map POSIX names → MSVC underscore variants */
#ifndef isatty
#  define isatty(fd)     _isatty(fd)
#endif
#ifndef fileno
#  define fileno(f)      _fileno(f)
#endif
#ifndef close
#  define close(fd)      _close(fd)
#endif
#ifndef read
#  define read(fd,b,n)   _read((fd),(b),(unsigned)(n))
#endif
#ifndef write
#  define write(fd,b,n)  _write((fd),(b),(unsigned)(n))
#endif
#ifndef getpid
#  define getpid()       _getpid()
#endif
#ifndef getcwd
#  define getcwd(b,n)    _getcwd((b),(int)(n))
#endif
#ifndef unlink
#  define unlink(p)      _unlink(p)
#endif
#ifndef access
#  define access(p,m)    _access((p),(m))
#endif

/* These don't have direct MSVC equivalents; provide no-op / always-fail stubs
   for symbols that are guarded by HAVE_* and won't be called. */
#ifndef STDIN_FILENO
#  define STDIN_FILENO  0
#  define STDOUT_FILENO 1
#  define STDERR_FILENO 2
#endif

#ifndef F_OK
#  define F_OK 0
#  define R_OK 4
#  define W_OK 2
#  define X_OK 0
#endif

/* MSVC doesn't define PATH_MAX; use MAX_PATH from windows.h */
#ifndef PATH_MAX
#  include <windows.h>
#  define PATH_MAX MAX_PATH
#endif

#endif /* _WIN32_UNISTD_H_STUB */
