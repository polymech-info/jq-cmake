/*
 * pthread.h stub for Win32/MSVC.
 * Routes to the compatibility shim that provides pthread_mutex_t,
 * pthread_once_t, and pthread_key_* via Win32 primitives.
 */
#pragma once
#include "jq_pthread_win32.h"
/* jv_thread.h provides pthread_mutex_t; include it for mutex types. */
#include "jv_thread.h"
