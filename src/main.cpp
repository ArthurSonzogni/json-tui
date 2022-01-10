// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "mytoggle.hpp"
#include "version.hpp"

using JSON = nlohmann::json;
using namespace ftxui;

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

Component From(const JSON& json, bool is_last, int depth);
Component FromObject(Component prefix,
                     const JSON& json,
                     bool is_last,
                     int depth);
Component FromArray(Component prefix,
                    const JSON& json,
                    bool is_last,
                    int depth);
Component FromString(const JSON& json, bool is_last);
Component FromNumber(const JSON& json, bool is_last);
Component FromBoolean(const JSON& json, bool is_last);
Component FromNull(const JSON& json, bool is_last);
Component FromKeyValue(const std::string& key,
                       const JSON& value,
                       bool is_last,
                       int depth);

Component Unimplemented() {
  return Renderer([] { return text("Unimplemented"); });
}

Component Empty() {
  return Renderer([] { return text(""); });
}

Component From(const JSON& json, bool is_last, int depth) {
  if (json.is_object())
    return FromObject(Empty(), json, is_last, depth);
  if (json.is_array())
    return FromArray(Empty(), json, is_last, depth);
  if (json.is_string())
    return FromString(json, is_last);
  if (json.is_number())
    return FromNumber(json, is_last);
  if (json.is_boolean())
    return FromBoolean(json, is_last);
  if (json.is_null())
    return FromNull(json, is_last);
  return Unimplemented();
}

Component FromObject(Component prefix,
                     const JSON& json,
                     bool is_last,
                     int depth) {
  class Impl : public ComponentBase {
   public:
    Impl(Component prefix, const JSON& json, bool is_last, int depth) {
      is_expanded_ = (depth <= 1);

      auto children = Container::Vertical({});
      int size = json.size();
      for (auto& it : json.items()) {
        bool is_last = --size == 0;
        children->Add(Indentation(
            FromKeyValue(it.key(), it.value(), is_last, depth + 1)));
      }

      if (is_last)
        children->Add(Renderer([] { return text("}"); }));
      else
        children->Add(Renderer([] { return text("},"); }));

      CheckboxOption opt;

      auto toggle =
          MyToggle("{", is_last ? "{...}" : "{...},", &is_expanded_, opt);

      if (!is_last)
        opt.style_unchecked += ",";
      Add(Container::Vertical({
          FakeHorizontal(prefix, toggle),
          Maybe(children, &is_expanded_),
      }));
    }
    bool is_expanded_ = false;
  };
  return Make<Impl>(prefix, json, is_last, depth);
}

Component FromKeyValue(const std::string& key,
                       const JSON& value,
                       bool is_last,
                       int depth) {
  std::string str = "\"" + key + "\"";
  if (value.is_object() || value.is_array()) {
    auto prefix = Renderer([str] {
      return hbox({
          text(str) | color(Color::BlueLight),
          text(": "),
      });
    });
    if (value.is_object())
      return FromObject(prefix, value, is_last, depth);
    else
      return FromArray(prefix, value, is_last, depth);
  }

  auto child = From(value, is_last, depth);
  return Renderer(child, [str, child] {
    return hbox({
        text(str) | color(Color::BlueLight),
        text(": "),
        child->Render(),
    });
  });
}

Component FromArray(Component prefix,
                    const JSON& json,
                    bool is_last,
                    int depth) {
  class Impl : public ComponentBase {
   public:
    Impl(Component prefix, const JSON& json, bool is_last, int depth) {
      is_expanded_ = (depth <= 0);
      auto children = Container::Vertical({});
      int size = json.size();
      for (auto& it : json.items()) {
        bool is_last = --size == 0;
        children->Add(Indentation(From(it.value(), is_last, depth + 1)));
      }

      if (is_last)
        children->Add(Renderer([] { return text("]"); }));
      else
        children->Add(Renderer([] { return text("],"); }));

      CheckboxOption opt;
      auto toggle =
          MyToggle("[", is_last ? "[...]" : "[...],", &is_expanded_, opt);
      if (!is_last)
        opt.style_unchecked += ",";
      Add(Container::Vertical({
          FakeHorizontal(prefix, toggle),
          Maybe(children, &is_expanded_),
      }));
    }
    bool is_expanded_ = false;
  };
  return Make<Impl>(prefix, json, is_last, depth);
}

Component Basic(std::string value, Color c, bool is_last) {
  if (is_last) {
    return Renderer([value, c](bool focused) {
      auto element = paragraph(value);
      if (focused)
        element = element | inverted | focus;
      return element | color(c);
    });
  } else {
    return Renderer([value, c](bool focused) {
      auto element = paragraph(value) | color(c);
      if (focused)
        element = element | inverted | focus;
      return hbox({
          element,
          text(","),
      });
    });
  }
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

Component FromNull(const JSON& json, bool is_last) {
  return Basic("null", Color::RedLight, is_last);
}

void Main(const JSON& json) {
  auto screen = ScreenInteractive::FitComponent();
  auto component = From(json, /*is_last=*/true, /*depth=*/0);

  // Wrap it inside a frame, to allow scrolling.
  component =
      Renderer(component, [component] { return component->Render() | yframe; });

  // Convert mouse whell into their corresponding Down/Up events.
  component = CatchEvent(component, [&](Event event) {
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

  screen.Loop(component);
}

int usage() {
  std::cout << R"(
Name
  json-tui - a simple JSON viewer

Synopsis
  json-tui [OPTION]... [FILE]...
  print_json | json-tui

Description
  json-tui reads JSON from stdin or from FILE and prints it in a expandable
  tree.

  If no FILE is given, json-tui reads JSON from stdin.

Options
  -h, --help
      display this help and exit
  -v, --version
      output version information and exit

Report bugs to https://github.com/ArthurSonzogni/json-tui/issues")"
  << std::endl;

  return EXIT_SUCCESS;
}

int version() {
  std::cout << project_version << std::endl;
  return EXIT_SUCCESS;
}

int main(int argument_count, char** arguments) {
  if (argument_count >= 2) {
    std::string arg = arguments[1];
    if (arg == "-h" || arg == "--help")
      return usage();
    if (arg == "-v" || arg == "--version")
      return version();
  }

  // Route file_descriptor to either stdin or the file argument.
  int file_descriptor = 0;
  if (argument_count == 2) {
    FILE* file = fopen(arguments[1], "r");
    if (!file) {
      std::cerr << "Could not open file " << arguments[1] << std::endl;
      return EXIT_FAILURE;
    }
    file_descriptor = dup(fileno(file));
    fclose(file);
  } else {
    std::cout << "Reading from stdin..." << std::flush;
    file_descriptor = dup(fileno(stdin));
  }

  // Read from the file descriptor.
  std::string buffer;
  const int buff_size = 1<<10;
  char buff[buff_size];
  while (int used = read(file_descriptor, buff, buff_size) > 0)
    buffer += std::string(buff, used);

  // Reroute stdin to /dev/tty to handle user input.
  stdin = freopen("/dev/tty", "r", stdin);

  JSON json;
  try {
    json = JSON::parse(buffer);
  } catch (...) {
    std::cerr << "Error: Could not parse JSON." << std::endl;
    return EXIT_FAILURE;
  }

  Main(json);
  return EXIT_SUCCESS;
}
