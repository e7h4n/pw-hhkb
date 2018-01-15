cmake_minimum_required(VERSION 3.6)
project(pw-hhkb C ASM)

include(CMakeEnv.cmake)
include(CMake_nRF5x.cmake)

nRF5x_setup()

nRF5x_addAppFDS()
nRF5x_addAppScheduler()
nRF5x_addAppTimer()
nRF5x_addBLEAdvertising()
nRF5x_addBLEGATT()
nRF5x_addBLEPeerManager()

include_directories(
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_bas
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_dis
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hids
        ${NRF5_SDK_PATH}/components/libraries/sensorsim
)

list(
        APPEND
        SDK_SOURCE_FILES
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_bas/ble_bas.c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_dis/ble_dis.c
        ${NRF5_SDK_PATH}/components/ble/ble_services/ble_hids/ble_hids.c
        ${NRF5_SDK_PATH}/components/libraries/sensorsim/sensorsim.c
        ${NRF5_SDK_PATH}/components/libraries/timer/app_timer_appsh.c
)


include_directories(
        ./src
)

list(
        APPEND
        SOURCE_FILES
        src/keyboard/keyboard.c
        src/keyboard/matrix.c
        src/service/advertising.c
        src/service/battery.c
        src/service/ble.c
        src/service/connection.c
        src/service/device.c
        src/service/hid.c
        src/service/peer_manager.c
        src/util/error.c
        src/util/util.c
        src/main.c
)

nRF5x_addExecutable(${PROJECT_NAME} "${SOURCE_FILES}")