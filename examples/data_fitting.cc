#include <iostream>
#include <string>

#include "cat_dist/categorical_distribution.h"

int main() {
  cat_dist::CategoricalDistribution<char, int> categories;

  std::cerr << "Reading data from stdin..." << std::endl;
  std::string input;
  while (std::cin >> input) {
    for (char category : input) {
      categories.AdjustWeight(category, 1);
    }
  }

  for (char category : categories.GetUniqueCategories()) {
    std::cout << category << ": " << categories.GetWeight(category) << std::endl;
  }
}
