cmake_minimum_required(VERSION 3.16)
project(libjq C)

# ── Generated files ──────────────────────────────────────────────────────────
#
# builtin.c includes "src/builtin.inc" — the jq-coded built-in library
# (builtin.jq) as a C byte-array literal.  We generate it here at configure
# time from the vendored builtin.jq so that flex/bison/od are not required.
#
# The include resolves as <packages/jq>/src/builtin.inc because we add
# CMAKE_CURRENT_SOURCE_DIR (= packages/jq) to the include path below.

set(_jq_builtin_inc_path "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin.inc")

if(NOT EXISTS "${_jq_builtin_inc_path}")
    message(STATUS "libjq: generating src/builtin.inc from src/builtin.jq")
    file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/builtin.jq" _jq_builtin_hex HEX)
    # Convert hex pairs → comma-separated 0xNN C literals
    string(REGEX REPLACE "([0-9a-fA-F][0-9a-fA-F])" "0x\\1," _jq_builtin_content "${_jq_builtin_hex}")
    file(WRITE "${_jq_builtin_inc_path}" "${_jq_builtin_content}")
endif()

# version.h — only used by main.c which we do NOT compile.  Write a stub so
# that any accidental inclusion does not fail.
set(_jq_version_h "${CMAKE_CURRENT_SOURCE_DIR}/src/version.h")
if(NOT EXISTS "${_jq_version_h}")
    file(WRITE "${_jq_version_h}" "#define JQ_VERSION \"1.8.0\"\n")
endif()

# ── Library sources ──────────────────────────────────────────────────────────
# Mirrors LIBJQ_SRC in Makefile.am, minus main.c, jq_test.c, inject_errors.c.

set(_jq_sources
    src/builtin.c
    src/bytecode.c
    src/compile.c
    src/execute.c
    src/jv.c
    src/jv_alloc.c
    src/jv_aux.c
    src/jv_dtoa.c
    src/jv_dtoa_tsd.c
    src/jv_file.c
    src/jv_parse.c
    src/jv_print.c
    src/jv_unicode.c
    src/lexer.c
    src/linker.c
    src/locfile.c
    src/parser.c
    src/util.c
)

if(WIN32 AND NOT MINGW)
    list(APPEND _jq_sources win32/jq_pthread_win32.c)
endif()

add_library(jq STATIC ${_jq_sources})

# ── Include directories ──────────────────────────────────────────────────────
# PUBLIC: consumers need jq.h (the C API header).
# The root packages/jq dir is needed for `#include "src/builtin.inc"` in
# builtin.c (the include path is relative to the project root, not to src/).
target_include_directories(jq
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}          # resolves "src/builtin.inc"
)

if(WIN32 AND NOT MINGW)
    # win32/ shadows missing POSIX headers (unistd.h, sys/time.h) and provides
    # the pthread compat shim.  Must come before system includes.
    target_include_directories(jq BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/win32
    )
endif()

# ── Compile definitions ──────────────────────────────────────────────────────
#
# HAVE_LIBONIG=0   — Oniguruma submodule is not populated; regex builtins
#                    (test/match/scan/sub/gsub) are compiled out cleanly via
#                    #ifdef HAVE_LIBONIG in builtin.c.
# USE_DECNUM       — omitted: double precision is sufficient for xblox JSON
#                    filters; avoids vendoring decNumber.
# WIN32            — required so jv_thread.h activates its Win32 mutex shim.
# HAVE_MEMMEM      — not defined on MSVC; _jq_memmem() has a built-in fallback.
# HAVE__ISATTY     — MSVC has _isatty(); used by jv_file.c for terminal detect.
# Common math      — define HAVE_* for all functions available in MSVC's math.h.

