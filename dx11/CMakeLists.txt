cmake_minimum_required(VERSION 3.16)

project(rtow LANGUAGES CXX)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

add_executable(rtow
	main.cpp
)

target_link_libraries(rtow PRIVATE d3d11 dxgi dxguid d3dcompiler)
target_compile_features(rtow PUBLIC cxx_std_17)
