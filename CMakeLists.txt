cmake_minimum_required( VERSION 3.0 )

project( web_browser_app )

include( ExternalProject )

find_package( Boost 1.60.0 COMPONENTS system filesystem regex iostreams unit_test_framework REQUIRED ) 

find_package( wxWidgets REQUIRED webview stc adv core base )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 /std:c++latest ) 
else( )
	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" )
		add_compile_options(-std=c++14 -Weverything -pthread -Wno-c++98-compat -g -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options(-std=c++14 -g -Wall -pthread -Wno-deprecated-declarations)
	endif( )
endif( )

externalproject_add(
    header_libraries_prj
    GIT_REPOSITORY "https://github.com/beached/header_libraries.git"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/header_libraries"
    GIT_TAG "master"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add(
    char_range_prj
    GIT_REPOSITORY "https://github.com/beached/char_range.git"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/char_range"
    GIT_TAG "master"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add(
    parse_json_prj
    GIT_REPOSITORY "https://github.com/beached/parse_json.git"
    SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/parse_json"
    GIT_TAG "master"
    INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

externalproject_add(
	date_prj
	GIT_REPOSITORY "https://github.com/beached/date.git"
	SOURCE_DIR "${CMAKE_BINARY_DIR}/dependencies/date"
	GIT_TAG "master"
	INSTALL_DIR "${CMAKE_BINARY_DIR}/install"
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
)

set( HEADER_FOLDER "include" )
set( SOURCE_FOLDER "src" )
set( TEST_FOLDER "tests" )

include_directories( SYSTEM "${CMAKE_BINARY_DIR}/install/include" )
link_directories( "${CMAKE_BINARY_DIR}/install/lib" )
include_directories( ${HEADER_FOLDER} )


set( SOURCE_FILES
	${SOURCE_FOLDER}/web_browser_app.cpp
	${SOURCE_FOLDER}/config.cpp
)

set( HEADER_FILES
	${HEADER_FOLDER}/web_browser_app.h
	${HEADER_FOLDER}/config.h
)

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

include( ${wxWidgets_USE_FILE} )

add_executable( web_browser_app_bin ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( web_browser_app_bin header_libraries_prj char_range_prj date_prj parse_json_prj  )
target_link_libraries( web_browser_app_bin char_range parse_json ${CMAKE_DL_LIBS} ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${COMPILER_SPECIFIC_LIBS} ${wxWidgets_LIBRARIES} )
