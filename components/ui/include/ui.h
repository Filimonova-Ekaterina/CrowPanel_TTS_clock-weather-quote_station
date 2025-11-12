#ifndef _CLOCK_UI_H
#define _CLOCK_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif

#include "ui_helpers.h"
#include "ui_events.h"


#include "ui_Screen1.h"
#include "ui_Screen2.h"
#include "ui_Screen3.h"
#include "ui_Screen4.h"

extern lv_obj_t * ui____initial_actions0;

LV_FONT_DECLARE(ui_font_Font1);
LV_FONT_DECLARE(ui_font_Font2);
LV_FONT_DECLARE(ui_font_Font3);
LV_FONT_DECLARE(ui_font_Font4);

void ui_init(void);
void ui_destroy(void);

#ifdef __cplusplus
}
#endif

#endif
