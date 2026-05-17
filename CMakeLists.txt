cmake_minimum_required(VERSION 3.14)
project(XWord VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Place runtime DLL next to test/demo .exe so they can run in-tree.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

option(XWORD_BUILD_EXAMPLES "Build examples" ON)
option(XWORD_BUILD_TESTS    "Build tests"    ON)

# ---- Third-party: miniz (linked statically into the DLL, not exposed) ----
add_library(miniz STATIC
    thirdparty/miniz/miniz.c
    thirdparty/miniz/miniz_zip.c
    thirdparty/miniz/miniz_tdef.c
    thirdparty/miniz/miniz_tinfl.c
)
target_include_directories(miniz PUBLIC thirdparty/miniz)
set_target_properties(miniz PROPERTIES POSITION_INDEPENDENT_CODE ON)

# ---- XWord shared library ----
add_library(xword SHARED
    src/Document.cpp
    src/BulletList.cpp
    src/Paragraph.cpp
    src/Table.cpp
    src/Image.cpp
    src/Equation.cpp
    src/internal/ZipWriter.cpp
    src/internal/ZipReader.cpp
)
target_include_directories(xword
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_definitions(xword PRIVATE XWORD_BUILDING)
target_link_libraries(xword PRIVATE miniz)
set_target_properties(xword PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    DEBUG_POSTFIX d
)
if(MSVC)
    target_compile_options(xword PRIVATE /utf-8 /wd4996 /wd4251)
endif()

# ---- Examples ----
if(XWORD_BUILD_EXAMPLES)
    add_executable(xword_demo examples/demo.cpp)
    target_link_libraries(xword_demo PRIVATE xword)
    if(MSVC)
        target_compile_options(xword_demo PRIVATE /utf-8 /wd4996)
    endif()

    add_executable(xword_demo_template examples/demo_template.cpp)
    target_link_libraries(xword_demo_template PRIVATE xword)
    if(MSVC)
        target_compile_options(xword_demo_template PRIVATE /utf-8 /wd4996)
    endif()
endif()

# ---- Tests ----
if(XWORD_BUILD_TESTS)
    enable_testing()
    add_executable(xword_test tests/test_basic.cpp)
    target_link_libraries(xword_test PRIVATE xword)
    if(MSVC)
        target_compile_options(xword_test PRIVATE /utf-8 /wd4996)
    endif()
    add_test(NAME xword_test COMMAND xword_test)

    add_executable(xword_test_eq tests/test_eq_dump.cpp)
    target_link_libraries(xword_test_eq PRIVATE xword)
    if(MSVC)
        target_compile_options(xword_test_eq PRIVATE /utf-8 /wd4996)
    endif()
    add_test(NAME xword_test_eq COMMAND xword_test_eq)
endif()

# ---- Install (clean redistributable layout) ----
include(GNUInstallDirs)
install(TARGETS xword
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY include/xword
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.hpp"
)
