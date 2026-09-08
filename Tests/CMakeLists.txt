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
