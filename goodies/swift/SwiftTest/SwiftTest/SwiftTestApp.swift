//
//  SwiftTestApp.swift
//  SwiftTest
//
//  Created by Claes Wihlborg on 2024-04-07.
//

import SwiftUI

@main
struct SwiftTestApp: App {

    @State var cnt: Int

    var body: some Scene {
        WindowGroup {
            ContentView(cnt: $cnt)
        }
    }

    init() {
        var icnt = 0
        func err(_ msg: String) {
            icnt += 1
            print( "Error: " + msg + "\n")
        }

        do {
            // som file and nil tests
            do {
                let packer = try CWFilePacker(to:"SwiftTest2")
                let im3 = -3
                let cs: String? = nil
                packer + true + 1 + im3
                packer + "Hello" + "World"
                packer + CWNil() + cs + nil
                if !packer.OK {err("In packer")}
            }
                let unpacker = try CWFileUnpacker(from: "SwiftTest2")

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

            // enum tests

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

            let eInt = EInt.second
            let dp = CWDataPacker()
            dp + eInt + EDouble.pi + ECharacter.newLine + EString.alpha + 50 + 2.73 + 0x20 + "omega"

            let du = CWDataUnpacker(from: dp.data)
            try print("\(Int(du))")         // 3
            try print("\(Double(du))")      // 3.14
            try print("\(UInt(du))")        // 10
            try print("\(String(du))")      // alpha
            try print("\(EInt(du))")        // first
            try print("\(EDouble(du))")     // e
            try print("\(ECharacter(du))")  // space
            try print("\(EString(du))")     // omega

        } catch {
            print("\(error)")
            icnt += 10000
        }
        cnt = icnt
    }
}
