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

#include <cassert>
#include <ctime>
#include <iostream>
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

  // Show the items in order.
  for (int i = 0; i < categories.GetTotalWeight(); ++i) {
    const std::optional<std::string> chosen = categories.LocateByWeight(i);
    // Should never be empty if `i` is in the valid range.
    assert(chosen);
    std::cout << *chosen;
  }
  std::cout << std::endl;

  // Generate 3 permutations.
  for (int i = 0; i < 3; ++i) {
    // Permutation generation is destructive, so we need to make a copy.
    auto copy = categories.DeepCopy();
    while (copy.GetTotalWeight() > 0) {
      const int target_weight = copy.GetTotalWeight() * distribution(generator);
      // Use a -1 adjustment to decrement the weight of the chosen category.
      const std::optional<std::string> chosen = copy.LocateByWeight(target_weight, -1);
      // Should never be empty if `target_weight` is in the valid range.
      assert(chosen);
      std::cout << *chosen;
    }
    std::cout << std::endl;
  }
}
