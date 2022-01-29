// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef JSON_TUI_MYTOGGLE_HPP
#define JSON_TUI_MYTOGGLE_HPP

#include "ftxui/component/component.hpp"

ftxui::Component MyToggle(const char* label_on,
                          const char* label_off,
                          bool* state);

#endif /* end of include guard: JSON_TUI_MYTOGGLE_HPP */
