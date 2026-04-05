#ifndef CAT_DIST_AUTO_BALANCED_TREE_H_
#define CAT_DIST_AUTO_BALANCED_TREE_H_

#include<memory>

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
  const V* Get(const K& key) const;
  V* Get(const K& key);
  // Ownership of value is not passed.
  const void Set(const K& key, const V* value);

 private:
  void ClearRoot();

  static int GetBalance(const N* node);
  static int Exchange(std::unique_ptr<N>& node, const K& key, const V* value, std::unique_ptr<N>& new_root);
  static const V* Find(const N* node, const K& key);
  static V* Find(N* node, const K& key);
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
const typename AutoBalancedTree<N>::V* AutoBalancedTree<N>::Get(const K& key) const {
  return Find(root_node_.get(), key);
}

template<class N>
typename AutoBalancedTree<N>::V* AutoBalancedTree<N>::Get(const K& key) {
  return Find(root_node_.get(), key);
}

template<class N>
const void AutoBalancedTree<N>::Set(const K& key, const V* value) {
  std::unique_ptr<N> new_root;
  const int size_change = Exchange(root_node_, key, value, new_root);
  node_count_ += size_change;
  if (size_change != 0) {
    root_node_ = std::move(new_root);
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
    if (auto& lower = TreeNodeOperations<N>::GetLowerNode(*node)) {
      balance -= TreeNodeOperations<N>::GetHeight(*lower);
    }
    if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
      balance += TreeNodeOperations<N>::GetHeight(*higher);
    }
  }
  return balance;
}

