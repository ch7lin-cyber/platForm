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

add_executable(test_modbus_master
    Unit/ModbusMaster/test_modbus_master.c
)
target_link_libraries(test_modbus_master PRIVATE Platform::L03Serial)
add_test(NAME modbus_master COMMAND test_modbus_master)

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

add_executable(test_fb_logic
    Unit/L04_FunctionBlock/test_fb_logic.c
)
target_link_libraries(test_fb_logic PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_logic COMMAND test_fb_logic)

add_executable(test_fb_divide
    Unit/L04_FunctionBlock/test_fb_divide.c
)
target_link_libraries(test_fb_divide PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_divide COMMAND test_fb_divide)

add_executable(test_fb_weighted_sum
    Unit/L04_FunctionBlock/test_fb_weighted_sum.c
)
target_link_libraries(test_fb_weighted_sum PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_weighted_sum COMMAND test_fb_weighted_sum)

add_executable(test_fb_selector
    Unit/L04_FunctionBlock/test_fb_selector.c
)
target_link_libraries(test_fb_selector PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_selector COMMAND test_fb_selector)

add_executable(test_fb_moving_average
    Unit/L04_FunctionBlock/test_fb_moving_average.c
)
target_link_libraries(test_fb_moving_average PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_moving_average COMMAND test_fb_moving_average)

add_executable(test_fb_alarm
    Unit/L04_FunctionBlock/test_fb_alarm.c
)
target_link_libraries(test_fb_alarm PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_alarm COMMAND test_fb_alarm)

add_executable(test_event_service
    Unit/L03_SystemPlatform/test_event_service.c
)
target_link_libraries(test_event_service PRIVATE Platform::L03Serial)
add_test(NAME event_service COMMAND test_event_service)
add_executable(test_fault_service
    Unit/L03_SystemPlatform/test_fault_service.c
)
target_link_libraries(test_fault_service PRIVATE Platform::L03Serial)
add_test(NAME fault_service COMMAND test_fault_service)
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
add_executable(test_analog_input_service
    Unit/L03_SystemPlatform/test_analog_input_service.c
)
target_link_libraries(test_analog_input_service PRIVATE Platform::L03Serial)
add_test(NAME analog_input_service COMMAND test_analog_input_service)
add_executable(test_sensor_conversion_service
    Unit/L03_SystemPlatform/test_sensor_conversion_service.c
)
target_link_libraries(test_sensor_conversion_service PRIVATE Platform::L03Serial)
add_test(NAME sensor_conversion_service COMMAND test_sensor_conversion_service)
add_executable(test_sensor_tables
    Unit/L03_SystemPlatform/test_sensor_tables.c
)
target_link_libraries(test_sensor_tables PRIVATE Platform::L03Serial)
add_test(NAME sensor_tables COMMAND test_sensor_tables)

add_executable(test_snapshot_service
    Unit/L03_SystemPlatform/test_snapshot_service.c
)
target_link_libraries(test_snapshot_service PRIVATE Platform::L03Serial)
add_test(NAME snapshot_service COMMAND test_snapshot_service)

add_executable(test_safety_service
    Unit/L03_SystemPlatform/test_safety_service.c
)
target_link_libraries(test_safety_service PRIVATE Platform::L03Serial)
add_test(NAME safety_service COMMAND test_safety_service)

add_executable(test_warning_service
    Unit/L03_SystemPlatform/test_warning_service.c
)
target_link_libraries(test_warning_service PRIVATE Platform::L03Serial)
add_test(NAME warning_service COMMAND test_warning_service)

add_executable(test_system_routine
    Unit/L03_SystemPlatform/test_system_routine.c
)
target_link_libraries(test_system_routine PRIVATE Platform::L03Serial)
add_test(NAME system_routine COMMAND test_system_routine)

add_executable(test_system_event_service
    Unit/L03_SystemPlatform/test_system_event_service.c
)
target_link_libraries(test_system_event_service PRIVATE Platform::L03Serial)
add_test(NAME system_event_service COMMAND test_system_event_service)

add_executable(test_system_fault_service
    Unit/L03_SystemPlatform/test_system_fault_service.c
)
target_link_libraries(test_system_fault_service PRIVATE Platform::L03Serial)
add_test(NAME system_fault_service COMMAND test_system_fault_service)

add_executable(test_system_safety_fault_injection
    Unit/L03_SystemPlatform/test_system_safety_fault_injection.c
)
target_link_libraries(test_system_safety_fault_injection
    PRIVATE Platform::L03Serial
)
add_test(NAME system_safety_fault_injection
    COMMAND test_system_safety_fault_injection
)

add_executable(test_hal_dac
    Unit/L02_PlatformHAL/test_hal_dac.c
)
target_link_libraries(test_hal_dac PRIVATE Platform::L02Hal)
add_test(NAME hal_dac COMMAND test_hal_dac)

add_executable(test_digital_io_services
    Unit/L03_SystemPlatform/test_digital_io_services.c
)
target_link_libraries(test_digital_io_services PRIVATE Platform::L03Serial)
add_test(NAME digital_io_services COMMAND test_digital_io_services)

