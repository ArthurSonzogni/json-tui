// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#define ARGS_NOEXCEPT
#include <args.hxx>
#include <cstdio>
#include <fstream>
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>
#include <iostream>
#include <nlohmann/json.hpp>

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
      int size = static_cast<int>(json.size());
      for (auto& it : json.items()) {
        bool is_last = --size == 0;
        children->Add(Indentation(
            FromKeyValue(it.key(), it.value(), is_last, depth + 1)));
      }

      if (is_last)
        children->Add(Renderer([] { return text("}"); }));
      else
        children->Add(Renderer([] { return text("},"); }));

      auto toggle = MyToggle("{", is_last ? "{...}" : "{...},", &is_expanded_);
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
      int size = static_cast<int>(json.size());
      for (auto& it : json.items()) {
        bool is_last = --size == 0;
        children->Add(Indentation(From(it.value(), is_last, depth + 1)));
      }

      if (is_last)
        children->Add(Renderer([] { return text("]"); }));
      else
        children->Add(Renderer([] { return text("],"); }));

      auto toggle = MyToggle("[", is_last ? "[...]" : "[...],", &is_expanded_);
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

void Main(const JSON& json, bool fullscreen) {
  auto screen = fullscreen ? ScreenInteractive::Fullscreen()
                           : ScreenInteractive::FitComponent();
  auto component = From(json, /*is_last=*/true, /*depth=*/0);

  // Wrap it inside a frame, to allow scrolling.
  component =
      Renderer(component, [component] { return component->Render() | yframe; });

  component = CatchEvent(component, [&](Event event) {
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

  screen.Loop(component);
}

class JsonParser : public nlohmann::detail::json_sax_dom_parser<JSON> {
 public:
  JsonParser(JSON& j) : nlohmann::detail::json_sax_dom_parser<JSON>(j, false) {}

  bool parse_error(std::size_t position,
                   const std::string& last_token,
                   const JSON::exception& ex) {
    std::cerr << std::endl;
    std::cerr << ex.what() << std::endl;
    return false;
  }
};

int main(int argument_count, const char** arguments) {
  args::ArgumentParser args("");
  args.Prog("json-tui");
  args.Description("A JSON terminal UI");
  args.Epilog(
      "If no file is given, json-tui reads JSON from the standard input\n"
      "\n"
      "Please report bugs to:"
      "https://github.com/ArthurSonzogni/json-tui/issues");

  args::Positional<std::string> file(args, "file",
                                     "A JSON file. Omit to read from stdin.");
  args::Flag help(args, "help", "Display this help menu.", {'h', "help"});
  args::Flag version(args, "version", "Print version.", {'v', "version"});
  args::Flag fullscreen(
      args, "fullscreen",
      "Display the JSON in fullscreen, in an alternate buffer",
      {'f', "fullscreen"});
  bool success = args.ParseCLI(argument_count, arguments);
  if (!success)
    std::cout << "Invalid arguments" << std::endl;

  if (help || !success) {
    std::cout << args;
    return EXIT_SUCCESS;
  }

  if (version) {
    std::cout << project_version << std::endl;
    return EXIT_SUCCESS;
  }

  std::stringstream ss;
  if (file) {
    auto file_stream = std::ifstream(args::get(file));
    if (!file_stream) {
      std::cerr << "Could not open file " << args::get(file) << std::endl;
      return EXIT_FAILURE;
    }
    ss << file_stream.rdbuf();
  } else {
#if defined(_WIN32)
    std::cout << "Error. Reading from stdin is not supported on Windows. "
                 "Please provide a file."
              << std::endl;

    std::cout << args;
    return EXIT_FAILURE;
#else
    std::cout << "Reading from stdin..." << std::flush;
    ss << std::cin.rdbuf();
    stdin = freopen("/dev/tty", "r", stdin);
#endif
  }

  JSON json;
  JsonParser parser(json);
  if (!JSON::sax_parse(ss.str(), &parser))
    return EXIT_FAILURE;

  Main(json, fullscreen);
  return EXIT_SUCCESS;
}
