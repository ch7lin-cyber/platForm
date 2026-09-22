add_library(PlatformL03Serial STATIC
    Communication/SerialService/SerialConfiguration.c
    Communication/SerialService/SerialRingBuffer.c
    Communication/SerialService/SerialService.c
    Communication/ModbusCore/ModbusPdu.c
    Communication/ModbusCore/ModbusCrc16.c
    Communication/ModbusCore/ModbusLrc.c
    Communication/ModbusCore/ModbusRtuFramer.c
    Communication/ModbusCore/ModbusAsciiFramer.c
    Communication/ModbusCore/ModbusFunction03.c
    Communication/ModbusCore/ModbusFunction04.c
    Communication/ModbusCore/ModbusFunction06.c
    Communication/ModbusCore/ModbusFunction10.c
    Communication/ModbusSlave/ModbusSlave.c
    Communication/ModbusMaster/ModbusMaster.c
    MiddlewareAdapter/ModbusRegisterAdapter.c
    SystemService/EventService/EventService.c
    SystemService/PwmOutputService/PwmOutputService.c
    SystemService/NvmService/NvmService.c
    SystemService/NvmService/NvmConfigurationEventConsumer.c
    SystemService/FaultService/FaultService.c
    SystemService/AnalogInputService/AnalogInputService.c
    SystemService/AnalogInputService/PiecewiseLinearTable.c
    SystemService/AnalogInputService/SensorConversionService.c
    SystemService/AnalogInputService/SensorTables/tc_b_table.c
    SystemService/AnalogInputService/SensorTables/tc_c_table.c
    SystemService/AnalogInputService/SensorTables/tc_d_table.c
    SystemService/AnalogInputService/SensorTables/tc_e_table.c
    SystemService/AnalogInputService/SensorTables/tc_j_table.c
    SystemService/AnalogInputService/SensorTables/tc_k_table.c
    SystemService/AnalogInputService/SensorTables/tc_l_table.c
    SystemService/AnalogInputService/SensorTables/tc_n_table.c
    SystemService/AnalogInputService/SensorTables/tc_r_table.c
    SystemService/AnalogInputService/SensorTables/tc_s_table.c
    SystemService/AnalogInputService/SensorTables/tc_t_table.c
    SystemService/AnalogInputService/SensorTables/tc_txk_table.c
    SystemService/AnalogInputService/SensorTables/tc_u_table.c
    SystemService/AnalogInputService/SensorTables/rtd_cu50_table.c
    SystemService/AnalogInputService/SensorTables/rtd_jpt100_table.c
    SystemService/AnalogInputService/SensorTables/rtd_ni120_table.c
    SystemService/AnalogInputService/SensorTables/rtd_pt100_table.c
    SystemService/AnalogInputService/SensorTables/rtd_pt1000_table.c
    SystemService/FactoryCalibrationService/FactoryCalibrationService.c
    SystemService/SerialConfigurationApplyService/SerialConfigurationApplyService.c
)

add_library(Platform::L03Serial ALIAS PlatformL03Serial)

target_include_directories(PlatformL03Serial
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/Communication/SerialService
        ${CMAKE_CURRENT_LIST_DIR}/Communication/ModbusCore
        ${CMAKE_CURRENT_LIST_DIR}/Communication/ModbusSlave
        ${CMAKE_CURRENT_LIST_DIR}/Communication/ModbusMaster
        ${CMAKE_CURRENT_LIST_DIR}/MiddlewareAdapter
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/EventService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/PwmOutputService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/NvmService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/FaultService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/AnalogInputService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/AnalogInputService/SensorTables
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/FactoryCalibrationService
        ${CMAKE_CURRENT_LIST_DIR}/SystemService/SerialConfigurationApplyService
)

target_link_libraries(PlatformL03Serial PUBLIC Platform::L02Hal)
target_compile_features(PlatformL03Serial PUBLIC c_std_99)
