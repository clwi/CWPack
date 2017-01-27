/*      CWPack/example - basic_contexts.c   */
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

#include "basic_contexts.h"




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


void init_dynamic_memory_pack_context (dynamic_memory_pack_context* dmpc, unsigned long initial_buffer_length)
{
    unsigned long buffer_length = (initial_buffer_length > 0 ? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        dmpc->pc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }
    
    cw_pack_context_init((cw_pack_context*)dmpc, buffer, buffer_length, &handle_memory_pack_overflow);
}


void free_dynamic_memory_pack_context(dynamic_memory_pack_context* dmpc)
{
    if (dmpc->pc.return_code != CWP_RC_MALLOC_ERROR)
        free(dmpc->pc.start);
}



/*****************************************  FILE PACK CONTEXT  **********************************/



static int handle_stream_pack_overflow(struct cw_pack_context* pc, unsigned long more)
{
    int rc = flush_stream_pack_context((stream_pack_context*)pc);
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


void init_stream_pack_context (stream_pack_context* spc, unsigned long initial_buffer_length, FILE* file)
{
    unsigned long buffer_length = (initial_buffer_length > 0 ? initial_buffer_length : 4096);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        spc->pc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }
    spc->file = file;
    
    cw_pack_context_init((cw_pack_context*)spc, buffer, buffer_length, &handle_stream_pack_overflow);
}


int flush_stream_pack_context(stream_pack_context* spc)
{
    unsigned long contains = (unsigned long)(spc->pc.current - spc->pc.start);
    if (contains)
    {
        unsigned long rc = fwrite(spc->pc.start, contains, 1, spc->file);
        if (rc != 1)
        {
            spc->pc.err_no = ferror(spc->file);
            return CWP_RC_ERROR_IN_HANDLER;
        }
    }
    return CWP_RC_OK;
}

void free_stream_pack_context(stream_pack_context* spc)
{
    if (spc->pc.return_code != CWP_RC_MALLOC_ERROR)
        free(spc->pc.start);
}





/*****************************************  FILE UNPACK CONTEXT  ********************************/


static int handle_stream_unpack_underflow(struct cw_unpack_context* uc, unsigned long more)
{
    stream_unpack_context* suc = (stream_unpack_context*)uc;
    unsigned long remains = (unsigned long)(uc->end - uc->current);
    if (remains)
    {
        memmove (uc->start, uc->current, remains);
    }
    
    if (suc->buffer_length < more)
    {
        while (suc->buffer_length < more)
            suc->buffer_length = 2 * suc->buffer_length;
        
        void *new_buffer = realloc (uc->start, suc->buffer_length);
        if (!new_buffer)
            return CWP_RC_BUFFER_UNDERFLOW;
        
        uc->start = (uint8_t*)new_buffer;
    }
    uc->current = uc->start;
    uc->end = uc->start + remains;
    unsigned long l = fread(uc->end, 1, suc->buffer_length - remains, suc->file);
    if (!l)
    {
        if (feof(suc->file))
            return CWP_RC_END_OF_INPUT;
        suc->uc.err_no = ferror(suc->file);
        return CWP_RC_ERROR_IN_HANDLER;
    }
    
    uc->end += l;

    return CWP_RC_OK;
}


void init_stream_unpack_context (stream_unpack_context* suc, unsigned long initial_buffer_length, FILE* file)
{
    unsigned long buffer_length = (initial_buffer_length? initial_buffer_length : 1024);
    void *buffer = malloc (buffer_length);
    if (!buffer)
    {
        suc->uc.return_code = CWP_RC_MALLOC_ERROR;
        return;
    }
    suc->file = file;
    suc->buffer_length = buffer_length;
    
    cw_unpack_context_init((cw_unpack_context*)suc, buffer, 0, &handle_stream_unpack_underflow);
}


void free_stream_unpack_context(stream_unpack_context* suc)
{
    if (suc->uc.return_code != CWP_RC_MALLOC_ERROR)
        free(suc->uc.start);
}


