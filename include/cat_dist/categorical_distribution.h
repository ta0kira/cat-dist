#ifndef CAT_DIST_CATEGORICAL_DISTRIBUTION_H_
#define CAT_DIST_CATEGORICAL_DISTRIBUTION_H_

#include <memory>

#include "auto_balanced_tree.h"
#include "node_traits.h"

namespace cat_dist {

template<class C, class W = double>
class CategoricalDistribution {
 public:
  W GetTotalWeight() const;
  int GetUniqueCount() const;
  void SetWeight(const C& category, const W& weight);
  W GetWeight(const C& category) const;
  const C* LocateByWeight(const W& weight) const;
  const C* LocateByWeight(const W& weight, const W& adjust);

 private:
  class CategoricalNode;

  AutoBalancedTree<CategoricalNode> tree_;
};

}  // namespace cat_dist

namespace cat_dist {

template<class C, class W>
class CategoricalDistribution<C, W>::CategoricalNode {
 public:
  using K = C;
  using V = W;

  explicit CategoricalNode(C category) : category_(std::move(category)) {}
  CategoricalNode(const CategoricalNode& other) :
    category_(other.category_),
    higher_child_(other.higher_child_ ? TreeNodeAllocation<CategoricalNode>::CopyNode(*other.higher_child_) : nullptr),
    lower_child_(other.lower_child_ ? TreeNodeAllocation<CategoricalNode>::CopyNode(*other.lower_child_) : nullptr) {}

  static const CategoricalNode* LocateByWeight(const CategoricalNode* node, const W& weight);
  static W GetTotalWeight(const CategoricalNode* node) { return node ? node->total_ : W(); }

  const std::unique_ptr<CategoricalNode>& GetHigherNode() const { return higher_child_; }
  const std::unique_ptr<CategoricalNode>& GetLowerNode() const { return lower_child_; }
  const C& GetKey() const { return category_; }
  const W& GetValue() const { return size_; }
  int GetHeight() const { return height_; }

  std::unique_ptr<CategoricalNode>& GetHigherNode() { return higher_child_; }
  std::unique_ptr<CategoricalNode>& GetLowerNode() { return lower_child_; }
  W& GetValue() { return size_; }

  std::unique_ptr<CategoricalNode> SetHigherNode(std::unique_ptr<CategoricalNode> child) {
    auto old_child = std::move(higher_child_);
    higher_child_ = std::move(child);
    return old_child;
  }
  std::unique_ptr<CategoricalNode> SetLowerNode(std::unique_ptr<CategoricalNode> child) {
    auto old_child = std::move(lower_child_);
    lower_child_ = std::move(child);
    return old_child;
  }
  void SetValue(W value) { size_ = std::move(value); }
  void UpdateNode();

 private:
  int height_ = 0;
  const C category_;
  W size_ {};
  W total_ {};
  std::unique_ptr<CategoricalNode> higher_child_;
  std::unique_ptr<CategoricalNode> lower_child_;
};

template<class C, class W>
const typename CategoricalDistribution<C, W>::CategoricalNode* CategoricalDistribution<C, W>::CategoricalNode::LocateByWeight(const CategoricalNode* node, const W& weight) {
  if (node && weight < node->total_) {
    if (node->lower_child_) {
      if (weight < node->lower_child_->total_) {
        return LocateByWeight(node->lower_child_.get(), weight);
      } else {
        weight -= node->lower_child_->total_;
      }
    }
    if (weight < node->size_) {
      return node;
    } else {
      weight -= node->size_;
    }
    return LocateByWeight(node->higher_child_.get(), weight);
  }
  return nullptr;
}

template<class C, class W>
void CategoricalDistribution<C, W>::CategoricalNode::UpdateNode() {
  const int higher_height = higher_child_ ? higher_child_->height_ : 0;
  const int lower_height = lower_child_ ? lower_child_->height_ : 0;
  if (higher_height > lower_height) {
    height_ = higher_height+1;
  } else {
    height_ = lower_height+1;
  }
  total_ = size_;
  if (higher_child_) {
    total_ += higher_child_->total_;
  }
  if (lower_child_) {
    total_ += lower_child_->total_;
  }
}

template<class C, class W>
W CategoricalDistribution<C, W>::GetTotalWeight() const {
  return CategoricalNode::GetTotalWeight(tree_.root_node());
}

template<class C, class W>
int CategoricalDistribution<C, W>::GetUniqueCount() const {
  return tree_.node_count();
}

template<class C, class W>
void CategoricalDistribution<C, W>::SetWeight(const C& category, const W& weight) {
  tree_.Set(category, (weight <= W()) ? nullptr : &weight);
}

template<class C, class W>
W CategoricalDistribution<C, W>::GetWeight(const C& category) const {
  const CategoricalNode* node = tree_.Get(category);
  return node ? node->size_ : W();
}

template<class C, class W>
const C* CategoricalDistribution<C, W>::LocateByWeight(const W& weight) const {
  const CategoricalNode* node = CategoricalNode::LocateByWeight(tree_.root_node(), weight);
  return node ? node->category_ : nullptr;
}

template<class C, class W>
const C* CategoricalDistribution<C, W>::LocateByWeight(const W& weight, const W& adjust) {
  const CategoricalNode* node = CategoricalNode::LocateByWeight(tree_.root_node(), weight);
  if (node) {
    SetWeight(node->category_, node->size_ + adjust);
    return node->category_;
  } else {
    return nullptr;
  }
}

}  // namespace cat_dist

#endif  // CAT_DIST_CATEGORICAL_DISTRIBUTION_H_
