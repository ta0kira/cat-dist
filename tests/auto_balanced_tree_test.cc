/* -----------------------------------------------------------------------------
Copyright 2026 Kevin P. Barry

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
----------------------------------------------------------------------------- */

// Author: Kevin P. Barry [ta0kira@gmail.com]

#include "cat_dist/auto_balanced_tree.h"

#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "helpers.h"

using namespace cat_dist;
using namespace cat_dist::tests;

namespace {

class TestNode {
 public:
  using K = std::string;
  using V = int;

  TestNode(K key, V value) : key_(std::move(key)), value_(std::move(value)) {}

  const std::unique_ptr<TestNode>& GetHigherNode() const { return higher_child_; }
  const std::unique_ptr<TestNode>& GetLowerNode() const { return lower_child_; }
  const K& GetKey() const { return key_; }
  const V& GetValue() const { return value_; }
  int GetHeight() const { return height_; }

  std::unique_ptr<TestNode>& GetHigherNode() { return higher_child_; }
  std::unique_ptr<TestNode>& GetLowerNode() { return lower_child_; }
  V& GetValue() { return value_; }

  std::unique_ptr<TestNode> SetHigherNode(std::unique_ptr<TestNode> child) {
    auto old_child = std::move(higher_child_);
    higher_child_ = std::move(child);
    return old_child;
  }
  std::unique_ptr<TestNode> SetLowerNode(std::unique_ptr<TestNode> child) {
    auto old_child = std::move(lower_child_);
    lower_child_ = std::move(child);
    return old_child;
  }
  void SetValue(V value) { value_ = std::move(value); }
  void UpdateNode() {
    const int higher_height = higher_child_ ? higher_child_->height_ : 0;
    const int lower_height = lower_child_ ? lower_child_->height_ : 0;
    if (higher_height > lower_height) {
      height_ = higher_height+1;
    } else {
      height_ = lower_height+1;
    }
  }

 private:
  int height_ = 1;
  const K key_;
  V value_ = 0;
  std::unique_ptr<TestNode> higher_child_;
  std::unique_ptr<TestNode> lower_child_;
};

using TestTree = AutoBalancedTree<TestNode>;

void DescribeTree(const TestTree& tree, std::ostream& output);

std::string ZeroPad(int x, int digits);

std::unique_ptr<const MatcherNode<std::string>> EmptyNode();

std::unique_ptr<const MatcherNode<std::string>> MatchNode(
    std::string key,
    std::unique_ptr<const MatcherNode<std::string>> lower = EmptyNode(),
    std::unique_ptr<const MatcherNode<std::string>> higher = EmptyNode());

}  // namespace