template<class N>
int AutoBalancedTree<N>::Exchange(std::unique_ptr<N>& node, const K& key, const V* value, std::unique_ptr<N>& new_root) {
  int size_change = 0;
  if (!node) {
    if (value) {
      size_change = 1;
      new_root = TreeNodeAllocation<N>::NewNode(key);
      new_root->SetValue(*value);
    } else {
      new_root.reset();
    }
  } else if (TreeNodeOperations<N>::KeyLessThan(key, TreeNodeOperations<N>::GetKey(*node))) {
    size_change = Exchange(TreeNodeOperations<N>::GetLowerNode(*node), key, value, new_root);
    TreeNodeOperations<N>::SetLowerNode(*node, std::move(new_root));
    new_root = Rebalance(node);
  } else if (TreeNodeOperations<N>::KeyLessThan(TreeNodeOperations<N>::GetKey(*node), key)) {
    size_change = Exchange(TreeNodeOperations<N>::GetHigherNode(*node), key, value, new_root);
    TreeNodeOperations<N>::SetHigherNode(*node, std::move(new_root));
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
const typename AutoBalancedTree<N>::V* AutoBalancedTree<N>::Find(const N* node, const K& key) {
  if (node) {
    if (TreeNodeOperations<N>::KeyLessThan(key, TreeNodeOperations<N>::GetKey(*node))) {
      return Find(TreeNodeOperations<N>::GetLowerNode(*node));
    }
    if (TreeNodeOperations<N>::KeyLessThan(TreeNodeOperations<N>::GetKey(*node), key)) {
      return Find(TreeNodeOperations<N>::GetHigherNode(*node));
    }
    return &TreeNodeOperations<N>::GetValue(*node);
  }
  return nullptr;
}

template<class N>
typename AutoBalancedTree<N>::V* AutoBalancedTree<N>::Find(N* node, const K& key) {
  if (node) {
    if (TreeNodeOperations<N>::KeyLessThan(key, TreeNodeOperations<N>::GetKey(*node))) {
      return Find(TreeNodeOperations<N>::GetLowerNode(*node));
    }
    if (TreeNodeOperations<N>::KeyLessThan(TreeNodeOperations<N>::GetKey(*node), key)) {
      return Find(TreeNodeOperations<N>::GetHigherNode(*node));
    }
    return &TreeNodeOperations<N>::GetValue(*node);
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
      TreeNodeOperations<N>::SetLowerNode(*node, PivotLower(lower));
    }
  }
  // NOTE: `lower` might be different than above.
  if (auto& lower = TreeNodeOperations<N>::GetLowerNode(*node)) {
    TreeNodeOperations<N>::SetLowerNode(*node, std::move(TreeNodeOperations<N>::GetHigherNode(*lower)));
    TreeNodeOperations<N>::UpdateNode(*node);
    TreeNodeOperations<N>::SetHigherNode(*lower, std::move(node));
    TreeNodeOperations<N>::UpdateNode(*lower);
    return std::move(lower);
  } else {
    return nullptr;
  }
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::PivotLower(std::unique_ptr<N>& node) {
  if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
    if (GetBalance(higher.get()) < 0) {
      TreeNodeOperations<N>::SetHigherNode(*node, PivotHigher(higher));
    }
  }
  // NOTE: `higher` might be different than above.
  if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
    TreeNodeOperations<N>::SetHigherNode(*node, std::move(TreeNodeOperations<N>::GetLowerNode(*higher)));
    TreeNodeOperations<N>::UpdateNode(*node);
    TreeNodeOperations<N>::SetLowerNode(*higher, std::move(node));
    TreeNodeOperations<N>::UpdateNode(*higher);
    return std::move(higher);
  } else {
    return nullptr;
  }
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::Remove(std::unique_ptr<N>& node) {
  std::unique_ptr<N> new_root;
  if (GetBalance(node.get()) < 0) {
    TreeNodeOperations<N>::SetLowerNode(*node, RemoveHighest(TreeNodeOperations<N>::GetLowerNode(*node), new_root));
  } else {
    TreeNodeOperations<N>::SetHigherNode(*node, RemoveLowest(TreeNodeOperations<N>::GetHigherNode(*node), new_root));
  }
  if (new_root) {
    std::unique_ptr<N> lower = std::move(TreeNodeOperations<N>::GetLowerNode(*node));
    TreeNodeOperations<N>::SetLowerNode(*node, std::move(TreeNodeOperations<N>::GetLowerNode(*new_root)));
    TreeNodeOperations<N>::SetLowerNode(*new_root, std::move(lower));
    std::unique_ptr<N> higher = std::move(TreeNodeOperations<N>::GetHigherNode(*node));
    TreeNodeOperations<N>::SetHigherNode(*node, std::move(TreeNodeOperations<N>::GetHigherNode(*new_root)));
    TreeNodeOperations<N>::SetHigherNode(*new_root, std::move(higher));
    TreeNodeOperations<N>::UpdateNode(*new_root);
  }
  return new_root;
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::RemoveHighest(std::unique_ptr<N>& node, std::unique_ptr<N>& new_root) {
  if (!node) {
    new_root.reset();
    return nullptr;
  } else if (auto& higher = TreeNodeOperations<N>::GetHigherNode(*node)) {
    TreeNodeOperations<N>::SetHigherNode(*node, RemoveHighest(higher, new_root));
    return Rebalance(node);
  } else {
    std::unique_ptr<N> removed = std::move(TreeNodeOperations<N>::GetLowerNode(*node));
    new_root = std::move(node);
    return removed;
  }
}

template<class N>
std::unique_ptr<N> AutoBalancedTree<N>::RemoveLowest(std::unique_ptr<N>& node, std::unique_ptr<N>& new_root) {
  if (!node) {
    new_root.reset();
    return nullptr;
  } else if (auto& higher = TreeNodeOperations<N>::GetLowerNode(*node)) {
    TreeNodeOperations<N>::SetLowerNode(*node, RemoveLowest(higher, new_root));
    return Rebalance(node);
  } else {
    std::unique_ptr<N> removed = std::move(TreeNodeOperations<N>::GetHigherNode(*node));
    new_root = std::move(node);
    return removed;
  }
}

}  // namespace cat_dist

#endif  // CAT_DIST_AUTO_BALANCED_TREE_H_
