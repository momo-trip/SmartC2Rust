dnl configure.ac for gv

dnl Time-stamp: "07/06/28 15:57:39 jemarch"

dnl Copyright (C) 2004 José E. Marchesi

dnl Please process this file with `autoconf' to get a configure script


AC_INIT(src/ps.c)
AM_INIT_AUTOMAKE(gv,3.7.4)
AM_CONFIG_HEADER(src/ac_config.h)
dnl AM_CONFIG_HEADER(src/ac_config.h)

AM_MAINTAINER_MODE

dnl gv version
VERSION=3.7.4
dnl manual edition
MANUALEDITION=First

AC_SUBST(VERSION)
AC_SUBST(MANUALEDITION)

AC_PROG_MKDIR_P

dnl DEFUNS

AC_DEFUN([AC_XT_VERSION6],
[AC_MSG_CHECKING([For Xt Revision > 4])
AC_CACHE_VAL(XT_cv_version,[
  AC_TRY_LINK([#include <X11/Intrinsic.h>],
[#if XtSpecificationRelease < 5
fail;
#endif
], XT_cv_version="yes",
   [AC_MSG_RESULT(no)])
])
])


dnl Check for programs
AC_PROG_CC
AC_PROG_RANLIB
gl_EARLY
gl_INIT

dnl Search for libraries

LIBS="$X_LIBS"

AC_CHECK_LIB(m, sqrt)
AC_CHECK_LIB(fl, yywrap)

# Needed e. g. on Mac OS
AC_CHECK_LIB(iconv, iconv_open)

AC_ARG_WITH(libzio, AC_HELP_STRING([--without-libzio],[Do not probe for libzio]), opt_zio=$withval, opt_zio=yes)

if test "$opt_zio" != "no" ; then
#
# Check for zlib and libbz2 libraries to use this together
# with SuSE's libzio to open compressed files.
#
savedLIBS=$LIBS
AC_CHECK_HEADER(zlib.h,[
  for lib in z gz
  do
    AC_CHECK_LIB($lib, gzopen, [LIBS="$LIBS -l$lib" ; break])
  done])
AC_CHECK_HEADER(bzlib.h,[
  for lib in bz2 bzip2
  do
    AC_CHECK_LIB($lib, BZ2_bzopen, [LIBS="$LIBS -l$lib" ; break])
  done])
AC_CHECK_HEADER(zio.h,[
    AC_CHECK_LIB(zio, fzopen, [LIBS="$LIBS -lzio" ; AC_DEFINE(HAVE_ZIO, 1, [Have ZIO Library])],[LIBS=$savedLIBS])
  ],[LIBS=$savedLIBS])
fi


dnl WITH options

AC_ARG_WITH(default-papersize, AC_HELP_STRING([--with-default-papersize=PSIZE],[The default papersize (can be changed at runtime). Defaults to `A4']), opt_default_papersize=$withval, opt_default_papersize=A4)

test "$opt_default_papersize" = "yes" && {

     opt_default_papersize=A4

}

PAPERSIZE=$opt_default_papersize
AC_SUBST(PAPERSIZE)

