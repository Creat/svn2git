project( svn-all-fast-export )

cmake_minimum_required( VERSION 3.4 )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/" )

find_package( APR REQUIRED )
find_package( Git REQUIRED )
find_package( Svn REQUIRED )
find_package( Qt4 REQUIRED QtCore )

add_subdirectory( src )

add_executable( svn-all-fast-export ${SVN_ALL_FAST_EXPORT_SRC} )

set( CMAKE_AUTOMOC true )

execute_process( COMMAND ${GIT_EXECUTABLE} --no-pager show --pretty=format:%H --no-notes --abbrev-commit -s OUTPUT_VARIABLE GIT_SHA1 )

target_compile_definitions( svn-all-fast-export	PRIVATE	CURRENT_VERSION=\"${GIT_SHA1}\" )

target_include_directories( svn-all-fast-export PRIVATE ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src ${APR_INCLUDE_DIR} ${SVN_INCLUDE_DIR})

target_link_libraries( svn-all-fast-export ${APR_LIBRARIES} ${SVN_LIBS}	Qt4::QtCore )
