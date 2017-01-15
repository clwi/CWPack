clang -I ../src/ -o cwpackModuleTest cwpack_module_test.c ../src/cwpack.c
./cwpackModuleTest
rm -f *.o cwpackModuleTest
