#CWPack

CWPack is a minimalistic and yet fast and complete implementation of the 
[MessagePack](http://msgpack.org) serialization format 
[version 5](https://github.com/msgpack/msgpack/blob/master/spec.md).

## Design

CWPack does no memory allocations and no file handling. All that is done 
outside of CWPack.

CWPack is working against memory buffers. User defined handlers are called when buffers are 
filled up (packing) or needs refill (unpack). 

Containers (arrays, maps) are read/written in parts, first the item containing the size and 
then the contained items one by one. Exception to this is the `cw_skip_items` function which 
skip whole containers.

## Example

Pack and unpack example from the MessagePack home page:

```c
void example (void)
{
    cw_pack_context pc;
    char buffer[20];
    cw_pack_context_init (&pc, buffer, 20, 0, 0);

    if (cw_pack_map_size (&pc, 2)) ERROR;
    if (cw_pack_str (&pc, "compact", 7)) ERROR;
    if (cw_pack_boolean (&pc, true)) ERROR;
    if (cw_pack_str (&pc, "schema", 6)) ERROR;
    if (cw_pack_unsigned (&pc, 0)) ERROR;

    int length = pc.current - pc.start;
    if (length > 18) ERROR

    cw_unpack_context uc;
    cw_unpack_context_init (&uc, pc.start, length, 0, 0);

    if (cw_unpack_next(&uc))  ERROR;
    if (uc.item.type != CWP_ITEM_MAP || uc.item.as.map.size != 2) ERROR;

    if (cw_unpack_next(&uc))  ERROR;
    if (uc.item.type != CWP_ITEM_STR || uc.item.as.str.length != 7)) ERROR;
    if (strncmp("compact", uc.item.as.str.start, 7)) ERROR;

    if (cw_unpack_next(&uc))  ERROR;
    if (uc.item.type != CWP_ITEM_BOOLEAN || uc.item.as.boolean != true) ERROR;

    if (cw_unpack_next(&uc))  ERROR;
    if (uc.item.type != CWP_ITEM_STR || uc.item.as.str.length != 6)) ERROR;
    if (strncmp("schema", uc.item.as.str.start, 6)) ERROR;

    if (cw_unpack_next(&uc))  ERROR;
    if (uc.item.type != CWP_ITEM_POSITIVE_INTEGER || uc.item.as.u64 != 0) ERROR;

    if (cw_unpack_next(&uc) != CWP_RC_END_OF_INPUT)  ERROR;
}
```

## Error handling

CWPack does not check for illegal values (e.g. in STRs for illegal unicode characters).

When an error is detected in a context, that context is return_code and all calls are returned 
with the return_code error code.

## Build

CWPack consists of a single src file with corresponding header file. It is written 
in C and the files are together ~ 950 lines. No build is neccesary, just include the 
files in your own build.

CWPack has no dependencies to other libraries.

## Module test

Included in the test folder in the repository is a simple module test and a shell script to run it.
