#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])
AC_INIT([action-replay], [1.3], [matthew.jemielity@gmail.com])
AC_CONFIG_SRCDIR([src/object_oriented_programming.c])
AC_CONFIG_FILES([Makefile])

AM_INIT_AUTOMAKE([-Wall])

m4_ifdef([AM_PROG_AR],[AM_PROG_AR])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT64_T
AC_TYPE_UNSIGNED_LONG_LONG_INT
AC_CHECK_TYPES([_Bool])

# Checks for sizes (needed by some headers)
AC_CHECK_SIZEOF([unsigned short int])
AC_CHECK_SIZEOF([signed int])
AC_CHECK_SIZEOF([uint64_t])
AC_CHECK_SIZEOF([unsigned long int])
AC_CHECK_SIZEOF([unsigned long long int])

# Checks for libraries.
AC_CHECK_LIB(pthread, pthread_create, [], [AC_MSG_ERROR([cannot find pthread shared library])])
AC_CHECK_LIB(jsmn, jsmn_init, [], [AC_MSG_ERROR([cannot find jsmn shared library])])
AC_CHECK_LIB(opa, OPA_Queue_init, [], [AC_MSG_ERROR([cannot find OPA (Open Portable Atomics) shared library])])

# Checks for header files.
AC_CHECK_HEADERS([assert.h inttypes.h limits.h stdbool.h stddef.h stdint.h sys/types.h time.h])
AC_CHECK_HEADERS([errno.h fcntl.h jsmn.h linux/input.h linux/types.h opa_primitives.h opa_queue.h poll.h pthread.h stdarg.h stdio.h stdlib.h string.h sys/time.h unistd.h], [], [AC_MSG_ERROR([cannot find or include prerequisite header])])

# Checks for library functions.
AC_CHECK_FUNCS([clock_gettime])
if test "$ac_cv_func_clock_gettime" != yes; then
    AC_CHECK_FUNCS([gettimeofday], [], [AC_MSG_ERROR([neither clock_gettime nor gettimeofday were found])])
fi
AC_CHECK_FUNCS([strndup])
AC_CHECK_FUNCS([fileno strtol strtoul strtoull], [], [AC_MSG_ERROR([cannot find prerequisite method])])

AC_OUTPUT
