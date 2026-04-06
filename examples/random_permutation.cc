#include <cassert>
#include <ctime>
#include <optional>
#include <random>
#include <string>

#include "cat_dist/categorical_distribution.h"

int main() {
  std::default_random_engine generator(std::time(nullptr));
  std::uniform_real_distribution<double> distribution(0.0, 1.0);

  cat_dist::CategoricalDistribution<std::string, int> categories;

  // Sample from bins without replacement.
  categories.SetWeight("0", 5);
  categories.SetWeight("1", 2);
  categories.SetWeight("2", 9);

  while (categories.GetTotalWeight() > 0) {
    const int target_weight = categories.GetTotalWeight()*distribution(generator);
    // Use a -1 adjustment to decrement the weight of the chosen category.
    const std::optional<std::string> chosen = categories.LocateByWeight(target_weight, -1);
    // Should never be empty if `target_weight` is in the valid range.
    assert(chosen);
    std::cout << *chosen;
  }
  std::cout << std::endl;
}
