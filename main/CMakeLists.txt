set (src
    src/mainGarage.cpp
    src/pwmManager.cpp
    src/kbd.cpp
    src/u8g2_esp32_hal.c
    src/StateMachine/openState.cpp
    src/StateMachine/closingState.cpp
    src/StateMachine/offlineState.cpp
    src/StateMachine/unknownState.cpp
    src/StateMachine/openingState.cpp
    src/StateMachine/closedState.cpp
    src/rmakercmds.cpp
    src/flashcommon.cpp
    src/ifrtest.cpp
    src/inits.cpp
    src/utils.cpp
    src/logmanager.cpp
    )


idf_component_register(SRCS "${src}"
INCLUDE_DIRS inc
REQUIRES driver esp_timer nvs_flash  esp_timer esp_wifi esp_event esp_netif esp_rainmaker app_wifi spiffs json console u8g2 app_update wifi_provisioning
# REQUIRES driver efuse esp_timer lwip vfs esp_wifi esp_event esp_netif esp_eth esp_phy 
# the REQUIRES name comes from the directory wherre the .h file resides. So find it via search in Components then add the directory line app_update thas several of include(TestForANSIStreamHeaders)
# esp_partition.h m VERY consuming
)


idf_build_set_property(COMPILE_OPTIONS "-Wno-missing-field-initializers" APPEND)

set(COMPONENT_ADD_INCLUDEDIRS "")
# idf_component_register(SRCS ./app_main.c
                    #    INCLUDE_DIRS ".")

