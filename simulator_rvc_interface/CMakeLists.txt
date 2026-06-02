add_library(rvc_simulator_interface
    Class/CleanerController.cpp
    Class/EventBus.cpp
    Class/MotorController.cpp
    Class/PowerController.cpp
    Class/SensorController.cpp
    Class/Timer.cpp
)

target_include_directories(rvc_simulator_interface
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(rvc_simulator_interface PUBLIC rvc_core)
target_compile_features(rvc_simulator_interface PUBLIC cxx_std_20)

add_library(rvc_imported_simulator STATIC EXCLUDE_FROM_ALL
    ${PROJECT_SOURCE_DIR}/Simulator/GridMap.cpp
    ${PROJECT_SOURCE_DIR}/Simulator/RvcSimulator.cpp
    ${PROJECT_SOURCE_DIR}/Simulator/SimulatedCleaner.cpp
    ${PROJECT_SOURCE_DIR}/Simulator/SimulatedMotor.cpp
    ${PROJECT_SOURCE_DIR}/Simulator/SimulatedSensor.cpp
    ${PROJECT_SOURCE_DIR}/Simulator/SystemTestCases.cpp
    ${PROJECT_SOURCE_DIR}/Simulator/SystemTestMode.cpp
)

target_include_directories(rvc_imported_simulator
    PUBLIC
        ${PROJECT_SOURCE_DIR}
)

target_link_libraries(rvc_imported_simulator PUBLIC rvc_simulator_interface)
target_compile_features(rvc_imported_simulator PUBLIC cxx_std_20)

if(BUILD_TESTING)
    add_executable(rvc_simulator_interface_smoke
        tests/SimulatorInterfaceSmoke.cpp
    )

    target_link_libraries(rvc_simulator_interface_smoke PRIVATE rvc_simulator_interface)

    add_test(
        NAME SimulatorRvcInterfaceSmoke
        COMMAND $<TARGET_FILE:rvc_simulator_interface_smoke>
    )
endif()
