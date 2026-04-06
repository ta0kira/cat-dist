#ifndef CAT_DIST_CATEGORICAL_DISTRIBUTION_H_
#define CAT_DIST_CATEGORICAL_DISTRIBUTION_H_

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <set>

#include "auto_balanced_tree.h"
#include "node_traits.h"

namespace cat_dist {

template<class C, class W = int>
class CategoricalDistribution {
 public:
  CategoricalDistribution() = default;
  CategoricalDistribution DeepCopy() const;
  void ClearAll();

  W GetTotalWeight() const;
  int GetUniqueCount() const;
  std::set<C> GetUniqueCategories() const;
  std::map<C, W> ExportWeights() const;
  void SetWeight(const C& category, const W& weight);
  void AdjustWeight(const C& category, const W& adjust);
  W GetWeight(const C& category) const;
  const std::optional<C> LocateByWeight(const W& weight) const;
  const std::optional<C> LocateByWeight(const W& weight, const W& adjust);

#ifdef CAT_DIST_TESTING
  struct TestVisitor;
  friend struct TestVisitor;
#endif  // CAT_DIST_TESTING

 private:
  static constexpr W kZero{};
  class CategoricalNode;

  explicit CategoricalDistribution(AutoBalancedTree<CategoricalNode> tree) : tree_(std::move(tree)) {}

  AutoBalancedTree<CategoricalNode> tree_;
};

}  // namespace cat_dist

namespace cat_dist {

#ifdef CAT_DIST_TESTING
template<class C, class W>
struct CategoricalDistribution<C, W>::TestVisitor {
  static void ValidateTree(const CategoricalDistribution& dist, const std::function<void(const AutoBalancedTree<CategoricalNode>&)>& check) {
    check(dist.tree_);
  }
};
#endif  // CAT_DIST_TESTING

template<class C, class W>
class CategoricalDistribution<C, W>::CategoricalNode {
 public:
  using K = C;
  using V = W;

  CategoricalNode(C category, W weight) : category_(std::move(category)), size_(std::move(weight)) {}
  std::unique_ptr<CategoricalNode> CopyNode() const;

  static const CategoricalNode* LocateByWeight(const CategoricalNode* node, W weight);
  static void TraversePreorder(const CategoricalNode* node, const std::function<void(const CategoricalNode&)>& function);
  static W GetTotalWeight(const CategoricalNode* node) { return node ? node->total_ : kZero; }

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
  int height_ = 1;
  const C category_;
  W size_ {};
  W total_ {};
  std::unique_ptr<CategoricalNode> higher_child_;
  std::unique_ptr<CategoricalNode> lower_child_;
};

template<class C, class W>
std::unique_ptr<typename CategoricalDistribution<C, W>::CategoricalNode> CategoricalDistribution<C, W>::CategoricalNode::CopyNode() const {
  std::unique_ptr<CategoricalNode> new_node = std::make_unique<CategoricalNode>(category_, size_);
  if (higher_child_) {
    new_node->lower_child_ = lower_child_->CopyNode();
  }
  if (higher_child_) {
    new_node->higher_child_ = higher_child_->CopyNode();
  }
  new_node->UpdateNode();
  return new_node;
}

template<class C, class W>
const typename CategoricalDistribution<C, W>::CategoricalNode* CategoricalDistribution<C, W>::CategoricalNode::LocateByWeight(const CategoricalNode* node, W weight) {
  if (node && weight >= kZero && weight < node->total_) {
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
void CategoricalDistribution<C, W>::CategoricalNode::TraversePreorder(const CategoricalNode* node, const std::function<void(const CategoricalNode&)>& function) {
  if (node) {
    function(*node);
    TraversePreorder(node->GetLowerNode().get(), function);
    TraversePreorder(node->GetHigherNode().get(), function);
  }
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
CategoricalDistribution<C, W> CategoricalDistribution<C, W>::DeepCopy() const {
  return CategoricalDistribution(tree_.DeepCopy());
}

template<class C, class W>
void CategoricalDistribution<C, W>::ClearAll() {
  tree_.ClearAll();
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
std::set<C> CategoricalDistribution<C, W>::GetUniqueCategories() const {
  std::set<C> output;
  CategoricalNode::TraversePreorder(tree_.root_node(), [&output](const auto& node) {
    output.insert(node.GetKey());
  });
  return output;
}

template<class C, class W>
std::map<C, W> CategoricalDistribution<C, W>::ExportWeights() const {
  std::map<C, W> output;
  CategoricalNode::TraversePreorder(tree_.root_node(), [&output](const auto& node) {
    output.emplace(node.GetKey(), node.GetValue());
  });
  return output;
}

template<class C, class W>
void CategoricalDistribution<C, W>::SetWeight(const C& category, const W& weight) {
  if (weight <= kZero) {
    tree_.Unset(category);
  } else {
    tree_.Set(category, weight);
  }
}

template<class C, class W>
void CategoricalDistribution<C, W>::AdjustWeight(const C& category, const W& adjust) {
  SetWeight(category, GetWeight(category) + adjust);
}

template<class C, class W>
W CategoricalDistribution<C, W>::GetWeight(const C& category) const {
  const CategoricalNode* node = tree_.Get(category);
  return node ? node->GetValue() : kZero;
}

template<class C, class W>
const std::optional<C> CategoricalDistribution<C, W>::LocateByWeight(const W& weight) const {
  const CategoricalNode* node = CategoricalNode::LocateByWeight(tree_.root_node(), weight);
  if (node) {
    return node->GetKey();
  } else {
    return std::nullopt;
  }
}

template<class C, class W>
const std::optional<C> CategoricalDistribution<C, W>::LocateByWeight(const W& weight, const W& adjust) {
  const CategoricalNode* node = CategoricalNode::LocateByWeight(tree_.root_node(), weight);
  if (node) {
    const auto key = node->GetKey();
    SetWeight(node->GetKey(), node->GetValue() + adjust);
    return key;
  } else {
    return std::nullopt;
  }
}

}  // namespace cat_dist

#endif  // CAT_DIST_CATEGORICAL_DISTRIBUTION_H_
