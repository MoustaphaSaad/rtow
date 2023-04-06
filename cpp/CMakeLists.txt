cmake_minimum_required(VERSION 3.16)

project(rtow LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

find_program(ISPC_EXE ispc REQUIRED)
message(STATUS "ISPC Compiler: ${ISPC_EXE}")

set(ISPC_FILES
	spheres_hit.ispc
)
set(ISPC_TARGETS "sse2,sse4,avx1,avx2")
set(OUTPUT_ISPC_FILES)
foreach(ISPC_FILE ${ISPC_FILES})
	get_filename_component(ISPC_TU_NAME ${ISPC_FILE} NAME_WLE)
	set(TU_OUTPUT_OBJ_FILES "${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}.o")
	if (NOT ARM_HOST)
		list(APPEND
			TU_OUTPUT_OBJ_FILES
			"${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}_sse2.o"
			"${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}_sse4.o"
			"${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}_avx.o"
			"${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}_avx2.o"
		)
	endif ()

	add_custom_command(
		OUTPUT ${TU_OUTPUT_OBJ_FILES} "${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}.h"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMAND ${ISPC_EXE} --target=${ISPC_TARGETS} ${ISPC_FILE} -o "${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}.o" -h "${CMAKE_CURRENT_BINARY_DIR}/${ISPC_TU_NAME}.h"
		DEPENDS ${ISPC_FILE}
		COMMENT "compiling ispc programs"
	)
	list(APPEND
		OUTPUT_ISPC_FILES
		${TU_OUTPUT_OBJ_FILES}
	)
endforeach(ISPC_FILE)


add_subdirectory(enkiTS)

add_executable(rtow
	main.cpp
	vec3.h
	color.h
	ray.h
	hittable.h
	sphere.h
	hittable_list.h
	rtweekend.h
	camera.h
	material.h
	image.h
	${OUTPUT_ISPC_FILES}
)

target_link_libraries(rtow PRIVATE enkiTS)
target_compile_features(rtow PUBLIC cxx_std_17)
target_include_directories(rtow PRIVATE ${CMAKE_CURRENT_BINARY_DIR})