# CWPack / Goodies / Swift

This folder contains a pack and an unpack context and a packable protocol to enable Swift items in the MessagePack stream.

## Contexts

The swift interface contains 4 contexts `CWDataPacker`, `CWFilePacker`, `CWDataUnpacker` and `CWFileUnpacker`. All are layered on the corresponding c structures accessable though the property `p`.

The file packer/unpacker comes in 2 flavours, inited with an URL or with a file descriptor.

#### Pack

`CWDataPacker` is layered on top of a `dynamic_memory_pack_context` and `CWFilePacker` is layerd on top of a `file_pack_context`.

To pack an item you call: `item.cwPack(packer)`

After packing you can check the result by inspecting the packer property `OK`.

The packers have a property `optimizeReal`. When true, a check is performed to see if real items losslessly could be casted to a shorter representation, e.g. 0.0 is saved as an (1 byte) integer and Double 0.5 is saved as a Float.

#### Unpack

`CWDataUnpacker` is layered on top of a `cw_unpack_context` and
`CWFileUnpacker` is layered on `file_unpack_context`.

To retreive items you call the init(_ unpacker: CWUnpacker) initializer e.g:
`let i = Int(unpacker)` and
`let ar: [String] = Array(unpacker)`

If the messagepack stream doesn't contain the expected item, an exception is thrown.

Doubles accept both Float and Integer as valid values at unpack.

## Conveniance operators '+' and '-'

To simplify packing and unpacking two operators are defined for packers/unpackers. When packing you can write:

`packer + value1 + value2 ...` (think of it as adding values to the packer stream) and when unpacking you can write:

`unpacker - variable1 - variable2...` 

Note however, this has limited usage in inits, as the variables are handled as `inout` parameters and if they are properties, they are considered used before assigned by the compiler.

## Packable items

When choosing if the packing should be automatic by inspection or explicit, we have chosen the latter, as it gives better control over the (un)packing, which can be important when communicating with others. For that reason, to be packable, objects need to fulfill the protocol `CWPackable`. 

To fulfill the `CWPackable` protocol, an item must implement 2 methods:

- `init(_ unpacker:CWUnpacker) throws` and
- `cwPack(_ packer:CWPacker)`

The file CWPackable.swift contains CWPackable exstensions of some system types.

### MessagePack items

MessagePack has some types that don't have exact match in Swift. To simplify usage they are defined in CWPack.swift. They are:

- `CWNil` to be able to handle nil items.
- `ArrayHeader` and
- `DictionaryHeader` to be able to pack/unpack structures in an incremental fashion.
- `MsgPackExt` to handle extension types. However, the standard extension type Timestamp is mapped to Swift type Date.
 
## Installation
 
The interface is delivered as source files that you include in your project. The following source files (and corresponding header files) must be included:

- CWPack/src/cwpack.c
- CWPack/goodies/basic-contexts/basic_contexts.c
- CWPack/goodies/utils/cwpack_utils.c
- CWPack/goodies/swift/CWPack.swift
- CWPack/goodies/swift/CWPackable.swift

If you use XCode you should place this snippet in the file xxx-Bridging-Header.h

```
#include "cwpack.h"
#include "cwpack_utils.h"
#include "basic_contexts.h"
```

## Example

When you write pack code it's a good routine to contain struct and class properties in an array. This way it works if you in turn put those structures in other containers (Arrays and Dictionaries)

```
struct RGB {
	let r,g,b: UInt8
	cwPack(_ packer:CWPacker) {
		packer + ArrayHeader(3) + r + g + b
	}
	init(_ unpacker:CWUnpacker) throws {
		guard try ArrayHeader(unpacker).count == 3 else {throw some error}
		r = try UInt8(unpacker)
		g = try UInt8(unpacker)
		b = try UInt8(unpacker)
	}
}
```
The packing line above could also be written as:
`packer + [r,g,b]` with the same result.

Now you can use this struct in another struct:

```
struct palette {
	colors: [RGB]
	alpha: UInt8
	cwPack(_ packer:CWPacker) {
		packer + ArrayHeader(2) + colors + alpha
	}
	init(_ unpacker:CWUnpacker) throws {
		guard try ArrayHeader(unpacker).count == 2 else {throw some error}
		colors = try Array(unpacker)
		alpha = try UInt8(unpacker)
	}
}
```

## Failings

I'm sorry for this section but one has to face the facts. To be able to unpack (at least with this library), you must know the type of the next item. So if you have a definition like:
`var ar: [Any]`, you can't write:
`ar = try Array(unpacker)`

The same problem arises with subclasses. In both cases one can often solv the dilemma with incremental unpacking and intelligent lookahead of what's coming next.
