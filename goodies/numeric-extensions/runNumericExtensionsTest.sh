clang -ansi -I ../../src/ -o numericExtensionsTest *.c ../../src/cwpack.c
./numericExtensionsTest
rm -f *.o numericExtensionsTest
