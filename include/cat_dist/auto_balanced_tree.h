#ifndef CAT_DIST_AUTO_BALANCED_TREE_H_
#define CAT_DIST_AUTO_BALANCED_TREE_H_

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>

#include "node_traits.h"

namespace cat_dist {

template<class N>
class AutoBalancedTree {
 public:
  using K = typename TreeNodeOperations<N>::K;
  using V = typename TreeNodeOperations<N>::V;

  AutoBalancedTree() = default;
  AutoBalancedTree(const AutoBalancedTree& other);
  AutoBalancedTree& operator=(const AutoBalancedTree& other);
  ~AutoBalancedTree();

  N* root_node() { return root_node_.get(); }
  const N* root_node() const { return root_node_.get(); }
  const int node_count() const { return node_count_; }

  // Ownership of value is not passed.
  const N* Get(const K& key) const;
  N* Get(const K& key);
  // Ownership of value is not passed.
  const void Set(const K& key, const V& value);
  const void Unset(const K& key);

  bool CheckBalance(std::ostream* error_log) const;
  bool ValidateCount(std::ostream* error_log) const;

 private:
  void ClearRoot();

  static int GetBalance(const N* node);
  static bool CheckBalance(const N* node, std::ostream* error_log);
  static int CountNodes(const N* node);
  static int Exchange(std::unique_ptr<N>& node, const K& key, const V* value, std::unique_ptr<N>& new_root);
  static const N* Find(const N* node, const K& key);
  static N* Find(N* node, const K& key);
  static std::unique_ptr<N> Rebalance(std::unique_ptr<N>& node);
  static std::unique_ptr<N> PivotHigher(std::unique_ptr<N>& node);
  static std::unique_ptr<N> PivotLower(std::unique_ptr<N>& node);
  static std::unique_ptr<N> Remove(std::unique_ptr<N>& node);
  static std::unique_ptr<N> RemoveHighest(std::unique_ptr<N>& node, std::unique_ptr<N>& new_root);
  static std::unique_ptr<N> RemoveLowest(std::unique_ptr<N>& node, std::unique_ptr<N>& new_root);

  int node_count_ = 0;
  std::unique_ptr<N> root_node_;
};

}  // namespace cat_dist

namespace cat_dist {

template<class N>
AutoBalancedTree<N>::AutoBalancedTree(const AutoBalancedTree& other) {
  root_node_ = other.root_node_ ? TreeNodeAllocation<N>::CopyNode(*other.root_node_) : nullptr;
}

template<class N>
AutoBalancedTree<N>& AutoBalancedTree<N>::operator=(const AutoBalancedTree& other) {
  if (*other != this) {
    ClearRoot();
    root_node_ = other.root_node_ ? TreeNodeAllocation<N>::CopyNode(*other.root_node_) : nullptr;
  }
  return *this;
}

template<class N>
AutoBalancedTree<N>::~AutoBalancedTree() {
  ClearRoot();
}

template<class N>
const N* AutoBalancedTree<N>::Get(const K& key) const {
  return Find(root_node_.get(), key);
}

template<class N>
N* AutoBalancedTree<N>::Get(const K& key) {
  return Find(root_node_.get(), key);
}

template<class N>
const void AutoBalancedTree<N>::Set(const K& key, const V& value) {
  std::unique_ptr<N> new_root;
  const int size_change = Exchange(root_node_, key, &value, new_root);
  node_count_ += size_change;
  if (size_change != 0 || new_root) {
    root_node_ = std::move(new_root);
  }
}

template<class N>
const void AutoBalancedTree<N>::Unset(const K& key) {
  std::unique_ptr<N> new_root;
  const int size_change = Exchange(root_node_, key, nullptr, new_root);
  node_count_ += size_change;
  if (size_change != 0 || new_root) {
    root_node_ = std::move(new_root);
  }
}

template<class N>
bool AutoBalancedTree<N>::CheckBalance(std::ostream* error_log) const {
  return CheckBalance(root_node_.get(), error_log);
}

template<class N>
bool AutoBalancedTree<N>::ValidateCount(std::ostream* error_log) const {
  const int actual_count = CountNodes(root_node_.get());
  if (actual_count != node_count_) {
    if (error_log) {
      *error_log << "expected count " << node_count_ << " but got " << actual_count << std::endl;
    }
    return false;
  } else {
    return true;
  }
}

template<class N>
void AutoBalancedTree<N>::ClearRoot() {
  root_node_ = nullptr;
}

template<class N>
int AutoBalancedTree<N>::GetBalance(const N* node) {
  int balance = 0;
  if (node) {
    if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
      balance += TreeNodeOperations<N>::GetHeight(*higher);
    }
    if (auto& lower = TreeNodeOperations<N>::GetLowerNode(*node)) {
      balance -= TreeNodeOperations<N>::GetHeight(*lower);
    }
  }
  return balance;
}

