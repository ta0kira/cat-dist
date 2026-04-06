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

  cat_dist::CategoricalDistribution<std::string, double> categories;

  // Relative weights of each category. The scale doesn't matter for this example.
  categories.SetWeight("0", 10.1);
  categories.SetWeight("1", 2.3);

  for (int i = 0; i < 32; ++i) {
    const int target_weight = categories.GetTotalWeight()*distribution(generator);
    const std::optional<std::string> chosen = categories.LocateByWeight(target_weight);
    // Should never be empty if `target_weight` is in the valid range.
    assert(chosen);
    std::cout << *chosen;
  }
  std::cout << std::endl;
}
