add_executable(test_serial_service
    Unit/SerialService/test_serial_service.c
)

target_link_libraries(test_serial_service PRIVATE Platform::L03Serial)

add_test(NAME serial_service COMMAND test_serial_service)
