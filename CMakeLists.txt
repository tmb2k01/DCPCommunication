cmake_minimum_required(VERSION 3.10)
project(fmusim)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(fmusim_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include_directories("${fmusim_SOURCE_DIR}/lib/fmi")
include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared")
include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/include")
include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/parser")
include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/parser/fmu20")
include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/parser/libxml")
include_directories("${fmusim_SOURCE_DIR}/src")

file(GLOB SOURCES 
    "src/*.cpp" 
    "lib/fmilib/shared/*.c" 
    "lib/fmilib/shared/*.cpp"
    "lib/fmilib/shared/parser/*.c" 
    "lib/fmilib/shared/parser/*.cpp"
    "lib/fmilib/shared/parser/libxml/*.c" 
    "lib/fmilib/shared/parser/libxml/*.cpp"
    "lib/fmilib/shared/parser/fmu20/*.c" 
    "lib/fmilib/shared/parser/fmu20/*.cpp"
    "lib/dcp/*.hpp")

set(fmusim_CMAKE_DEFINITIONS 
    -Wall 
    -Wextra
    -Wno-deprecated-declarations 
    -Wno-unused-parameter 
    -Wno-comment
)
add_compile_options(${fmusim_CMAKE_DEFINITIONS})

add_definitions(-DSTANDALONE_XML_PARSER)
add_definitions(-DLIBXML_STATIC)
add_definitions(-DLIBXML_READER_ENABLED)

find_package(LibXml2 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR})


SET(EXECUTABLE_NAME ${PROJECT_NAME})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} ${LIBXML2_LIBRARIES} dl)