/*      CWPack/example - item.h   */
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


#ifndef item_h
#define item_h




    /**************  ITEMS  **********************/

    typedef enum {
        ITEM_MAP,
        ITEM_ARRAY,
        ITEM_NIL,
        ITEM_TRUE,
        ITEM_FALSE,
        ITEM_INTEGER,
        ITEM_REAL,
        ITEM_STRING
    } item_types;

    typedef struct {
        item_types     item_type;
    } item_root;

    typedef struct {
        item_types     item_type;
        int            count;       /* in maps every association counts for 2 */
        item_root*     items[];
    } item_container;

    typedef struct {
        item_types     item_type;
        long long      value;
    } item_integer;

    typedef struct {
        item_types     item_type;
        double         value;
    } item_real;

    typedef struct {
        item_types     item_type;
        char           string[];
    } item_string;


    void freeItem3 (item_root* root);



    /**************  ITEMS TO/FROM FILE  **********************/

    void item32JsonFile (FILE* file, item_root* item);

    item_root* jsonFile2item3 (FILE* file);

    void item32cwpackFile (FILE* file, item_root* item);

    item_root* cwpackFile2item3 (FILE* file);



#endif /* item_h */
