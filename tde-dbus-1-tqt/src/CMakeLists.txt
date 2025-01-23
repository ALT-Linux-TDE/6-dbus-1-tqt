#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TQT_INCLUDE_DIRS}
  ${DBUS_INCLUDE_DIRS}
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)


##### install headers ###########################

install( FILES
    tqdbuserror.h tqdbusmessage.h tqdbusconnection.h
    tqdbusvariant.h tqdbusobject.h tqdbusproxy.h
    tqdbusmacros.h tqdbusdata.h tqdbusdatalist.h
    tqdbusdatamap.h tqdbusobjectpath.h tqdbusunixfd.h
    tqdbusdataconverter.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### dbus-1-tqt (shared) #########################

tde_add_library( dbus-1-tqt SHARED AUTOMOC
  SOURCES ${dbus_tqt_MOCS}
    tqdbusconnection.cpp tqdbuserror.cpp tqdbusintegrator.cpp
    tqdbusmarshall.cpp tqdbusmessage.cpp tqdbusserver.cpp
    tqdbusproxy.cpp tqdbusdata.cpp tqdbusdatalist.cpp
    tqdbusobjectpath.cpp tqdbusunixfd.cpp
    tqdbusdataconverter.cpp
  VERSION 0.0.0
  LINK ${TQT_LIBRARIES} ${DBUS_LIBRARIES}
  DESTINATION ${LIB_INSTALL_DIR}
)


##### dbusxml2qt3 (executable) ##################

tde_add_executable( dbusxml2qt3
  SOURCES
    tools/dbusxml2qt3/classgen.cpp
    tools/dbusxml2qt3/main.cpp
    tools/dbusxml2qt3/methodgen.cpp
  LINK ${TQT_LIBRARIES}
  DESTINATION ${BIN_INSTALL_DIR}
)


##### add apidox targets ############

add_custom_target( apidox
  COMMAND doxygen ${CMAKE_BINARY_DIR}/dbus-1-tqt.Doxyfile
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target( install-apidox
  COMMAND ${CMAKE_COMMAND} -E echo Apidox installed in ${APIDOX_HTML_DIRECTORY}
)

install( CODE "
  if( EXISTS ${CMAKE_BINARY_DIR}/apidocs )
    file(
      INSTALL ${CMAKE_BINARY_DIR}/apidocs/
      DESTINATION ${APIDOX_HTML_DIRECTORY}/en
    )
  endif()"
)
