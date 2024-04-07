//
//  ContentView.swift
//  SwiftTest
//
//  Created by Claes Wihlborg on 2024-04-07.
//

import SwiftUI

struct ContentView: View {
    @State var cnt: Int
    var body: some View {
        VStack {
            Image(systemName: "globe")
                .imageScale(.large)
                .foregroundStyle(.tint)
            Text("Test finished!")
            HStack {
                Spacer()
                if cnt == 0 {Text("No errors")}
                else if cnt == 1 {Text("One error")}
                else {Text("\(cnt) errors")}
                Text("detected")
                Spacer()
            }
        }
        .padding()
    }
}

#Preview {
    ContentView(cnt:1)
}
