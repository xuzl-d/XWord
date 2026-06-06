cmake_minimum_required(VERSION 3.14)
project(XWordPybind VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find pybind11
find_package(pybind11 REQUIRED)

# ---- miniz (static) ----
add_library(miniz_py STATIC
    ${CMAKE_SOURCE_DIR}/../../thirdparty/miniz/miniz.c
    ${CMAKE_SOURCE_DIR}/../../thirdparty/miniz/miniz_zip.c
    ${CMAKE_SOURCE_DIR}/../../thirdparty/miniz/miniz_tdef.c
    ${CMAKE_SOURCE_DIR}/../../thirdparty/miniz/miniz_tinfl.c
)
target_include_directories(miniz_py PUBLIC
    ${CMAKE_SOURCE_DIR}/../../thirdparty/miniz)
set_target_properties(miniz_py PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---- _native module ----
pybind11_add_module(_native
    ${CMAKE_SOURCE_DIR}/../../src/Document.cpp
    ${CMAKE_SOURCE_DIR}/../../src/BulletList.cpp
    ${CMAKE_SOURCE_DIR}/../../src/Paragraph.cpp
    ${CMAKE_SOURCE_DIR}/../../src/Table.cpp
    ${CMAKE_SOURCE_DIR}/../../src/Image.cpp
    ${CMAKE_SOURCE_DIR}/../../src/Equation.cpp
    ${CMAKE_SOURCE_DIR}/../../src/internal/ZipWriter.cpp
    ${CMAKE_SOURCE_DIR}/../../src/internal/ZipReader.cpp
    ${CMAKE_SOURCE_DIR}/../../src/pybind/bindings.cpp
)
target_include_directories(_native PRIVATE
    ${CMAKE_SOURCE_DIR}/../../include
    ${CMAKE_SOURCE_DIR}/../../src
)
target_compile_definitions(_native PRIVATE XWORD_BUILDING)
target_link_libraries(_native PRIVATE miniz_py)
if(MSVC)
    target_compile_options(_native PRIVATE /utf-8 /wd4996 /wd4251)
endif()

# Copy output .pyd to python/xword/
add_custom_command(TARGET _native POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:_native>"
        "${CMAKE_SOURCE_DIR}/../xword/$<TARGET_FILE_NAME:_native>"
    COMMENT "Copying _native module to python/xword/"
)
