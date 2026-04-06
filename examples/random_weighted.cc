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

  // Relative weights of each category. The scale doesn't matter for this example.
  categories.SetWeight("0", 10);
  categories.SetWeight("1", 2);

  for (int i = 0; i < 32; ++i) {
    const int target_weight = categories.GetTotalWeight()*distribution(generator);
    const std::optional<std::string> chosen = categories.LocateByWeight(target_weight);
    // Should never be empty if `target_weight` is in the valid range.
    assert(chosen);
    std::cout << *chosen;
  }
  std::cout << std::endl;
}
