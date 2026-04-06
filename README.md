# Categorical Distribution C++ Library

This library provides `cat_dist::CategoricalDistribution` (`#include "cat_dist/categorical_distribution.h"`), which represents a categorical distribution.

## Motivation

This library is optimized for random sampling of a finite set of objects (e.g., _A_, _B_, _C_) while being able to efficiently update their relative weights dynamically. `cat_dist::CategoricalDistribution` is implemented so that both updating and sampling are **_O(log n)_**.

There are two simple ways to handle sampling and updating _without_ this library, but each approach is inefficient for one type of operation.

1. **Vector of individual weights**: Assign each category to a vector index, and store the category's weight in its respective position.
   - _Updating_: Directly update the weight in the vector [**_O(1)_**]
   - _Sampling_: Select a random number between 0 and the total weight, then iterate the weight vector to choose a category  [**_O(n)_**]
2. **Vector of cumulative weights**: Same as the approach above, but each vector item is the cumulative sum from the beginning of the vector.
   - _Updating_: Update the weight in the vector, plus all weights that follow it [**_O(n)_**]
   - _Sampling_: Select a random number between 0 and the total weight, and perform a binary search on the vector [**_O(log n)_**]

Both of these are **_O(n)_** for one operation, and therefore might not be suitable for frequent updates and sampling. Updating the vectors for newly-encountered values also adds complexity.

## Uses

For all use-cases, sampling a random value from `cat_dist::CategoricalDistribution` is done by multiplying a uniform value in the range [0,1) by the total weight of the distribution, and using that to locate a value.

```c++
cat_dist::CategoricalDistribution<std::string> distribution;
// ... fill in the distribution elsewhere ...

const double random = ...  // uniform value from [0,1)
const int target_weight = categories.GetTotalWeight()*random;
const std::optional<std::string> chosen = categories.LocateByWeight(target_weight);
```

This approach is optimal for all of the examples below. (Note that permuting requires decrementing the weight for each selected category.)

### Random Sampling with Weights

See `examples/random_weighted.cc`.

1. Category weights are modified independently of sampling.
2. Using `double` or `int` for weights is fine.

### Generating Permutations

See `examples/random_permutation.cc`.

1. Category weights are set up front based on the required counts.
2. The weight of the chosen category is decremented by 1.
3. Avoid floating-point weights (use `int`) to avoid precision issues when decrementing weights.

### Data Fitting

See `examples/data_fitting.cc`.

1. `cat_dist::CategoricalDistribution` can be used as a Dirichlet prior for a categorical distribution.
   1. _[optional]_ Initialize known categories with a baseline weight as a prior.
   2. Increment the weight by 1 each time a category is seen.
2. The category weights divided by the total distribution weight yields the maximum likelihood estimate for the categorical distribution fitted to the data.
3. Rather than trying to normalize, you can directly sample from the distribution using the approach described above.

# Additional Info

## Concept

`cat_dist::CategoricalDistribution` is just a self-balancing binary search tree with additional structure for searching the tree based on node weights.

- In addition to the usual state stored in search-tree nodes, each node keeps track of the sum of all weights of its children. These totals need to be updated whenever a node weight or its subtree structure changes.
- Seaching by weight uses an alternative binary search on the tree. This is similar to the key-centric binary search:
  1. Check if the target is within the size of the lower node. If so, return early with the result of a recursive call to the lower node.
  2. Check if the target is within the size of the node itself. If so, return the node's key.
  3. Return the result of a recursive call to the higher node.

For these reasons, this approach needs to be built into the logic of the search tree. `cat_dist::AutoBalancedTree` generalizes the balancing and binary-search logic in a way that the logic above can be integrated while still leaving `cat_dist::AutoBalancedTree` generic enough for regular binary search trees.

## History

1. **2014**: I first came up with this idea and implemented it in C++ for an unreleased project for fitting Bayesian networks to empirical data.
3. **2021**: I implemented the same concept for the (still experimental) [Zeolite Programming Language](https://github.com/ta0kira/zeolite) using a generic AVL binary search tree that was also used for a map implementation.
4. **2026**: I started this project by converting the Zeolite implementation (by hand) to C++, close to 1:1 except where language features differed.
