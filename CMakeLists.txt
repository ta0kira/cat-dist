add_library(cat_dist INTERFACE)

target_compile_features(cat_dist INTERFACE cxx_std_17)

target_include_directories(cat_dist INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_sources(cat_dist INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}/include/cat_dist/auto_balanced_tree.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/cat_dist/categorical_distribution.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/cat_dist/node_traits.h"
)
