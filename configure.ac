AC_INIT([Ambient], [1.0], [ambient@raindawn.com])
AC_PREREQ([2.69])
AC_LANG([C])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_SRCDIR([src/ambient.h])
AM_INIT_AUTOMAKE
: ${CFLAGS=""}
AC_PROG_CC(gcc)
AC_SUBST(AM_CFLAGS, ["-Wall -Wextra -O0 -g3 -pedantic"])
dnl AX_APPEND_FLAG("-std=gnu11", [AM_CFLAGS])

AX_CHECK_COMPILE_FLAG(-std=gnu11, AX_APPEND_FLAG("-std=gnu11", [AM_CFLAGS]))
dnl AX_CHECK_COMPILE_FLAG([-std=c11], AX_APPEND_FLAG("-std=c11", [AM_CFLAGS]))
dnl AX_CHECK_COMPILE_FLAG([-std=c99], AX_APPEND_FLAG("-std=c99", [AM_CFLAGS]))

#AC_DEFUN([AX_CHECK_STDC],[
#  AX_CHECK_COMPILE_FLAG(-std=gnu11, [AX_APPEND_FLAG("-std=gnu11", [AM_CFLAGS]))],
#  AX_CHECK_COMPILE_FLAG(-std=c11, [AX_APPEND_FLAG("-std=c11",   [AM_CFLAGS]])),
#  AX_CHECK_COMPILE_FLAG(-std=c99, [AX_APPEND_FLAG("-std=c99",   [AM_CFLAGS]])),
#    [AC_MSG_ERROR([C compiler needs at least C99 capability!])]])

#AX_CHECK_STDC

AC_CONFIG_HEADERS([config.h])
AC_CHECK_HEADERS([stdio.h stdlib.h string.h unistd.h time.h errno.h dirent.h libudev.h linux/input.h\
                  linux/hidraw.h sys/ioctl.h fcntl.h pthread.h sys/types.h sys/stat.h ])
# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
# Checks for library functions.
AC_CHECK_FUNCS([bzero select strerror strrchr strstr])
AC_CHECK_HEADER_STDBOOL 

AC_SEARCH_LIBS([pthread_create], [pthread], [], [
  AC_MSG_ERROR([unable to find pthread_create()])
])
AC_SEARCH_LIBS([udev_new], [udev], [], [
  AC_MSG_ERROR([unable to find udev_new()])
])
AC_SEARCH_LIBS([udev_enumerate_new], [udev], [], [
  AC_MSG_ERROR([unable to find udev_enumerate_new()])
])
AC_SEARCH_LIBS([udev_enumerate_add_match_subsystem], [udev], [], [
  AC_MSG_ERROR([unable to find udev_enumerate_add_match_subsystem()])
])
AC_SEARCH_LIBS([udev_enumerate_scan_devices], [udev], [], [
  AC_MSG_ERROR([unable to find udev_enumerate_scan_devices()])
])
AC_SEARCH_LIBS([udev_enumerate_get_list_entry], [udev], [], [
  AC_MSG_ERROR([unable to find udev_(enumerate_get_list_entry)])
])
AC_SEARCH_LIBS([udev_list_entry_get_name], [udev], [], [
  AC_MSG_ERROR([unable to find udev_list_entry_get_name)])
])
AC_SEARCH_LIBS([udev_device_new_from_syspath], [udev], [], [
  AC_MSG_ERROR([unable to find udev_device_new_from_syspath)])
])
AC_SEARCH_LIBS([udev_device_unref], [udev], [], [
  AC_MSG_ERROR([unable to find udev_device_unref()])
])
AC_SEARCH_LIBS([udev_enumerate_unref], [udev], [], [
  AC_MSG_ERROR([unable to find udev_()])
])
AC_SEARCH_LIBS([udev_enumerate_unref], [udev], [], [
  AC_MSG_ERROR([unable to find udev_()])
])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_PROG_MAKE_SET
AC_OUTPUT

AC_MSG_NOTICE( Success!)
AC_MSG_NOTICE( Now run make)
