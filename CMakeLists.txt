cmake_minimum_required(VERSION 3.6)
project(pw-hhkb C ASM)

include(CMakeEnv.cmake)

if (NOT ARM_NONE_EABI_TOOLCHAIN_PATH)
    message(FATAL_ERROR "The path to the arm-none-eabi-gcc toolchain (ARM_NONE_EABI_TOOLCHAIN_PATH) must be set.")
endif ()

if (NOT NRF5_SDK_PATH)
    message(FATAL_ERROR "The path to the nRF5 SDK (NRF5_SDK_PATH) must be set.")
endif ()

if (NOT NRFJPROG)
    message(FATAL_ERROR "The path to the nrfjprog utility (NRFJPROG) must be set.")
endif ()

set(EXECUTABLE_NAME pw-hhkb)
# fix on macOS: prevent cmake from adding implicit parameters to Xcode
set(CMAKE_OSX_SYSROOT "/")
set(CMAKE_OSX_DEPLOYMENT_TARGET "")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 98)

set(CMAKE_C_COMPILER "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin/arm-none-eabi-c++")
set(CMAKE_ASM_COMPILER "${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin/arm-none-eabi-gcc")

set(NRF5_LINKER_SCRIPT "${CMAKE_SOURCE_DIR}/gcc_nrf52_xxaa.ld")
set(CPU_FLAGS "-mcpu=cortex-m4 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -u _printf_float")
add_definitions(-DNRF5 -DNRF52 -DNRF52832_XXAA -DUSE_LFXO)
add_definitions(-DSOFTDEVICE_PRESENT)
add_definitions(-DUSE_APP_CONFIG)

set(COMMON_FLAGS "-MP -MD -mthumb -mabi=aapcs -Wall -Werror -O3 -g3 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin --short-enums ${CPU_FLAGS} -DNRF_SD_BLE_API_VERSION=5")

