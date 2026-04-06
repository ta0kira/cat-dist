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

#ifndef CAT_DIST_NODE_TRAITS_H_
#define CAT_DIST_NODE_TRAITS_H_

#include <memory>

namespace cat_dist {

template<class N>
struct TreeNodeOperations {
  using K = typename N::K;
  using V = typename N::V;

  static __attribute__((warn_unused_result)) std::unique_ptr<N> NewNode(const K& key, const V& value) { return std::make_unique<N>(key, value); }
  static __attribute__((warn_unused_result)) std::unique_ptr<N> CopyNode(const N& node) { return node.CopyNode(); }

  static const std::unique_ptr<N>& GetHigherNode(const N& node) { return node.GetHigherNode(); }
  static const std::unique_ptr<N>& GetLowerNode(const N& node) { return node.GetLowerNode(); }
  static const K& GetKey(const N& node) { return node.GetKey(); }
  static const V& GetValue(const N& node) { return node.GetValue(); }
  static int GetHeight(const N& node) { return node.GetHeight(); }

  static bool KeyLessThan(const K& key1, const K& key2) { return key1 < key2; }

  static std::unique_ptr<N>& GetHigherNode(N& node) { return node.GetHigherNode(); }
  static std::unique_ptr<N>& GetLowerNode(N& node) { return node.GetLowerNode(); }
  static V& GetValue(N& node) { return node.GetValue(); }

  static __attribute__((warn_unused_result)) std::unique_ptr<N> SetHigherNode(N& node, std::unique_ptr<N> child) { return node.SetHigherNode(std::move(child)); }
  static __attribute__((warn_unused_result)) std::unique_ptr<N> SetLowerNode(N& node, std::unique_ptr<N> child) { return node.SetLowerNode(std::move(child)); }
  static void SetValue(N& node, V value) { node.SetValue(std::move(value)); }
  static void UpdateNode(N& node) { node.UpdateNode(); }
};

}  // namespace cat_dist

#endif  // CAT_DIST_NODE_TRAITS_H_