template<class N>
bool AutoBalancedTree<N>::CheckBalance(const N* node, std::ostream* error_log) {
  const int balance = GetBalance(node);
  bool is_balanced = std::abs(balance) <= 1;
  if (!is_balanced && error_log) {
    *error_log << "node " << TreeNodeOperations<N>::GetKey(*node) << " has balance " << balance << std::endl;
  }
  if (node) {
    is_balanced &= CheckBalance(TreeNodeOperations<N>::GetHigherNode(*node).get(), error_log);
    is_balanced &= CheckBalance(TreeNodeOperations<N>::GetLowerNode(*node).get(), error_log);
  }
  return is_balanced;
}

template<class N>
int AutoBalancedTree<N>::CountNodes(const N* node) {
  int count = 0;
  if (node) {
    count += 1;
    count += CountNodes(TreeNodeOperations<N>::GetHigherNode(*node).get());
    count += CountNodes(TreeNodeOperations<N>::GetLowerNode(*node).get());
  }
  return count;
}

template<class N>
int AutoBalancedTree<N>::Exchange(std::unique_ptr<N>& node, const K& key, const V* value, std::unique_ptr<N>& new_root) {
  int size_change = 0;
  if (!node) {
    if (value) {
      size_change = 1;
      new_root = TreeNodeAllocation<N>::NewNode(key);
      new_root->SetValue(*value);
    }
  } else if (TreeNodeOperations<N>::KeyLessThan(key, TreeNodeOperations<N>::GetKey(*node))) {
    std::unique_ptr<N> new_root2;
    size_change = Exchange(TreeNodeOperations<N>::GetLowerNode(*node), key, value, new_root2);
    auto discard = TreeNodeOperations<N>::SetLowerNode(*node, std::move(new_root2));
    assert(!discard);
    new_root = Rebalance(node);
  } else if (TreeNodeOperations<N>::KeyLessThan(TreeNodeOperations<N>::GetKey(*node), key)) {
    std::unique_ptr<N> new_root2;
    size_change = Exchange(TreeNodeOperations<N>::GetHigherNode(*node), key, value, new_root2);
    auto discard = TreeNodeOperations<N>::SetHigherNode(*node, std::move(new_root2));
    assert(!discard);
    new_root = Rebalance(node);
  } else if (value) {
    TreeNodeOperations<N>::SetValue(*node, *value);
    TreeNodeOperations<N>::UpdateNode(*node);
    new_root = std::move(node);
  } else {
    size_change = -1;
    new_root = Remove(node);
    new_root = Rebalance(new_root);
  }
  return size_change;
}

template<class N>
const N* AutoBalancedTree<N>::Find(const N* node, const K& key) {
  if (node) {
    if (TreeNodeOperations<N>::KeyLessThan(key, TreeNodeOperations<N>::GetKey(*node))) {
      return Find(TreeNodeOperations<N>::GetLowerNode(*node).get(), key);
    }
    if (TreeNodeOperations<N>::KeyLessThan(TreeNodeOperations<N>::GetKey(*node), key)) {
      return Find(TreeNodeOperations<N>::GetHigherNode(*node).get(), key);
    }
    return node;
  }
  return nullptr;
}

