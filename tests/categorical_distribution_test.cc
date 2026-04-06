#include "cat_dist/categorical_distribution.h"

#include <catch2/catch_test_macros.hpp>

#include "helpers.h"

using namespace cat_dist;
using namespace cat_dist::tests;

using TestDistribution = CategoricalDistribution<std::string, int>;

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

    distribution.SetWeight("3", 7);
    distribution.SetWeight("4", 0);
    CHECK(distribution.GetTotalWeight() == 33);
    CHECK(distribution.GetUniqueCount() == 4);
  }

  SECTION("locate") {
    distribution.SetWeight("1", 5);
    distribution.SetWeight("2", 5);
    distribution.SetWeight("3", 5);
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
}
