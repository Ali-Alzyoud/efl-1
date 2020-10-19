
ELM_API void
elm_popup_align_set(Elm_Popup *obj, double horizontal, double vertical)
{
   elm_obj_popup_align_set(obj, horizontal, vertical);
}

ELM_API void
elm_popup_align_get(const Elm_Popup *obj, double *horizontal, double *vertical)
{
   elm_obj_popup_align_get(obj, horizontal, vertical);
}

ELM_API void
elm_popup_allow_events_set(Elm_Popup *obj, Eina_Bool allow)
{
   elm_obj_popup_allow_events_set(obj, allow);
}

ELM_API Eina_Bool
elm_popup_allow_events_get(const Elm_Popup *obj)
{
   return elm_obj_popup_allow_events_get(obj);
}

ELM_API void
elm_popup_content_text_wrap_type_set(Elm_Popup *obj, Elm_Wrap_Type wrap)
{
   elm_obj_popup_content_text_wrap_type_set(obj, wrap);
}

ELM_API Elm_Wrap_Type
elm_popup_content_text_wrap_type_get(const Elm_Popup *obj)
{
   return elm_obj_popup_content_text_wrap_type_get(obj);
}

ELM_API void
elm_popup_orient_set(Elm_Popup *obj, Elm_Popup_Orient orient)
{
   elm_obj_popup_orient_set(obj, orient);
}

ELM_API Elm_Popup_Orient
elm_popup_orient_get(const Elm_Popup *obj)
{
   return elm_obj_popup_orient_get(obj);
}

ELM_API void
elm_popup_timeout_set(Elm_Popup *obj, double timeout)
{
   elm_obj_popup_timeout_set(obj, timeout);
}

ELM_API double
elm_popup_timeout_get(const Elm_Popup *obj)
{
   return elm_obj_popup_timeout_get(obj);
}

ELM_API void
elm_popup_scrollable_set(Elm_Popup *obj, Eina_Bool scroll)
{
   elm_obj_popup_scrollable_set(obj, scroll);
}

ELM_API Eina_Bool
elm_popup_scrollable_get(const Elm_Popup *obj)
{
   return elm_obj_popup_scrollable_get(obj);
}

ELM_API Elm_Widget_Item *
elm_popup_item_append(Elm_Popup *obj, const char *label, Efl_Canvas_Object *icon, Evas_Smart_Cb func, const void *data)
{
   return elm_obj_popup_item_append(obj, label, icon, func, data);
}

ELM_API void
elm_popup_dismiss(Elm_Popup *obj)
{
   elm_obj_popup_dismiss(obj);
}
