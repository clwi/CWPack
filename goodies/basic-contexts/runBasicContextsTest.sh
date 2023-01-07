clang -ansi -I ../../src/ -I ../utils/ -o basicContextTest *.c ../../src/cwpack.c ../utils/cwpack_utils.c
./basicContextTest
rm -f *.o basicContextTest
