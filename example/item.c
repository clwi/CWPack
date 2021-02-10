/*      CWPack/example - item.c   */
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


#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "item.h"
#include "basic_contexts.h"



void freeItem3 (item_root* root)
{
    int i;
    item_container* ic;
    switch (root->item_type)
    {
        case ITEM_MAP:
        case ITEM_ARRAY:
            ic = (item_container*)root;
            for (i=0; i < ic->count; i++)
            {
                freeItem3(ic->items[i]);
            }

        default:
            free(root);
    }
}

/**********************************  ITEM-TREE  to  JSON FILE  *********************/


static void item32jsonFile (FILE* file, item_root* item)
{
    char        tmp[30];
    int    i,j = 0;
    item_container* jc;
    char* cp;
    char c;
    unsigned u, ti;
    static unsigned tabs = 0;

#define NEW_LINE {fprintf (file, "\n"); for (ti=0; ti<tabs; ti++) fprintf (file, "\t");}

    switch (item->item_type)
    {
        case ITEM_MAP:
            jc = (item_container*)item;
            fprintf (file, "{");
            tabs++;
            for( i=0; i< jc->count; i++)
            {
                if (i) fprintf (file, ",");
                NEW_LINE
                item32jsonFile (file, jc->items[i++]);
                fprintf(file, ": ");
                item32jsonFile (file, jc->items[i]);
            }
            tabs--;
            NEW_LINE
            fprintf (file, "}");
            break;

        case ITEM_ARRAY:
            jc = (item_container*)item;
            fprintf (file, "[");
            tabs++;
            for( i=0; i< jc->count; i++)
            {
                if (i) fprintf (file, ",");
                NEW_LINE
                item32jsonFile (file, jc->items[i]);
            }
            tabs--;
            NEW_LINE
            fprintf(file, "]");
            break;

        case ITEM_NIL:
            fprintf (file, "null");
            break;

        case ITEM_TRUE:
            fprintf (file, "true");
            break;

        case ITEM_FALSE:
            fprintf (file, "false");
            break;

        case ITEM_INTEGER:
            fprintf (file, "%lld", ((item_integer*)item)->value);
            break;

        case ITEM_REAL:
            sprintf (tmp, "%-25.15g", ((item_real*)item)->value);
            for (i=0;i<30;i++)
            {
                if(tmp[i] == 0)
                    break;
                if(tmp[i] == ' ')
                {
                    tmp[i] = 0;
                    break;
                }
            }
            fprintf (file, "%s", tmp);
            break;

        case ITEM_STRING:
            fprintf (file, "\"");
            cp = ((item_string*)item)->string;
            while ((c = *cp++))
            {
                if (c & 0x80) { /* unicode, codepoint at most 16 bits */
                    if (c & 0x20)
                    {
                        u = c & 0x0f;
                        j = 2;
                    }
                    else
                    {
                        u = c & 0x1f;
                        j = 1;
                    }
                    for (i=0; i < j; i++) {
                        u = (u << 6) | (*cp++ & 0x3f);
                    }
                    fprintf (file, "\\u%04x", u);
                }
                else
                    switch (c) {
                        case '"':  fprintf (file, "\\\"");  break;
                        case '/':  fprintf (file, "\\/");   break;
                        case '\\': fprintf (file, "\\\\");  break;
                        case 0x08: fprintf (file, "\\b");   break; /* BS */
                        case 0x09: fprintf (file, "\\t");   break; /* HT */
                        case 0x0a: fprintf (file, "\\n");   break; /* LF */
                        case 0x0c: fprintf (file, "\\f");   break; /* FF */
                        case 0x0d: fprintf (file, "\\r");   break; /* CR */

                        default:
                            fprintf (file, "%c", c);
                            break;
                    }
            }
            fprintf (file, "\"");
            break;

        default:    break;
    }
}

void item32JsonFile (FILE* file, item_root* item)
{
    item32jsonFile (file, item);
    fprintf(file, "\n");
}



/**********************************  JSON FILE  to  ITEM-TREE  *********************/
/* correct JSON is assumed. No error checks whatsoever!!! */

#define scanSpace while (**ptr == ' ' || **ptr == '\n' || **ptr == '\t') (*ptr)++

#define  allocate_item(typ, typeMark, extra) \
malloc (sizeof(typ) + extra); \
result->item_type = typeMark


static item_container*  allocate_container(item_types type, int cnt)
{
    item_container* result = allocate_item(item_container, type, cnt*sizeof(void*));
    result->count = cnt;
    return result;
}


