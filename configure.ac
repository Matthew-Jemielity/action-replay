#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([action-replay], [0.9], [matthew.jemielity@gmail.com])
AC_CONFIG_SRCDIR([src/object_oriented_programming.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE([-Wall])
m4_ifdef([AM_PROG_AR],[AM_PROG_AR])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create)
AC_CHECK_LIB(jsmn, jsmn_init, [], [AC_MSG_ERROR([cannot find jsmn shared library])])
AC_CHECK_LIB(opa, OPA_Queue_init, [], [AC_MSG_ERROR([cannot find OPA (Open Portable Atomics) shared library])])

# Checks for header files.
AC_CHECK_HEADERS([assert.h errno.h fcntl.h inttypes.h jsmn.h limits.h linux/input.h opa_primitives.h opa_queue.h poll.h pthread.h stdarg.h stdbool.h stddef.h stdint.h stdio.h stdlib.h string.h sys/time.h sys/types.h time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_HEADER_TIME
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT64_T
AC_TYPE_UNSIGNED_LONG_LONG_INT

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime gettimeofday strndup strtol strtoul strtoull])

AC_OUTPUT
