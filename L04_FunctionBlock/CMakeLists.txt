add_library(PlatformL04FunctionBlock STATIC
    Alarm/FbAlarm.c
    Utility/FbLogic.c
    src/FunctionBlockLifecycle.c
    src/AlarmConfigurationEventConsumer.c
    src/SafetyConfigurationEventConsumer.c
)

add_library(Platform::L04FunctionBlock ALIAS PlatformL04FunctionBlock)

target_include_directories(PlatformL04FunctionBlock
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/Alarm
        ${CMAKE_CURRENT_LIST_DIR}/Safety
        ${CMAKE_CURRENT_LIST_DIR}/Utility
)

target_link_libraries(PlatformL04FunctionBlock PUBLIC Platform::L03Serial)
target_compile_features(PlatformL04FunctionBlock PUBLIC c_std_99)
