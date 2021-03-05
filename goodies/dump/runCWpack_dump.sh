clang -ansi -I ../../src/ -I ../basic-contexts/ -I ../numeric-extensions/ -o cwpack_dump *.c ../../src/cwpack.c ../basic-contexts/*.c ../numeric-extensions/numeric_extensions.c
./cwpack_dump < testdump.msgpack
./cwpack_dump -t 4 < testdump.msgpack
