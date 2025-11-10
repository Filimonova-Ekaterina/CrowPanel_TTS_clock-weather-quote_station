#ifndef UI_SCREEN1_H
#define UI_SCREEN1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

void ui_Screen1_screen_init(void);
void ui_Screen1_screen_destroy(void);
void ui_event_SpeakButton(lv_event_t * e);
void ui_event_FormatSwitch(lv_event_t * e);
extern void ui_event_Screen1(lv_event_t * e);
extern void ui_event_LeftNavBtn_S1(lv_event_t * e);
extern void ui_event_RightNavBtn_S1(lv_event_t * e);
extern lv_obj_t * ui_Screen1;
extern lv_obj_t * ui_SpeakButton;
extern lv_obj_t * ui_AnalogClock;
extern lv_obj_t * ui_SpeakButtonLabel;
extern lv_obj_t * ui_TimeLabel;
extern lv_obj_t * ui_DateLabel;
extern lv_obj_t * ui_StatusLabel;
extern lv_obj_t * ui_FormatSwitch;
extern lv_obj_t * ui_Format24Label;
extern lv_obj_t * ui_Format12Label;
extern lv_obj_t * ui_SpeakDateButton;
extern lv_obj_t * ui_SpeakDateButtonLabel;
extern lv_obj_t * ui_LeftNavBtn_S1;
extern lv_obj_t * ui_RightNavBtn_S1;

#ifdef __cplusplus
}
#endif

#endif