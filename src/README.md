# CWPack / src

The src folder contains all basic functionallity to use CWPack.

## Files

**cwpack.h** contains the interface to CWPack. You should include this in your code.

**cwpack_config.h** contains info about your processor. Byte order, alignment etc. Update this file to suit you.

**cwpack_internals.h** contains internal macros for cwpack.c. If you are an experienced developer you can use them to access the inner mechanics of CWPack.

**cwpack.c** contains the code.

## Contexts
Central to CWPack is the concept of contexts. There are two: `cw_pack_context` and `cw_unpack_context`. They contains all the necessary bookkeeping and a reference to the appropriate context is given in all routine calls.

CWPack is working against memory buffers. Handlers, stored in the context, are called when a buffer is filled up (packing) or needs refill (unpack). The contexts in this folder handles static memory buffers, but more complex contexts that handles dynamic memory, files and sockets can be found in [goodies/basic-contexts](https://github.com/clwi/CWPack/tree/master/goodies/basic-contexts).

## How to use
First you choose a context that suits your needs and initiates it. Then you can do the packing/unpacking.

CWpack is using a streaming model, containers (arrays, maps) are read/written in parts, first the item containing the size and then the contained items one by one. Exception to this is the `cw_skip_items` function which skips whole containers.

You find some convenience routines for packing and an expect api for unpacking in [goodies/utils](https://github.com/clwi/CWPack/tree/master/goodies/utils).
You find an Objective-C wrapper in [goodies/objC](https://github.com/clwi/CWPack/tree/master/goodies/objC).


