/*      CWPack/goodies - basic_contexts.h   */
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

#ifndef basic_contexts_h
#define basic_contexts_h

#include <stdio.h>
#include "cwpack.h"


/*****************************************  DYNAMIC MEMORY PACK CONTEXT  ************************/

typedef struct
{
    cw_pack_context pc;
} dynamic_memory_pack_context;


void init_dynamic_memory_pack_context (dynamic_memory_pack_context* dmpc, unsigned long initial_buffer_length);

void free_dynamic_memory_pack_context(dynamic_memory_pack_context* dmpc);



/*****************************************  STREAM PACK CONTEXT  ********************************/

typedef struct
{
    cw_pack_context pc;
    FILE*           file;
} stream_pack_context;


void init_stream_pack_context (stream_pack_context* spc, unsigned long initial_buffer_length, FILE* file);

void terminate_stream_pack_context(stream_pack_context* spc);



/*****************************************  STREAM UNPACK CONTEXT  ******************************/

typedef struct
{
    cw_unpack_context   uc;
    unsigned long       buffer_length;
    FILE*               file;
} stream_unpack_context;


void init_stream_unpack_context (stream_unpack_context* suc, unsigned long initial_buffer_length, FILE* file);

void terminate_stream_unpack_context(stream_unpack_context* suc);



/*****************************************  FILE PACK CONTEXT  ********************************/

typedef struct
{
    cw_pack_context pc;
    int             fileDescriptor;
    uint8_t         *barrier;
} file_pack_context;


void init_file_pack_context (file_pack_context* spc, unsigned long initial_buffer_length, int fileDescriptor);

void file_pack_context_set_barrier (file_pack_context* spc);
void file_pack_context_release_barrier (file_pack_context* spc);

void terminate_file_pack_context(file_pack_context* spc);



/*****************************************  FILE UNPACK CONTEXT  ******************************/

typedef struct
{
    cw_unpack_context   uc;
    unsigned long       buffer_length;
    int                 fileDescriptor;
    uint8_t             *barrier;
} file_unpack_context;


void init_file_unpack_context (file_unpack_context* suc, unsigned long initial_buffer_length, int fileDescriptor);

void file_unpack_context_set_barrier (file_unpack_context* suc);
void file_unpack_context_rescan_from_barrier (file_unpack_context* suc);
void file_unpack_context_release_barrier (file_unpack_context* suc);

void terminate_file_unpack_context(file_unpack_context* suc);



/*****************************************  E P I L O G U E  **********************************/


#endif /* basic_contexts_h */
