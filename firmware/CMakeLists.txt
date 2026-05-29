cmake_minimum_required(VERSION 3.16)
project(esc32_firmware C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT DEFINED ESC_TARGET)
    set(ESC_TARGET "ESC_SIM")
endif()
if(NOT DEFINED ESC_PRODUCT_ID)
    if(DEFINED ESC_BOARD_ID)
        set(ESC_PRODUCT_ID ${ESC_BOARD_ID})
    else()
        set(ESC_PRODUCT_ID 0x80)
    endif()
endif()

set(FW_CORE
    main.c
    boards/product.c
    boards/target_meta.c
    boards/mcu_catalog.c
    app/app.c
    core/fault.c
    core/state.c
    core/scheduler.c
    core/fault_log.c
    foc/math.c
    foc/foc.c
    foc/observer.c
    motor_ctrl/motor.c
    protection/protect.c
    params/params.c
    comm/pwm.c
    comm/pwm_in.c
    comm/comm.c
    comm/dronecan.c
    bootloader/boot.c
    ../shared/protocol/protocol.c
)

set(FW_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/boards
    ${CMAKE_CURRENT_SOURCE_DIR}/boards/targets
    ${CMAKE_CURRENT_SOURCE_DIR}/app
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/foc
    ${CMAKE_CURRENT_SOURCE_DIR}/motor_ctrl
    ${CMAKE_CURRENT_SOURCE_DIR}/protection
    ${CMAKE_CURRENT_SOURCE_DIR}/params
    ${CMAKE_CURRENT_SOURCE_DIR}/comm
    ${CMAKE_CURRENT_SOURCE_DIR}/bootloader
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_SOURCE_DIR}/../shared/protocol
)

set(FW_DEFS
    ESC_TARGET_${ESC_TARGET}=1
    ESC_PRODUCT_ID=${ESC_PRODUCT_ID}
    ESC_BOARD_ID=${ESC_PRODUCT_ID}
)

if(ESC_TARGET STREQUAL "ESC_SIM")
    set(FW_HAL boards/mcu/sim/hal_sim.c)
    list(APPEND FW_DEFS ESC_PLATFORM_SIM=1)
    set(FW_EXE esc32_sim)
elseif(ESC_TARGET STREQUAL "ESC80_STM32G474_V1")
    set(FW_HAL boards/mcu/stm32g474/hal_stub.c)
    set(FW_EXE esc32_ESC80_STM32G474_V1)
else()
    message(FATAL_ERROR "Unknown ESC_TARGET=${ESC_TARGET}. See docs/命名与系列规范.md")
endif()

add_executable(${FW_EXE} ${FW_CORE} ${FW_HAL})
target_compile_definitions(${FW_EXE} PRIVATE ${FW_DEFS})
target_include_directories(${FW_EXE} PRIVATE ${FW_INCLUDES})
target_link_libraries(${FW_EXE} PRIVATE m)
if(WIN32)
    target_link_libraries(${FW_EXE} PRIVATE ws2_32)
endif()
