// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "main_ui.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include "button.hpp"
#include "expander.hpp"
#include "mytoggle.hpp"

using JSON = nlohmann::json;
using namespace ftxui;

namespace {

Component From(const JSON& json, bool is_last, int depth, Expander& expander);
Component FromString(const JSON& json, bool is_last);
Component FromNumber(const JSON& json, bool is_last);
Component FromBoolean(const JSON& json, bool is_last);
Component FromNull(bool is_last);
Component FromObject(Component prefix,
                     const JSON& json,
                     bool is_last,
                     int depth,
                     Expander& expander);
Component FromArrayAny(Component prefix,
                       const JSON& json,
                       bool is_last,
                       int depth,
                       Expander& expander);
Component FromArray(Component prefix,
                    const JSON& json,
                    bool is_last,
                    int depth,
                    Expander& expander);
Component FromTable(Component prefix,
                    const JSON& json,
                    bool is_last,
                    int depth,
                    Expander& expander);
Component FromKeyValue(const std::string& key,
                       const JSON& value,
                       bool is_last,
                       int depth,
                       Expander& expander);
Component Empty();
Component Unimplemented();
Component Basic(std::string value, Color c, bool is_last);
Component Indentation(Component child);
Component FakeHorizontal(Component a, Component b);
bool IsSuitableForTableView(const JSON& json);

Component From(const JSON& json, bool is_last, int depth, Expander& expander) {
  if (json.is_object())
    return FromObject(Empty(), json, is_last, depth, expander);
  if (json.is_array())
    return FromArrayAny(Empty(), json, is_last, depth, expander);
  if (json.is_string())
    return FromString(json, is_last);
  if (json.is_number())
    return FromNumber(json, is_last);
  if (json.is_boolean())
    return FromBoolean(json, is_last);
  if (json.is_null())
    return FromNull(is_last);
  return Unimplemented();
}

Component FromString(const JSON& json, bool is_last) {
  std::string value = json;
  std::string str = "\"" + value + "\"";
  return Basic(str, Color::GreenLight, is_last);
}

Component FromNumber(const JSON& json, bool is_last) {
  return Basic(json.dump(), Color::CyanLight, is_last);
}

Component FromBoolean(const JSON& json, bool is_last) {
  bool value = json;
  std::string str = value ? "true" : "false";
  return Basic(str, Color::YellowLight, is_last);
}

Component FromNull(bool is_last) {
  return Basic("null", Color::RedLight, is_last);
}

Component Unimplemented() {
  return Renderer([] { return text("Unimplemented"); });
}

Component Empty() {
  return Renderer([] { return text(""); });
}

Component Basic(std::string value, Color c, bool is_last) {
  return Renderer([value, c, is_last](bool focused) {
    auto element = paragraph(value) | color(c);
    if (focused)
      element = element | inverted | focus;
    if (!is_last)
      element = hbox({element, text(",")});
    return element;
  });
}

bool IsSuitableForTableView(const JSON& json) {
  if (!json.is_array())
    return false;
  size_t columns = 0;
  for (const auto& element : json.items()) {
    if (!element.value().is_object())
      return false;
    columns = std::max(columns, element.value().size());
  }
  return columns >= 2 || json.size() >= 2;
}

Component Indentation(Component child) {
  return Renderer(child, [child] {
    return hbox({
        text("  "),
        child->Render(),
    });
  });
}

Component FakeHorizontal(Component a, Component b) {
  auto c = Container::Vertical({a, b});
  c->SetActiveChild(b);

  return Renderer(c, [a, b] {
    return hbox({
        a->Render(),
        b->Render(),
    });
  });
}

class ComponentExpandable : public ComponentBase {
 public:
  ComponentExpandable(Expander& expander) : expander_(expander->Child()) {}

  bool& Expanded() {
    return expander_->expanded;
  }

  bool OnEvent(Event event) override {
    if (ComponentBase::OnEvent(event)) {
      return true;
    }

    if (event == Event::Character('+')) {
      expander_->Expand();
      return true;
    }

    if (event == Event::Character('-')) {
      TakeFocus();
      return expander_->Collapse();
    }

    return false;
  }

