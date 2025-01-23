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

##### setup architecture flags ##################

tde_setup_architecture_flags( )

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

tde_setup_largefiles( )


# dbus-1
pkg_search_module( DBUS dbus-1 )
if( NOT DBUS_FOUND )
  tde_message_fatal( "dbus-1 is required, but was not found on your system" )
endif( )

# tqt
find_package( TQt )

# gcc visibility
if( WITH_GCC_VISIBILITY )
  tde_setup_gcc_visibility( )
endif( )
