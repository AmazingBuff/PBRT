CMAKE_MINIMUM_REQUIRED(VERSION 3.20)

project(pbrt)

set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/sources
        ${THIRD_PARTY_DIR}
        ${THIRD_PARTY_DIR}/openexr
        ${THIRD_PARTY_DIR}/ptex/src
        ${THIRD_PARTY_DIR}/zlib
        ${THIRD_PARTY_DIR}/spdlog/include)

add_executable(pbrt_exe sources/main/pbrt.cpp)