  Expander expander_;
};

Component FromObject(Component prefix,
                     const JSON& json,
                     bool is_last,
                     int depth,
                     Expander& expander) {
  class Impl : public ComponentExpandable {
   public:
    Impl(Component prefix,
         const JSON& json,
         bool is_last,
         int depth,
         Expander& expander)
        : ComponentExpandable(expander) {
      Expanded() = (depth <= 1);

      auto children = Container::Vertical({});
      int size = static_cast<int>(json.size());
      for (auto& it : json.items()) {
        bool is_children_last = --size == 0;
        children->Add(Indentation(FromKeyValue(
            it.key(), it.value(), is_children_last, depth + 1, expander_)));
      }

      if (is_last)
        children->Add(Renderer([] { return text("}"); }));
      else
        children->Add(Renderer([] { return text("},"); }));

      auto toggle = MyToggle("{", is_last ? "{...}" : "{...},", &Expanded());
      Add(Container::Vertical({
          FakeHorizontal(prefix, toggle),
          Maybe(children, &Expanded()),
      }));
    }
  };
  return Make<Impl>(prefix, json, is_last, depth, expander);
}

Component FromKeyValue(const std::string& key,
                       const JSON& value,
                       bool is_last,
                       int depth,
                       Expander& expander) {
  std::string str = "\"" + key + "\"";
  if (value.is_object() || value.is_array()) {
    auto prefix = Renderer([str] {
      return hbox({
          text(str) | color(Color::BlueLight),
          text(": "),
      });
    });
    if (value.is_object())
      return FromObject(prefix, value, is_last, depth, expander);
    else
      return FromArrayAny(prefix, value, is_last, depth, expander);
  }

  auto child = From(value, is_last, depth, expander);
  return Renderer(child, [str, child] {
    return hbox({
        text(str) | color(Color::BlueLight),
        text(": "),
        child->Render(),
    });
  });
}

Component FromArrayAny(Component prefix,
                       const JSON& json,
                       bool is_last,
                       int depth,
                       Expander& expander) {
  class Impl : public ComponentBase {
   public:
    Impl(Component prefix,
         const JSON& json,
         bool is_last,
         int depth,
         Expander& expander) {
      Add(FromArray(prefix, json, is_last, depth,expander));
    }
  };

  return Make<Impl>(prefix, json, is_last, depth, expander);
}

Component FromArray(Component prefix,
                    const JSON& json,
                    bool is_last,
                    int depth,
                    Expander& expander) {
  class Impl : public ComponentExpandable {
   public:
    Impl(Component prefix,
         const JSON& json,
         bool is_last,
         int depth,
         Expander& expander)
        : ComponentExpandable(expander),
          prefix_(prefix),
          json_(json),
          is_last_(is_last),
          depth_(depth) {
      Expanded() = (depth <= 0);
      auto children = Container::Vertical({});
      int size = static_cast<int>(json_.size());
      for (auto& it : json_.items()) {
        bool is_children_last = --size == 0;
        children->Add(Indentation(
            From(it.value(), is_children_last, depth + 1, expander_)));
      }

      if (is_last)
        children->Add(Renderer([] { return text("]"); }));
      else
        children->Add(Renderer([] { return text("],"); }));

      auto toggle = MyToggle("[", is_last ? "[...]" : "[...],", &Expanded());

      auto upper = Container::Horizontal({
          FakeHorizontal(prefix_, toggle),
      });

      // Turn this array into a table.
      if (IsSuitableForTableView(json)) {
        auto expand_button = MyButton("   ", "(table view)", [this, &expander] {
          auto* parent = Parent();
          auto replacement =
              FromTable(prefix_, json_, is_last_, depth_, expander);
          parent->DetachAllChildren();  // Detach this.
          parent->Add(replacement);
        });

        upper = Container::Horizontal({upper, expand_button});
      }

      Add(Container::Vertical({
          upper,
          Maybe(children, &Expanded()),
      }));
    }

    Component prefix_;
    const JSON& json_;
    bool is_last_;
    int depth_;
  };
  return Make<Impl>(prefix, json, is_last, depth, expander);
}

Component FromTable(Component prefix,
                    const JSON& json,
                    bool is_last,
                    int depth,
                    Expander& expander) {
  class Impl : public ComponentBase {
   public:
    Impl(Component prefix,
         const JSON& json,
         bool is_last,
         int depth,
         Expander& expander)
        : prefix_(prefix), json_(json), is_last_(is_last), depth_(depth) {
      std::vector<Component> components;

      // Turn this array into a table.
      expand_button_ = MyButton("", "(array view)", [this, &expander] {
        auto* parent = Parent();
        auto replacement =
            FromArray(prefix_, json_, is_last_, depth_, expander);
        replacement->OnEvent(Event::ArrowRight);
        parent->DetachAllChildren();  // Detach this.
        parent->Add(replacement);
      });
      components.push_back(expand_button_);

      std::map<std::string, int> columns_index;
      for (auto& row : json_.items()) {
        children_.push_back({});
        auto& children_row = children_.back();
        for (auto& cell : row.value().items()) {
          // Does it require a new column?
          if (!columns_index.count(cell.key())) {
            columns_index[cell.key()] = columns_.size();
            columns_.push_back(cell.key());
          }

          // Does the current row fits in the current column?
          if ((int)children_row.size() <= columns_index[cell.key()]) {
            children_row.resize(columns_index[cell.key()] + 1);
          }

          // Fill in the data
          auto child =
              From(cell.value(), /*is_last=*/true, depth_ + 1, expander);
          children_row[columns_index[cell.key()]] = child;
        }
      }
      // Layout
      for (auto& rows : children_) {
        auto row = Container::Horizontal({});
        for (auto& cell : rows) {
          if (cell)
            row->Add(cell);
        }
        components.push_back(row);
      }
      Add(Container::Vertical(std::move(components)));
    }

   private:
    Element Render() override {
      std::vector<std::vector<Element>> data;
      data.push_back({text("") | color(Color::GrayDark)});
      for (auto& title : columns_)
        data.back().push_back(text(title));
      int i = 0;
      for (auto& row_children : children_) {
        std::vector<Element> data_row;
        data_row.push_back(text(std::to_string(i++)) | color(Color::GrayDark));
        for (auto& child : row_children) {
          if (child) {
            data_row.push_back(child->Render());
          } else {
            data_row.push_back(text(""));
          }
        }
        data.push_back(std::move(data_row));
      }
      auto table = Table(std::move(data));
      table.SelectColumns(1, -1).SeparatorVertical(LIGHT);
      table.SelectColumns(1, -1).Border(LIGHT);
      table.SelectRectangle(1, -1, 0, 0).SeparatorVertical(HEAVY);
      table.SelectRectangle(1, -1, 0, 0).Border(HEAVY);

      return vbox({
          hbox({
              prefix_->Render(),
              expand_button_->Render(),
          }),
          table.Render(),
      });
    }

    std::vector<std::string> columns_;
    std::vector<std::vector<Component>> children_;

    Component prefix_;
    Component expand_button_;
    const JSON& json_;
    bool is_last_;
    int depth_;
  };

  return Make<Impl>(prefix, json, is_last, depth, expander);
}

}  // anonymous namespace

