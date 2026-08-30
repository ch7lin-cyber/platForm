add_executable(test_serial_service
    Unit/SerialService/test_serial_service.c
)

target_link_libraries(test_serial_service PRIVATE Platform::L03Serial)

add_test(NAME serial_service COMMAND test_serial_service)

add_executable(test_function_block_lifecycle
    Unit/L04_FunctionBlock/test_function_block_lifecycle.c
)

target_link_libraries(test_function_block_lifecycle
    PRIVATE Platform::L04FunctionBlock
)

add_test(NAME function_block_lifecycle COMMAND test_function_block_lifecycle)
