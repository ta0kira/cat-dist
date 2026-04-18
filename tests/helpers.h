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

#ifndef CAT_DIST_TESTS_HELPERS_H_
#define CAT_DIST_TESTS_HELPERS_H_

#include <memory>
#include <optional>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace cat_dist::tests {

class QuickFormat {
 public:
  template <class T>
  QuickFormat& operator<<(const T& value) {
    static_cast<std::ostream&>(output_) << value;
    return *this;
  }

  operator std::string() const { return output_.str(); }

 private:
  std::ostringstream output_;
};

class IsBalanced : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit IsBalanced(std::string note = "") : note_(std::move(note)) {}

  template <class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.CheckBalance(&output_);
  }

  std::string describe() const override {
    return QuickFormat() << note_ << "should be balanced\n" << output_.str();
  }

 private:
  const std::string note_;
  mutable std::ostringstream output_;
};

class IsOrderedCorrectly : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit IsOrderedCorrectly(std::string note = "") : note_(std::move(note)) {}

  template <class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.CheckOrder(&output_);
  }

  std::string describe() const override {
    return QuickFormat() << note_ << "should be ordered correctly\n" << output_.str();
  }

 private:
  const std::string note_;
  mutable std::ostringstream output_;
};

class HasCorrectCount : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit HasCorrectCount(std::string note = "") : note_(std::move(note)) {}

  template <class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.ValidateCount(&output_);
  }

  std::string describe() const override {
    return QuickFormat() << note_ << "should have correct node count\n" << output_.str();
  }

 private:
  const std::string note_;
  mutable std::ostringstream output_;
};

template <class T>
class CheckNodeValueMatches : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit CheckNodeValueMatches(const T& expected, std::string note = "")
      : note_(std::move(note)), expected_(expected) {}

  template <class N>
  bool match(const N* node) const {
    return node ? (node->GetValue() == expected_) : false;
  }

  std::string describe() const override {
    return QuickFormat() << note_ << "matches " << expected_;
  }

 private:
  const std::string note_;
  const T& expected_;
};

template <class T>
CheckNodeValueMatches<T> NodeValueMatches(const T& expected, std::string note = "") {
  return CheckNodeValueMatches<T>(expected, std::move(note));
}

template <class T>
class CheckOptionalMatches : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit CheckOptionalMatches(const T& expected, std::string note = "")
      : note_(std::move(note)), expected_(expected) {}

  template <class O>
  bool match(const O& actual) const {
    return actual ? (*actual == expected_) : false;
  }

  std::string describe() const override {
    return QuickFormat() << note_ << "matches " << expected_;
  }

 private:
  const std::string note_;
  const T& expected_;
};

template <class T>
CheckOptionalMatches<T> OptionalMatches(const T& expected, std::string note = "") {
  return CheckOptionalMatches<T>(expected, std::move(note));
}

template <class T>
struct MatcherNode {
  const T key;
  std::unique_ptr<const MatcherNode> lower;
  std::unique_ptr<const MatcherNode> higher;
};

template <class T>
class StructureMatcher : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit StructureMatcher(std::unique_ptr<const MatcherNode<T>> nodes = nullptr)
      : nodes_(std::move(nodes)) {}

  template <class N>
  bool match(const N* root_node) const {
    return RecursiveMatch(root_node, nodes_.get());
  }

  std::string describe() const override {
    return QuickFormat() << "matches structure\n" << output_.str();
    ;
  }

 private:
  template <class N>
  bool RecursiveMatch(const N* actual, const MatcherNode<T>* expected) const {
    if (!actual && !expected) {
      return true;
    }
    if (!actual) {
      output_ << "expected key " << expected->key << " but got missing" << std::endl;
      return false;
    }
    if (!expected) {
      output_ << "expected missing but got key " << actual->GetKey() << std::endl;
      return false;
    }
    bool matches = true;
    if (actual->GetKey() != expected->key) {
      matches = false;
      output_ << "expected key " << expected->key << " but got key " << actual->GetKey()
              << std::endl;
    }
    matches &= RecursiveMatch(actual->GetLowerNode().get(), expected->lower.get());
    matches &= RecursiveMatch(actual->GetHigherNode().get(), expected->higher.get());
    return matches;
  }

  const std::unique_ptr<const MatcherNode<T>> nodes_;
  mutable std::ostringstream output_;
};

template <class T>
StructureMatcher<T> MatchesStructure(std::unique_ptr<const MatcherNode<T>> expected = nullptr) {
  return StructureMatcher<T>(std::move(expected));
}

}  // namespace cat_dist::tests

#endif  // CAT_DIST_TESTS_HELPERS_H_
