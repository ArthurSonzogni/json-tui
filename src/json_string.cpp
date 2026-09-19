// Copyright 2026 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include "json_string.hpp"

#include <algorithm>
#include <ftxui/component/event.hpp>
#include <ftxui/component/mouse.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <ftxui/screen/string.hpp>
#include <ftxui/screen/terminal.hpp>
#include <memory>

using JSON = nlohmann::json;
using namespace ftxui;

namespace {

bool g_focus_from_below = false;

// Strings taller than this are collapsed, until expanded using enter/space.
constexpr int kCollapsedRows = 5;

}  // namespace

void SetFocusFromBelow(bool from_below) {
  g_focus_from_below = from_below;
}

StringCells::StringCells(std::string_view dump, bool is_last) {
  size_t begin = 0;
  for (size_t i = 0; i < dump.size(); ++i) {
    if (dump[i] != '\\')
      continue;
    // The dump is a quoted string, so an escape always has a second character.
    if (dump[i + 1] == 'n') {
      AppendLine(dump.substr(begin, i - begin));
      begin = i + 2;
    }
    ++i;
  }
  AppendLine(dump.substr(begin));

  string_end_ = Cells();
  if (!is_last) {
    bytes_ += ',';
    offsets_.push_back(bytes_.size());
    line_begin_.back() = Cells();
  }

  for (size_t i = 0; i + 1 < line_begin_.size(); ++i)
    longest_line_ =
        std::max(longest_line_, line_begin_[i + 1] - line_begin_[i]);
}

void StringCells::Wrap(int width) {
  if (width == width_)
    return;
  width_ = width;
  rows_.clear();
  for (size_t line = 0; line + 1 < line_begin_.size(); ++line) {
    int begin = line_begin_[line];
    const int end = line_begin_[line + 1];
    while (end - begin > width) {
      int cut = begin + width;
      // Prefer breaking at a space, consumed by the break.
      int space = cut;
      while (space > begin && Glyph(space) != " ")
        --space;
      if (space > begin) {
        rows_.push_back({begin, space});
        begin = space + 1;
        continue;
      }
      // Don't separate a fullwidth glyph from the empty cell following it.
      if (Glyph(cut).empty() && cut - 1 > begin)
        --cut;
      rows_.push_back({begin, cut});
      begin = cut;
    }
    rows_.push_back({begin, end});
  }
}

std::string_view StringCells::Glyph(int cell) const {
  return std::string_view(bytes_).substr(offsets_[cell],
                                         offsets_[cell + 1] - offsets_[cell]);
}

void StringCells::AppendLine(std::string_view line) {
  // Converted by chunks, to bound the memory used by the glyphs.
  constexpr size_t kChunk = 1 << 16;
  while (!line.empty()) {
    size_t size = std::min(line.size(), kChunk);
    // Don't cut a UTF-8 sequence.
    while (size < line.size() && (line[size] & 0xC0) == 0x80)
      ++size;
    for (const std::string& glyph : Utf8ToGlyphs(line.substr(0, size))) {
      bytes_ += glyph;
      offsets_.push_back(bytes_.size());
    }
    line.remove_prefix(size);
  }
  line_begin_.push_back(Cells());
}

