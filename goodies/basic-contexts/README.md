# CWPack / Goodies / Basic Contexts


Basic contexts contains 5 contexts that meet most demands:

- **Dynamic Memory Pack Context** is used when you want to pack to a malloc´d memory buffer. At buffer overflow the context handler tries to reallocate the buffer to a larger size.

- **Stream Pack Context** is used when you pack to a C stream. At buffer overflow the context handler writes the buffer out and then reuses it. If an item is larger than the buffer, the handler tries to reallocate the buffer so the item would fit.

- **Stream Unpack Context** is used when you unpack from a C stream. As with Stream Pack Context, the handler asserts that an item will always fit in the buffer.

- **File Pack Context** is used when you pack to a file descriptor. At buffer overflow the context handler writes the buffer out and then reuses it. However, if the barrier is active, the subsequent content is kept in the buffer. If an item is larger than the buffer, the handler tries to reallocate the buffer so the item would fit.

- **File Unpack Context** is used when you unpack from a file descriptor. If the barrier is active, the subsequent content is always kept in buffer. The handler asserts that an item will always fit in the buffer.

With the stream/file contexts, it is assumed that the stream/file has been opened before the context is initialized. Before a packed stream/file is closed, the corresponding terminate context should be called so the last buffer is saved.
