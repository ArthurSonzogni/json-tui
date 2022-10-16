// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "button.hpp"
#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>

using namespace ftxui;

Component MyButton(const char* prefix,
                   const char* title,
                   std::function<void()> on_click) {
  class Impl : public ComponentBase {
   public:
    Impl(const char* prefix, const char* title, std::function<void()> on_click)
        : on_click_(on_click), prefix_(prefix), title_(title) {}

    // Component implementation:
    Element Render() override {
      auto style = Focused() ? (Decorator(inverted) | focus) : nothing;
      return hbox({
          text(prefix_),
          text(title_) | style | color(Color::GrayDark) | reflect(box_),
      });
    }

    bool OnEvent(Event event) override {
      if (event.is_mouse() && box_.Contain(event.mouse().x, event.mouse().y)) {
        if (!CaptureMouse(event))
          return false;

        TakeFocus();

        if (event.mouse().button == Mouse::Left &&
            event.mouse().motion == Mouse::Pressed) {
          on_click_();
          return true;
        }

        return false;
      }

      if (event == Event::Return) {
        on_click_();
        return true;
      }
      return false;
    }

    bool Focusable() const final { return true; }

   private:
    std::function<void()> on_click_;
    const char* prefix_;
    const char* title_;
    Box box_;
  };

  return Make<Impl>(prefix, title, std::move(on_click));
}
