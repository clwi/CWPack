clang -O3 -flto -I ../src/ -I ../goodies/basic-contexts/ -I ../../cmp-master/ -I ../../mpack/src/mpack/ cwpack_performance_test.c ../src/cwpack.c ../goodies/basic-contexts/basic_contexts.c ../../cmp-master/cmp.c ../../mpack/src/mpack/*.c -o cwpackPerformanceTest

./cwpackPerformanceTest
rm -f *.o cwpackPerformanceTest
