/*      CWPack/example - json2cwpack2json.c   */
/*
 The MIT License (MIT)

 Copyright (c) 2017 Claes Wihlborg

 Permission is hereby granted, free of charge, to any person obtaining a copy of this
 software and associated documentation files (the "Software"), to deal in the Software
 without restriction, including without limitation the rights to use, copy, modify,
 merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <string.h>
#include <stdio.h>

#include "item.h"
#include "basic_contexts.h"


int main(int argc, const char * argv[])
{
    if (argc < 2)
    {
        printf("Call: json2cwpack2json <file>\n");
        return 1;
    }
    char filename[200];
    strcpy(filename, argv[1]);

    FILE* jsonFileIn;
    FILE* jsonFileOut;
    FILE* cwpackFileIn;
    FILE* cwpackFileOut;

    jsonFileIn = fopen (filename, "r");
    item_root* root = jsonFile2item3 (jsonFileIn);
    fclose(jsonFileIn);

    strcat (filename, ".msgpack");
    cwpackFileOut = fopen (filename, "w");
    item32cwpackFile (cwpackFileOut, root);
    fclose(cwpackFileOut);
    freeItem3(root);

    cwpackFileIn = fopen (filename, "r");
    root = cwpackFile2item3 (cwpackFileIn);
    fclose(cwpackFileIn);

    strcat (filename, ".json");
    jsonFileOut = fopen (filename, "w");
    item32JsonFile (jsonFileOut, root);
    fclose(jsonFileOut);
    freeItem3(root);

    return 0;
}
