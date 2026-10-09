add_library(grbl INTERFACE)

target_sources(grbl INTERFACE
 ${CMAKE_CURRENT_LIST_DIR}/grbllib.c
 ${CMAKE_CURRENT_LIST_DIR}/coolant_control.c
 ${CMAKE_CURRENT_LIST_DIR}/crossbar.c
 ${CMAKE_CURRENT_LIST_DIR}/crc.c
 ${CMAKE_CURRENT_LIST_DIR}/nvs_buffer.c
 ${CMAKE_CURRENT_LIST_DIR}/gcode.c
 ${CMAKE_CURRENT_LIST_DIR}/machine_limits.c
 ${CMAKE_CURRENT_LIST_DIR}/messages.c
 ${CMAKE_CURRENT_LIST_DIR}/modbus.c
 ${CMAKE_CURRENT_LIST_DIR}/modbus_rtu.c
 ${CMAKE_CURRENT_LIST_DIR}/motion_control.c
 ${CMAKE_CURRENT_LIST_DIR}/my_plugin.c
 ${CMAKE_CURRENT_LIST_DIR}/nuts_bolts.c
 ${CMAKE_CURRENT_LIST_DIR}/override.c
 ${CMAKE_CURRENT_LIST_DIR}/planner.c
 ${CMAKE_CURRENT_LIST_DIR}/probe.c
 ${CMAKE_CURRENT_LIST_DIR}/protocol.c
 ${CMAKE_CURRENT_LIST_DIR}/report.c
 ${CMAKE_CURRENT_LIST_DIR}/settings.c
 ${CMAKE_CURRENT_LIST_DIR}/sleep.c
 ${CMAKE_CURRENT_LIST_DIR}/spindle_control.c
 ${CMAKE_CURRENT_LIST_DIR}/state_machine.c
 ${CMAKE_CURRENT_LIST_DIR}/stream.c
 ${CMAKE_CURRENT_LIST_DIR}/stream_file.c
 ${CMAKE_CURRENT_LIST_DIR}/stream_json.c
 ${CMAKE_CURRENT_LIST_DIR}/stream_passthru.c
 ${CMAKE_CURRENT_LIST_DIR}/stepper.c
 ${CMAKE_CURRENT_LIST_DIR}/stepper2.c
 ${CMAKE_CURRENT_LIST_DIR}/strutils.c
 ${CMAKE_CURRENT_LIST_DIR}/system.c
 ${CMAKE_CURRENT_LIST_DIR}/tool_change.c
 ${CMAKE_CURRENT_LIST_DIR}/alarms.c
 ${CMAKE_CURRENT_LIST_DIR}/errors.c
 ${CMAKE_CURRENT_LIST_DIR}/ngc_params.c
 ${CMAKE_CURRENT_LIST_DIR}/ngc_expr.c
 ${CMAKE_CURRENT_LIST_DIR}/ngc_flowctrl.c
 ${CMAKE_CURRENT_LIST_DIR}/regex.c
 ${CMAKE_CURRENT_LIST_DIR}/ioports.c
 ${CMAKE_CURRENT_LIST_DIR}/utf8.c
 ${CMAKE_CURRENT_LIST_DIR}/vfs.c
 ${CMAKE_CURRENT_LIST_DIR}/fs_embedded.c
 ${CMAKE_CURRENT_LIST_DIR}/fs_ram.c
 ${CMAKE_CURRENT_LIST_DIR}/canbus.c
 ${CMAKE_CURRENT_LIST_DIR}/encoders.c
 ${CMAKE_CURRENT_LIST_DIR}/pid.c
 ${CMAKE_CURRENT_LIST_DIR}/fs_device.c
 ${CMAKE_CURRENT_LIST_DIR}/kinematics/asymmetric_ganging.c
 ${CMAKE_CURRENT_LIST_DIR}/kinematics/corexy.c
 ${CMAKE_CURRENT_LIST_DIR}/kinematics/wall_plotter.c
 ${CMAKE_CURRENT_LIST_DIR}/kinematics/delta.c
 ${CMAKE_CURRENT_LIST_DIR}/kinematics/polar.c
 ${CMAKE_CURRENT_LIST_DIR}/kinematics/rtcp_ac.c
)

# Refresh versions on every build, including tag changes without reconfiguration.
function(grbl_generate_version TARGET SOURCE MACRO OVERRIDE_NAME)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    set(${OVERRIDE_NAME} "" CACHE STRING "Explicit firmware version for source archives")
    set(HEADER "${CMAKE_CURRENT_BINARY_DIR}/${MACRO}.h")
    set(SCRIPT "${GRBL_VERSION_GENERATOR}")
    set(ARGS --source-dir "${SOURCE}" --output "${HEADER}" --macro "${MACRO}")
    if(${OVERRIDE_NAME})
        list(APPEND ARGS --version-override "${${OVERRIDE_NAME}}")
    endif()
    execute_process(COMMAND "${Python3_EXECUTABLE}" "${SCRIPT}" ${ARGS} RESULT_VARIABLE RESULT)
    if(NOT RESULT EQUAL 0)
        message(FATAL_ERROR "Could not generate ${MACRO}")
    endif()
    add_custom_target(${TARGET}_version
        COMMAND "${Python3_EXECUTABLE}" "${SCRIPT}" ${ARGS}
        BYPRODUCTS "${HEADER}" DEPENDS "${SCRIPT}" VERBATIM)
    add_dependencies(${TARGET} ${TARGET}_version)
    get_target_property(TYPE ${TARGET} TYPE)
    if(TYPE STREQUAL "INTERFACE_LIBRARY")
        set(SCOPE INTERFACE)
    else()
        set(SCOPE PUBLIC)
    endif()
    target_compile_definitions(${TARGET} ${SCOPE} ${MACRO}_HEADER="${HEADER}")
endfunction()

set(GRBL_VERSION_GENERATOR "${CMAKE_CURRENT_LIST_DIR}/tools/generate_version.py")
grbl_generate_version(grbl "${CMAKE_CURRENT_LIST_DIR}" GRBL_CORE_VERSION GRBL_CORE_VERSION_OVERRIDE)
