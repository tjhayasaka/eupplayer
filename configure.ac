dnl -*-indented-text-*-
dnl Process this file with autoconf to produce a configure script.
dnl $Id: configure.in,v 1.7 2000/04/15 00:01:02 hayasaka Exp $

define(EUPPLAYER_CHECK_LITTLE_ENDIAN,
[AC_MSG_CHECKING([whether the box is little-endian])
AC_CACHE_VAL(eupplayer_cv_little_endian,
[AC_TRY_RUN([
#ifdef STDC_HEADERS
#include <stdlib.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
main() { /* returns 1 if little-endian */
int i=1;
exit(*(char*)(&i));
}],[eupplayer_cv_little_endian=no],[eupplayer_cv_little_endian=yes])])
if test "$eupplayer_cv_little_endian" = yes; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(EUPPLAYER_LITTLE_ENDIAN)
else
  AC_MSG_RESULT(no)
fi
])

dnl ############################

AC_INIT(eupplay.cc)
AC_PROG_MAKE_SET
AM_INIT_AUTOMAKE(eupplayer, 0.009)
AC_PROG_CXX
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)
EUPPLAYER_CHECK_LITTLE_ENDIAN
if test "$ac_cv_prog_gxx" = yes; then
  CXXFLAGS="$CXXFLAGS -Wall -fomit-frame-pointer -funroll-loops"
fi
AC_SUBST(AM_CPPFLAGS)
AC_OUTPUT(Makefile)