TEST_CASE("AutoBalancedTree") {
  TestTree tree;

  SECTION("insertion with no balancing") {
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    CHECK(tree.node_count() == 3);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("2",
        MatchNode("1"),
        MatchNode("3"))
    ));
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
  }

  SECTION("insertion with lower+lower balancing") {
    tree.Set("4", 4);
    tree.Set("3", 3);
    tree.Set("2", 2);
    tree.Set("1", 1);
    CHECK(tree.node_count() == 4);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("3",
        MatchNode("2",
          MatchNode("1")),
        MatchNode("4"))
    ));
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("insertion with higher+higher balancing") {
    tree.Set("1", 1);
    tree.Set("2", 2);
    tree.Set("3", 3);
    tree.Set("4", 4);
    CHECK(tree.node_count() == 4);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("2",
        MatchNode("1"),
        MatchNode("3",
          EmptyNode(),
          MatchNode("4")))
    ));
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("insertion with lower+higher balancing") {
    tree.Set("4", 4);
    tree.Set("3", 3);
    tree.Set("1", 1);
    tree.Set("2", 2);
    CHECK(tree.node_count() == 4);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("3",
        MatchNode("1",
          EmptyNode(),
          MatchNode("2")),
        MatchNode("4"))
    ));
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("insertion with higher+lower balancing") {
    tree.Set("1", 1);
    tree.Set("2", 2);
    tree.Set("4", 4);
    tree.Set("3", 3);
    CHECK(tree.node_count() == 4);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("2",
        MatchNode("1"),
        MatchNode("4",
          MatchNode("3")))
    ));
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("clear all") {
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    tree.ClearAll();
    CHECK(tree.node_count() == 0);
    CHECK(tree.Get("1") == nullptr);
    CHECK(tree.Get("2") == nullptr);
    CHECK(tree.Get("3") == nullptr);
  }

  SECTION("removal of missing") {
    tree.Unset("1");
    CHECK(tree.node_count() == 0);
  }

  SECTION("removal of leaf with no rebalance") {
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    tree.Unset("1");
    CHECK(tree.node_count() == 2);
    CHECK(tree.Get("1") == nullptr);
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
  }

  SECTION("removal of non-leaf with no rebalance") {
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    tree.Unset("2");
    CHECK(tree.node_count() == 2);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK(tree.Get("2") == nullptr);
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
  }

  SECTION("removal with higher rebalancing") {
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    tree.Set("4", 4);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("2",
        MatchNode("1"),
        MatchNode("3",
          EmptyNode(),
          MatchNode("4")))
    ));
    tree.Unset("2");
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("3",
        MatchNode("1"),
        MatchNode("4"))
    ));
    CHECK(tree.node_count() == 3);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK(tree.Get("2") == nullptr);
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("removal with lower rebalancing") {
    tree.Set("3", 3);
    tree.Set("2", 2);
    tree.Set("4", 4);
    tree.Set("1", 1);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("3",
        MatchNode("2",
          MatchNode("1")),
        MatchNode("4"))
    ));
    tree.Unset("3");
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("2",
        MatchNode("1"),
        MatchNode("4"))
    ));
    CHECK(tree.node_count() == 3);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK(tree.Get("3") == nullptr);
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("removal with higher+lower rebalancing") {
    tree.Set("3", 3);
    tree.Set("1", 1);
    tree.Set("5", 5);
    tree.Set("2", 2);
    tree.Set("4", 4);
    tree.Set("6", 6);
    tree.Set("7", 7);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("3",
        MatchNode("1",
          EmptyNode(),
          MatchNode("2")),
        MatchNode("5",
          MatchNode("4"),
          MatchNode("6",
            EmptyNode(),
            MatchNode("7"))))
    ));
    tree.Unset("3");
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("4",
        MatchNode("1",
          EmptyNode(),
          MatchNode("2")),
        MatchNode("6",
          MatchNode("5"),
          MatchNode("7")))
    ));
    CHECK(tree.node_count() == 6);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK(tree.Get("3") == nullptr);
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
    CHECK_THAT(tree.Get("5"), NodeValueMatches(5));
    CHECK_THAT(tree.Get("6"), NodeValueMatches(6));
    CHECK_THAT(tree.Get("7"), NodeValueMatches(7));
  }

  SECTION("removal with lower+higher rebalancing") {
    tree.Set("5", 5);
    tree.Set("3", 3);
    tree.Set("7", 7);
    tree.Set("6", 6);
    tree.Set("4", 4);
    tree.Set("2", 2);
    tree.Set("1", 1);
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("5",
        MatchNode("3",
          MatchNode("2",
            MatchNode("1")),
          MatchNode("4")),
        MatchNode("7",
          MatchNode("6")))
    ));
    tree.Unset("5");
    CHECK_THAT(tree.root_node(), MatchesStructure(
      MatchNode("4",
        MatchNode("2",
          MatchNode("1"),
          MatchNode("3")),
        MatchNode("7",
          MatchNode("6")))
    ));
    CHECK(tree.node_count() == 6);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
    CHECK(tree.Get("5") == nullptr);
    CHECK_THAT(tree.Get("6"), NodeValueMatches(6));
    CHECK_THAT(tree.Get("7"), NodeValueMatches(7));
  }

  SECTION("functional test") {
    const int count = 100;
    const int perm = 47;
    const int offset = 12;

    for (int i = 0; i < count; ++i) {
      const std::string error_note = QuickFormat() << "[insert iteration " << i << "] ";
      const int value = (i*perm+offset)%count;
      const std::string key = ZeroPad(value, 3);
      tree.Set(key, value);
      REQUIRE_THAT(tree.Get(key), NodeValueMatches(value, error_note));
      REQUIRE(tree.node_count() == i+1);
      REQUIRE_THAT(tree, IsBalanced(error_note));
      REQUIRE_THAT(tree, IsOrderedCorrectly(error_note));
      REQUIRE_THAT(tree, HasCorrectCount(error_note));
    }

#ifdef VERBOSE_TESTS
    std::cerr << "### Tree after creation ###" << std::endl;
    DescribeTree(tree, std::cerr);
#endif  // VERBOSE_TESTS

    for (int i = 0; i < count; ++i) {
      const std::string error_note = QuickFormat() << "[check iteration " << i << "] ";
      const int value = (i*perm+offset)%count;
      const std::string key = ZeroPad(value, 3);
      REQUIRE_THAT(tree.Get(key), NodeValueMatches(value, error_note));
    }

    for (int i = 0; i < count; ++i) {
      const std::string error_note = QuickFormat() << "[remove iteration " << i << "] ";
      const int value = (i*perm+offset)%count;
      const std::string key = ZeroPad(value, 3);
      tree.Unset(key);
#ifdef VERBOSE_TESTS
      std::cerr << "### Tree after removal " << i << " ###" << std::endl;
      DescribeTree(tree, std::cerr);
#endif  // VERBOSE_TESTS
      REQUIRE(tree.Get(key) == nullptr);
      REQUIRE(tree.node_count() == count-i-1);
      for (int j = i+1; j < count; ++j) {
        const std::string error_note2 = QuickFormat() << "[remove iteration " << i << ":" << j << "] ";
        const int value2 = (j*perm+offset)%count;
        const std::string key2 = ZeroPad(value2, 3);
        REQUIRE_THAT(tree.Get(key2), NodeValueMatches(value2, error_note2));
      }
      REQUIRE_THAT(tree, IsBalanced(error_note));
      REQUIRE_THAT(tree, IsOrderedCorrectly(error_note));
      REQUIRE_THAT(tree, HasCorrectCount(error_note));
    }
  }

  CHECK_THAT(tree, IsBalanced());
  CHECK_THAT(tree, IsOrderedCorrectly());
  CHECK_THAT(tree, HasCorrectCount());
}

