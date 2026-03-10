add_library(protocol_lib
            protocol.cpp
            ${PROJECT_SOURCE_DIR}/common/serialization/binary_push_serializer.cpp
)

target_include_directories(protocol_lib 
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${PROJECT_SOURCE_DIR}/common/
)

target_link_libraries(protocol_lib
    PRIVATE
        $<$<PLATFORM_ID:Windows>:ws2_32>
)

target_compile_features(protocol_lib PUBLIC cxx_std_20)
