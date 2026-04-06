cmake_minimum_required(VERSION 3.10)
project(CatDistExamples VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/.." cat_dist EXCLUDE_FROM_ALL)

add_executable(data_fitting data_fitting.cc)
target_link_libraries(data_fitting cat_dist)

add_executable(random_weighted random_weighted.cc)
target_link_libraries(random_weighted cat_dist)

add_executable(random_permutation random_permutation.cc)
target_link_libraries(random_permutation cat_dist)
