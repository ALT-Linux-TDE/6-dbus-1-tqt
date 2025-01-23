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


##### set project version ########################

include( TDEVersion )
cmake_minimum_required( VERSION ${TDE_CMAKE_MINIMUM_VERSION} )
tde_set_project_version( )


##### general package setup #####################

project( dbus-1-tqt )


##### include essential cmake modules ###########

include( FindPkgConfig )
include( CheckCXXSourceCompiles )
include( CheckSymbolExists )

include( TDEMacros )


##### optional stuff

option( WITH_ALL_OPTIONS "Enable all optional support" OFF )
option( WITH_GCC_VISIBILITY "Enable fvisibility and fvisibility-inlines-hidden" ${WITH_ALL_OPTIONS} )


##### configure checks ##########################

include( ConfigureChecks.cmake )


##### install paths setup #######################

tde_setup_install_path( EXEC_INSTALL_PREFIX   "${CMAKE_INSTALL_PREFIX}" )
tde_setup_install_path( BIN_INSTALL_DIR       "${EXEC_INSTALL_PREFIX}/bin"  )
tde_setup_install_path( LIB_INSTALL_DIR       "${EXEC_INSTALL_PREFIX}/lib${LIB_SUFFIX}" )
tde_setup_install_path( INCLUDE_INSTALL_DIR   "${CMAKE_INSTALL_PREFIX}/include/${CMAKE_PROJECT_NAME}" )
tde_setup_install_path( PKGCONFIG_INSTALL_DIR "${LIB_INSTALL_DIR}/pkgconfig" )


##### write pkgconfig file ######################

string( REGEX REPLACE "^${CMAKE_INSTALL_PREFIX}" "\${prefix}" PC_EXEC_PREFIX ${EXEC_INSTALL_PREFIX} )
string( REGEX REPLACE "^${CMAKE_INSTALL_PREFIX}" "\${prefix}" PC_INCLUDE_DIR ${INCLUDE_INSTALL_DIR} )
string( REGEX REPLACE "^${CMAKE_INSTALL_PREFIX}" "\${prefix}" PC_LIB_DIR ${LIB_INSTALL_DIR} )
configure_file( dbus-1-tqt.pc.cmake dbus-1-tqt.pc @ONLY )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/dbus-1-tqt.pc DESTINATION ${PKGCONFIG_INSTALL_DIR} )


##### build setup ###############################

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined" )

add_definitions(
  -DDBUS_COMPILATION
  -DTQT_THREAD_SUPPORT
  ${TQT_CFLAGS_OTHER}
)

if( CMAKE_APIDOX_HTML_DIRECTORY )
	set( APIDOX_HTML_DIRECTORY "${CMAKE_APIDOX_HTML_DIRECTORY}" )
else()
	set( APIDOX_HTML_DIRECTORY "/usr/share/doc/lib${PROJECT_NAME}/HTML" )
endif()


##### source directories ########################

add_subdirectory( src )


##### write configure files #####################

configure_file( ${CMAKE_SOURCE_DIR}/dbus-1-tqt.Doxyfile.cmake ${CMAKE_BINARY_DIR}/dbus-1-tqt.Doxyfile )

