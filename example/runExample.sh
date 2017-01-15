clang -ansi -I ../src/ -o json2cwpack2json *.c ../src/*.c
./json2cwpack2json test1.json
diff -a test1.json test1.json.msgpack.json
rm -f *.o test1.json.msgpack.json json2cwpack2json
