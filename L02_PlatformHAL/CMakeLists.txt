add_library(PlatformL02Hal STATIC
    src/HalSerial.c
)

add_library(Platform::L02Hal ALIAS PlatformL02Hal)

target_include_directories(PlatformL02Hal
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

target_compile_features(PlatformL02Hal PUBLIC c_std_99)
