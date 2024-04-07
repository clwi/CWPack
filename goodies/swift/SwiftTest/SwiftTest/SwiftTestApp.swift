//
//  SwiftTestApp.swift
//  SwiftTest
//
//  Created by Claes Wihlborg on 2024-04-07.
//

import SwiftUI

@main
struct SwiftTestApp: App {

    @State var cnt: Int = 0

    var body: some Scene {
        WindowGroup {
            ContentView(cnt: cnt)
        }
    }

    func err(_ msg: String) {
        cnt += 1
        print( "Error: " + msg + "\n")
    }

    init() {
        let packer = CWDataPacker()
        let im3 = -3
        let cs: String? = nil
        packer + true + 1 + im3
        packer + "Hello" + "World"
        packer + CWNil() + cs + nil
        if !packer.OK {err("In packer")}

        do {
            let unpacker = CWDataUnpacker(from: packer.data)

            let b:Bool = try Bool(unpacker)
            if !b {err("b:Bool = Bool(unpacker)")}

            let u:UInt = try UInt(unpacker)
            if u != 1 {err("u:UInt = UInt(unpacker)")}

            var i: Int = 0
            try unpacker - i
            if i != -3 {err("packer - i")}

            var s: String?
            s = try String(unpacker)
            if s! != "Hello" {err("String(unpacker) != Hello")}

            s = try String(unpacker)
            if s! != "World" {err("String(unpacker) != World")}

            s = try String(optional: unpacker)
            if s != nil {err("String(unpacker) != nil")}

            let n: CWNil? = try CWNil(unpacker)
            if n == nil {err("CWNil(unpacker) == nil")}

            let n1: CWNil? = try CWNil(optional: unpacker)
            if n1 != nil {err("CWNil(unpacker) != nil")}

            if !unpacker.OK {err("!unpacker.OK")}
            else {print("Test passed \n")}

        } catch {
            err( "\(error)" )
        }
    }
}