add_executable(test_fb_setpoint_ramp Unit/L04_FunctionBlock/test_fb_setpoint_ramp.c)
target_link_libraries(test_fb_setpoint_ramp PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_setpoint_ramp COMMAND test_fb_setpoint_ramp)

add_executable(test_fb_output_limiter Unit/L04_FunctionBlock/test_fb_output_limiter.c)
target_link_libraries(test_fb_output_limiter PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_output_limiter COMMAND test_fb_output_limiter)

add_executable(test_fb_in_band_timer Unit/L04_FunctionBlock/test_fb_in_band_timer.c)
target_link_libraries(test_fb_in_band_timer PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_in_band_timer PRIVATE m)
endif()
add_test(NAME fb_in_band_timer COMMAND test_fb_in_band_timer)

add_executable(test_fb_linearization Unit/L04_FunctionBlock/test_fb_linearization.c)
target_link_libraries(test_fb_linearization PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_linearization PRIVATE m)
endif()
add_test(NAME fb_linearization COMMAND test_fb_linearization)
add_executable(test_fb_signal_linearization Unit/L04_FunctionBlock/test_fb_signal_linearization.c)
target_link_libraries(test_fb_signal_linearization PRIVATE Platform::L04FunctionBlock)
if(NOT MSVC)
    target_link_libraries(test_fb_signal_linearization PRIVATE m)
endif()
add_test(NAME fb_signal_linearization COMMAND test_fb_signal_linearization)
add_executable(test_fb_input_switch_over Unit/L04_FunctionBlock/test_fb_input_switch_over.c)
target_link_libraries(test_fb_input_switch_over PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_input_switch_over COMMAND test_fb_input_switch_over)
add_executable(test_fb_multi_channel_alarm Unit/L04_FunctionBlock/test_fb_multi_channel_alarm.c)
target_link_libraries(test_fb_multi_channel_alarm PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_multi_channel_alarm COMMAND test_fb_multi_channel_alarm)
add_executable(test_fb_pid_group_selector Unit/L04_FunctionBlock/test_fb_pid_group_selector.c)
target_link_libraries(test_fb_pid_group_selector PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_pid_group_selector COMMAND test_fb_pid_group_selector)
add_executable(test_fb_profile_sequencer Unit/L04_FunctionBlock/test_fb_profile_sequencer.c)
target_link_libraries(test_fb_profile_sequencer PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_profile_sequencer COMMAND test_fb_profile_sequencer)
add_executable(test_fb_cascade_coordinator Unit/L04_FunctionBlock/test_fb_cascade_coordinator.c)
target_link_libraries(test_fb_cascade_coordinator PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_cascade_coordinator COMMAND test_fb_cascade_coordinator)
add_executable(test_fb_feedforward Unit/L04_FunctionBlock/test_fb_feedforward.c)
target_link_libraries(test_fb_feedforward PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_feedforward PRIVATE m)
endif()
add_test(NAME fb_feedforward COMMAND test_fb_feedforward)
add_executable(test_fb_batch_metrics Unit/L04_FunctionBlock/test_fb_batch_metrics.c)
target_link_libraries(test_fb_batch_metrics PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_batch_metrics PRIVATE m)
endif()
add_test(NAME fb_batch_metrics COMMAND test_fb_batch_metrics)
add_executable(test_fb_process_response Unit/L04_FunctionBlock/test_fb_process_response.c)
target_link_libraries(test_fb_process_response PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_process_response PRIVATE m)
endif()
add_test(NAME fb_process_response COMMAND test_fb_process_response)
add_executable(test_fb_record_trigger Unit/L04_FunctionBlock/test_fb_record_trigger.c)
target_link_libraries(test_fb_record_trigger PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_record_trigger COMMAND test_fb_record_trigger)
add_executable(test_fb_heater_resistance Unit/L04_FunctionBlock/test_fb_heater_resistance.c)
target_link_libraries(test_fb_heater_resistance PRIVATE Platform::L04FunctionBlock)
add_test(NAME fb_heater_resistance COMMAND test_fb_heater_resistance)
add_executable(test_fb_power_voltage_compensation Unit/L04_FunctionBlock/test_fb_power_voltage_compensation.c)
target_link_libraries(test_fb_power_voltage_compensation PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_power_voltage_compensation PRIVATE m)
endif()
add_test(NAME fb_power_voltage_compensation COMMAND test_fb_power_voltage_compensation)

add_executable(test_fb_dead_time Unit/L04_FunctionBlock/test_fb_dead_time.c)
target_link_libraries(test_fb_dead_time PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_dead_time PRIVATE m)
endif()
add_test(NAME fb_dead_time COMMAND test_fb_dead_time)

add_executable(test_fb_lead_lag Unit/L04_FunctionBlock/test_fb_lead_lag.c)
target_link_libraries(test_fb_lead_lag PRIVATE Platform::L04FunctionBlock)
if(UNIX)
    target_link_libraries(test_fb_lead_lag PRIVATE m)
endif()
add_test(NAME fb_lead_lag COMMAND test_fb_lead_lag)
