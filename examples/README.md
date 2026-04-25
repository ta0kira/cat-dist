```shell
examples$ mkdir build && cd build
examples/build$ cmake ..
examples/build$ make

examples/build$ ./random_weighted
00000000000000000100000001000100

examples/build$ ./random_permutation
0000011222222222
2202010022022122
2212102202020022
2220122120002220

examples/build$ ./random_permutation | ./data_fitting
Reading data from stdin...
0: 20
1: 8
2: 36
```
