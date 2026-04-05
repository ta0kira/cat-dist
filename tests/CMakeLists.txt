cmake_minimum_required(VERSION 3.10)
project(CatDistTests VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/.." cat_dist EXCLUDE_FROM_ALL)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Catch2" EXCLUDE_FROM_ALL)

add_executable(tests
  auto_balanced_tree_test.cc
  categorical_distribution_test.cc)

target_include_directories(tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_options(tests PRIVATE -Wall -g)

target_link_libraries(tests cat_dist Catch2::Catch2WithMain)
