/*      CWPack/example - contexts.c   */
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


#include <stdlib.h>
#include <string.h>

#include "contexts.h"




/*****************************************  MEMORY PACK CONTEXT  ********************************/


static int handle_memory_pack_overflow(struct cw_pack_context* pc, unsigned long more)
{
    unsigned long contains = (unsigned long)(pc->current - pc->start);
    unsigned long tot_len = contains + more;
    unsigned long buffer_length = (unsigned long)(pc->end - pc->start);
    while (buffer_length < tot_len)
        buffer_length = 2 * buffer_length;
    void *new_buffer = realloc (pc->start, buffer_length);
    if (!new_buffer)
        return CWP_RC_BUFFER_OVERFLOW;
    
    pc->start = (uint8_t*)new_buffer;
    pc->current = pc->start + contains;
    pc->end = pc->start + buffer_length;
    return CWP_RC_OK;
}


cw_pack_context* new_memory_pack_context (unsigned long initial_buffer_length)
{
    cw_pack_context* pc = malloc(sizeof(cw_pack_context));
    if (!pc)
        return NULL;
    
    unsigned long buffer_length = (initial_buffer_length? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        free(pc);
        return NULL;
    }
    
    if (cw_pack_context_init(pc, buffer, buffer_length, &handle_memory_pack_overflow))
    {
        free(pc->start);
        free(pc);
        return NULL;
    }
    return pc;
}


void free_memory_pack_context(cw_pack_context* pc)
{
    free(pc->start);
    free(pc);
}



/*****************************************  FILE PACK CONTEXT  **********************************/


typedef struct  {
    cw_pack_context pc;
    FILE*           file;
} file_pack_context;


static int handle_file_pack_overflow(struct cw_pack_context* pc, unsigned long more)
{
    int rc = flush_file_pack_context(pc);
    if (rc != CWP_RC_OK)
        return rc;

    unsigned long buffer_length = (unsigned long)(pc->end - pc->start);
    if (buffer_length < more)
    {
        while (buffer_length < more)
            buffer_length = 2 * buffer_length;
        
        void *new_buffer = malloc (buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_OVERFLOW;
        
        free(pc->start);
        pc->start = (uint8_t*)new_buffer;
        pc->end = pc->start + buffer_length;
    }
    pc->current = pc->start;
    return CWP_RC_OK;
}


cw_pack_context* new_file_pack_context (unsigned long initial_buffer_length, FILE* file)
{
    file_pack_context* fpc = malloc(sizeof(file_pack_context));
    
    if (!fpc)
        return NULL;
    
    unsigned long buffer_length = (initial_buffer_length? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        free(fpc);
        return NULL;
    }
    fpc->file = file;
    
    cw_pack_context* pc = (cw_pack_context*)fpc;
    if (cw_pack_context_init(pc, buffer, buffer_length, &handle_file_pack_overflow))
    {
        free(pc->start);
        free(fpc);
        return NULL;
    }
    return pc;
}


int flush_file_pack_context(cw_pack_context* pc)
{
    unsigned long contains = (unsigned long)(pc->current - pc->start);
    if (contains)
    {
        file_pack_context* fpc = (file_pack_context*)pc;
        unsigned long rc = fwrite(pc->start, contains, 1, fpc->file);
        if (rc != 1)
        {
            pc->errno = ferror(fpc->file);
            return CWP_RC_ERROR_IN_HANDLER;
        }
    }
    return CWP_RC_OK;
}

void free_file_pack_context(cw_pack_context* pc)
{
    free(pc->start);
    free(pc);
}





/*****************************************  FILE UNPACK CONTEXT  ********************************/


typedef struct  {
    cw_unpack_context   uc;
    unsigned long       buffer_length;
    FILE*               file;
} file_unpack_context;


static int handle_file_unpack_underflow(struct cw_unpack_context* uc, unsigned long more)
{
    file_unpack_context* fuc = (file_unpack_context*)uc;
    unsigned long remains = (unsigned long)(uc->end - uc->current);
    if (remains)
    {
        memmove (uc->start, uc->current, remains);
    }
    
    if (fuc->buffer_length < more)
    {
        while (fuc->buffer_length < more)
            fuc->buffer_length = 2 * fuc->buffer_length;
        
        void *new_buffer = realloc (uc->start, fuc->buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_UNDERFLOW;
        
        uc->start = (uint8_t*)new_buffer;
    }
    uc->current = uc->start;
    uc->end = uc->start + remains;
    unsigned long l = fread(uc->end, 1, fuc->buffer_length - remains, fuc->file);
    if (!l)
    {
        if (feof(fuc->file))
            return CWP_RC_END_OF_INPUT;
        uc->errno = ferror(fuc->file);
        return CWP_RC_ERROR_IN_HANDLER;
    }
    
    uc->end += l;

    return CWP_RC_OK;
}


cw_unpack_context* new_file_unpack_context (unsigned long initial_buffer_length, FILE* file)
{
    file_unpack_context* fuc = malloc(sizeof(file_unpack_context));
    
    if (!fuc)
        return NULL;
    
    unsigned long buffer_length = (initial_buffer_length? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        free(fuc);
        return NULL;
    }
    fuc->file = file;
    fuc->buffer_length = buffer_length;
    
    cw_unpack_context* uc = (cw_unpack_context*)fuc;
    if (cw_unpack_context_init(uc, buffer, 0, &handle_file_unpack_underflow))
    {
        free(uc->start);
        free(fuc);
        return NULL;
    }
    
    return uc;
}


void free_file_unpack_context(cw_unpack_context* uc)
{
    free(uc->start);
    free(uc);
}


