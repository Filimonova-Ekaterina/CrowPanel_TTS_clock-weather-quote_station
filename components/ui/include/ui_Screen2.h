#ifndef UI_SCREEN2_H
#define UI_SCREEN2_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ui_Screen2_screen_init(void);
extern void ui_Screen2_screen_destroy(void);
extern void ui_event_Screen2(lv_event_t * e);
extern void ui_event_Connect(lv_event_t * e);
extern void ui_event_LeftNavBtn_S2(lv_event_t * e);
extern void ui_event_RightNavBtn_S2(lv_event_t * e);
extern lv_obj_t * ui_Screen2;
extern lv_obj_t * ui_Panel13;
extern lv_obj_t * ui_Panel6;
extern lv_obj_t * ui_Label7;
extern lv_obj_t * ui_wifiLb;
extern lv_obj_t * ui_Panel8;
extern lv_obj_t * ui_Panel10;
extern lv_obj_t * ui_Label1;
extern lv_obj_t * ui_passLb;
extern lv_obj_t * ui_Container1;
extern lv_obj_t * ui_Button1;
extern lv_obj_t * ui_Label3;
extern lv_obj_t * ui_Panel7;
extern lv_obj_t * ui_Panel5;
extern lv_obj_t * ui_Panel4;
extern lv_obj_t * ui_ShowPassBtn;
extern lv_obj_t * ui_LeftNavBtn_S2;
extern lv_obj_t * ui_RightNavBtn_S2;

#ifdef __cplusplus
}
#endif

#endif

