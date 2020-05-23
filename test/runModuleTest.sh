clang -O3 -I ../src/ -I ../goodies/utils/ -o cwpackModuleTest cwpack_module_test.c ../src/cwpack.c ../goodies/utils/cwpack_utils.c
./cwpackModuleTest
rm -f *.o cwpackModuleTest
