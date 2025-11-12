#ifndef UI_SCREEN4_H
#define UI_SCREEN4_H

#ifdef __cplusplus
extern "C" {
#endif

extern void ui_Screen4_screen_init(void);
extern void ui_Screen4_screen_destroy(void);
extern void ui_event_Screen4(lv_event_t * e);
extern lv_obj_t * ui_Screen4;
extern lv_obj_t * ui_LeftNavBtn_S4;
extern lv_obj_t * ui_RightNavBtn_S4;
extern lv_obj_t * ui_QuoteStatusLabel;
extern lv_obj_t * ui_QuoteTextArea;
extern lv_obj_t * ui_UpdateQuoteBtn;
extern lv_obj_t * ui_SpeakQuoteBtn;
extern lv_obj_t * ui_NavDots_S4;
extern lv_obj_t * ui_NavDot1_S4;
extern lv_obj_t * ui_NavDot2_S4;
extern lv_obj_t * ui_NavDot3_S4;
extern lv_obj_t * ui_NavDot4_S4;

extern void ui_event_LeftNavBtn_S4(lv_event_t * e);
extern void ui_event_RightNavBtn_S4(lv_event_t * e);
extern void ui_event_UpdateQuoteBtn(lv_event_t * e);
extern void ui_event_SpeakQuoteBtn(lv_event_t * e);
extern void update_quote_status(const char* status, bool is_error);
extern void update_quote_text(const char* quote_text);
extern void lock_quote_buttons(void);
extern void unlock_quote_buttons(void);

#ifdef __cplusplus
} 
#endif

#endif