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
    case fileError(_ errNo:Int)
    case contextError(_ err:Int32)
    case packerError(_ err:String)
    case unpackerError(_ err:String)
}


// MARK: ------------------------------ MessagePack Objects

struct CWNil {
}

struct ArrayHeader {
    let count:Int
    init (_ count:Int) {self.count = count}
    init () {count = 0}
}

struct DictionaryHeader {
    let count:UInt32
    init (_ count:UInt32) {self.count = count}
    init () {count = 0}
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
    let fh: FileHandle?

    func flush() {cw_pack_flush(&context.pc)}

    init(to descriptor:Int32) {
        ownsChannel = false
        fh = nil
        super.init(&context.pc)
        init_file_pack_context(&context, 1024, descriptor)
    }

    init(to url:URL) throws {
        fh = try FileHandle(forWritingTo: url)
        ownsChannel = true
        super.init(&context.pc)
        init_file_pack_context(&context, 1024, fh!.fileDescriptor)
    }

    deinit {
        terminate_file_pack_context(&context)
//        if ownsChannel {close(context.fileDescriptor)}
    }
}


// MARK: ------------------------------ MessageUnpacker

class CWUnpacker {
    let p: UnsafeMutablePointer<cw_unpack_context>
    var OK: Bool {p.pointee.return_code == CWP_RC_OK}

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
    let fh: FileHandle?

    init(from descriptor:Int32) {
        ownsChannel = false
        fh = nil
        super.init(&context.uc)
        init_file_unpack_context(&context, 1024, descriptor)
    }

    init(from url:URL) throws {
        fh = try FileHandle(forReadingFrom: url)
        ownsChannel = true
        super.init(&context.uc)
        init_file_unpack_context(&context, 1024, fh!.fileDescriptor)
    }

    deinit {
        terminate_file_unpack_context(&context)
//        if ownsChannel {close(context.fileDescriptor)}
    }
}
