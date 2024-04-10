# CWPack / Goodies / Swift

This folder contains a pack and an unpack context and a packable protocol to enable Swift items in the MessagePack stream.

## Contexts

The swift interface contains 4 contexts `CWDataPacker`, `CWFilePacker`, `CWDataUnpacker` and `CWFileUnpacker`. All are layered on the corresponding c structures accessable though the property `p`.

The file packer/unpacker comes in 2 flavours, inited with a path or with a file descriptor.

#### Pack

`CWDataPacker` is layered on top of a `dynamic_memory_pack_context` and `CWFilePacker` is layerd on top of a `file_pack_context`.

To pack an item you call: `packer.pack(item)`

After packing you can check the result by inspecting the packer property `OK`.

The packers have a property `optimizeReal`. When true (default), a check is performed to see if real items losslessly could be casted to a shorter representation, e.g. 0.0 is saved as an (1 byte) integer and Double 0.5 is saved as a Float.

#### Unpack

`CWDataUnpacker` is layered on top of a `cw_unpack_context` and
`CWFileUnpacker` is layered on `file_unpack_context`.

To retreive items you call the `init(_ unpacker: CWUnpacker)` initializer e.g:

- `let i = Int(unpacker)` and
- `let ar: [String] = Array(unpacker)`

If you are retreiving the value of an optional type, you should use the `init(optional unpacker: CWUnpacker)` initializer e.g:

- `let i: Int? = Int(optional: unpacker)` and
- `let ar: [String]? = Array(optional: unpacker)`

If the messagepack stream doesn't contain the expected item, an exception is thrown.

Doubles accept both Float and Integer as valid values at unpack.

## Convenience operators '+' and '-'

To simplify packing and unpacking two operators are defined for packers/unpackers. When packing you can write:

`packer + value1 + value2 ...` (think of it as adding values to the packer stream)

When unpacking you can write:

`unpacker - variable1 - variable2 ...` 

Note however, this has limited usage in inits, as the variables are handled as `inout` parameters and if they are properties, they are considered used before assigned by the compiler. Another restriction for the `-` operator is that the variables can't be optionals.

## Packable items

When choosing if the packing should be automatic by inspection or explicit, we have chosen the latter, as it gives better control over the (un)packing, which can be important when communicating with others. 

For a type to be packable it should fulfill the protocol `CWPackPackable`. The protocol specifies a mandatory method to be implemented:

- `cwPack(_ packer:CWPacker)`

For a type to be unpackable it should fulfill the protocol `CWPackUnpackable`. The protocol specifies a mandatory method to be implemented:

- `init(_ unpacker:CWUnpacker) throws`

The protocol `CWPackable`combines both protocols above and the type can be used for both packing and unpacking. 

The file CWPackable.swift contains the protocol definitions and `CWPackable` exstensions of some system types.

### MessagePack items

MessagePack has some types that don't have exact match in Swift. To simplify usage they are defined in CWPack.swift together with their CWPackable implementation. They are:

- `CWNil` to be able to handle nil items. This will be deprecated as it is no longer needed.
- `ArrayHeader` and
- `DictionaryHeader` to be able to pack/unpack structures in an incremental fashion.
- `MsgPackExt` to handle extension types. However, the standard extension type Timestamp is mapped to Swift type Date.

### Enums with rawValue

The file CWPackable.swift contains protocols for enums with a rawValue type of Int, Double, Character and String. The protocol extensions contain a complete implementationof the CWPackable protocol so no more code is needed. E.g.:

```
enum EInt: Int, CWPackableInt {              // Packed as a MessagePack Integer
    case first = 50
    case second = 3
}

enum EDouble: Double, CWPackableDouble {     // Packed as a MessagePack Float
    case pi = 3.14
    case e = 2.73
}

enum ECharacter: Character, CWPackableChar{  // Packed as a MessagePack Unsigned Integer
    case space = " "
    case newLine = "\n"
}

enum EString: String, CWPackableString {     // Packed as a MessagePack String
    case alpha
    case omega
}
```
 
## Installation
 
The interface is delivered as source files that you include in your project. The following source files (and corresponding header files) must be included:

- CWPack/src/cwpack.c
- CWPack/goodies/basic-contexts/basic_contexts.c
- CWPack/goodies/utils/cwpack_utils.c
- CWPack/goodies/swift/CWPack.swift
- CWPack/goodies/swift/CWPackable.swift


If you are using Apple Core Graphic you may also need the file:

- CWPack/goodies/swift/CG+CWPackable.swift

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