template<class N>
N* AutoBalancedTree<N>::Find(N* node, const K& key) {
  if (node) {
    if (TreeNodeOperations<N>::KeyLessThan(key, TreeNodeOperations<N>::GetKey(*node))) {
      return Find(TreeNodeOperations<N>::GetLowerNode(*node).get(), key);
    }
    if (TreeNodeOperations<N>::KeyLessThan(TreeNodeOperations<N>::GetKey(*node), key)) {
      return Find(TreeNodeOperations<N>::GetHigherNode(*node).get(), key);
    }
    return node;
  }
  return nullptr;
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::Rebalance(std::unique_ptr<N>& node) {
  if (node) {
    TreeNodeOperations<N>::UpdateNode(*node);
    const int balance = GetBalance(node.get());
    if (balance > 1) {
      return PivotLower(node);
    } else if (balance < -1) {
      return PivotHigher(node);
    }
  }
  return std::move(node);
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::PivotHigher(std::unique_ptr<N>& node) {
  if (auto& lower = TreeNodeOperations<N>::GetLowerNode(*node)) {
    if (GetBalance(lower.get()) > 0) {
      auto discard = TreeNodeOperations<N>::SetLowerNode(*node, PivotLower(lower));
      assert(!discard);
    }
  }
  // NOTE: `lower` might be different than above.
  if (std::unique_ptr<N> lower = TreeNodeOperations<N>::SetLowerNode(*node, nullptr)) {
    auto discard1 = TreeNodeOperations<N>::SetLowerNode(*node, TreeNodeOperations<N>::SetHigherNode(*lower, nullptr));
    assert(!discard1);
    TreeNodeOperations<N>::UpdateNode(*node);
    auto discard2 = TreeNodeOperations<N>::SetHigherNode(*lower, std::move(node));
    assert(!discard2);
    TreeNodeOperations<N>::UpdateNode(*lower);
    return lower;
  } else {
    return nullptr;
  }
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::PivotLower(std::unique_ptr<N>& node) {
  if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
    if (GetBalance(higher.get()) < 0) {
      auto discard = TreeNodeOperations<N>::SetHigherNode(*node, PivotHigher(higher));
      assert(!discard);
    }
  }
  // NOTE: `higher` might be different than above.
  if (std::unique_ptr<N> higher = TreeNodeOperations<N>::SetHigherNode(*node, nullptr)) {
    auto discard1 = TreeNodeOperations<N>::SetHigherNode(*node, TreeNodeOperations<N>::SetLowerNode(*higher, nullptr));
    assert(!discard1);
    TreeNodeOperations<N>::UpdateNode(*node);
    auto discard2 = TreeNodeOperations<N>::SetLowerNode(*higher, std::move(node));
    assert(!discard2);
    TreeNodeOperations<N>::UpdateNode(*higher);
    return higher;
  } else {
    return nullptr;
  }
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::Remove(std::unique_ptr<N>& node) {
  std::unique_ptr<N> replacement;
  std::unique_ptr<N> new_higher;
  std::unique_ptr<N> new_lower;
  if (GetBalance(node.get()) < 0) {
    replacement = RemoveHighest(TreeNodeOperations<N>::GetLowerNode(*node), new_lower);
    new_higher = TreeNodeOperations<N>::SetHigherNode(*node, nullptr);
  } else {
    replacement = RemoveLowest(TreeNodeOperations<N>::GetHigherNode(*node), new_higher);
    new_lower = TreeNodeOperations<N>::SetLowerNode(*node, nullptr);
  }
  if (replacement) {
    auto discard1 = TreeNodeOperations<N>::SetHigherNode(*replacement, std::move(new_higher));
    assert(!discard1);
    auto discard2 = TreeNodeOperations<N>::SetLowerNode(*replacement, std::move(new_lower));
    assert(!discard2);
    TreeNodeOperations<N>::UpdateNode(*replacement);
  }
  node.reset();
  return replacement;
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::RemoveHighest(std::unique_ptr<N>& node, std::unique_ptr<N>& new_root) {
  if (!node) {
    return nullptr;
  } else if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
    std::unique_ptr<N> new_root2;
    std::unique_ptr<N> removed = RemoveHighest(higher, new_root2);
    auto discard = TreeNodeOperations<N>::SetHigherNode(*node, std::move(new_root2));
    assert(!discard);
    new_root = Rebalance(node);
    return removed;
  } else {
    new_root = TreeNodeOperations<N>::SetLowerNode(*node, nullptr);
    return std::move(node);
  }
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::RemoveLowest(std::unique_ptr<N>& node, std::unique_ptr<N>& new_root) {
  if (!node) {
    return nullptr;
  } else if (auto& lower = TreeNodeOperations<N>::GetLowerNode(*node)) {
    std::unique_ptr<N> new_root2;
    std::unique_ptr<N> removed = RemoveLowest(lower, new_root2);
    auto discard = TreeNodeOperations<N>::SetLowerNode(*node, std::move(new_root2));
    assert(!discard);
    new_root = Rebalance(node);
    return removed;
  } else {
    new_root = TreeNodeOperations<N>::SetHigherNode(*node, nullptr);
    return std::move(node);
  }
}

}  // namespace cat_dist

#endif  // CAT_DIST_AUTO_BALANCED_TREE_H_
