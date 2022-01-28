// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "mytoggle.hpp"
#include "ftxui/component/event.hpp"

namespace {
using namespace ftxui;

class MyToggleImpl : public ComponentBase {
 public:
  MyToggleImpl(const char* label_on, const char* label_off, bool* state)
      : label_on_(label_on), label_off_(label_off), state_(state) {}

 private:
  // Component implementation.
  Element Render() override {
    bool is_focused = Focused();
    bool is_active = Active();
    auto style = (is_focused || hovered_) ? inverted
                 : is_active              ? bold
                                          : nothing;
    auto focus_management = is_focused  ? focus
                            : is_active ? ftxui::select
                                        : nothing;

    Element my_text = *state_ ? text(label_on_) : text(label_off_);
    return my_text | style | focus_management | reflect(box_);
  }

  bool OnEvent(Event event) override {
    if (!CaptureMouse(event))
      return false;

    if (event.is_mouse())
      return OnMouseEvent(event);

    hovered_ = false;
    if (event == Event::Character(' ') || event == Event::Return) {
      *state_ = !*state_;
      TakeFocus();
      return true;
    }
    return false;
  }

  bool OnMouseEvent(Event event) {
    hovered_ = box_.Contain(event.mouse().x, event.mouse().y);

    if (!CaptureMouse(event))
      return false;

    if (!hovered_)
      return false;

    if (event.mouse().button == Mouse::Left &&
        event.mouse().motion == Mouse::Pressed) {
      *state_ = !*state_;
      return true;
    }

    return false;
  }

  bool Focusable() const final { return true; }

  const char* label_on_;
  const char* label_off_;
  bool* const state_;
  bool hovered_ = false;
  Box box_;
};

}  // namespace

ftxui::Component MyToggle(const char* label_on,
                          const char* label_off,
                          bool* state) {
  return ftxui::Make<MyToggleImpl>(label_on, label_off, state);
}