target_compile_definitions(jq PRIVATE
    $<$<NOT:$<BOOL:${HAVE_LIBONIG}>>:>   # sentinel — HAVE_LIBONIG deliberately absent
    $<$<BOOL:${WIN32}>:WIN32=1>
    $<$<AND:$<BOOL:${WIN32}>,$<NOT:$<BOOL:${MINGW}>>>:HAVE__ISATTY=1>
    # Standard C math functions present in MSVC
    HAVE_ACOS=1   HAVE_ACOSH=1  HAVE_ASIN=1   HAVE_ASINH=1
    HAVE_ATAN=1   HAVE_ATAN2=1  HAVE_ATANH=1  HAVE_CBRT=1
    HAVE_CEIL=1   HAVE_COS=1    HAVE_COSH=1   HAVE_EXP=1
    HAVE_EXP2=1   HAVE_FABS=1   HAVE_FDIM=1   HAVE_FLOOR=1
    HAVE_FMA=1    HAVE_FMAX=1   HAVE_FMIN=1   HAVE_FMOD=1
    HAVE_FREXP=1  HAVE_HYPOT=1  HAVE_LDEXP=1  HAVE_LGAMMA=1
    HAVE_LOG=1    HAVE_LOG10=1  HAVE_LOG1P=1  HAVE_LOG2=1
    HAVE_LOGB=1   HAVE_MODF=1   HAVE_NEARBYINT=1 HAVE_NEXTAFTER=1
    HAVE_POW=1    HAVE_REMAINDER=1 HAVE_RINT=1 HAVE_ROUND=1
    HAVE_SCALBLN=1 HAVE_SCALBN=1 HAVE_ILOGB=1
    # scalb: not in MSVC — mapped to scalbn() via force-include jq_pthread_win32.h
    HAVE_SIN=1    HAVE_SINH=1   HAVE_SQRT=1   HAVE_TAN=1
    HAVE_TANH=1   HAVE_TGAMMA=1 HAVE_TRUNC=1  HAVE_COPYSIGN=1
    HAVE_EXPM1=1  HAVE_EXP2=1
    # MSVC does not have: drem, erf, erfc, exp10, gamma, j0/j1/jn,
    # lgamma_r, nexttoward, significand, y0/y1/yn — leave undefined.
    # MSVC does not have: strptime, timegm, gmtime_r, localtime_r,
    # gettimeofday, arc4random, getentropy, setenv — leave undefined.
    # jq's util.c provides portable fallbacks for most of these.
    HAVE_GMTIME=1
    HAVE_LOCALTIME=1
    HAVE_STRFTIME=1
    HAVE_SETLOCALE=1
    HAVE_ISATTY=0    # use _isatty on MSVC; declared above via HAVE__ISATTY
    # jv_dtoa.c requires exactly one IEEE/VAX/IBM endianness flag.
    # x86/x64 (little-endian IEEE) → IEEE_8087.
    IEEE_8087=1
)

# ── MSVC-specific flags ───────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(jq PRIVATE
        /TC              # treat all .c files as C (not C++)
        /W1              # minimal warnings for vendored code
        /wd4996          # suppress "deprecated POSIX names"
        /wd4267          # suppress size_t → int conversion warnings
        /wd4244          # suppress int ↔ float implicit conversions
        /wd4018          # suppress signed/unsigned comparison warnings
        /wd4146          # suppress negation of unsigned type
        /wd4305          # suppress truncation from double to float
        /wd4101          # suppress unreferenced local variable
        /wd4013          # suppress "undefined; assuming extern returning int"
        # Force-include the Win32 pthread extension header before every unit.
        # This injects pthread_once_t / PTHREAD_ONCE_INIT / pthread_once()
        # which jv_thread.h omits for non-MinGW Win32 builds.
        "/FI${CMAKE_CURRENT_SOURCE_DIR}/win32/jq_pthread_win32.h"
    )
    # jq requires C99 or later; /std:c11 gives us inline + designated inits.
    target_compile_options(jq PRIVATE /std:c11)
endif()

# ── Link libraries ────────────────────────────────────────────────────────────
if(WIN32)
    # shlwapi is used internally by jq's linker.c path routines.
    # PRIVATE prevents it from propagating to consumers (pm-media already
    # links shlwapi through its own dependency chain).
    target_link_libraries(jq PRIVATE shlwapi)
endif()

set_target_properties(jq PROPERTIES FOLDER "Vendored")
