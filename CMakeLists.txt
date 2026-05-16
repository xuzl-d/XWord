cmake_minimum_required(VERSION 3.14)
project(XWord VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Third-party: miniz ----
add_library(miniz STATIC
    thirdparty/miniz/miniz.c
    thirdparty/miniz/miniz_zip.c
    thirdparty/miniz/miniz_tdef.c
    thirdparty/miniz/miniz_tinfl.c
)
target_include_directories(miniz PUBLIC thirdparty/miniz)

# ---- Third-party: pugixml ----
add_library(pugixml STATIC
    thirdparty/pugixml/pugixml.cpp
)
target_include_directories(pugixml PUBLIC thirdparty/pugixml)

# ---- XWord library ----
add_library(xword STATIC
    src/Document.cpp
    src/Paragraph.cpp
    src/Table.cpp
    src/Image.cpp
    src/Equation.cpp
    src/internal/ZipWriter.cpp
)
target_include_directories(xword PUBLIC include)
target_link_libraries(xword PUBLIC pugixml miniz)
if(MSVC)
    target_compile_options(xword PRIVATE /utf-8 /wd4996)
endif()

# ---- Examples ----
add_executable(xword_demo examples/demo.cpp)
target_link_libraries(xword_demo xword)
if(MSVC)
    target_compile_options(xword_demo PRIVATE /utf-8 /wd4996)
endif()

# ---- Tests ----
enable_testing()
add_executable(xword_test tests/test_basic.cpp)
target_link_libraries(xword_test xword)
if(MSVC)
    target_compile_options(xword_test PRIVATE /utf-8 /wd4996)
endif()
add_test(NAME xword_test COMMAND xword_test)
