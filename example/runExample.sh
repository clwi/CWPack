clang -ansi -I ../src/ -I ../goodies/basic-contexts/ *.c ../src/*.c ../goodies/basic-contexts/*.c  -o json2cwpack2json
./json2cwpack2json test1.json
diff -a test1.json test1.json.msgpack.json
rm -f *.o json2cwpack2json
