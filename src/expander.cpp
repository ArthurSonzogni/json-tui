#include "expander.hpp"
#include <algorithm>

ExpanderImpl::~ExpanderImpl() {
  // Remove this from parent:
  if (parent_) {
    parent_->children_.erase(
        std::remove(parent_->children_.begin(), parent_->children_.end(), this),
        parent_->children_.end());
    parent_ = nullptr;
  }

  // Remove this from children:
  for (auto& child : children_) {
    child->parent_ = nullptr;
  }
}

// static
Expander ExpanderImpl::Root() {
  return std::make_unique<ExpanderImpl>();
}

Expander ExpanderImpl::Child() {
  auto expander = Root();
  expander->parent_ = this;
  children_.push_back(expander.get());
  return expander;
}

bool ExpanderImpl::Expand() {
  int min_level = MinLevel();
  Expand(min_level + 1);
  return MinLevel() != min_level;
}

bool ExpanderImpl::Collapse() {
  int max_level = MaxLevel();
  Collapse(max_level - 1);
  return MaxLevel() != max_level;
}

int ExpanderImpl::MinLevel() const {
  if (!expanded) {
    return 0;
  }
  if (children_.size() == 0) {
    return 1;
  }
  int min_level = 999999;
  for (auto& child : children_) {
    min_level = std::min(min_level, 1 + child->MinLevel());
  }
  return min_level;
}

int ExpanderImpl::MaxLevel() const {
  if (!expanded) {
    return 0;
  }
  int max_level = 1;
  for (auto& child : children_) {
    max_level = std::max(max_level, 1 + child->MaxLevel());
  }
  return max_level;
}

void ExpanderImpl::Expand(int minLevel) {
  if (minLevel <= 0)
    return;
  expanded = true;
  minLevel--;
  for (auto& child : children_) {
    child->Expand(minLevel);
  }
}

void ExpanderImpl::Collapse(int maxLevel) {
  if (maxLevel <= 0) {
    expanded = false;
  }
  maxLevel--;
  for (auto& child : children_) {
    child->Collapse(maxLevel);
  }
}
