idf_component_register(
    SRCS
    "lvgl_v8_port.cpp"
    "main.cpp"
    "HxTTS.cpp"
    "uart.c"
    "time_formatter.cpp"
    "timezone_service.cpp"
    "weather_service.cpp"
    "location_service.cpp"
    "hm_ctrl/crc.c"
    "hm_ctrl/crc_table.c"
    "hm_ctrl/hm_comm_protocol.c"
    "hm_ctrl/hm_regs.c"
    "uart_json.c"
    "uart_manager.c"
    "uart_rx_task.c"
    "tts_bridge.cpp"
    INCLUDE_DIRS
    .
    "hm_ctrl"
    PRIV_REQUIRES
    json
    REQUIRES
    driver
    esp_timer
    lvgl__lvgl
    ui
    esp_wifi
    esp_netif
    nvs_flash
    lwip
)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-missing-field-initializers)

idf_build_get_property(build_components BUILD_COMPONENTS)
foreach(COMPONENT ${build_components})
    if(COMPONENT MATCHES "lvgl" OR COMPONENT MATCHES "lvgl__lvgl")
        set(TARGET_COMPONENT ${COMPONENT})
        break()
    endif()
endforeach()

if(TARGET_COMPONENT STREQUAL "")
    message(FATAL_ERROR "Component 'lvgl' not found.")
else()
    idf_component_get_property(LVGL_LIB ${TARGET_COMPONENT} COMPONENT_LIB)
endif()
target_compile_options(${LVGL_LIB} PRIVATE "-Wno-format")
set(TARGET_COMPONENT "")