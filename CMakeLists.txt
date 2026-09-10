cmake_minimum_required(VERSION 3.14)
project(XWord VERSION 2.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Place runtime DLL next to test/demo .exe so they can run in-tree.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

option(XWORD_BUILD_EXAMPLES "Build examples" ON)
option(XWORD_BUILD_TESTS    "Build tests"    ON)
option(XWORD_BUILD_PYTHON "Build Python bindings" OFF)

# Vendored pugixml 1.14, MIT; never downloads dependencies during a normal build.
add_library(pugixml STATIC thirdparty/pugixml/pugixml.cpp)
target_include_directories(pugixml PUBLIC thirdparty/pugixml)
set_target_properties(pugixml PROPERTIES POSITION_INDEPENDENT_CODE ON)

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
    src/Style.cpp
    src/Content.cpp
    src/Section.cpp
    src/internal/Package.cpp
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
target_link_libraries(xword PRIVATE miniz pugixml)
set_target_properties(xword PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    DEBUG_POSTFIX d
)
if(MSVC)
    target_compile_options(xword PRIVATE /utf-8 /wd4996 /wd4251)
endif()

# ---- Examples ----
if(XWORD_BUILD_EXAMPLES)
    add_executable(xword_demo_v2 examples/demo_v2.cpp)
    target_link_libraries(xword_demo_v2 PRIVATE xword)
    if(MSVC)
        target_compile_options(xword_demo_v2 PRIVATE /utf-8)
    endif()
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

    add_executable(xword_demo_platform examples/demo_platform.cpp)
    target_link_libraries(xword_demo_platform PRIVATE xword)
    if(MSVC)
        target_compile_options(xword_demo_platform PRIVATE /utf-8 /wd4996)
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
    set_tests_properties(xword_test xword_test_eq PROPERTIES TIMEOUT 30)
    add_executable(xword_test_v2 tests/test_v2.cpp)
    target_link_libraries(xword_test_v2 PRIVATE xword pugixml)
    target_include_directories(xword_test_v2 PRIVATE src)
    if(MSVC)
        target_compile_options(xword_test_v2 PRIVATE /utf-8)
    endif()
    add_test(NAME xword_test_v2 COMMAND xword_test_v2)
    set_tests_properties(xword_test_v2 PROPERTIES TIMEOUT 30)
    add_executable(xword_test_regressions tests/test_regressions.cpp)
    target_link_libraries(xword_test_regressions PRIVATE xword pugixml)
    target_include_directories(xword_test_regressions PRIVATE src)
    if(MSVC)
        target_compile_options(xword_test_regressions PRIVATE /utf-8)
    endif()
    add_test(NAME xword_test_regressions COMMAND xword_test_regressions)
    set_tests_properties(xword_test_regressions PROPERTIES TIMEOUT 30)
endif()

if(XWORD_BUILD_PYTHON)
    set(PYBIND11_FINDPYTHON ON)
    find_package(Python COMPONENTS Interpreter Development REQUIRED)
    find_package(pybind11 CONFIG REQUIRED)
    pybind11_add_module(_native src/pybind/bindings.cpp)
    target_link_libraries(_native PRIVATE xword)
    if(APPLE)
        set_target_properties(_native PROPERTIES INSTALL_RPATH "@loader_path")
    elseif(UNIX)
        set_target_properties(_native PROPERTIES INSTALL_RPATH "$ORIGIN")
    endif()
    if(MSVC)
        target_compile_options(_native PRIVATE /utf-8 /bigobj)
    endif()
    install(TARGETS _native LIBRARY DESTINATION xword COMPONENT python RUNTIME DESTINATION xword COMPONENT python)
    install(TARGETS xword LIBRARY DESTINATION xword COMPONENT python RUNTIME DESTINATION xword COMPONENT python)
    install(FILES python/xword/__init__.py DESTINATION xword COMPONENT python)
    if(XWORD_BUILD_TESTS)
        add_test(NAME xword_test_python COMMAND ${Python_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_python.py $<TARGET_FILE_DIR:_native>)
        set_tests_properties(xword_test_python PROPERTIES TIMEOUT 60 DEPENDS xword_test_v2)
    endif()
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