void DisplayMainUI(const JSON& json, bool fullscreen) {
  auto screen_fullscreen = ScreenInteractive::Fullscreen();
  auto screen_fit = ScreenInteractive::FitComponent();
  auto& screen = fullscreen ? screen_fullscreen : screen_fit;
  Expander expander = ExpanderImpl::Root();
  auto component = From(json, /*is_last=*/true, /*depth=*/0, expander);

  // Wrap it inside a frame, to allow scrolling.
  component =
      Renderer(component, [component] { return component->Render() | yframe; });

  Event previous_event;
  Event next_event;
  auto wrapped_component = CatchEvent(component, [&](Event event) {
    previous_event = next_event;
    next_event = event;

    // 'G' and 'gg -------------------------------------------------------------
    if (event == Event::Character('G')) {
      while (component->OnEvent(Event::ArrowUp))
        ;
      return true;
    }
    if (previous_event == Event::Character('g') &&
        next_event == Event::Character('g')) {
      while (component->OnEvent(Event::ArrowDown))
        ;
      return true;
    }

    // Allow the user to quit using 'q' or ESC ---------------------------------
    if (event == Event::Character('q') || event == Event::Escape) {
      screen.ExitLoopClosure()();
      return true;
    }

    // Convert mouse whell into their corresponding Down/Up events.-------------
    if (!event.is_mouse())
      return false;
    if (event.mouse().button == Mouse::WheelDown) {
      screen.PostEvent(Event::ArrowDown);
      return true;
    }
    if (event.mouse().button == Mouse::WheelUp) {
      screen.PostEvent(Event::ArrowUp);
      return true;
    }
    return false;
  });

  screen.Loop(wrapped_component);
}
