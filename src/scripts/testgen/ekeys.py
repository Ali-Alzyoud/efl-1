#!/usr/bin/env python3
# encoding: utf-8
from enum import IntEnum, IntFlag

from testgen import name_helpers


class Function_List_Type(IntFlag):
    OWN = 1
    IMPLEMENTS = 2  # Overrides
    EXTENSIONS = 4  # Interfaces/Mixins
    INHERITED = 8  # Inherited but not overriden methods and classes
    INHERITS_FULL = 4 & 8


class EKeys:
    def __init__(self, ext):
        self.ext = ext
        self.dicttypes = {}
        self.keywords = []
        self.verbs = []
        self.blacklist = ["efl_constructor"]
        self.keyloads = ["init", "shutdown", "custom"]
        self.implementsbl = ["construtor", "destructor", "finalize"]
        self.funclist = Function_List_Type.OWN | Function_List_Type.IMPLEMENTS

    def type_convert(self, eotype):
        return eotype.name

    def event_convert(self, event):
        return event.c_macro

    def print_arg(self, eoarg):
        return "arg_{}".format(eoarg.name)

    def format_name(self, func):
        return self.name


class EMonoKeys(EKeys):
    def __init__(self, ext):
        super().__init__(ext)
        self.funclist = (
            Function_List_Type.OWN
            | Function_List_Type.IMPLEMENTS
            | Function_List_Type.EXTENSIONS
        )
        self.dicttypes = {
            "byte": "sbyte",
            "llong": "long",
            "int8": "sbyte",
            "int16": "short",
            "int32": "int",
            "int64": "long",
            "ssize": "long",
            "ubyte": "byte",
            "ullong": "ulong",
            "uint8": "byte",
            "uint16": "ushort",
            "uint32": "uint",
            "uint64": "ulong",
            "size": "ulong",
            "ptrdiff": "long",
            "intptr": "System.IntPtr",
            "uintptr": "System.IntPtr",
            "void_ptr": "System.IntPtr",
            "void": "System.IntPtr",  # only if is out/inout
            "Error": "Eina.Error",
            "string": "System.String",
            "mstring": "System.String",
            "stringshare": "System.String",
            "any_value": "Eina.Value",
            "any_value_ptr": "Eina.Value"
            # complex Types
            ,
            "list": "Eina.List",
            "inlist": "Eina.Inlist",
            "array": "Eina.Array",
            "inarray": "Eina.Inarray",
            "hash": "Eina.Hash",
            "promise": "int",
            "future": "int",
            "iterator": "Eina.Iterator",
            "accessor": "Eina.Accessor",
            "strbuf": "Eina.Strbuf",
            "Efl.Class": "System.Type",
            "rw_slice": "Eina.RwSlice",
            "slice": "Eina.Slice",
        }

        self.keywords = [
            "delete",
            "do",
            "lock",
            "event",
            "in",
            "object",
            "interface",
            "string",
            "internal",
            "fixed",
            "base",
        ]

        self.verbs = [
            "add",
            "get",
            "is",
            "del",
            "thaw",
            "freeze",
            "save",
            "wait",
            "eject",
            "raise",
            "lower",
            "load",
            "dup",
            "reset",
            "unload",
            "close",
            "set",
            "interpolate",
            "has",
            "grab",
            "check",
            "find",
            "ungrab",
            "unset",
            "clear",
            "pop",
            "new",
            "peek",
            "push",
            "update",
            "show",
            "move",
            "hide",
            "calculate",
            "resize",
            "attach",
            "pack",
            "unpack",
            "emit",
            "call",
            "append",
        ]

        self.blacklist = [
            "efl_event_callback_array_priority_add",
            "efl_event_callback_forwarder_priority_add",
            "efl_player_position_get",
            "efl_text_font_source",
            "efl_ui_focus_manager_focus_get",
            "efl_ui_widget_focus",
            "efl_ui_text_password",
            "elm_interface_scrollable_repeat_events",
            "elm_wdg_item_del",
            "elm_wdg_item_focus",
            "elm_interface_scrollable_mirrored_set",
            "evas_obj_table_mirrored",
            "edje_obj_load_error_get",
            "efl_ui_focus_user_parent_get",
            "efl_canvas_object_scale",  # duplicated signature
            "efl_access_parent_get",
            "efl_access_name",
            "efl_access_root_get",
            "efl_access_type_get",
            "efl_access_role_get",
            "efl_access_action_description",
            "efl_access_image_description",
            "efl_access_component_layer_get",  # duplicated signature
            "efl_access_component_alpha_get",
            "efl_access_component_size_get",
            "efl_ui_spin_button_loop_get",
            "efl_ui_list_model_size_get",
            "efl_ui_list_relayout_layout_do",
            "efl_constructor",
        ]

        self.struct_blacklist = [
            "Efl.Event_Description",
            "Eina.Binbuf",
            "Eina.Strbuf",
            "Eina.Slice",
            "Eina.Rw_Slice",
            "Eina.Promise",
            "Eina.Value",
            "Eina.Value_Type",
            "Eina.Future",
        ]

    def escape_keyword(self, key):
        key = "kw_{}".format(key) if key in self.keywords else key
        return "{}Add".format(key) if key == "Finalize" else key

    def direction_get(self, param):
        direction = param.direction

        if direction == direction.INOUT:
            return "ref"
        elif direction != direction.IN:
            if param.type.name in ("slice", "rw_slice"):
                return "ref"
            else:
                return "out"
        elif (direction == direction.IN) and param.type.is_ptr:
            if param.type.typedecl and (
                param.type.typedecl.type == param.type.typedecl.type.STRUCT
            ):
                return "ref" if param.type.name not in self.struct_blacklist else None

        return None

    def klass_name(self, eotype):
        *namespaces, name = eotype.name.split(".")
        namespaces = [self.escape_keyword(x.lower()) for x in namespaces]
        is_interface = eotype.type == eotype.type.CLASS
        k_name = ("I" if is_interface else "") + name
        return ".".join(namespaces + [k_name])

    def type_convert(self, eotype):
        if eotype.type == eotype.type.VOID:
            return "System.IntPtr"

        new_type = self.dicttypes.get(
            eotype.name, name_helpers.type_managed_name(eotype)
        )
        if new_type not in ("Eina.RwSlice", "Eina.Slice") and eotype.base_type:
            # Stringshare is a special case where its C# type differs if inside or outside
            # a container:
            # - Non-contained stringshares are directly converted to strings.
            # - Contained stringshares are kept as the container parameter as a tag to
            #   marshal the value correctly whem adding/removing items from the container.
            if eotype.base_type.name == "stringshare":
                base_type = "Eina.Stringshare"
            else:
                base_type = self.dicttypes.get(
                    eotype.base_type.name,
                    name_helpers.type_managed_name(eotype.base_type),
                )
            new_type = "{}<{}>".format(new_type, base_type)

        return new_type

    def event_convert(self, event):
        return "{}Evt".format("".join([i.capitalize() for i in event.name.split(",")]))

    def print_arg(self, eoarg):
        r = super().print_arg(eoarg)
        prefix = self.direction_get(eoarg) or None

        return " ".join([prefix, r]) if prefix else r

    def format_name(self, func):
        names = func.comp.name.split("_")

        if func.type == func.type.METHOD and names[-1] in self.verbs:
            names.insert(0, names.pop())

        fname = "".join([name.capitalize() for name in names])

        if func.type == func.type.METHOD:
            fname = self.escape_keyword(fname)

        return fname


def GetKey(ext):
    if ext == ".cs":
        return EMonoKeys(ext)
    return EKeys(ext)