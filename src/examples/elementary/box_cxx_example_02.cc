/*
 * Copyright 2019 by its authors. See AUTHORS.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// g++ -g `pkg-config --cflags --libs elementary-cxx efl-cxx eina-cxx eo-cxx ecore-cxx evas-cxx edje-cxx` box_cxx_example_02.cc -o box_cxx_example_02

#include <Efl_Ui.hh>

using efl::eo::instantiate;

static void
efl_main(void *data EINA_UNUSED, const Efl_Event *ev EINA_UNUSED)
{
   elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_HIDDEN);

   efl::ui::Win win(instantiate);
   win.text_set("Simple Box Example");
   win.autohide_set(true);

   efl::ui::Box box(instantiate, win);
   win.content_set(box);

   for (int i = 0; i < 5; i++)
     {
        efl::ui::Button bt(instantiate, win);
        bt.text_set("Button " + std::to_string(i));
        box.pack_end(bt);
     }

   win.size_set({320,320});
}
EFL_MAIN()
