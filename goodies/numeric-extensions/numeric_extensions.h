/*      CWPack/goodies - numeric_extensions.h   */
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

#ifndef numeric_extensions_h
#define numeric_extensions_h

#include "cwpack.h"



#define NUMEXT_ERROR_NOT_EXT        CWP_RC_TYPE_ERROR;
#define NUMEXT_ERROR_WRONG_LENGTH   CWP_RC_VALUE_ERROR;




    void cw_pack_ext_integer (cw_pack_context* pack_context, int8_t type, int64_t i);
    void cw_pack_ext_float (cw_pack_context* pack_context, int8_t type, float f);
    void cw_pack_ext_double (cw_pack_context* pack_context, int8_t type, double d);


    int64_t get_ext_integer (cw_unpack_context* unpack_context);
    float get_ext_float (cw_unpack_context* unpack_context);
    double get_ext_double (cw_unpack_context* unpack_context);



#endif /* numeric_extensions_h */
