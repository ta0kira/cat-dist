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

#include "cat_dist/categorical_distribution.h"

#include <catch2/catch_test_macros.hpp>

#include <vector>

#include "helpers.h"

using namespace cat_dist;
using namespace cat_dist::tests;

namespace {

using TestDistribution = CategoricalDistribution<std::string, int>;

template<class C>
std::vector<typename C::value_type> ToVector(const C& container) {
  return std::vector<typename C::value_type>(container.begin(), container.end());
}

}  // namespace

TEST_CASE("CategoricalDistribution") {
  TestDistribution distribution;

  SECTION("defaults") {
    CHECK(distribution.GetTotalWeight() == 0);
    CHECK(distribution.GetUniqueCount() == 0);
  }

  SECTION("set and get weight") {
    CHECK(distribution.GetWeight("1") == 0);

    distribution.SetWeight("1", 5);
    CHECK(distribution.GetWeight("1") == 5);
    CHECK(distribution.GetTotalWeight() == 5);
    CHECK(distribution.GetUniqueCount() == 1);

    distribution.SetWeight("1", 0);
    CHECK(distribution.GetWeight("1") == 0);
    CHECK(distribution.GetTotalWeight() == 0);
    CHECK(distribution.GetUniqueCount() == 0);

    distribution.SetWeight("1", -1);
    CHECK(distribution.GetWeight("1") == 0);
    CHECK(distribution.GetTotalWeight() == 0);
    CHECK(distribution.GetUniqueCount() == 0);
  }

  SECTION("update weights") {
    distribution.SetWeight("1", 5);
    distribution.SetWeight("2", 15);
    distribution.SetWeight("3", 2);
    distribution.SetWeight("4", 9);
    distribution.SetWeight("5", 6);
    CHECK(distribution.GetTotalWeight() == 37);
    CHECK(distribution.GetUniqueCount() == 5);

    distribution.SetWeight("3", 6);
    distribution.AdjustWeight("3", 1);
    distribution.SetWeight("4", 0);
    CHECK(distribution.GetTotalWeight() == 33);
    CHECK(distribution.GetUniqueCount() == 4);
  }

  SECTION("get unique") {
    distribution.SetWeight("1", 5);
    distribution.SetWeight("2", 15);
    distribution.SetWeight("3", 2);
    CHECK(ToVector(distribution.GetUniqueCategories()) == std::vector<std::string>{ "1", "2", "3" });
  }

  SECTION("export") {
    distribution.SetWeight("1", 5);
    distribution.SetWeight("2", 15);
    distribution.SetWeight("3", 2);
    std::map<std::string, int> exported = distribution.ExportWeights();
    CHECK((signed) exported.size() == distribution.GetUniqueCount());
    CHECK(exported["1"] == 5);
    CHECK(exported["2"] == 15);
    CHECK(exported["3"] == 2);
  }

  SECTION("locate") {
    distribution.SetWeight("1", 5);
    distribution.SetWeight("2", 5);
    distribution.SetWeight("3", 5);
    CHECK(distribution.LocateByWeight(-1) == std::nullopt);
    CHECK_THAT(distribution.LocateByWeight(0), OptionalMatches("1"));
    CHECK_THAT(distribution.LocateByWeight(3), OptionalMatches("1"));
    CHECK_THAT(distribution.LocateByWeight(5), OptionalMatches("2"));
    CHECK_THAT(distribution.LocateByWeight(7), OptionalMatches("2"));
    CHECK_THAT(distribution.LocateByWeight(10), OptionalMatches("3"));
    CHECK_THAT(distribution.LocateByWeight(12), OptionalMatches("3"));
    CHECK(distribution.LocateByWeight(15) == std::nullopt);
  }

  SECTION("locate with adjustment") {
    distribution.SetWeight("1", 3);
    distribution.SetWeight("2", 3);
    distribution.SetWeight("3", 3);
    CHECK_THAT(distribution.LocateByWeight(0, -1), OptionalMatches("1"));
    CHECK(distribution.GetWeight("1") == 2);
    CHECK(distribution.GetTotalWeight() == 8);
    CHECK(distribution.GetUniqueCount() == 3);
    CHECK_THAT(distribution.LocateByWeight(0, -1), OptionalMatches("1"));
    CHECK_THAT(distribution.LocateByWeight(0, -1), OptionalMatches("1"));
    CHECK_THAT(distribution.LocateByWeight(0, -1), OptionalMatches("2"));
    CHECK(distribution.GetTotalWeight() == 5);
    CHECK(distribution.GetUniqueCount() == 2);
  }

  SECTION("deep copy") {
    distribution.SetWeight("1", 1);
    distribution.SetWeight("2", 2);
    distribution.SetWeight("3", 3);
    auto distribution2 = distribution.DeepCopy();
    CHECK(distribution.GetWeight("1") == 1);
    CHECK(distribution.GetWeight("2") == 2);
    CHECK(distribution.GetWeight("3") == 3);
    CHECK(distribution2.GetWeight("1") == 1);
    CHECK(distribution2.GetWeight("2") == 2);
    CHECK(distribution2.GetWeight("3") == 3);
    distribution.ClearAll();
    CHECK(distribution2.GetWeight("1") == 1);
    CHECK(distribution2.GetWeight("2") == 2);
    CHECK(distribution2.GetWeight("3") == 3);

  }

  TestDistribution::TestVisitor::ValidateTree(distribution, [](const auto& tree) {
    CHECK_THAT(tree, IsBalanced());
    CHECK_THAT(tree, IsOrderedCorrectly());
    CHECK_THAT(tree, HasCorrectCount());
  });
}