static item_root* jsonString2item3 (const char** ptr); /* prototype */

static item_container* pullMapPair (const char** ptr, int count)
{
    item_container* result;
    scanSpace;
    item_root*  it1 = jsonString2item3(ptr);
    scanSpace;
    (*ptr)++; /* ':' */
    scanSpace;
    item_root*  it2 = jsonString2item3(ptr);
    scanSpace;
    char c = *(*ptr)++;
    if (c == ',')
        result = pullMapPair (ptr, count + 2);
    else
        result = allocate_container (ITEM_MAP, count + 2);
    result->items[count] = it1;
    result->items[count+1] = it2;
    return result;
}

static item_container* pullArray (const char** ptr, int count)
{
    item_container* result;
    scanSpace;
    item_root* it = jsonString2item3 (ptr);
    scanSpace;
    char c = *(*ptr)++;
    if (c == ',')
        result = pullArray (ptr, count + 1);
    else
        result = allocate_container (ITEM_ARRAY, count + 1);
    result->items[count] = it;
    return result;
}

static item_string* pullString (const char** ptr, int length)
{
    item_string* result;
    char c = *(*ptr)++;
    if (c != '"')
    {
        int cl = 0;
        unsigned mask = 0;
        unsigned codepoint = (uint8_t)c;
        if (c == '\\')
        {
            c = *(*ptr)++;
            switch (c) {
                case '\\': codepoint = '\\'; break;
                case '/': codepoint = '/';  break;
                case '"': codepoint = '"';  break;
                case 'b': codepoint = '\b'; break;
                case 't': codepoint = '\t'; break;
                case 'n': codepoint = '\n'; break;
                case 'f': codepoint = '\f'; break;
                case 'r': codepoint = '\r'; break;
                case 'u':
                    sscanf(*ptr,"%4x",&codepoint);
                    *ptr += 4;
                    if (codepoint & 0xff80)
                    {
                        if (codepoint & 0xf800)
                        {
                            cl = 2;
                            mask = 0xe0;
                        }
                        else
                        {
                            cl = 1;
                            mask = 0xc0;
                        }
                    }

                default:
                    break;
            }
        }
        result = pullString (ptr, length + cl + 1);
        for (;cl > 0;cl--)
        {
            result->string[length+cl] = (codepoint & 0x3F) | 0x80;
            codepoint >>= 6;
        }
        result->string[length] = (char)(codepoint | mask);
    }
    else
    {
        result = allocate_item(item_string,ITEM_STRING,length + 1);
        result->string[length] = 0;
    }
    return result;
}

