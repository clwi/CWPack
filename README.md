# CWPack

CWPack is a lightweight and yet complete implementation of the 
[MessagePack](http://msgpack.org) serialization format 
[version 5](https://github.com/msgpack/msgpack/blob/master/spec.md).

## Excellent Performance

Together with [MPack](https://github.com/ludocode/mpack), CWPack is the fastest open-source messagepack implementation. Both totally outperform
[CMP](https://github.com/camgunz/cmp) and [msgpack-c](https://github.com/msgpack/msgpack-c)

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

    cw_pack_map_size (&pc, 2);
    cw_pack_str (&pc, "compact", 7);
    cw_pack_boolean (&pc, true);
    cw_pack_str (&pc, "schema", 6);
    cw_pack_unsigned (&pc, 0);

    int length = pc.current - pc.start;
    if (length > 18) ERROR;

    cw_unpack_context uc;
    cw_unpack_context_init (&uc, pc.start, length, 0, 0);

    cw_unpack_next(&uc);
    if (uc.item.type != CWP_ITEM_MAP || uc.item.as.map.size != 2) ERROR;

    cw_unpack_next(&uc);
    if (uc.item.type != CWP_ITEM_STR || uc.item.as.str.length != 7)) ERROR;
    if (strncmp("compact", uc.item.as.str.start, 7)) ERROR;

    cw_unpack_next(&uc);
    if (uc.item.type != CWP_ITEM_BOOLEAN || uc.item.as.boolean != true) ERROR;

    cw_unpack_next(&uc);
    if (uc.item.type != CWP_ITEM_STR || uc.item.as.str.length != 6)) ERROR;
    if (strncmp("schema", uc.item.as.str.start, 6)) ERROR;

    cw_unpack_next(&uc);
    if (uc.item.type != CWP_ITEM_POSITIVE_INTEGER || uc.item.as.u64 != 0) ERROR;

    cw_unpack_next(&uc);
    if (uc.return_code != CWP_RC_END_OF_INPUT)  ERROR;
}
```

In the examples folder there are more examples.

## Backward compatibility

CWPack may be run in compatibility mode. It affects only packing; EXT is considered illegal, BIN are transformed to STR and generation of STR8 is supressed.

## Error handling

When an error is detected in a context, the context is stopped and all future calls to that context are immediatly returned without any actions.

CWPack does not check for illegal values (e.g. in STR for illegal unicode characters).

## Build

CWPack consists of a single src file and two header files. It is written 
in strict ansi C and the files are together ~ 1.2K lines. No separate build is neccesary, just include the 
files in your own build.

CWPack has no dependencies to other libraries.

## Test

Included in the test folder are a module test and a performance test and shell scripts to run them.
