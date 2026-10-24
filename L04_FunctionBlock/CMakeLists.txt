add_library(PlatformL04FunctionBlock STATIC
    Control/FbPid.c
    Control/FbPidGroupSelector.c
    Control/FbProfileSequencer.c
    Control/FbCascadeCoordinator.c
    Control/FbFeedforward.c
    Control/FbPowerVoltageCompensation.c
    Diagnostics/FbBatchMetrics.c
    Diagnostics/FbProcessResponse.c
    Diagnostics/FbHeaterResistance.c
    Recording/FbRecordTrigger.c
    Control/FbSetpointRamp.c
    Control/FbOutputLimiter.c
    Alarm/FbAlarm.c
    Alarm/FbMultiChannelAlarm.c
    Utility/FbLogic.c
    Utility/FbDivide.c
    Utility/FbWeightedSum.c
    Utility/FbSelector.c
    Utility/FbInBandTimer.c
    Utility/FbDeadTime.c
    Utility/FbLeadLag.c
    Utility/FbLinearization.c
    Utility/FbSignalLinearization.c
    Utility/FbInputSwitchOver.c
    Utility/FbMovingAverage.c
    src/FunctionBlockLifecycle.c
    src/AlarmConfigurationEventConsumer.c
    src/SafetyConfigurationEventConsumer.c
)

add_library(Platform::L04FunctionBlock ALIAS PlatformL04FunctionBlock)

target_include_directories(PlatformL04FunctionBlock
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_LIST_DIR}/Control
        ${CMAKE_CURRENT_LIST_DIR}/Alarm
        ${CMAKE_CURRENT_LIST_DIR}/Safety
        ${CMAKE_CURRENT_LIST_DIR}/Utility
        ${CMAKE_CURRENT_LIST_DIR}/Diagnostics
        ${CMAKE_CURRENT_LIST_DIR}/Recording
)

target_include_directories(PlatformL04FunctionBlock
    PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/CKLIB/Include
)

target_link_libraries(PlatformL04FunctionBlock PUBLIC Platform::L03Serial)
target_compile_features(PlatformL04FunctionBlock PUBLIC c_std_99)
