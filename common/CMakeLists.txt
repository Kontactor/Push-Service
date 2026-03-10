cmake_minimum_required(VERSION 3.16)

project(common LANGUAGES CXX)

add_library(push_common
    push/push.h
    serialization/push_serializer.h
    serialization/binary_push_serializer.h
    serialization/binary_push_serializer.cpp
)

target_include_directories(push_common PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)
