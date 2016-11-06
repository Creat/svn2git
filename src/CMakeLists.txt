add_subdirectory( git )
add_subdirectory( svn )
add_subdirectory( rules )
add_subdirectory( logging )
add_subdirectory( commandline )

set( SVN_ALL_FAST_EXPORT_SRC ${SVN_ALL_FAST_EXPORT_SRC} src/main.cpp PARENT_SCOPE )
