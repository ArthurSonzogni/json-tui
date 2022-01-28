// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef JSON_TUI_BUTTON_HPP
#define JSON_TUI_BUTTON_HPP

#include <ftxui/component/component.hpp>
#include <functional>

ftxui::Component MyButton(const char* prefix,
                          const char* title,
                          std::function<void()>);

#endif /* end of include guard: JSON_TUI_BUTTON_HPP */
