add_executable(test_serial_service
    Unit/SerialService/test_serial_service.c
)

target_link_libraries(test_serial_service PRIVATE Platform::L03Serial)

add_test(NAME serial_service COMMAND test_serial_service)

add_executable(test_modbus_write_functions
    Unit/ModbusCore/test_modbus_write_functions.c
)
target_link_libraries(test_modbus_write_functions PRIVATE Platform::L03Serial)
add_test(NAME modbus_write_functions COMMAND test_modbus_write_functions)

add_executable(test_modbus_rtu_slave
    Unit/ModbusCore/test_modbus_rtu_slave.c
)
target_link_libraries(test_modbus_rtu_slave PRIVATE Platform::L03Serial)
add_test(NAME modbus_rtu_slave COMMAND test_modbus_rtu_slave)

add_executable(test_modbus_ascii_slave
    Unit/ModbusCore/test_modbus_ascii_slave.c
)
target_link_libraries(test_modbus_ascii_slave PRIVATE Platform::L03Serial)
add_test(NAME modbus_ascii_slave COMMAND test_modbus_ascii_slave)

add_executable(test_modbus_register_adapter
    Unit/ModbusRegisterAdapter/test_modbus_register_adapter.c
)
target_link_libraries(test_modbus_register_adapter PRIVATE Platform::L03Serial)
add_test(NAME modbus_register_adapter COMMAND test_modbus_register_adapter)

add_executable(test_serial_configuration_apply_service
    Unit/SerialConfigurationApplyService/test_serial_configuration_apply_service.c
)
target_link_libraries(test_serial_configuration_apply_service
    PRIVATE Platform::L03Serial
)
add_test(NAME serial_configuration_apply_service
    COMMAND test_serial_configuration_apply_service
)

add_executable(test_function_block_lifecycle
    Unit/L04_FunctionBlock/test_function_block_lifecycle.c
)

target_link_libraries(test_function_block_lifecycle
    PRIVATE Platform::L04FunctionBlock
)

add_test(NAME function_block_lifecycle COMMAND test_function_block_lifecycle)
add_executable(test_event_service
    Unit/L03_SystemPlatform/test_event_service.c
)
target_link_libraries(test_event_service PRIVATE Platform::L03Serial)
add_test(NAME event_service COMMAND test_event_service)
add_executable(test_alarm_configuration_event_consumer
    Unit/L04_FunctionBlock/test_alarm_configuration_event_consumer.c
)
target_link_libraries(test_alarm_configuration_event_consumer
    PRIVATE Platform::L04FunctionBlock
)
add_test(NAME alarm_configuration_event_consumer
    COMMAND test_alarm_configuration_event_consumer
)
add_executable(test_safety_configuration_event_consumer
    Unit/L04_FunctionBlock/test_safety_configuration_event_consumer.c
)
target_link_libraries(test_safety_configuration_event_consumer
    PRIVATE Platform::L04FunctionBlock
)
add_test(NAME safety_configuration_event_consumer
    COMMAND test_safety_configuration_event_consumer
)
add_executable(test_pwm_output_service
    Unit/L03_SystemPlatform/test_pwm_output_service.c
)
target_link_libraries(test_pwm_output_service PRIVATE Platform::L03Serial)
add_test(NAME pwm_output_service COMMAND test_pwm_output_service)
add_executable(test_nvm_service
    Unit/L03_SystemPlatform/test_nvm_service.c
)
target_link_libraries(test_nvm_service PRIVATE Platform::L03Serial)
add_test(NAME nvm_service COMMAND test_nvm_service)
