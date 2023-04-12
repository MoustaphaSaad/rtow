cmake_minimum_required(VERSION 3.16)

project(rtow LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

find_program(DXC_EXE dxc REQUIRED)
message(STATUS "DXC Compiler: ${DXC_EXE}")

set(HLSL_FILES
	simple.hlsl
)
set(OUTPUT_HLSL_FILES)
foreach(HLSL_FILE ${HLSL_FILES})
	get_filename_component(HLSL_TU_NAME ${HLSL_FILE} NAME_WLE)
	set(TU_OUTPUT_OBJ_FILES "${CMAKE_CURRENT_BINARY_DIR}/bin/$<CONFIG>/${HLSL_TU_NAME}.so")

	add_custom_command(
		OUTPUT "${TU_OUTPUT_OBJ_FILES}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
		COMMAND "${DXC_EXE}" "${HLSL_FILE}" -T "cs_6_0" -E main -Fo "${TU_OUTPUT_OBJ_FILES}"
		DEPENDS ${HLSL_FILE}
		COMMENT "compiling hlsl programs"
	)
	list(APPEND
		OUTPUT_HLSL_FILES
		${TU_OUTPUT_OBJ_FILES}
	)
endforeach(HLSL_FILE)

add_executable(rtow
	main.cpp
	${TU_OUTPUT_OBJ_FILES}
)

target_link_libraries(rtow PRIVATE d3d12 dxgi dxguid d3dcompiler)
target_compile_features(rtow PUBLIC cxx_std_17)