Component FromString(const JSON& json, bool is_last) {
  class Impl : public ComponentBase {
   public:
    Impl(const JSON& json, bool is_last) : json_(json), is_last_(is_last) {}

   private:
    // Gets its width from the layout, and requests another layout iteration
    // when the resulting height differs from the one it required.
    class StringNode : public Node {
     public:
      StringNode(Impl* impl, bool focused) : impl_(impl), focused_(focused) {}

     private:
      void ComputeRequirement() override {
        requirement_ = Requirement();
        requirement_.min_x = std::min(impl_->cells_->LongestLine(), 10);
        requirement_.min_y = impl_->Rows();
        requirement_.flex_grow_x = 1;
        requirement_.flex_shrink_x = 1;
        if (focused_) {
          // Focusing only the cursor row makes the frame scroll to it.
          const int cursor = std::min(impl_->cursor_, impl_->Rows() - 1);
          requirement_.focused.enabled = true;
          requirement_.focused.node = this;
          requirement_.focused.box = {0, requirement_.min_x - 1, cursor,
                                      cursor};
        }
      }

      void SetBox(Box box) override {
        Node::SetBox(box);
        impl_->box_ = box;
        impl_->cells_->Wrap(std::max(1, box.x_max - box.x_min + 1));
        need_iteration_ = impl_->Rows() != requirement_.min_y;
      }

      void Check(Status* status) override {
        Node::Check(status);
        status->need_iteration |= need_iteration_;
      }

      // Only the visible rows are drawn.
      void Render(Screen& screen) override {
        const Box visible = Box::Intersection(screen.stencil, box_);
        for (int y = visible.y_min; y <= visible.y_max; ++y)
          impl_->RenderRow(screen, y - box_.y_min, y, focused_);
      }

      Impl* const impl_;
      const bool focused_;
      bool need_iteration_ = false;
    };

    Element OnRender() override {
      // Built on first render: most strings of a large document are never
      // displayed.
      if (!cells_)
        cells_ = std::make_unique<StringCells>(json_.dump(), is_last_);

      const bool focused = Focused();
      if (focused && !was_focused_)
        cursor_ = g_focus_from_below ? std::max(0, Rows() - 1) : 0;
      was_focused_ = focused;
      return std::make_shared<StringNode>(this, focused);
    }

    void RenderRow(Screen& screen, int row, int y, bool focused) const {
      if (row >= Rows())
        return;
      const bool cursor = focused && row == cursor_;

      if (IsIndicator(row)) {
        const int hidden = static_cast<int>(cells_->Rows().size()) - row;
        int x = box_.x_min;
        for (const std::string& glyph :
             Utf8ToGlyphs("… " + std::to_string(hidden) + " more lines")) {
          if (x > box_.x_max)
            break;
          Cell& cell = screen.CellAt(x++, y);
          cell.character = glyph;
          cell.foreground_color = Color::GrayLight;
          cell.inverted = cursor;
        }
        return;
      }

      const StringCells::Row& cells = cells_->Rows()[row];
      int x = box_.x_min;
      for (int i = cells.begin; i < cells.end && x <= box_.x_max; ++i) {
        Cell& cell = screen.CellAt(x++, y);
        cell.character = cells_->Glyph(i);
        if (i < cells_->StringEnd())
          cell.foreground_color = Color::GreenLight;
        cell.inverted = cursor;
      }
      // Keep the cursor visible on empty rows.
      if (cursor && cells.begin == cells.end)
        screen.CellAt(box_.x_min, y).inverted = true;
    }

    bool OnEvent(Event event) override {
      if (!cells_)
        return false;

      if (event.is_mouse()) {
        if (!box_.Contain(event.mouse().x, event.mouse().y) ||
            !CaptureMouse(event))
          return false;
        if (event.mouse().button != Mouse::Left ||
            event.mouse().motion != Mouse::Pressed)
          return false;
        cursor_ = event.mouse().y - box_.y_min;
        if (IsIndicator(cursor_))
          expanded_ = true;
        TakeFocus();
        was_focused_ = true;
        return true;
      }

      // The focus may have moved here without a render, e.g. using 'gg'.
      was_focused_ = true;

      if (event == Event::Return || event == Event::Character(' ')) {
        if (!Collapsible())
          return false;
        expanded_ = !expanded_;
        cursor_ = std::min(cursor_, Rows() - 1);
        return true;
      }

      const int page = std::max(1, Terminal::Size().dimy - 2);
      int cursor = cursor_;
      if (event == Event::ArrowDown || event == Event::Character('j'))
        cursor += 1;
      else if (event == Event::ArrowUp || event == Event::Character('k'))
        cursor -= 1;
      else if (event == Event::PageDown)
        cursor += page;
      else if (event == Event::PageUp)
        cursor -= page;
      else if (event == Event::Home)
        cursor = 0;
      else if (event == Event::End)
        cursor = Rows() - 1;
      else
        return false;

      // Let the parent move the focus when there is nothing to scroll.
      cursor = std::clamp(cursor, 0, std::max(0, Rows() - 1));
      if (cursor == cursor_)
        return false;
      cursor_ = cursor;
      return true;
    }

    bool Focusable() const override { return true; }

    bool Collapsible() const {
      return static_cast<int>(cells_->Rows().size()) > kCollapsedRows;
    }
    bool Collapsed() const { return Collapsible() && !expanded_; }
    // The last row of a collapsed string tells how many are hidden.
    bool IsIndicator(int row) const {
      return Collapsed() && row == kCollapsedRows - 1;
    }
    int Rows() const {
      return Collapsed() ? kCollapsedRows
                         : static_cast<int>(cells_->Rows().size());
    }

    const JSON& json_;
    const bool is_last_;
    std::unique_ptr<StringCells> cells_;
    bool expanded_ = false;
    int cursor_ = 0;  // The focused row.
    bool was_focused_ = false;
    Box box_;
  };
  return Make<Impl>(json, is_last);
}