static item_root* jsonString2item3 (const char** ptr)
{
    scanSpace;
    item_root* result = NULL;

    char c = *(*ptr)++;
    switch (c) {
        case '{':
            scanSpace;
            if (**ptr == '}')
                result = (item_root*)allocate_container (ITEM_MAP, 0);
            else
                result = (item_root*)pullMapPair (ptr, 0);
            break;

        case '[':
            scanSpace;
            if (**ptr == ']')
                result = (item_root*)allocate_container (ITEM_ARRAY, 0);
            else
                result = (item_root*)pullArray (ptr, 0);
            break;

        case '"':   result = (item_root*)pullString (ptr, 0);break;
        case 'n':   result = allocate_item(item_root,ITEM_NIL,0); *ptr+=3;break;
        case 't':   result = allocate_item(item_root,ITEM_TRUE,0); *ptr+=3;break;
        case 'f':   result = allocate_item(item_root,ITEM_FALSE,0); *ptr+=4;break;

        case '-':
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
        {
            char buffer[100];
            int i = 0;
            bool real = false;;
            buffer[i++] = c;
            c = **ptr;
            while (c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E' || (('0' <= c) && (c <= '9')))
            {
                buffer[i++] = c;
                if (c == '.' || c == 'e' || c == 'E')
                    real = true;
                c = *++*ptr;
            }
            buffer[i++] = 0;
            if (real)
            {
                result = (item_root*)allocate_item(item_real,ITEM_REAL,0);
                double d;
                sscanf(buffer,"%lg",&d);
                ((item_real*)result)->value = d;
            }
            else
            {
                result = (item_root*)allocate_item(item_integer,ITEM_INTEGER,0);
                long d;
                sscanf(buffer,"%ld",&d);
                ((item_integer*)result)->value = d;
            }
        }
            break;

        default:
            break;
    }

    return result;
}


item_root* jsonFile2item3 (FILE* file)
{
    fseek (file, 0, SEEK_END);
    long length = ftell(file);
    char* buffer = malloc (length+1);

    fseek (file, 0l, SEEK_SET);
    fread (buffer, 1, length, file);
    buffer[length] = 0;
    const char* ptr = buffer;
    item_root* result = jsonString2item3(&ptr);
    free(buffer);
    return result;
}


/**********************************  ITEM-TREE  to  cwpack FILE  *********************/

static void item32packContext(cw_pack_context* pc, item_root* item)
{
    int    i;
    item_container* ic;
    char* cp;

    switch (item->item_type)
    {
        case ITEM_MAP:
            ic = (item_container*)item;
            cw_pack_map_size(pc, ic->count / 2);
            for( i=0; i< ic->count; i++)
                item32packContext (pc, ic->items[i]);
            break;

        case ITEM_ARRAY:
            ic = (item_container*)item;
            cw_pack_array_size(pc, ic->count);
            for( i=0; i< ic->count; i++)
                item32packContext (pc, ic->items[i]);
            break;

        case ITEM_NIL:
            cw_pack_nil (pc);
            break;

        case ITEM_TRUE:
            cw_pack_boolean(pc, true);
            break;

        case ITEM_FALSE:
            cw_pack_boolean(pc, false);
            break;

        case ITEM_INTEGER:
            cw_pack_signed(pc, ((item_integer*)item)->value);
            break;

        case ITEM_REAL:
            cw_pack_double(pc, ((item_real*)item)->value);
            break;

        case ITEM_STRING:
            cp = ((item_string*)item)->string;
            cw_pack_str(pc, cp, (unsigned)strlen(cp));
            break;

        default:    break;
    }
}

void item32cwpackFile (FILE* file, item_root* item)
{
    stream_pack_context spc;
    init_stream_pack_context(&spc, 10, file);
    item32packContext (&spc.pc, item);
    terminate_stream_pack_context(&spc);
}


/**********************************  CWPACK FILE  to  ITEM-TREE  *********************/

static item_root* packContext2item3 (cw_unpack_context* uc)
{
    int i,dim;
    item_root* result;
    cw_unpack_next(uc);
    if (uc->return_code)
        exit(uc->return_code);
    item_container* ic;
    switch (uc->item.type)
    {
        case CWP_ITEM_NIL:
            result = allocate_item(item_root,ITEM_NIL,0);
            break;

        case CWP_ITEM_BOOLEAN:
            if (uc->item.as.boolean)
            {
                result = allocate_item(item_root,ITEM_TRUE,0);
            }
            else
            {
                result = allocate_item(item_root,ITEM_FALSE,0);
            }
            break;

        case CWP_ITEM_POSITIVE_INTEGER:
        case CWP_ITEM_NEGATIVE_INTEGER:
            result = (item_root*)allocate_item(item_integer,ITEM_INTEGER,0);
            ((item_integer*)result)->value = uc->item.as.i64;
            break;

        case CWP_ITEM_FLOAT:
            result = (item_root*)allocate_item(item_real,ITEM_REAL,0);
            ((item_real*)result)->value = uc->item.as.real;
            break;

        case CWP_ITEM_DOUBLE:
            result = (item_root*)allocate_item(item_real,ITEM_REAL,0);
            ((item_real*)result)->value = uc->item.as.long_real;
            break;

        case CWP_ITEM_STR:
            result = (item_root*)allocate_item(item_string,ITEM_STRING,uc->item.as.str.length + 1);
            strncpy(((item_string*)result)->string, (const char*)uc->item.as.str.start, uc->item.as.str.length);
            ((item_string*)result)->string[uc->item.as.str.length] = 0;
            break;

        case CWP_ITEM_MAP:
            dim = 2 * uc->item.as.map.size;
            ic = allocate_container(ITEM_MAP, dim);
            for (i=0; i<dim; i++)
            {
                ic->items[i] = packContext2item3 (uc);
            }
            result = (item_root*)ic;
            break;

        case CWP_ITEM_ARRAY:
            dim = uc->item.as.array.size;
            ic = allocate_container(ITEM_ARRAY, dim);
            for (i=0; i<dim; i++)
            {
                ic->items[i] = packContext2item3 (uc);
            }
            result = (item_root*)ic;
            break;

        default:
            result = NULL;
            break;
    }
    return result;
}

item_root* cwpackFile2item3 (FILE* file)
{
    stream_unpack_context suc;
    init_stream_unpack_context(&suc, 0, file);
    item_root* result = packContext2item3(&suc.uc);
    terminate_stream_unpack_context(&suc);
    return result;
}



