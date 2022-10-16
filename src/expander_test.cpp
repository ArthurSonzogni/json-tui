#include <gtest/gtest.h>
#include "expander.hpp"

TEST(Expander, Basic) {
  auto a = ExpanderImpl::New();
  auto a0 = a->Expander();
  auto a1 = a->Expander();
  auto a00 = a0->Expander();
  auto a01 = a0->Expander();
  auto a10 = a1->Expander();
  auto a11 = a1->Expander();

  EXPECT_EQ(a->MinLevel(), 0);
  EXPECT_EQ(a->MaxLevel(), 0);

  EXPECT_EQ(a00->MinLevel(), 0);
  EXPECT_EQ(a00->MaxLevel(), 0);

  a->expanded = true;

  EXPECT_EQ(a->MinLevel(), 1);
  EXPECT_EQ(a->MaxLevel(), 1);

  EXPECT_EQ(a00->MinLevel(), 0);
  EXPECT_EQ(a00->MaxLevel(), 0);

  a0->expanded = true;

  EXPECT_EQ(a->MinLevel(), 1);
  EXPECT_EQ(a->MaxLevel(), 2);

  a1->expanded = true;

  EXPECT_EQ(a->MinLevel(), 2);
  EXPECT_EQ(a->MaxLevel(), 2);

  a00->expanded = true;

  EXPECT_EQ(a->MinLevel(), 2);
  EXPECT_EQ(a->MaxLevel(), 3);

  a01->expanded = true;

  EXPECT_EQ(a->MinLevel(), 2);
  EXPECT_EQ(a->MaxLevel(), 3);

  a10->expanded = true;

  EXPECT_EQ(a->MinLevel(), 2);
  EXPECT_EQ(a->MaxLevel(), 3);

  a11->expanded = true;

  EXPECT_EQ(a->MinLevel(), 3);
  EXPECT_EQ(a->MaxLevel(), 3);
}

TEST(Expander, Expand) {
  auto a = ExpanderImpl::New();
  auto a0 = a->Expander();
  auto a1 = a->Expander();
  auto a00 = a0->Expander();
  auto a01 = a0->Expander();
  auto a10 = a1->Expander();
  auto a11 = a1->Expander();

  EXPECT_EQ(a->MinLevel(), 0);

  a->Expand();
  EXPECT_EQ(a->MinLevel(), 1);

  a->Expand();
  EXPECT_EQ(a->MinLevel(), 2);

  a->Expand();
  EXPECT_EQ(a->MinLevel(), 3);

  a->Expand();
  EXPECT_EQ(a->MinLevel(), 3);

  // ----

  a1->expanded = false;
  EXPECT_EQ(a->MinLevel(), 1);

  a->Expand();
  EXPECT_TRUE(a1->expanded);
  EXPECT_EQ(a->MinLevel(), 3);

  // ----

  a->expanded = false;
  EXPECT_EQ(a->MinLevel(), 0);

  a->Expand();
  EXPECT_TRUE(a->expanded);
  EXPECT_EQ(a->MinLevel(), 3);
}

TEST(Expander, Collapse) {
  auto a = ExpanderImpl::New();
  auto a0 = a->Expander();
  auto a1 = a->Expander();
  auto a00 = a0->Expander();
  auto a01 = a0->Expander();
  auto a10 = a1->Expander();
  auto a11 = a1->Expander();

  EXPECT_EQ(a->MaxLevel(), 0);

  a->Expand();
  a->Expand();
  a->Expand();
  a->Expand();
  EXPECT_EQ(a->MaxLevel(), 3);

  a->Collapse();
  EXPECT_EQ(a->MaxLevel(), 2);
  
  a->Collapse();
  EXPECT_EQ(a->MaxLevel(), 1);
  
  a->Collapse();
  EXPECT_EQ(a->MaxLevel(), 0);
}

TEST(Expander, CollapseBranch) {
  auto a = ExpanderImpl::New();
  auto a0 = a->Expander();
  auto a1 = a->Expander();
  auto a00 = a0->Expander();
  auto a01 = a0->Expander();
  auto a10 = a1->Expander();
  auto a11 = a1->Expander();

  a->expanded = true;
  a0->expanded = true;
  a00->expanded = true;
  EXPECT_EQ(a->MaxLevel(), 3);

  a->Collapse();
  EXPECT_EQ(a->expanded, true);
  EXPECT_EQ(a0->expanded, true);
  EXPECT_EQ(a00->expanded, false);
  EXPECT_EQ(a->MaxLevel(), 2);

  a->Collapse();
  EXPECT_EQ(a->expanded, true);
  EXPECT_EQ(a0->expanded, false);
  EXPECT_EQ(a00->expanded, false);
  EXPECT_EQ(a->MaxLevel(), 1);

  a->Collapse();
  EXPECT_EQ(a->expanded, false);
  EXPECT_EQ(a0->expanded, false);
  EXPECT_EQ(a00->expanded, false);
  EXPECT_EQ(a->MaxLevel(), 0);
}
