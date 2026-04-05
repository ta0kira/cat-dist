#ifndef CAT_DIST_TESTS_HELPERS_H_
#define CAT_DIST_TESTS_HELPERS_H_

#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_templated.hpp>

namespace cat_dist::tests {

class IsBalanced : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit IsBalanced(std::string note = "") : note_(std::move(note)) {}

  template<class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.CheckBalance(&output_);
  }

  std::string describe() const override {
    return note_ + "should be balanced\n" + output_.str();
  }

 private:
  const std::string note_;
  mutable std::ostringstream output_;
};

class IsOrderedCorrectly : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit IsOrderedCorrectly(std::string note = "") : note_(std::move(note)) {}

  template<class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.CheckOrder(&output_);
  }

  std::string describe() const override {
    return note_ + "should be ordered correctly\n" + output_.str();
  }

 private:
  const std::string note_;
  mutable std::ostringstream output_;
};

class HasCorrectCount : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit HasCorrectCount(std::string note = "") : note_(std::move(note)) {}

  template<class N>
  bool match(const AutoBalancedTree<N>& tree) const {
    return tree.ValidateCount(&output_);
  }

  std::string describe() const override {
    return note_ + "should have correct node count\n" + output_.str();
  }

 private:
  const std::string note_;
  mutable std::ostringstream output_;
};

template<class T>
class CheckNodeValueMatches : public Catch::Matchers::MatcherGenericBase {
 public:
  explicit CheckNodeValueMatches(const T& expected, std::string note = "") : note_(std::move(note)), expected_(expected) {}

  template<class N>
  bool match(const N* node) const {
    return node ? (node->GetValue() == expected_) : false;
  }

  std::string describe() const override {
    return note_ + "matches " + std::to_string(expected_);
  }

 private:
  const std::string note_;
  const T& expected_;
};

template<class T>
CheckNodeValueMatches<T> NodeValueMatches(const T& expected, std::string note = "") {
  return CheckNodeValueMatches<T>(expected, std::move(note));
}

}  // namespace cat_dist::tests

#endif  // CAT_DIST_TESTS_HELPERS_H_
