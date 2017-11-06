#ifdef HAVE_CONFIG_H
# include "elementary_config.h"
#endif

#ifndef EFL_BETA_API_SUPPORT
# define EFL_BETA_API_SUPPORT
#endif

#ifndef EFL_EO_API_SUPPORT
# define EFL_EO_API_SUPPORT
#endif

#define EO_BETA_API

#include <Elementary.h>

#define ERR(fmt, args...) printf("%s %d: " fmt "\n", __func__, __LINE__, ##args)


Efl_Input_Device *seat = NULL;

static void
_selection_get_cb(void *data, Efl_Event const *event)
{
   ERR("in");
   Efl_Cnp_Selection *sd = (Efl_Cnp_Selection *)((Efl_Future_Event_Success *)event->info)->value;
   printf("sel: %s, length: %d\n", (char *)sd->data, sd->length);
}

static void
//_selection_data_ready_cb(void *data, Eo *obj, void *buf, int length)
_selection_data_ready_cb(void *data, Eo *obj, Efl_Selection_Data *seldata)
{
    printf("obj: %p, data: %s, length: %d\n", obj, (char *)seldata->data, seldata->len);
}

static void
_selection_loss_cb(void *data, Efl_Event const *event)
{
    Eo *obj = data;
    ERR("obj: %p has lost selection; %p", obj, event->object);
}

static void
_selection_failure_cb(void *data, Efl_Event const *event)
{
   ERR("in");
}

static void
_selection_progress_cb(void *data, Efl_Event const *event)
{
   ERR("in");
}

static void
_selection_loss_event_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Eo *obj = event->object;
   ERR("Lost selection for obj: %p", obj);
}

static void
_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *en = data;
   //evas_object_del(en);
   //efl_unref(en);
   efl_del(en);
}

static void
_selection_get_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
   Evas_Object *win = data;
   //Evas_Object *cnp = efl_add(EFL_CNP_CLASS, win);

   efl_selection_get(obj, EFL_SELECTION_TYPE_PRIMARY, EFL_SELECTION_FORMAT_TEXT,
	   NULL, _selection_data_ready_cb, NULL, seat);
}

static void
_selection_set_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
    efl_selection_set(obj, EFL_SELECTION_TYPE_PRIMARY, EFL_SELECTION_FORMAT_TARGETS,
	    "new", 4, NULL);
}

static void
_canvas_focus_in_cb(void *data EINA_UNUSED, const Efl_Event *event)
{
   Efl_Input_Focus *ev;
   Evas_Object *focused;

   ev = event->info;
   seat = efl_input_device_get(ev);
   focused = efl_input_focus_object_get(ev);

   printf("Object %s was focused by seat %s\n",
          evas_object_name_get(focused),
          efl_name_get(seat));
}

static Efl_Object *
_drag_icon_cb(void *data, Eo *win, int *xoff, int *yoff)
{
   Evas_Object *lb = data;
   Evas_Object *icon;
   const char *text = "dnd text";
   Evas_Coord x, y, w, h;
   int xm, ym;

   icon = elm_label_add(win);
   text = elm_object_text_get(lb);
   elm_object_text_set(icon, "dnd text");

   evas_object_geometry_get(lb, &x, &y, &w, &h);
   evas_object_move(icon, x, y);
   evas_object_resize(icon, w, h);
   evas_object_show(icon);
   printf("icon: %d %d\n", w, h);

   evas_pointer_canvas_xy_get(evas_object_evas_get(lb), &xm, &ym);
   if (xoff) *xoff = xm - (w / 2);
   if (yoff) *yoff = ym - (h / 2);

   return icon;
}

static void
_dnd_drag_pos_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
   Efl_Dnd_Drag_Pos *pos = (Efl_Dnd_Drag_Pos *)ev->info;
   printf("%s:: pos: %d %d, action: %d\n", __func__, pos->x, pos->y, pos->action);
}

static void
_dnd_drag_accept_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
   Eina_Bool *accept = (Eina_Bool *)ev->info;
   printf("%s:: accept: %d\n", __func__, *accept);
}

static void
_dnd_drag_done_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
}


//drop side
static void
_dnd_drop_enter_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
}

static void
_dnd_drop_leave_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
}

static void
_dnd_drop_pos_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
}

static void
_dnd_drop_drop_cb(void *data, const Efl_Event *ev)
{
   printf("%s:: In\n", __func__);
}

static void
_en_mouse_down_cb(void *data, Evas *e, Evas_Object *obj, void *event)
{
   printf("dnd start\n");
   Evas_Object *en = data;
   efl_event_callback_add(en, EFL_DND_DRAG_EVENT_DRAG_POS, _dnd_drag_pos_cb, en);
   efl_event_callback_add(en, EFL_DND_DRAG_EVENT_DRAG_ACCEPT, _dnd_drag_accept_cb, en);
   efl_event_callback_add(en, EFL_DND_DRAG_EVENT_DRAG_DONE, _dnd_drag_done_cb, en);
   efl_dnd_drag_start(en, EFL_SELECTION_FORMAT_TEXT, "dnd Text", 9,
                 EFL_SELECTION_ACTION_COPY, en, _drag_icon_cb, NULL);
}


