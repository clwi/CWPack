# CWPack Quick Start

CWPack is not a library file, it is a collection of subroutines that you include in your project.


## Packing & Unpacking Contexts

All packing and unpacking are done through a context. A context is a struct that contains all bookkeeping that is needed. The primary packing and unpacking contexts are defined in [src/cwpack.h](https://github.com/clwi/CWPack/blob/master/src/cwpack.h). Other contexts are defined in [/goodies/basic-contexts/basic_contexts.h](https://github.com/clwi/CWPack/blob/master/goodies/basic-contexts/basic_contexts.h)


### Packing to fixed memory buffer

When packing to a fixed memory buffer you use the primary  context `cw_pack_context`. Packing to fixed buffer is most suitable for short messages, e.g. settings.

If you should pack the message *{"compact":true, "schema":0}* do like this:

```c
cw_pack_context pc;
cw_pack_context *ctx = &pc;
char buffer[20];
cw_pack_context_init (ctx, buffer, 20, 0);

cw_pack_map_size (ctx, 2);
cw_pack_str (ctx, "compact", 7);
cw_pack_boolean (ctx, true);
cw_pack_str (ctx, "schema", 6);
cw_pack_unsigned (ctx, 0);

if (ctx->return_code != CWP_RC_OK)  ERROR;
int length = ctx->current - ctx->start;
```
You obtain the result in `buffer`.


### Packing to dynamic memory buffer

When you want to pack to memory but doesn't know the message length, the `dynamic_memory_pack_context` in basic-contexts is your best pick. When you initiate the context you give a start value for buffer length. If you specify 0(zero), a default value (currently 1024) is used.

Creating our favorite demo message would look like:

```c
dynamic_memory_pack_context dmpc;
init_dynamic_memory_pack_context (&dmpc, 0);
cw_pack_context *ctx = &dmpc.pc;

cw_pack_map_size (ctx, 2);
cw_pack_str (ctx, "compact", 7);
cw_pack_boolean (ctx, true);
cw_pack_str (ctx, "schema", 6);
cw_pack_unsigned (ctx, 0);

if (ctx->return_code != CWP_RC_OK)  ERROR;
int length = ctx->current - ctx->start;
```
You obtain the result in `ctx->start`. When you have used the result you should call `free_dynamic_memory_pack_context(&dmpc)` to free the memory buffer.


### Unpacking from memory buffer

Unpacking a memory message is quite straightforward. You use the primary context `cw_unpack_context`.
Suppose `message` points to the message and `messagelength` contains the length, then the setup is as follows:

```c
cw_unpack_context uc;
cw_unpack_context_init (&uc, message, messagelength, NULL);
cw_unpack_context *ctx = &uc;
```
You then collects the message items one at a time with:

```c
cw_unpack_next(ctx)
if (ctx->return_code != CWP_RC_OK) ERROR;
```
The collected item is found in `ctx->item`

If the collected item is an array or a map, you don't get the elements of the collection, just an element count. The elements must then be collected separately. Note! a map element contains two items, a key and a value.


### Packing to a file descriptor

When your environment has a Posix interface and you pack to a disc file or to a network socket, use the `file_pack_context` found in basic-contexts.  It uses a temporary memory buffer for packing before writing it out to the file descriptor. You use it this way:

```c
file_pack_context fpc;
init_file_pack_context (&fpc, initial_buffer_length, fileDescriptor);
file_pack_context *ctx = &fpc.pc;
```
If you set `initial_buffer_length` to less than 32, a default value (currently 4096) is applied.

When you pack to a socket, issue a `cw_pack_flush(ctx)` after every message so output buffers are flushed.

When packing is finished, call `terminate_file_pack_context(&fpc)` to flush all buffers and release temporary memory buffer.

The file descriptor should be inited before it is used in the `init_file_pack_context` call, and it is the users responsibility that it is closed after `terminate_file_pack_context`.


### Unpacking from a file descriptor

When the MewssagePack input comes from a file descriptor, you should use the `file_unpack_context` found in basic-contexts. It keeps a temporary buffer filled so it's about the same as reading from a memory buffer. The setup is:

```c
file_unpack_context fuc;
init_file_unpack_context (&fuc, initial_buffer_length, fileDescriptor);
file_unpack_context *ctx = &fuc.uc;
```

If reading from a socket, the context takes care of splitted items so `get_next_item` doesn't return until the whole item is received. (Exception: large Str/Bin/Ext items. Se further down.)

When unpacking is finished, call `terminate_file_unpack_context(&fpc)` to release temporary memory buffer.

The file descriptor should be inited before it is used in the `init_file_unpack_context` call, and it is the users responsibility that it is closed after `terminate_file_unpack_context`.


## Packing

### Large Data

The STR/BIN/EXT items may hold a large payload, e.g. the bible or a movie. It's not practical to pack all this as a single operation, keeping all data in memory simultaneously. But it is possible to split the packing operation and first pack the item header and then the payload in portions. Another advantage is that for file_contexts, the payload is written direct to the file descriptor and isn't using the contexts internal buffer.  Here is a subroutine example that packs a disk file to a pack context.

```c
void cwpack_disk_file (cw_pack_context* ctx, const char* fileName) {
    if (ctx->return_code != CWP_RC_OK) return;
    int fd = open(fileName, O_RDONLY);
    if (fd < 0) {
        ctx->return_code = CWP_RC_IO_ERROR;
        ctx->err_no = errno;
        return;
    }
    long fileSize = lseek (fd, 0L, SEEK_END);
    cw_pack_bin_size(ctx, (uint32_t)fileSize);
    long i, n = fileSize/4096;
    char buffer[4096];
    for (i=0; i<n; i++) {
        long result = read(fd, buffer, 4096);
        if (result < 4096) {
            close(fd);
            ctx->return_code = CWP_RC_IO_ERROR;
            ctx->err_no = errno;
            return;
        }
        cw_pack_insert(ctx, buffer, 4096);
    }
    uint32_t tail = (uint32_t)fileSize % 4096;
    if (tail > 0) {
        long result = read(fd, buffer, tail);
        if (result < tail) {
            close(fd);
            ctx->return_code = CWP_RC_IO_ERROR;
            ctx->err_no = errno;
            return;
        }
        cw_pack_insert(ctx, buffer, tail);
    }
}
```


### Containers

Containers (arrays, maps) are written in parts, first the item containing the size and then the contained elements one by one.


### Real Numbers

When reading a double or float with an expect api, some implementations (like CWPack) may accept other representations also, mainly because they are shorter, e.g. a double may accept a float value. To that end, the routines `cw_pack_float` and `cw_pack_double` has got sieblings in `cw_pack_float_opt` and `cw_pack_double_opt`. They try to find the shortest representation possible without any precision loss. You find them in [utils.h](https://github.com/clwi/CWPack/blob/master/goodies/utils/cwpack_utils.h). Example:

```c
cw_pack_double(ctx, 0.0)
```
creates a 9-bytes item, while

```c
cw_pack_double_opt(ctx, 0.0)
```
creates a 1-byte item.


## Unpacking

### Str / Bin / Ext

These items separates themselves from all other items in that they have variable length of the payload. When a `cw_unpack_next` call detects such an item, it makes shure that the whole data fits in the buffer, even if that causes the buffer to be extended. The item in the context contains the length and a pointer to the data in the buffer. That implies that the user must have saved the data before the next `cw_unpack_next` call as this could destroy the buffer contents.


### Large Data

As pointed out above, Str/Bin/Ext items could extend the input buffer quite a lot. The trick to avoid that is to use `cw_unpack_next_descriptor` instead of `cw_unpack_next`. This call behaves exactly like `cw_unpack_next` for all other items, but for Str/Bin/Ext items only the type and the length is saved in the context.item, and no effort is spent on getting the payload. The payload can then be obtained with the `cw_unpack_data` call. It's not neccesary to obtain all data in a single call but you can divide it in parts as you do in writing.

Example is a function that reads a Bin item from a context and saves the payload in a disc file.

```c
void cwunpack_disk_file (cw_unpack_context* ctx, const char* fileName) {
    if (ctx->return_code != CWP_RC_OK) return;
    int fd = open(fileName, O_RDWR + O_CREAT + O_TRUNC, 0666);
    if (fd < 0) {
        ctx->return_code = CWP_RC_IO_ERROR;
        ctx->err_no = errno;
        return;
    }
    cw_unpack_next_descriptor(ctx);
    if (ctx->item.type != CWP_ITEM_BIN) {
        ctx->return_code = CWP_RC_TYPE_ERROR;
        return;
    }
    long fileSize = ctx->item.as.bin.length;
    long i, n = fileSize/4096;
    char buffer[4096];
    for (i=0; i<n; i++) {
        cw_unpack_data(ctx, buffer, 4096);
        if (ctx->return_code != CWP_RC_OK) {
            close(fd);
            return;
        }
        long result = write(fd, buffer, 4096);
        if (result < 0) {
            close(fd);
            ctx->return_code = CWP_RC_IO_ERROR;
            ctx->err_no = errno;
            return;
        }
    }
    uint32_t tail = (uint32_t)fileSize % 4096;
    if (tail > 0) {
        cw_unpack_data(ctx, buffer, tail);
        if (ctx->return_code == CWP_RC_OK) {
            long result = write(fd, buffer, tail);
            if (result < 0) {
                ctx->return_code = CWP_RC_IO_ERROR;
                ctx->err_no = errno;
            }
        }
    }
    close(fd);
}
```


### Containers

Containers (arrays, maps) are read in parts, first the item containing the size and then the contained elements one by one. Exception to this is the `cw_skip_items` function which skip whole containers and count them as one item.


### Expect API

The expect api is a convenience layer on top of the `get_next_...` calls. You use it when you know what item should come next. E.g if you know that the next item is a boolean, instead of writing

```c
get_next_item(ctx)
if (ctx->item.type != CWP_ITEM_BOOLEAN) ERROR;
bool b = ctx->item.boolean;
```
you simply write

```c
bool b = cw_unpack_next_boolean(ctx)
```

If you expect a real (double or float), the expect api will accept and convert integers, floats and doubles to the expected format.


The expect api is found in [utils.h](https://github.com/clwi/CWPack/blob/master/goodies/utils/cwpack_utils.h)


## Backward compatibility

CWPack may be run in compatibility mode. It affects only packing; EXT & TIMESTAMP is considered illegal, BIN are transformed to STR and generation of STR8 is supressed.

You activate compatibility mode by:

```c
cw_pack_set_compatibility(ctx, true)
```
and you deactivate it with:

```c
cw_pack_set_compatibility(ctx, false)
```
Compatibility mode is deactivated by default.


## Error handling

When an error is detected in a context, the context is stopped and all future calls to that context are immediatly returned without any actions. Thus it is possible to make some calls and delay error checking until all calls are done.

You can check the error reason in the field `ctx->return_code`. For some errors (e.g. `CWP_RC_ERROR_IN_HANDLER`) thera may be complementary info in `ctx->err_no`.

CWPack does not check for illegal values (e.g. in STR for illegal unicode characters).

