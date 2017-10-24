#ifndef EFL_UI_POPUP_ALERT_SCROLL_H
#define EFL_UI_POPUP_ALERT_SCROLL_H

#include "Elementary.h"

typedef struct _Efl_Ui_Popup_Alert_Scroll_Data Efl_Ui_Popup_Alert_Scroll_Data;
struct _Efl_Ui_Popup_Alert_Scroll_Data
{
   Eo          *scroller;
   Eo          *content;
   Eina_Size2D  max_scroll;
   Eina_Bool    is_expandable_w : 1;
   Eina_Bool    is_expandable_h : 1;
   Eina_Bool    is_sizing_eval : 1;
};

#endif