AC_ARG_WITH(print-command, AC_HELP_STRING([--with-print-command=COMMAND],[Command for spool a print job. Defaults for `lpr']), opt_print_command=$withval, opt_print_command=lpr)

test "$opt_print_command" = "yes" && {

     opt_print_command=\"lpr\"

}

PRINT_COMMAND=$opt_print_command
AC_SUBST(PRINT_COMMAND)

AC_ARG_WITH(user-defaults-file, AC_HELP_STRING([--with-user-defaults-file=PATH],[The file containing your preferences. It defaults to `~/.gv'.]), opt_user_defaults_file=$withval, opt_user_defaults_file="~/.gv")

test "$opt_user_defaults_file" = "yes" && {

     opt_user_defaults_file="~/.gv"

}

AC_DEFINE_UNQUOTED(USER_DEFAULTS,"$opt_user_defaults_file", "Define USER_DEFAULTS")

AC_ARG_WITH(scratch-dir,AC_HELP_STRING([--with-scratch-dir=PATH],[Temporary directory. Must be terminated with a trailing `/'. Defaults to `/tmp/']), opt_scratch_dir=$withval, opt_scratch_dir="/tmp/")

test "$opt_scratch_dir" = "yes" && {

     opt_scratch_dir="/tmp/"

}

SCRATCH_DIR=$opt_scratch_dir
AC_SUBST(SCRATCH_DIR)

AC_ARG_WITH(save-dir,AC_HELP_STRING([--with-save-dir=PATH],[The default directory for saving. Must be terminated with a trailing `/'. Defaults to `~/']), opt_save_dir=$withval, opt_save_dir="~/")

test "$opt_save_dir" = "yes" && {

     opt_save_dir="~/"

}

SAVE_DIR=$opt_save_dir
AC_SUBST(SAVE_DIR)


dnl ENABLE options

AC_ARG_ENABLE(ps-level-one, AC_HELP_STRING([--enable-ps-level-one],[Produce Level 1 PostScript]), opt_ps_level_one=$enableval, opt_ps_level_one=no)

test "$opt_ps_level_one" = "yes" && {

     AC_DEFINE(PSLevel1,[], [Define PRODUCE_PS_LEVEL_ONE])     

}

dnl TESTED
AC_ARG_ENABLE(signal-handler, AC_HELP_STRING([--enable-signal-handler],[Use a signal handler intercepting SIGTERM,SIGQUIT, SIGHUP snd SIGINT signals. It requires X11R6. Defaults to `yes']), opt_signal_handler=$enableval, opt_signal_handler=yes)

test "x$opt_signal_handler" = "xyes" && {

     AC_DEFINE(USE_SIGNAL_HANDLER,[],[Define USE_SIGNAL_HANDLER])

}

AC_ARG_ENABLE(backing-pixmap, AC_HELP_STRING([--enable-backing-pixmap],[Use a backing pixmap and not rely on the XServer to maintain backing store (can be changed at runtime). Defaults to `yes']), opt_backing_pixmap=$enableval, opt_backing_pixmap=yes)

test "$opt_backing_pixmap" = "yes" && {

dnl     AC_DEFINE(USE_BACKING_PIXMAP, True, [Define USE_BACKING_PIXMAP])
	USE_BACKING_PIXMAP=True

}

test "$opt_backing_pixmap" = "no" && {

     USE_BACKING_PIXMAP=False

}

AC_SUBST(USE_BACKING_PIXMAP)

AC_ARG_ENABLE(scrollbar-code, AC_HELP_STRING([--enable-scrollbar-code],[Use gv's own Motif like scrollbar. It defaults to `yes']), 
[case "${enableval}" in
   yes) opt_scrollbar_code=true ;;
   no)  opt_scrollbar_code=false ;;
   *) AC_MSG_ERROR([bad value ${enableval} for --enable-scrollbar-code]) ;;
 esac], [opt_scrollbar_code=true])
 
AM_CONDITIONAL(USE_SCROLLBAR_CODE, test x$opt_scrollbar_code = xtrue)

AC_ARG_ENABLE(memmove-code, AC_HELP_STRING([--enable-memmove-code],[Use gv's own memmove. It defaults to `yes']), opt_memmove_code=$enableval, opt_memmove_code=yes)

test "$opt_memmove_code" = "yes" && {

     AC_DEFINE(USE_MEMMOVE_CODE,[],[Define USE_MEMMOVE_CODE])

}

AC_ARG_ENABLE(runtime-messages, AC_HELP_STRING([--enable-runtime-messages],[Generate informative messages at runtime. It defaults to `no']), opt_runtime_messages=$enableval, opt_runtime_messages=no)

test "$opt_runtime_messages" = "yes" && {

     AC_DEFINE(MESSAGES,[],[Define MESSAGES])
     AC_DEFINE(MESSAGE_NO_ESC,[],[Define MESSAGE_NO_ESC])

}

AC_ARG_ENABLE(fallback-styles, AC_HELP_STRING([--enable-fallback-styles],[Compile X resource files into the executable. Defaults to `yes']), opt_fallback_styles=$enableval, opt_fallback_styles=yes)

test "$opt_fallback_styles" = "yes" && {

dnl     USE_FALLBACK_STYLES=-DUSE_FALLBACK_STYLES
     AC_DEFINE(USE_FALLBACK_STYLES,[],[Define USE_FALLBACK_STYLES])

}
AM_CONDITIONAL(ENABLE_FALLBACK_STYLES, test x$opt_fallback_styles = xyes)

AC_ARG_ENABLE(memmove-code, AC_HELP_STRING([--enable-memmove-code],[Use gv's own memmove. It defaults to `yes']), opt_memmove_code=$enableval, opt_memmove_code=yes)

test "$opt_memmove_code" = "yes" && {

     AC_DEFINE(USE_MEMMOVE_CODE,[],[Define USE_MEMMOVE_CODE])

}

AC_ARG_ENABLE(international, AC_HELP_STRING([--enable-international],[Enable international by default. It defaults to `no']), opt_international=$enableval, opt_international=no)
AM_CONDITIONAL(ENABLE_INTERNATIONAL, test x$opt_international = xyes)


AC_MSG_CHECKING([for SIGCLD])
AC_EGREP_CPP(we_have_sigcld,
[#include <signal.h>
#ifdef SIGCLD
we_have_sigcld
#endif
], AC_DEFINE([HAVE_SIGCLD], [1], [Define to 1 if you have the sigcld signal.]) haveSIGCLD=yes, haveSIGCLD=no)
AC_MSG_RESULT([$haveSIGCLD])

AC_MSG_CHECKING([for SIGCHLD])
AC_EGREP_CPP(we_have_sigchld,
[#include <signal.h>
#ifdef SIGCHLD
we_have_sigchld
#endif
], AC_DEFINE([HAVE_SIGCHLD], [1], [Define to 1 if you have the sigchld signal.]) haveSIGCHLD=yes, haveSIGCHLD=no)
AC_MSG_RESULT([$haveSIGCHLD])

AC_ARG_ENABLE(SIGCHLD-fallback, AC_HELP_STRING([--enable-SIGCHLD-fallback],[Use SIGCHLD as a fallback to SIGCLD. Defaults to 'no']),
   [case "${enableval}" in
      yes) opt_sigchld=true ;;
      no)  opt_sigchld=false ;;
      *) AC_MSG_ERROR(bad value ${enableval} for --enable-SIGCHLD-fallback) ;;
esac],[opt_sigchld=false])

if test "x$haveSIGCLD" = "xno"
then
   if test x$opt_sigchld = xfalse
   then
      AC_ERROR([SIGCLD is needed. You can try --enable-SIGCHLD-fallback. It's secure on MacOS X, but on FreeBSD the fallback creates zombies.])
   else
      AC_DEFINE([SIGCLD], [SIGCHLD], [Use SIGCHLD as fallback for SIGCLD.])
   fi
fi



opt_setenv_code=unknown

AC_ARG_ENABLE(setenv-code, AC_HELP_STRING([--enable-setenv-code],[Use extra code for setenv() and getenv(). Defaults to `no']),
[case "${enableval}" in
   yes) opt_setenv_code=true ;;
   no)  opt_setenv_code=false ;;
   *) AC_MSG_ERROR(bad value ${enableval} for --enable-setenv-code) ;;
 esac],[opt_setenv_code=unknown])
 
if test x$opt_setenv_code = xunknown
then
   AC_CHECK_FUNCS([setenv unsetenv],[],[AC_MSG_ERROR(The configure option --disable-setenv-code or --enable-setenv-code should help.)])
fi

AM_CONDITIONAL(USE_SETENV_CODE, test x$opt_setenv_code = xtrue)



AC_ARG_ENABLE(off_t, AC_HELP_STRING([--enable-off_t],[Use code with off_t and fopen, ftello, fseeko, stat for potential large file support. It defaults to auto detection. largefile only has effects if this option is turned on.]),
 [case "${enableval}" in
   yes) opt_off_t=true ;;
   no)  opt_off_t=false ;;
   *) AC_MSG_ERROR(bad value ${enableval} for --enable-off_t) ;;
 esac],[
          AC_CHECK_FUNCS([ftello fseeko],
                [opt_off_t=true],
                [opt_off_t=false])
       ])




test "$opt_off_t" = "true" && {
     AC_SYS_LARGEFILE
     AC_FUNC_FSEEKO
     echo Enabling LFS.
     AC_DEFINE(HAVE_OFF_T, [], [Use fopen, ftello, fseeko, ...])
}

AM_CONDITIONAL(USE_OFF_T, test x$opt_off_t = xtrue)


#################### X11 related stuff ######################


AC_PATH_XTRA()
AC_CHECK_LIB(X11, XOpenDisplay, , , $X_LIBS)
AC_CHECK_LIB(ICE, main, , , $X_LIBS)
AC_CHECK_LIB(SM, main, , , $X_LIBS)
AC_CHECK_LIB(Xext, main, , , $X_LIBS)
AC_CHECK_LIB(Xt, XtOpenDisplay, , , $X_LIBS)
AC_CHECK_LIB(Xmu, main, , , $X_LIBS)
AC_CHECK_LIB(Xpm, main, , , $X_LIBS)

PKG_CHECK_MODULES([DEPS], [xaw3d], [true], [DEPS_CFLAGS=""; DEPS_LIB=""])

AC_CHECK_LIB([Xaw3d], XawInitializeWidgetSet,,
   AC_ERROR([Unable to find a valid Xaw3d library. Please install it and rerun configure.]),
   $X_LIBS)

CFLAGS="$CFLAGS $DEPS_CFLAGS"
LIBS="$LIBS $DEPS_LIBS"

myCPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CPPFLAGS $X_CFLAGS"
## AC_CHECK_MEMBER(SimplePart.international,,[AC_MSG_ERROR([Xaw3d misses internationalisation support])],[#include <X11/Xaw3d/SimpleP.h>])
AC_CHECK_MEMBER(SimplePart.international,[opt_xaw3d_international=true],[opt_xaw3d_international=false],[#include <X11/Xaw3d/SimpleP.h>])
test x$opt_xaw3d_international = xtrue && {
     AC_DEFINE(HAVE_XAW3D_INTERNATIONAL,[],[Define USE_XAW3D_INTERNATIONAL])
}
test x$opt_xaw3d_international = xfalse && {
     echo
     echo "****************************************************"
     echo "WARNING: international support has not been detected"
     echo "         in your version of Xaw3d.  The UTF8-Support"
     echo "         of GNU gv has been disabled."
     echo "****************************************************"
     echo
}
CPPFLAGS="$myCPPFLAGS"


AC_CHECK_LIB(Xinerama, main, , , $X_LIBS)


#################### END: X11 related stuff ######################



dnl Generate output files
AC_OUTPUT(Makefile lib/Makefile src/Makefile doc/Makefile)
