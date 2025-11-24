#ifndef EEZ_LVGL_UI_IMAGES_H
#define EEZ_LVGL_UI_IMAGES_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

extern const lv_img_dsc_t img_setting;
extern const lv_img_dsc_t img_wifi;
extern const lv_img_dsc_t img_battery;
extern const lv_img_dsc_t img_human;
extern const lv_img_dsc_t img_home;
extern const lv_img_dsc_t img_cat;
extern const lv_img_dsc_t img_ai;
extern const lv_img_dsc_t img_co2;
extern const lv_img_dsc_t img_tvoc;
extern const lv_img_dsc_t img_tempature;
extern const lv_img_dsc_t img_humidity;

#ifndef EXT_IMG_DESC_T
#define EXT_IMG_DESC_T
typedef struct _ext_img_desc_t {
    const char *name;
    const lv_img_dsc_t *img_dsc;
} ext_img_desc_t;
#endif

extern const ext_img_desc_t images[11];


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_IMAGES_H*/