# compiler/assambler/linker flags
set(CMAKE_C_FLAGS "${COMMON_FLAGS}")
set(CMAKE_CXX_FLAGS "${COMMON_FLAGS}")
set(CMAKE_ASM_FLAGS "-MP -MD -std=c99 -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS "-flto -O3 -g3 -mthumb -mabi=aapcs -L ${NRF5_SDK_PATH}/components/toolchain/gcc -T${NRF5_LINKER_SCRIPT} ${CPU_FLAGS} -Wl,--gc-sections --specs=nano.specs -lc -lnosys -lm")
# note: we must override the default cmake linker flags so that CMAKE_C_FLAGS are not added implicitly
set(CMAKE_C_LINK_EXECUTABLE "${CMAKE_C_COMPILER} <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_C_COMPILER} <LINK_FLAGS> <OBJECTS> -lstdc++ -o <TARGET>")

include_directories(
        ${NRF5_SDK_PATH}/components
        ${NRF5_SDK_PATH}/components/ble/ble_advertising
        ${NRF5_SDK_PATH}/components/ble/ble_dtm
        ${NRF5_SDK_PATH}/components/ble/ble_racp
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_ancs_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_ans_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_bas
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_bas_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_cscs
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_cts_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_dfu
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_dis
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_gls
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hids
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hrs
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hrs_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hts
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_ias
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_ias_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_lbs
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_lbs_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_lls
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_nus
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_nus_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_rscs
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_rscs_c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_tps
        ${NRF5_SDK_PATH}/components/ble/common
        ${NRF5_SDK_PATH}/components/ble/nrf_ble_gatt
        ${NRF5_SDK_PATH}/components/ble/nrf_ble_qwr
        ${NRF5_SDK_PATH}/components/ble/peer_manager
        ${NRF5_SDK_PATH}/components/device
        ${NRF5_SDK_PATH}/components/drivers_nrf/clock
        ${NRF5_SDK_PATH}/components/drivers_nrf/common
        ${NRF5_SDK_PATH}/components/drivers_nrf/comp
        ${NRF5_SDK_PATH}/components/drivers_nrf/delay
        ${NRF5_SDK_PATH}/components/drivers_nrf/gpiote
        ${NRF5_SDK_PATH}/components/drivers_nrf/hal
        ${NRF5_SDK_PATH}/components/drivers_nrf/i2s
        ${NRF5_SDK_PATH}/components/drivers_nrf/lpcomp
        ${NRF5_SDK_PATH}/components/drivers_nrf/pdm
        ${NRF5_SDK_PATH}/components/drivers_nrf/power
        ${NRF5_SDK_PATH}/components/drivers_nrf/ppi
        ${NRF5_SDK_PATH}/components/drivers_nrf/pwm
        ${NRF5_SDK_PATH}/components/drivers_nrf/qdec
        ${NRF5_SDK_PATH}/components/drivers_nrf/rng
        ${NRF5_SDK_PATH}/components/drivers_nrf/rtc
        ${NRF5_SDK_PATH}/components/drivers_nrf/saadc
        ${NRF5_SDK_PATH}/components/drivers_nrf/spi_master
        ${NRF5_SDK_PATH}/components/drivers_nrf/spi_slave
        ${NRF5_SDK_PATH}/components/drivers_nrf/swi
        ${NRF5_SDK_PATH}/components/drivers_nrf/timer
        ${NRF5_SDK_PATH}/components/drivers_nrf/twi_master
        ${NRF5_SDK_PATH}/components/drivers_nrf/twis_slave
        ${NRF5_SDK_PATH}/components/drivers_nrf/uart
        ${NRF5_SDK_PATH}/components/drivers_nrf/usbd
        ${NRF5_SDK_PATH}/components/drivers_nrf/wdt
        ${NRF5_SDK_PATH}/components/libraries/atomic
        ${NRF5_SDK_PATH}/components/libraries/atomic_fifo
        ${NRF5_SDK_PATH}/components/libraries/balloc
        ${NRF5_SDK_PATH}/components/libraries/cli
        ${NRF5_SDK_PATH}/components/libraries/crc16
        ${NRF5_SDK_PATH}/components/libraries/crc32
        ${NRF5_SDK_PATH}/components/libraries/csense
        ${NRF5_SDK_PATH}/components/libraries/csense_drv
        ${NRF5_SDK_PATH}/components/libraries/ecc
        ${NRF5_SDK_PATH}/components/libraries/experimental_memobj
        ${NRF5_SDK_PATH}/components/libraries/experimental_section_vars
        ${NRF5_SDK_PATH}/components/libraries/fds
        ${NRF5_SDK_PATH}/components/libraries/fstorage
        ${NRF5_SDK_PATH}/components/libraries/gpiote
        ${NRF5_SDK_PATH}/components/libraries/hardfault
        ${NRF5_SDK_PATH}/components/libraries/hci
        ${NRF5_SDK_PATH}/components/libraries/led_softblink
        ${NRF5_SDK_PATH}/components/libraries/low_power_pwm
        ${NRF5_SDK_PATH}/components/libraries/mem_manager
        ${NRF5_SDK_PATH}/components/libraries/mutex
        ${NRF5_SDK_PATH}/components/libraries/pwm
        ${NRF5_SDK_PATH}/components/libraries/pwr_mgmt
        ${NRF5_SDK_PATH}/components/libraries/queue
        ${NRF5_SDK_PATH}/components/libraries/scheduler
        ${NRF5_SDK_PATH}/components/libraries/sensorsim
        ${NRF5_SDK_PATH}/components/libraries/slip
        ${NRF5_SDK_PATH}/components/libraries/strerror
        ${NRF5_SDK_PATH}/components/libraries/timer
        ${NRF5_SDK_PATH}/components/libraries/twi
        ${NRF5_SDK_PATH}/components/libraries/twi_mngr
        ${NRF5_SDK_PATH}/components/libraries/uart
        ${NRF5_SDK_PATH}/components/libraries/usbd
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/audio
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/cdc/acm
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/hid
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/generic
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/kbd
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/hid/mouse
        ${NRF5_SDK_PATH}/components/libraries/usbd/class/msc
        ${NRF5_SDK_PATH}/components/libraries/usbd/config
        ${NRF5_SDK_PATH}/components/libraries/util
        ${NRF5_SDK_PATH}/components/softdevice/common
        ${NRF5_SDK_PATH}/components/softdevice/s132/headers
        ${NRF5_SDK_PATH}/components/softdevice/s132/headers/nrf52
        ${NRF5_SDK_PATH}/components/toolchain
        ${NRF5_SDK_PATH}/components/toolchain/cmsis/include
        ${NRF5_SDK_PATH}/components/toolchain/gcc
        ${NRF5_SDK_PATH}/components/libraries/experimental_log
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/
        ${NRF5_SDK_PATH}/external/fprintf
        ${NRF5_SDK_PATH}/external/segger_rtt
)

list(
        APPEND
        SDK_SOURCE_FILES
        ${NRF5_SDK_PATH}/components/ble/ble_advertising/ble_advertising.c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_bas/ble_bas.c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_dis/ble_dis.c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hids/ble_hids.c
        ${NRF5_SDK_PATH}/components/ble/common/ble_advdata.c
        ${NRF5_SDK_PATH}/components/ble/common/ble_conn_params.c
        ${NRF5_SDK_PATH}/components/ble/common/ble_conn_state.c
        ${NRF5_SDK_PATH}/components/ble/common/ble_srv_common.c
        ${NRF5_SDK_PATH}/components/ble/nrf_ble_gatt/nrf_ble_gatt.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/gatt_cache_manager.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/gatts_cache_manager.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/id_manager.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/peer_data_storage.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/peer_database.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/peer_id.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/peer_manager.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/pm_buffer.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/pm_mutex.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/security_dispatcher.c
        ${NRF5_SDK_PATH}/components/ble/peer_manager/security_manager.c
        ${NRF5_SDK_PATH}/components/drivers_nrf/clock/nrf_drv_clock.c
        ${NRF5_SDK_PATH}/components/drivers_nrf/common/nrf_drv_common.c
        ${NRF5_SDK_PATH}/components/drivers_nrf/gpiote/nrf_drv_gpiote.c
        ${NRF5_SDK_PATH}/components/drivers_nrf/uart/nrf_drv_uart.c
        ${NRF5_SDK_PATH}/components/libraries/atomic_fifo/nrf_atfifo.c
        ${NRF5_SDK_PATH}/components/libraries/balloc/nrf_balloc.c
        ${NRF5_SDK_PATH}/components/libraries/crc16/crc16.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_memobj/nrf_memobj.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_section_vars/nrf_section_iter.c
        ${NRF5_SDK_PATH}/components/libraries/fds/fds.c
        ${NRF5_SDK_PATH}/components/libraries/fstorage/nrf_fstorage.c
        ${NRF5_SDK_PATH}/components/libraries/fstorage/nrf_fstorage_sd.c
        ${NRF5_SDK_PATH}/components/libraries/hardfault/hardfault_implementation.c
        ${NRF5_SDK_PATH}/components/libraries/pwr_mgmt/nrf_pwr_mgmt.c
        ${NRF5_SDK_PATH}/components/libraries/scheduler/app_scheduler.c
        ${NRF5_SDK_PATH}/components/libraries/sensorsim/sensorsim.c
        ${NRF5_SDK_PATH}/components/libraries/strerror/nrf_strerror.c
        ${NRF5_SDK_PATH}/components/libraries/timer/app_timer.c
        ${NRF5_SDK_PATH}/components/libraries/util/app_error.c
        ${NRF5_SDK_PATH}/components/libraries/util/app_error_weak.c
        ${NRF5_SDK_PATH}/components/libraries/util/app_util_platform.c
        ${NRF5_SDK_PATH}/components/libraries/util/nrf_assert.c
        ${NRF5_SDK_PATH}/components/libraries/util/sdk_mapped_flags.c
        ${NRF5_SDK_PATH}/components/softdevice/common/nrf_sdh.c
        ${NRF5_SDK_PATH}/components/softdevice/common/nrf_sdh_ble.c
        ${NRF5_SDK_PATH}/components/softdevice/common/nrf_sdh_soc.c
        ${NRF5_SDK_PATH}/components/toolchain/gcc/gcc_startup_nrf52.S
        ${NRF5_SDK_PATH}/components/toolchain/system_nrf52.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/nrf_log_frontend.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/nrf_log_default_backends.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/nrf_log_backend_rtt.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/nrf_log_backend_uart.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/nrf_log_backend_serial.c
        ${NRF5_SDK_PATH}/components/libraries/experimental_log/src/nrf_log_str_formatter.c
        ${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf.c
        ${NRF5_SDK_PATH}/external/fprintf/nrf_fprintf_format.c
        ${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT.c
        ${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_printf.c
        ${NRF5_SDK_PATH}/external/segger_rtt/SEGGER_RTT_Syscalls_GCC.c
)

include_directories(
        .
        src
)

list(
        APPEND
        SOURCE_FILES
        src/example.c
)

add_executable(${EXECUTABLE_NAME} ${SDK_SOURCE_FILES} ${SOURCE_FILES})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES SUFFIX ".out")
set_target_properties(${EXECUTABLE_NAME} PROPERTIES LINK_FLAGS "-Wl,-Map=${EXECUTABLE_NAME}.map")

add_custom_command(TARGET ${EXECUTABLE_NAME}
        POST_BUILD
        COMMAND ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin/arm-none-eabi-size ${EXECUTABLE_NAME}.out
        COMMAND ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin/arm-none-eabi-objcopy -O binary ${EXECUTABLE_NAME}.out "${EXECUTABLE_NAME}.bin"
        COMMAND ${ARM_NONE_EABI_TOOLCHAIN_PATH}/bin/arm-none-eabi-objcopy -O ihex ${EXECUTABLE_NAME}.out "${EXECUTABLE_NAME}.hex"
        COMMENT "post build steps for ${EXECUTABLE_NAME}")

# custom target for flashing the board
set(SOFTDEVICE_PATH "${NRF5_SDK_PATH}/components/softdevice/s132/hex/s132_nrf52_5.0.0_softdevice.hex")

add_custom_target("FLASH_${EXECUTABLE_NAME}" ${EXECUTABLE_NAME}
        COMMAND ${NRFJPROG} --program ${EXECUTABLE_NAME}.hex -f NRF52 --sectorerase
        COMMAND sleep 0.5s
        COMMAND ${NRFJPROG} --reset -f NRF52
        DEPENDS ${EXECUTABLE_NAME}
        COMMENT "flashing ${EXECUTABLE_NAME}.hex"
        )

add_custom_target(FLASH_SOFTDEVICE
        COMMAND ${NRFJPROG} --program ${SOFTDEVICE_PATH} -f NRF52 --sectorerase
        COMMAND sleep 0.5s
        COMMAND ${NRFJPROG} --reset -f NRF52
        COMMENT "flashing SoftDevice"
        )
