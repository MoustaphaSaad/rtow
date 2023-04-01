cmake_minimum_required(VERSION 3.0)

project( enkiTS )

set( ENKITS_TASK_PRIORITIES_NUM "3" CACHE STRING "Number of task priorities, 1-5, 0 for defined by defaults in source" ) 

set( ENKITS_HEADERS
    src/LockLessMultiReadPipe.h
    src/TaskScheduler.h
    )

set( ENKITS_SRC
     src/TaskScheduler.cpp
     )

list( APPEND ENKITS_SRC ${ENKITS_HEADERS} )

add_library( enkiTS SHARED ${ENKITS_SRC} )
target_compile_definitions( enkiTS PRIVATE ENKITS_BUILD_DLL=1 )
target_compile_definitions( enkiTS INTERFACE ENKITS_DLL=1 )
if( UNIX )
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") 
			SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
	endif()
endif ()

target_include_directories( enkiTS PUBLIC "${PROJECT_SOURCE_DIR}/src")

if( ENKITS_TASK_PRIORITIES_NUM GREATER "0" )
    target_compile_definitions( enkiTS PUBLIC "ENKITS_TASK_PRIORITIES_NUM=${ENKITS_TASK_PRIORITIES_NUM}" )    
endif()

if( UNIX )
    set( CMAKE_THREAD_PREFER_PTHREAD TRUE )
    find_package( Threads REQUIRED )
    if( CMAKE_USE_PTHREADS_INIT )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread" )
    endif()
    target_link_libraries( enkiTS ${CMAKE_THREAD_LIBS_INIT} )
endif()

if( UNIX )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" )
endif()
if( APPLE )
    SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++" )
endif()
