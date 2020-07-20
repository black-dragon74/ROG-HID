//
//  ContentView.swift
//  ROG-HID
//
//  Created by Nick on 7/20/20.
//  Copyright © 2020 Nick. All rights reserved.
//

import SwiftUI
import SystemExtensions

struct ContentView : View {
    var body: some View {
        VStack {
            Text("Welcome to ROG-HID Extension Manager")
                .font(.headline)
            Spacer()
            HStack {
                Button(action: ExtensionManager.shared.activate) {
                    Text("Activate ROG-HID Extension")
                }
                Button(action: ExtensionManager.shared.deactivate) {
                    Text("Deactivate ROG_HID Extension")
                }
            }
        }
        .frame(width: 400, height: 150)
        .padding(20)
    }
}


#if DEBUG
struct ContentView_Previews : PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
#endif
