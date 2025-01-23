prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=@PC_EXEC_PREFIX@
libdir=@PC_LIB_DIR@
includedir=@PC_INCLUDE_DIR@

Name: dbus-tqt
Description: DBUS bindings for the Trinity Qt [TQt] interface
Version: 0.9.0
Libs: -L${libdir} -ldbus-1-tqt
Cflags: -I${includedir}
