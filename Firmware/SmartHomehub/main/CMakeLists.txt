idf_component_register(
    SRCS main.c
    ui.c 
    images.c
    screens.c
    styles.c
    ui_image_ai.c
    ui_image_battery.c
    ui_image_wifi.c
    ui_image_setting.c
    ui_image_home.c
    ui_image_co2.c
    ui_image_tempature.c
    ui_image_humidity.c
    ui_image_tvoc.c
    ui_image_human.c
    ui_image_cat.c
    wifi_connect.c
    scroll_bar.c
    INCLUDE_DIRS .
    )

idf_component_get_property(LVGL_LIB lvgl__lvgl COMPONENT_LIB)
target_compile_options(
    ${LVGL_LIB} 
    PRIVATE 
        -DLV_LVGL_H_INCLUDE_SIMPLE
        -DLV_USE_DEMO_MUSIC
)
