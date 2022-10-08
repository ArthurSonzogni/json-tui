// Copyright 2022 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
#ifndef JSON_TUI_EXPANDER_HPP
#define JSON_TUI_EXPANDER_HPP

#include <memory>
#include <vector>

class ExpanderImpl;
using Expander = std::unique_ptr<ExpanderImpl>;

class ExpanderImpl {
 public:
  ~ExpanderImpl();
  static Expander Root();
  Expander Child();

  bool Expand();
  bool Collapse();

  int MinLevel() const;
  int MaxLevel() const;

  bool expanded = false;
 private:
  void Expand(int minLevel);
  void Collapse(int maxLevel);

  ExpanderImpl* parent_ = nullptr;
  std::vector<ExpanderImpl*> children_;
};
#endif  // JSON_TUI_EXPANDER_HPP
