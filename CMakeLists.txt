cmake_minimum_required(VERSION 3.20)
project(LatinSquareCompletion)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CORE_NAME latin_square)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/${CORE_NAME})
set(HEADERS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

file(GLOB_RECURSE SOURCES
        ${SOURCES_DIR}/*.cpp
)

file(GLOB_RECURSE HEADERS
        ${HEADERS_DIR}/*.h
        ${HEADERS_DIR}/*.hpp
)

add_library(${CORE_NAME} STATIC)
target_sources(${CORE_NAME} PRIVATE ${SOURCES} PUBLIC ${HEADERS})
target_include_directories(${CORE_NAME}
        PUBLIC
        $<BUILD_INTERFACE:${HEADERS_DIR}>
)

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CORE_NAME})