AC_INIT([jamb],[version-0.1-pre])
dnl m4_include([macros/ax_cxx_check_lib.m4])
AC_CONFIG_MACRO_DIR([m4])

dnl AC_PROG_CXX

AC_PROG_CC

AC_SEARCH_LIBS([pow], [m], [],
   [AC_MSG_ERROR([math.h is required])], [])

AC_SEARCH_LIBS([_cstr_init], [stc], [],
   [AC_MSG_ERROR([stc is required])], [])

PKG_CHECK_MODULES([glib], [glib-2.0 >= 2.62.0])
PKG_CHECK_MODULES([json], [json-c])

AC_CHECK_HEADERS([stc/cstr.h], [],
		 [AC_MSG_ERROR(STC is required)]) dnl todo compress all this

AC_CHECK_HEADERS([sys/types.h], [],
		 [AC_MSG_ERROR(sys/types.h is required)])

AC_CHECK_HEADERS([sys/stat.h], [],
		 [AC_MSG_ERROR(sys/stat.h is required)])

AC_CHECK_HEADERS([dirent.h], [],
		 [AC_MSG_ERROR(dirent.h is required)])
dnl AC_SEARCH_LIBS(cubeb_init, [cubeb])
dnl AC_SEARCH_LIBS(_ZNK6TagLib10FileStream4nameEv, [tag])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
