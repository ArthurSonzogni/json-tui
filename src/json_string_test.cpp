#include "json_string.hpp"
#include <gtest/gtest.h>

namespace {

StringCells Cells(const std::string& value, bool is_last = true) {
  return StringCells(nlohmann::json(value).dump(), is_last);
}

std::vector<std::string> Wrap(StringCells& cells, int width) {
  cells.Wrap(width);
  std::vector<std::string> rows;
  for (const StringCells::Row& row : cells.Rows()) {
    std::string str;
    for (int i = row.begin; i < row.end; ++i)
      str += cells.Glyph(i);
    rows.push_back(str);
  }
  return rows;
}

using Rows = std::vector<std::string>;

}  // namespace

TEST(StringCells, Fits) {
  auto cells = Cells("hello world");
  EXPECT_EQ(Wrap(cells, 80), Rows({"\"hello world\""}));
  EXPECT_EQ(cells.LongestLine(), 13);
}

TEST(StringCells, BreakAtSpace) {
  auto cells = Cells("aaa bbb ccc");
  EXPECT_EQ(Wrap(cells, 8), Rows({"\"aaa bbb", "ccc\""}));
}

TEST(StringCells, BreakLongWord) {
  auto cells = Cells("abcdefgh");
  EXPECT_EQ(Wrap(cells, 4), Rows({"\"abc", "defg", "h\""}));
}

TEST(StringCells, Newline) {
  auto cells = Cells("a\n\nb");
  EXPECT_EQ(Wrap(cells, 80), Rows({"\"a", "", "b\""}));
}

TEST(StringCells, OtherEscapesKept) {
  auto cells = Cells("a\"b\tc\\d");
  EXPECT_EQ(Wrap(cells, 80), Rows({R"("a\"b\tc\\d")"}));
}

TEST(StringCells, Comma) {
  auto cells = Cells("abc", /*is_last=*/false);
  EXPECT_EQ(Wrap(cells, 80), Rows({"\"abc\","}));
  EXPECT_EQ(cells.StringEnd(), 5);
}

TEST(StringCells, FullwidthNotSplit) {
  auto cells = Cells("日本語");
  EXPECT_EQ(Wrap(cells, 4), Rows({"\"日", "本語", "\""}));
}

TEST(StringCells, FullwidthWidthOne) {
  auto cells = Cells("日");
  EXPECT_EQ(Wrap(cells, 1), Rows({"\"", "日", "", "\""}));
}

TEST(StringCells, Rewrap) {
  auto cells = Cells("aaa bbb");
  EXPECT_EQ(Wrap(cells, 4), Rows({"\"aaa", "bbb\""}));
  EXPECT_EQ(Wrap(cells, 80), Rows({"\"aaa bbb\""}));
}

// Long strings are converted by chunks. They must not split UTF-8 sequences.
TEST(StringCells, LongMultibyte) {
  std::string value;
  for (int i = 0; i < 100000; ++i)
    value += "é";
  auto cells = Cells(value);
  const Rows rows = Wrap(cells, 1 << 20);
  ASSERT_EQ(rows.size(), 1u);
  EXPECT_EQ(rows[0], "\"" + value + "\"");
  EXPECT_EQ(cells.LongestLine(), 100002);
}
