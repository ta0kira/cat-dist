```shell
examples$ mkdir build && cd build
examples/build$ cmake ..
examples/build$ make

examples/build$ ./random_weighted
00000000000000000100000001000100

examples/build$ ./random_permutation
0000011222222222
0210022212022202

examples/build$ ./random_permutation | ./data_fitting
Reading data from stdin...
0: 10
1: 4
2: 18
```
