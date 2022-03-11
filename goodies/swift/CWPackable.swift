//
//  CWPackable.swift
//  CWPack
//
//  Created by Claes Wihlborg on 2022-01-17.
//

/*
 The MIT License (MIT)

 Copyright (c) 2021 Claes Wihlborg

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


import Foundation


// MARK: ----------------------------------------------- CWPackable protocol


protocol CWPackable {
    @discardableResult static func + (lhs: CWPacker, rhs: Self) ->  CWPacker

    func cwPack(_ packer: CWPacker)

    @discardableResult static func - (lhs: CWUnpacker, rhs: inout Self) throws ->  CWUnpacker

    init (_ unpacker: CWUnpacker) throws
}


extension CWPackable {
    @discardableResult static func + (lhs: CWPacker, rhs: Self) ->  CWPacker {
        rhs.cwPack(lhs)
        return lhs
    }

    @discardableResult static func - (lhs: CWUnpacker, rhs: inout Self) throws ->  CWUnpacker {
        rhs = try self.init(lhs)
        return lhs
    }
}

// MARK: ----------------------------------------------- MessagePack type extensions

extension CWNil: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_nil(packer.p)}

    init (_ unpacker: CWUnpacker) throws {
        cw_unpack_next_nil(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("CWNil")}
    }
}

extension ArrayHeader: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_array_size(packer.p, UInt32(self.count))}

    init (_ unpacker: CWUnpacker) throws {
        self.count = Int(cw_unpack_next_array_size(unpacker.p))
        guard unpacker.OK else {throw CWPackError.unpackerError("ArrayHeader")}
    }
}

extension DictionaryHeader: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_map_size(packer.p, UInt32(self.count))}

    init (_ unpacker: CWUnpacker) throws {
        self.count = cw_unpack_next_map_size(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("DictionaryHeader")}
    }
}

extension MsgPackExt: CWPackable {
    func cwPack(_ packer: CWPacker) {
        data.withUnsafeBytes {ptr in cw_pack_ext (packer.p, type, ptr.baseAddress, UInt32(data.count))}
    }
    init (_ unpacker: CWUnpacker) throws {
        cw_unpack_next(unpacker.p)
        guard unpacker.OK && unpacker.p.pointee.item.type.rawValue <= CWP_ITEM_MAX_USER_EXT.rawValue
        else {throw CWPackError.unpackerError("MsgPackExt")}
        self.init ( Int8(unpacker.p.pointee.item.type.rawValue), Data(bytes: unpacker.p.pointee.item.as.ext.start, count: Int(unpacker.p.pointee.item.as.ext.length)))
    }
}


// MARK: ----------------------------------------------- System type extensions

extension Bool: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_boolean(packer.p, self)}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_boolean(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Bool")}
    }
}

extension Int: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_signed(packer.p, Int64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = Int(cw_unpack_next_signed64(unpacker.p))
        guard unpacker.OK else {throw CWPackError.unpackerError("Int")}
    }
}

extension Int8: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_signed(packer.p, Int64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_signed8(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Int8")}
    }
}

extension Int16: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_signed(packer.p, Int64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_signed16(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Int16")}
    }
}

extension Int32: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_signed(packer.p, Int64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_signed32(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Int32")}
    }
}

extension Int64: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_signed(packer.p, Int64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_signed64(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Int64")}
    }
}

extension UInt: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_unsigned(packer.p, UInt64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = UInt(cw_unpack_next_unsigned64(unpacker.p))
        guard unpacker.OK else {throw CWPackError.unpackerError("UInt")}
    }
}

extension UInt8: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_unsigned(packer.p, UInt64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_unsigned8(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("UInt8")}
    }
}

extension UInt16: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_unsigned(packer.p, UInt64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_unsigned16(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("UInt16")}
    }
}

extension UInt32: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_unsigned(packer.p, UInt64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_unsigned32(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("UInt32")}
    }
}

extension UInt64: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_unsigned(packer.p, UInt64(self))}

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_unsigned64(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("UInt64")}
    }
}

extension Float: CWPackable {
    func cwPack(_ packer: CWPacker) {
        if packer.optimizeReal { cw_pack_float_opt(packer.p, self) }
        else { cw_pack_float(packer.p, self) }
    }

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_float(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Float")}
    }
}

extension Double: CWPackable {
    func cwPack(_ packer: CWPacker) {
        if packer.optimizeReal { cw_pack_double_opt(packer.p, self) }
        else { cw_pack_double(packer.p, self) }
    }

    init (_ unpacker: CWUnpacker) throws {
        self = cw_unpack_next_double(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Double")}
    }
}

extension Date: CWPackable {
    func cwPack(_ packer: CWPacker) {
        cw_pack_time_interval(packer.p, self.timeIntervalSince1970)}

    init (_ unpacker: CWUnpacker) throws {
        self.init(timeIntervalSince1970: cw_unpack_next_time_interval(unpacker.p))
        guard unpacker.OK else {throw CWPackError.unpackerError("Date")}
    }
}

extension Data: CWPackable {
    func cwPack(_ packer: CWPacker) {
        self.withUnsafeBytes {ptr in cw_pack_bin (packer.p, ptr.baseAddress, UInt32(ptr.count))}
    }

    init (_ unpacker: CWUnpacker) throws {
        let l = cw_unpack_next_bin_lengh(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("Data")}
        if l > 0 {self.init(bytes: unpacker.p.pointee.item.as.bin.start, count: Int(l))}
        else {self.init()}
    }
}

extension String: CWPackable {
    func cwPack(_ packer: CWPacker) {
        let s = self.utf8CString
        s.withUnsafeBufferPointer {ptr in cw_pack_str (packer.p, ptr.baseAddress, UInt32(strlen(ptr.baseAddress!)))}
    }

    init (_ unpacker: CWUnpacker) throws {
        let l = cw_unpack_next_str_lengh(unpacker.p)
        guard unpacker.OK else {throw CWPackError.unpackerError("String")}
        if l > 0 {self.init(NSString(bytes: unpacker.p.pointee.item.as.str.start, length: Int(l), encoding: String.Encoding.utf8.rawValue)!)}
        else {self.init()}
    }
}

extension Array: CWPackable where Element: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(self.count)
        if self.count > 0 {
            for i in 0..<self.count {
                packer + self[i]
            }
        }
    }

    init (_ unpacker: CWUnpacker) throws {
        let ah = try ArrayHeader(unpacker)
        self.init()
        if ah.count > 0 {
            for _ in 1...ah.count {
                let e: Element = try Element(unpacker)
                self.append(e)
            }
        }
    }
}

extension ArraySlice: CWPackable where Element: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(self.count)
        if self.count > 0 {
            for i in startIndex..<endIndex {
                packer + self[i]
            }
        }
    }

    init (_ unpacker: CWUnpacker) throws {
        let ah = try ArrayHeader(unpacker)
        self.init()
        if ah.count > 0 {
            for _ in 1...ah.count {
                let e: Element = try Element(unpacker)
                self.append(e)
            }
        }
    }
}

extension Dictionary: CWPackable where Key: CWPackable , Value: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + DictionaryHeader(UInt32(self.count))
        if self.count > 0 {
            for key in self.keys {
                packer + key + self[key]!
            }
        }
    }

    init (_ unpacker: CWUnpacker) throws {
        let ah = try DictionaryHeader(unpacker)
        self.init()
        if ah.count > 0 {
            for _ in 1...ah.count {
                let k: Key = try Key(unpacker)
                let v: Value = try Value(unpacker)
                self[k] = v
            }
        }
    }
}

