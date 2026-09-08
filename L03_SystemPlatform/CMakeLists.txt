add_library(PlatformL03Serial STATIC
    Communication/SerialService/SerialConfiguration.c
    Communication/SerialService/SerialRingBuffer.c
    Communication/SerialService/SerialService.c
    Communication/ModbusCore/ModbusPdu.c
    Communication/ModbusCore/ModbusFunction06.c
    Communication/ModbusCore/ModbusFunction10.c
    MiddlewareAdapter/ModbusRegisterAdapter.c
    SystemService/EventService/EventService.c
    SystemService/SerialConfigurationApplyService/SerialConfigurationApplyService.c
)

add_library(Platform::L03Serial ALIAS PlatformL03Serial)

target_include_directories(PlatformL03Serial
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Communication/SerialService
        ${CMAKE_CURRENT_LIST_DIR}/Communication/ModbusCore
        ${CMAKE_CURRENT_LIST_DIR}/MiddlewareAdapter
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/EventService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/SerialConfigurationApplyService
)

target_link_libraries(PlatformL03Serial PUBLIC Platform::L02Hal)
target_compile_features(PlatformL03Serial PUBLIC c_std_99)
