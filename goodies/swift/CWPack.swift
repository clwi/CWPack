//
//  CWPack.swift
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
//import AppKit


enum CWPackError: Error {
    case fileError(_ errNo:Int32)
    case contextError(_ err:Int32)
    case packerError(_ err:String)
    case unpackerError(_ err:String)
}


// MARK: ------------------------------ MessagePack Objects

struct CWNil {}

struct ArrayHeader {
    let count:Int
    init (_ count:Int = 0) {self.count = count}
}

struct DictionaryHeader {
    let count:Int
    init (_ count:Int = 0) {self.count = count}
}

struct MsgPackExt  {
    let type: Int8
    let data: Data
    init (_ type:Int8, _ data: Data) {
        self.type = type
        self.data = data
    }
}


// MARK: ------------------------------ MessagePacker

class CWPacker {
    let p: UnsafeMutablePointer<cw_pack_context>

    var optimizeReal: Bool = true
    var OK: Bool {p.pointee.return_code == CWP_RC_OK}

    func pack (_ item: (any CWPackPackable)?) {
        if item == nil  {
            cw_pack_nil(p)
        } else {
            item!.cwPack(self)
        }
    }

    init(_ p:UnsafeMutablePointer<cw_pack_context>) {
        self.p = p
    }
}


class CWDataPacker: CWPacker {
    private var context = dynamic_memory_pack_context()

    var data: Data {
        let c:Int = context.pc.current - context.pc.start
        return Data(bytes:context.pc.start, count:c)}
    
    init() {
        super.init(&context.pc)
        init_dynamic_memory_pack_context(&context, 1024)
    }
}


class CWFilePacker: CWPacker {
    private var context = file_pack_context()
    private let ownsChannel: Bool
    private let fd: Int32

    func flush() {cw_pack_flush(&context.pc)}

    init(to descriptor:Int32) {
        ownsChannel = false
        fd = -1
        super.init(&context.pc)
        init_file_pack_context(&context, 1024, descriptor)
    }

    init(to path: String) throws {
        ownsChannel = true
        fd = open(path, O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )
        guard fd >= 0 else {
            print("Open write error: \(errno)\n")
            throw CWPackError.fileError(errno)
        }
        super.init(&context.pc)
        init_file_pack_context(&context, 1024, fd)
    }

    convenience init(to url:URL) throws {
        try self.init(to: url.path)
    }

    deinit {
        terminate_file_pack_context(&context)
        if ownsChannel && fd >= 0 {close(fd)}
    }
}


// MARK: ------------------------------ MessageUnpacker

class CWUnpacker {
    let p: UnsafeMutablePointer<cw_unpack_context>
    var OK: Bool {p.pointee.return_code == CWP_RC_OK}
    var nextItemType: cwpack_item_types {return cw_look_ahead(p)}
    func skipItems(_ count: Int) {cw_skip_items(p,count)}

    init(_ p:UnsafeMutablePointer<cw_unpack_context>) {
        self.p = p
    }
}


class CWDataUnpacker: CWUnpacker {
    private var context = cw_unpack_context()
    private var buffer: [UInt8]
    init(from data: Data) {
        buffer = Array(repeating: UInt8(0), count: data.count)
        super.init(&context)
        data.copyBytes(to: &buffer, count: data.count)
        cw_unpack_context_init(&context, buffer, UInt(data.count), nil)
    }
}


class CWFileUnpacker: CWUnpacker {
    private var context = file_unpack_context()
    private let ownsChannel: Bool
    let fd: Int32

    init(from descriptor:Int32) {
        ownsChannel = false
        fd = -1
        super.init(&context.uc)
        init_file_unpack_context(&context, 1024, descriptor)
    }

    init(from path: String) throws {
        ownsChannel = true
        fd = open(path, O_RDONLY)
        guard fd >= 0 else {
            print("Open read error: \(errno)\n")
            throw CWPackError.fileError(errno)
        }
        super.init(&context.uc)
        init_file_unpack_context(&context, 1024, fd)
    }

    convenience init(from url:URL) throws {
        try self.init(from: url.path)
    }

    deinit {
        terminate_file_unpack_context(&context)
        if ownsChannel && fd >= 0 {close(fd)}
    }
}
