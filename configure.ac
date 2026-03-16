AC_INIT([jamb],[version-0.1-pre])
m4_include([macros/ax_cxx_check_lib.m4])

AC_LANG(C++)
AC_PROG_CXX

AC_SEARCH_LIBS(cubeb_init, [cubeb])
AX_CXX_CHECK_LIB(tag, [TagLib::FileRef])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
