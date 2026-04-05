#ifndef CAT_DIST_TESTS_HELPERS_H_
#define CAT_DIST_TESTS_HELPERS_H_

#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace cat_dist::tests {

class IsBalanced : public Catch::Matchers::MatcherGenericBase {
 public:
  template<class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.CheckBalance(&output_);
  }

  std::string describe() const override {
    return std::string("should be balanced\n") + output_.str();
  }

 private:
  mutable std::ostringstream output_;
};

class HasCorrectCount : public Catch::Matchers::MatcherGenericBase {
 public:
  template<class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.ValidateCount(&output_);
  }

  std::string describe() const override {
    return std::string("should have correct node count\n") + output_.str();
  }

 private:
  mutable std::ostringstream output_;
};

template<class T>
class NodeValueMatches : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit NodeValueMatches(const T& expected) : expected_(expected) {}

  template<class N>
  bool match(const N* node) const {
    return node ? (node->GetValue() == expected_) : false;
  }

  std::string describe() const override {
    return std::string("matches ") + std::to_string(expected_);
  }

 private:
  const T& expected_;
};

}  // namespace cat_dist::tests

#endif  // CAT_DIST_TESTS_HELPERS_H_
