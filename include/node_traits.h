#ifndef CAT_DIST_NODE_TRAITS_H_
#define CAT_DIST_NODE_TRAITS_H_

#include<memory>

namespace cat_dist {

template<class N>
struct TreeNodeAllocation {
  using K = typename N::K;

  static std::unique_ptr<N> NewNode(K key) { return std::make_unique<N>(std::move(key)); }
  static std::unique_ptr<N> CopyNode(const N& node) { return std::make_unique<N>(node); }
};

template<class N>
struct TreeNodeOperations {
  using K = typename N::K;
  using V = typename N::V;

  static const std::unique_ptr<N>& GetHigherNode(const N& node) { return node.GetHigherNode(); }
  static const std::unique_ptr<N>& GetLowerNode(const N& node) { return node.GetLowerNode(); }
  static const K& GetKey(const N& node) { return node.GetKey(); }
  static const V& GetValue(const N& node) { return node.GetValue(); }
  static int GetHeight(const N& node) { return node.GetHeight(); }

  static bool KeyLessThan(const K& key1, const K& key2) { return key1 < key2; }

  static std::unique_ptr<N>& GetHigherNode(N& node) { return node.GetHigherNode(); }
  static std::unique_ptr<N>& GetLowerNode(N& node) { return node.GetLowerNode(); }
  static V& GetValue(N& node) { return node.GetValue(); }

  static void SetHigherNode(N& node, std::unique_ptr<N> child) { node.SetHigherNode(std::move(child)); }
  static void SetLowerNode(N& node, std::unique_ptr<N> child) { node.SetLowerNode(std::move(child)); }
  static void SetValue(N& node, V value) { node.SetValue(std::move(value)); }
  static void UpdateNode(N& node) { node.UpdateNode(); }
};

}  // namespace cat_dist

#endif  // CAT_DIST_NODE_TRAITS_H_
