clang -ansi -I ../../src/ -I ../basic-contexts/ -o cwpack_dump *.c ../../src/cwpack.c ../basic-contexts/*.c
./cwpack_dump < testdump.msgpack
./cwpack_dump -t 4 < testdump.msgpack
