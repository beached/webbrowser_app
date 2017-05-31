cmake_minimum_required( VERSION 3.0 )

project( web_browser_app )

include( ExternalProject )

find_package( Boost 1.60.0 COMPONENTS system filesystem regex iostreams unit_test_framework REQUIRED ) 

enable_testing( )
add_definitions( -DBOOST_TEST_DYN_LINK ) 


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

set( SOURCE_FILES
	web_browser_app.cpp
)


include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

include( ${wxWidgets_USE_FILE} )

add_executable( web_browser_app_bin ${SOURCE_FILES} )
target_link_libraries( web_browser_app_bin ${CMAKE_DL_LIBS} ${Boost_Libs} ${CMAKE_THREAD_LIBS_INIT} ${COMPILER_SPECIFIC_LIBS} ${wxWidgets_LIBRARIES} )
