//
//  CG+CWPackable.swift
//  CWPack
//
//  Created by Claes Wihlborg on 2023-03-13.
//  Copyright © 2023 Claes Wihlborg. All rights reserved.
//

import Foundation


// MARK: ----------------------------------------------- Core graphics type extensions

extension CGFloat: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + Double(self)
    }

    init (_ unpacker: CWUnpacker) throws {
        self = try CGFloat(Double(unpacker))
    }
}

extension CGPoint: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(2)
        packer + x + y
    }

    init (_ unpacker: CWUnpacker) throws {
        guard try ArrayHeader(unpacker).count == 2 else {throw CWPackError.unpackerError("CGPoint")}
        self = try CGPoint(x: CGFloat(unpacker), y: CGFloat(unpacker))
    }
}

extension CGSize: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(2)
        packer + width + height
    }

    init (_ unpacker: CWUnpacker) throws {
        guard try ArrayHeader(unpacker).count == 2 else {throw CWPackError.unpackerError("CGSize")}
        self = try CGSize(width: CGFloat(unpacker), height: CGFloat(unpacker))
    }
}

extension CGRect: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(4)
        packer + origin.x + origin.y
        packer + size.width + size.height
    }

    init (_ unpacker: CWUnpacker) throws {
        guard try ArrayHeader(unpacker).count == 4 else {throw CWPackError.unpackerError("CGRect")}
        self = try CGRect(x: CGFloat(unpacker), y: CGFloat(unpacker), width: CGFloat(unpacker), height: CGFloat(unpacker))
    }
}

extension CGVector: CWPackable {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(2)
        packer + dx + dy
    }

    init (_ unpacker: CWUnpacker) throws {
        guard try ArrayHeader(unpacker).count == 2 else {throw CWPackError.unpackerError("CGVector")}
        self = try CGVector(dx: CGFloat(unpacker), dy: CGFloat(unpacker))
    }
}

extension CGAffineTransform {
    func cwPack(_ packer: CWPacker) {
        packer + ArrayHeader(6)
        packer + a + b + c + d
        packer + tx + ty
    }

    init (_ unpacker: CWUnpacker) throws {
        guard try ArrayHeader(unpacker).count == 5 else {throw CWPackError.unpackerError("CGAffineTransform")}
        self = try CGAffineTransform(CGFloat(unpacker), CGFloat(unpacker), CGFloat(unpacker), CGFloat(unpacker), CGFloat(unpacker), CGFloat(unpacker))
    }
}