EAPI_MAIN int
elm_main(int argc EINA_UNUSED, char **argv EINA_UNUSED)
{
   Evas_Object *win, *bx, *bt, *bt2, *en;

   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);
   win = elm_win_util_standard_add("entry", "Entry");
   elm_win_autodel_set(win, EINA_TRUE);

   //getting seat
   Evas *e = evas_object_evas_get(win);
   efl_event_callback_add(e, EFL_EVENT_FOCUS_IN, _canvas_focus_in_cb, win);
   //

   bx = elm_box_add(win);
   evas_object_size_hint_weight_set(bx, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   elm_win_resize_object_add(win, bx);
   evas_object_show(bx);

   /*en = efl_add(EFL_UI_TEXT_CLASS, win,
         efl_text_multiline_set(efl_added, EINA_TRUE));

   printf("Added Efl.Ui.Text object\n");
   efl_key_data_set(en, "wrap_idx", 0);
   efl_text_set(en, "Hello world! Goodbye world! This is a test text for the"
         " new UI Text widget.\xE2\x80\xA9This is the next paragraph.\nThis"
         " is the next line.\nThis is Yet another line! Line and paragraph"
         " separators are actually different!");
   efl_text_font_set(en, "Sans", 14);
   efl_text_font_weight_set(en, EFL_TEXT_FONT_WEIGHT_BOLD);
   efl_text_font_slant_set(en, EFL_TEXT_FONT_SLANT_ITALIC);
   efl_text_font_width_set(en, EFL_TEXT_FONT_WIDTH_ULTRACONDENSED);
   efl_text_normal_color_set(en, 255, 255, 255, 255);*/


   bt = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(bt, "test sel/drop");
   efl_gfx_visible_set(bt, EINA_TRUE);
   efl_event_callback_add(bt, EFL_DND_DROP_EVENT_DRAG_ENTER, _dnd_drop_enter_cb, bt);
   efl_event_callback_add(bt, EFL_DND_DROP_EVENT_DRAG_LEAVE, _dnd_drop_leave_cb, bt);
   efl_event_callback_add(bt, EFL_DND_DROP_EVENT_DRAG_POS, _dnd_drop_pos_cb, bt);
   efl_event_callback_add(bt, EFL_DND_DROP_EVENT_DRAG_DROP, _dnd_drop_drop_cb, bt);
   efl_dnd_drop_target_add(bt, EFL_SELECTION_FORMAT_TEXT);
   elm_box_pack_end(bx, bt);

   en = efl_add(EFL_UI_TEXT_CLASS, win);
   efl_text_set(en, "Test Dnd");
   efl_ui_text_interactive_editable_set(en, EINA_FALSE);
   efl_gfx_visible_set(en, EINA_TRUE);
   elm_box_pack_end(bx, en);
   evas_object_event_callback_add(en, EVAS_CALLBACK_MOUSE_DOWN, _en_mouse_down_cb, en);
   

   //test cnp
   //Evas_Object *cnp = efl_add(EFL_CNP_CLASS, en);
   //efl_selection_set(bt, EFL_SELECTION_TYPE_PRIMARY, EFL_SELECTION_FORMAT_TEXT, "abc", 3, NULL);

   //efl_selection_get(bt, EFL_SELECTION_TYPE_PRIMARY, EFL_SELECTION_FORMAT_TEXT, NULL, _selection_data_ready_cb, NULL, seat);

   //efl_event_callback_add(bt, EFL_SELECTION_EVENT_SELECTION_LOSS, _selection_loss_event_cb, NULL);

   /*Efl_Future *f = efl_selection_loss_feedback(bt, EFL_SELECTION_TYPE_PRIMARY);
   if (f)
   {
       printf("register future callbacks\n");
       efl_future_then(f, _selection_loss_cb, NULL, NULL, bt);
   }*/
   //

   /*bt2 = efl_add(EFL_UI_BUTTON_CLASS, win);
   efl_text_set(bt2, "get selection");
   efl_gfx_visible_set(bt2, EINA_TRUE);
   elm_box_pack_end(bx, bt2);

   Efl_Future *f2 = efl_selection_get(bt2, EFL_CNP_TYPE_PRIMARY, EFL_CNP_FORMAT_TEXT, seat);
   if (f2)
   {
       ERR("register callbacks");
       efl_future_then(f2, _selection_get_cb, _selection_failure_cb, _selection_progress_cb, bt2);
   }
   else
   {
       ERR("future is NULL");
   }*/
   //

   /*efl_ui_text_interactive_editable_set(en, EINA_TRUE);
   efl_ui_text_scrollable_set(en, EINA_TRUE);
   evas_object_size_hint_weight_set(en, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(en, EVAS_HINT_FILL, EVAS_HINT_FILL);
   elm_box_pack_end(bx, en);
   evas_object_show(en);
   elm_object_focus_set(en, EINA_TRUE);*/

   Evas_Object *hbox = elm_box_add(win);
   elm_box_horizontal_set(hbox, EINA_TRUE);
   evas_object_size_hint_weight_set(hbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
   evas_object_size_hint_align_set(hbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
   evas_object_show(hbox);
   elm_box_pack_end(bx, hbox);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Delete");
   evas_object_smart_callback_add(bt, "clicked", _delete_cb, en);
   evas_object_show(bt);
   elm_box_pack_end(hbox, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Selection Get");
   evas_object_smart_callback_add(bt, "clicked", _selection_get_btn_cb, win);
   evas_object_show(bt);
   elm_box_pack_end(hbox, bt);

   bt = elm_button_add(win);
   elm_object_text_set(bt, "Selection Set");
   evas_object_smart_callback_add(bt, "clicked", _selection_set_btn_cb, win);
   efl_event_callback_add(bt, EFL_SELECTION_EVENT_SELECTION_LOSS, _selection_loss_event_cb, NULL);
   evas_object_show(bt);
   elm_box_pack_end(hbox, bt);


   evas_object_resize(win, 480, 320);
   evas_object_show(win);

   //efl_del(cnp);

   elm_run();

   return 0;
}
ELM_MAIN()
