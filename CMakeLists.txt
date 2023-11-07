cmake_minimum_required(VERSION 3.10)
project(fmusim)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(fmusim_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(MODE "None" CACHE STRING "Define the mode: Master, Slave_One, Slave_Two")

if(MODE STREQUAL "Master")
    message(STATUS "Mode is set to Master")
    add_definitions(-DMASTER_BUILD)
elseif(MODE STREQUAL "Slave_One")
    message(STATUS "Mode is set to Slave_One")
    add_definitions(-DSLAVEONE_BUILD)
elseif(MODE STREQUAL "Slave_Two")
    message(STATUS "Mode is set to Slave_Two")
    add_definitions(-DSLAVETWO_BUILD)
else()
    message(WARNING "Unknown mode or mode not set")
endif()

include_directories("${fmusim_SOURCE_DIR}/src")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/bluetooth")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/core")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/ethernet")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/master")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/slave")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/xml")
include_directories("${fmusim_SOURCE_DIR}/lib/dcp/zip")

if(NOT MODE STREQUAL "Master")
    include_directories("${fmusim_SOURCE_DIR}/lib/fmi")
    include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared")
    include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/include")
    include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/parser")
    include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/parser/fmu20")
    include_directories("${fmusim_SOURCE_DIR}/lib/fmilib/shared/parser/libxml")

    file(GLOB SOURCES
        "src/FMURunner.cpp" 
        "lib/fmilib/shared/*.c" 
        "lib/fmilib/shared/*.cpp"
        "lib/fmilib/shared/parser/*.c" 
        "lib/fmilib/shared/parser/*.cpp"
        "lib/fmilib/shared/parser/libxml/*.c" 
        "lib/fmilib/shared/parser/libxml/*.cpp"
        "lib/fmilib/shared/parser/fmu20/*.c" 
        "lib/fmilib/shared/parser/fmu20/*.cpp")

endif()

list(APPEND SOURCES 
    "src/main.cpp")

set(fmusim_CMAKE_DEFINITIONS 
    -g
    -Wno-deprecated-declarations 
    -Wno-unused-parameter 
    -Wno-comment
    -Wno-macro-redefined
    -Wno-return-type
)
add_compile_options(${fmusim_CMAKE_DEFINITIONS})

add_definitions(-DSTANDALONE_XML_PARSER)
add_definitions(-DLIBXML_STATIC)
add_definitions(-DLIBXML_READER_ENABLED)

find_package(LibXml2 REQUIRED)
find_package(XercesC REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIR} ${XercesC_INCLUDE_DIR})


SET(EXECUTABLE_NAME ${PROJECT_NAME})
add_executable(${EXECUTABLE_NAME} ${SOURCES})
target_link_libraries(${EXECUTABLE_NAME} ${LIBXML2_LIBRARIES} ${XercesC_LIBRARY} dl pthread)