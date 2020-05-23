/*      CWPack/goodies - cwpack_dump.c   */
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
#include <unistd.h>
#include <string.h>
#include "basic_contexts.h"

char tabString[21] = "                     ";

#define NEW_LINE {printf ("\n%6x  ",(unsigned)(context->current - context->start)); for (ti=0; ti<tabLevel; ti++) printf ("%s",tabString);}
#define CHECK_NEW_LINE if(*tabString) NEW_LINE else if (i) printf(" ")

/*******************************   DUMP NEXT ITEM   **********************************/

static void dump_as_hex(const void* area, long length)
{
    unsigned int i;
    unsigned char c;
    for (i=0; i < length; i++)
    {
        c = ((unsigned char*)area)[i];
        printf("%02x",c);
    }
}


static void dump_next_item( cw_unpack_context* context, int tabLevel)
{
    
    long long dim =99;
    int i,ti;
    double d;
    struct tm tm;
    char    s[128];

    if (!tabLevel) NEW_LINE;
    cw_unpack_next (context);
    if (context->return_code) return;

    switch (context->item.type)
    {
        case CWP_ITEM_NIL:
            printf("null");
            break;
            
        case CWP_ITEM_BOOLEAN:
            if (context->item.as.boolean)
                printf("true");
            else
                printf("false");
            break;
            
        case CWP_ITEM_POSITIVE_INTEGER:
            printf("%llu", context->item.as.u64);
            break;
            
        case CWP_ITEM_NEGATIVE_INTEGER:
            printf("%lld", context->item.as.i64);
            break;
            
        case CWP_ITEM_FLOAT:
            context->item.as.long_real = (double)context->item.as.real;
            
        case CWP_ITEM_DOUBLE:
            printf ("%g", context->item.as.long_real);
            break;
            
        case CWP_ITEM_STR: {
            printf("\"");
            
            for (i=0; i < (int)context->item.as.str.length; i++)
            {
                unsigned char c = ((unsigned char*)(context->item.as.str.start))[i];
                switch (c)
                {
                    case '"':  printf("\\\""); break;
                    case '\\': printf("\\\\"); break;
                    case '\b': printf("\\b"); break;
                    case '\f': printf("\\f"); break;
                    case '\n': printf("\\n"); break;
                    case '\r': printf("\\r"); break;
                    case '\t': printf("\\t"); break;

                    default:
                        if (c < ' ')
                            printf("\\u%04x",c);
                        else
                            printf("%c",c);
                        break;
                }
            }

            printf("\"");
            break;}
            
        case CWP_ITEM_BIN:
            printf("(");
            dump_as_hex (context->item.as.bin.start, context->item.as.bin.length);
            printf(")");
            break;
            
        case CWP_ITEM_ARRAY:
            printf("[");
            dim = context->item.as.array.size;
            tabLevel++;
            for (i = 0; i < dim; i++)
            {
                CHECK_NEW_LINE;
                dump_next_item(context,tabLevel);
            }
            tabLevel--;
            if(*tabString) NEW_LINE;
            printf("]");
            break;
            
        case CWP_ITEM_MAP:
            printf("{");
            dim = context->item.as.map.size;
            tabLevel++;
            for (i = 0; i < dim; i++)
            {
                CHECK_NEW_LINE;
                dump_next_item(context,tabLevel);
                printf(": ");
                dump_next_item(context,tabLevel);
            }
            tabLevel--;
            if(*tabString) NEW_LINE;
            printf("}");
            break;
            
        case CWP_ITEM_TIMESTAMP:
            printf("'");
            gmtime_r(&context->item.as.time.tv_sec,&tm);
            strftime(s,128,"%F %T", &tm);
            printf("%s",s);
            if (context->item.as.time.tv_nsec)
            {
                d = (double)(context->item.as.time.tv_nsec) / 1000000000;
                sprintf(s,"%f",d);
                printf("%s",s+1);
            }
            printf("'");
            break;
            
        default:
            if (CWP_ITEM_MIN_RESERVED_EXT <= context->item.type && context->item.type <= CWP_ITEM_MAX_USER_EXT)
            {
                printf("(%d,",context->item.type);
                dump_as_hex (context->item.as.ext.start, context->item.as.ext.length);
                printf(")");
            }
            else
                printf("????? type = %d", context->item.type );
            break;
    }
}


/*******************************   M A I N   ******************************/


int main(int argc, const char * argv[])
{
    int i;
    int t = 0;
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i],"-t") && (i++ < argc))
        {
                t = *argv[i] - '0';
                if (strlen(argv[i]) != 1 || t < 1 || t > 9) {
                    printf("Tab size must be between 1 and 9\n");
                    exit(0);
                }
        }
        else if (!strcmp(argv[i],"-v") || !strcmp(argv[i],"--version"))
        {
            printf("cwpack_dump version = 1.0\n");
            exit(0);
        }
        else
        {
            printf("cwpack_dump [-t 9] [-v] [-h]\n");
            printf("-t 9 Tab size\n");
            printf("-v   Version\n");
            printf("-h   Help\n");
            printf("\nIf Tab size isn't given, structures are written on a single line\n");
            printf("\nInput is taken from stdin and output is written to stdout\n");
            exit(0);
        }
    }
    tabString[t] = 0;

    file_unpack_context fuc;
    cw_unpack_context *context = (cw_unpack_context*)&fuc;
    
    init_file_unpack_context (&fuc, 4096, STDIN_FILENO);
    file_unpack_context_set_barrier (&fuc); /* keep whole file in memory buffer to simplify offset calculation */

    while (!context->return_code)
    {
        dump_next_item(context,0);
    }
    printf("\n");
    if (context->return_code != CWP_RC_END_OF_INPUT)
        printf("\nERROR RC = %d\n",context->return_code);

    terminate_file_unpack_context (&fuc);
}

