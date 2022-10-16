#include "keybinding.hpp"

#include <ftxui/dom/elements.hpp>
#include <ftxui/dom/table.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>

void KeyBinding() {
  using namespace ftxui;
  auto table = Table(std::vector<std::vector<std::string>>{
      {"keys", "action"},
      //
      {"Navigate", "← ↑ ↓ →"},
      {"", "h j k l "},
      {"", "Mouse::WheelUp"},
      {"", "Mouse::WheelDown"},
      //
      {"Toggle", "enter"},
      {"", "Mouse::Left"},
      //
      {"Deep", ""},
      {" - Expand", "+"},
      {" - Collapse", "-"},
      //
      {"Exit", "Escape"},
      {"", "q"},
      //
      {"Navigate", ""},
      {" - first", "page-up"},
      {" - last", "page-down"},
      {" - top", "gg"},
      {" - bottom", "G"},
      //
  });
  table.SelectRows(0, 0).DecorateCells(color(Color::Cyan));
  table.SelectRows(1, 4).Border(LIGHT);
  table.SelectRows(5, 6).Border(LIGHT);
  table.SelectRows(7, 9).Border(LIGHT);
  table.SelectRows(10, 11).Border(LIGHT);
  table.SelectRows(12, 16).Border(LIGHT);
  table.SelectAll().SeparatorVertical(LIGHT);
  table.SelectAll().Border(LIGHT);
  auto document = table.Render();
  auto screen = Screen::Create(Dimension::Fit(document));
  Render(screen, document);
  screen.Print();
  std::cout << std::endl;
}
