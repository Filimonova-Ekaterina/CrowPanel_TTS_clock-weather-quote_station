set(SRCS 
    ui_Screen1.c
    ui.c
    ui_Screen2.c
    ui_Screen3.c
    ui_helpers.c
    ui_events.c
    ui_font_Font1.c
    ui_font_Font2.c
    ui_font_Font3.c
    ui_font_Font4.c
    tts_bridge_impl.c)

set(INCLUDE_DIRS "include")


idf_component_register(
    SRCS ${SRCS}
    INCLUDE_DIRS ${INCLUDE_DIRS}
    REQUIRES lvgl__lvgl espressif__esp32_display_panel
)