namespace {

__attribute__((unused)) void DescribeNode(const TestNode* node, std::ostream& output, const std::string& prefix) {
  if (node) {
    output << node->GetKey() << std::endl;
    if (const auto& lower = node->GetLowerNode()) {
      output << prefix << "|->";
      DescribeNode(lower.get(), output, prefix + "|  ");
    } else if (node->GetHigherNode()) {
      std::cerr << prefix << "|->_" << std::endl;
    }
    if (const auto& higher = node->GetHigherNode()) {
      output << prefix << "\\->";
      DescribeNode(higher.get(), output, prefix + "   ");
    } else if (node->GetLowerNode()) {
      std::cerr << prefix << "\\->_" << std::endl;
    }
  }
}

__attribute__((unused)) void DescribeTree(const TestTree& tree, std::ostream& output) {
  DescribeNode(tree.root_node(), output, "");
}

std::string ZeroPad(int x, int digits) {
  std::ostringstream formatted;
  formatted << std::setfill('0') << std::setw(digits) << x;
  return formatted.str();
}

std::unique_ptr<const MatcherNode<std::string>> EmptyNode() {
  return nullptr;
}

std::unique_ptr<const MatcherNode<std::string>> MatchNode(
    std::string key,
    std::unique_ptr<const MatcherNode<std::string>> lower,
    std::unique_ptr<const MatcherNode<std::string>> higher) {
  return std::make_unique<MatcherNode<std::string>>(MatcherNode<std::string> {
    .key = std::move(key),
    .lower = std::move(lower),
    .higher = std::move(higher),
  });
}

}  // namespace
