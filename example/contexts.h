/*      CWPack/example - contexts.h   */
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

#ifndef contexts_h
#define contexts_h

#include <stdio.h>
#include "cwpack.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    
    /*****************************************  MEMORY PACK CONTEXT  ********************************/
    
    cw_pack_context* new_memory_pack_context (unsigned long initial_buffer_length);
    
    void free_memory_pack_context(cw_pack_context* cpc);
    
    
    
    /*****************************************  FILE PACK CONTEXT  **********************************/
    
    cw_pack_context* new_file_pack_context (unsigned long initial_buffer_length, FILE* file);
    
    int flush_file_pack_context(cw_pack_context* cpc);
    void free_file_pack_context(cw_pack_context* cpc);
    
    
    
    /*****************************************  FILE UNPACK CONTEXT  ********************************/
    
    cw_unpack_context* new_file_unpack_context (unsigned long initial_buffer_length, FILE* file);
    
    void free_file_unpack_context(cw_unpack_context* cuc);
    
    
    /*****************************************  E P I L O G U E  ************************************/
    
#ifdef __cplusplus
}
#endif

#endif /* contexts_h */
