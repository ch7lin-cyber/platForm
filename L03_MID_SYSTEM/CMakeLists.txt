add_library(PlatformL03Serial STATIC
    Communication/SerialService/SerialConfiguration.c
    Communication/SerialService/SerialRingBuffer.c
    Communication/SerialService/SerialService.c
)

add_library(Platform::L03Serial ALIAS PlatformL03Serial)

target_include_directories(PlatformL03Serial
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Communication/SerialService
)

target_link_libraries(PlatformL03Serial PUBLIC Platform::L02Hal)
target_compile_features(PlatformL03Serial PUBLIC c_std_99)
