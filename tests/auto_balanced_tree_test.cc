#include "cat_dist/auto_balanced_tree.h"

#include <memory>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include "helpers.h"

using namespace cat_dist;
using namespace cat_dist::tests;

class TestNode {
 public:
  using K = std::string;
  using V = int;

  explicit TestNode(K key) : key_(std::move(key)) {}

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
  int height_ = 0;
  const K key_;
  V value_ = 0;
  std::unique_ptr<TestNode> higher_child_;
  std::unique_ptr<TestNode> lower_child_;
};

using TestTree = AutoBalancedTree<TestNode>;

TEST_CASE("AutoBalancedTree") {
  TestTree tree;

  SECTION("insertion with no balancing") {
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    CHECK(tree.node_count() == 3);
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
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
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
    //   2            3
    // 1   3    ->  1   4
    //       4
    tree.Set("2", 2);
    tree.Set("1", 1);
    tree.Set("3", 3);
    tree.Set("4", 4);
    tree.Unset("2");
    CHECK(tree.node_count() == 3);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK(tree.Get("2") == nullptr);
    CHECK_THAT(tree.Get("3"), NodeValueMatches(3));
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("removal with lower rebalancing") {
    //     3          2
    //   2   4  ->  1   4
    // 1
    tree.Set("3", 3);
    tree.Set("2", 2);
    tree.Set("4", 4);
    tree.Set("1", 1);
    tree.Unset("3");
    CHECK(tree.node_count() == 3);
    CHECK_THAT(tree.Get("1"), NodeValueMatches(1));
    CHECK_THAT(tree.Get("2"), NodeValueMatches(2));
    CHECK(tree.Get("3") == nullptr);
    CHECK_THAT(tree.Get("4"), NodeValueMatches(4));
  }

  SECTION("functional test") {
    const int count = 100;
    const int perm = 47;
    const int offset = 12;

    for (int i = 0; i < count; ++i) {
      const std::string error_note = "[insert iteration " + std::to_string(i) + "] ";
      const int value = (i*perm+offset)%count;
      const std::string key = std::to_string(value);
      tree.Set(key, value);
      REQUIRE_THAT(tree.Get(key), NodeValueMatches(value, error_note));
      REQUIRE(tree.node_count() == i+1);
      REQUIRE_THAT(tree, IsBalanced(error_note));
      REQUIRE_THAT(tree, IsOrderedCorrectly(error_note));
      REQUIRE_THAT(tree, HasCorrectCount(error_note));
    }

    for (int i = 0; i < count; ++i) {
      const std::string error_note = "[check iteration " + std::to_string(i) + "] ";
      const int value = (i*perm+offset)%count;
      const std::string key = std::to_string(value);
      REQUIRE_THAT(tree.Get(key), NodeValueMatches(value, error_note));
    }

    for (int i = 0; i < count; ++i) {
      const std::string error_note = "[remove iteration " + std::to_string(i) + "] ";
      const int value = (i*perm+offset)%count;
      const std::string key = std::to_string(value);
      tree.Unset(key);
      REQUIRE(tree.Get(key) == nullptr);
      REQUIRE(tree.node_count() == count-i-1);
      for (int j = i+1; j < count; ++j) {
        const std::string error_note2 = "[remove iteration " + std::to_string(i) + ":" + std::to_string(i) + "] ";
        const int value2 = (j*perm+offset)%count;
        const std::string key2 = std::to_string(value2);
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
