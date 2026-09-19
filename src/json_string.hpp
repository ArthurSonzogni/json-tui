// Copyright 2026 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef JSON_TUI_JSON_STRING_HPP
#define JSON_TUI_JSON_STRING_HPP

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include "ftxui/component/component.hpp"

// Strings are word wrapped to the available width. While focused, a cursor
// selects one row, and the view scrolls to follow it. This way, strings taller
// than the screen can be read entirely.
ftxui::Component FromString(const nlohmann::json& json, bool is_last);

// Tells whether the last key moved the focus upward. A string entered from
// below starts on its last row.
void SetFocusFromBelow(bool from_below);

// The JSON representation of a string, as terminal cells. Escape sequences are
// kept as in the input (e.g. `\"`, `\t`), except `\n`, which starts a new line.
// Lines are wrapped into rows for a given width.
class StringCells {
 public:
  struct Row {
    int begin;
    int end;
  };

  StringCells(std::string_view dump, bool is_last);

  // Word wrap the lines into rows of at most |width| cells. Cached, as long as
  // the width doesn't change.
  void Wrap(int width);

  std::string_view Glyph(int cell) const;
  const std::vector<Row>& Rows() const { return rows_; }
  int LongestLine() const { return longest_line_; }
  int StringEnd() const { return string_end_; }

 private:
  int Cells() const { return static_cast<int>(offsets_.size()) - 1; }
  void AppendLine(std::string_view line);

  // The glyph of cell |i| is bytes_[offsets_[i], offsets_[i + 1]).
  std::string bytes_;
  std::vector<uint32_t> offsets_ = {0};
  // The cells of line |i| are [line_begin_[i], line_begin_[i + 1]).
  std::vector<int> line_begin_ = {0};
  int string_end_ = 0;  // The cells after it are the trailing ",".
  int longest_line_ = 0;

  int width_ = 0;
  std::vector<Row> rows_;
};

#endif  // JSON_TUI_JSON_STRING_HPP
