#CWPack - Goodies - Basic Contexts


Basic contexts contains 3 contexts:

- **Dynamic Memory Pack Context** is used when you want to pack to a malloc´d memory buffer. At buffer overflow the context handler tries to reallocate the buffer to a larger size.

- **File Pack Context** is used when you pack to a file. At buffer overflow the context handler writes the buffer out and then reuses it. If an item is larger than the buffer, the handler tries to reallocate the buffer so the item would fit.

- **File Unpack Context** is used when you unpack from a file. As with File Pack Context, the handler asserts that an items will always fit in the buffer.
