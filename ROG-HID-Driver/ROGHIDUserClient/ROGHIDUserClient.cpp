//
//  ROGHIDUserClient.cpp
//  ROG-HID-Driver
//
//  Created by Nick on 2/16/21.
//  Copyright © 2021 Nick. All rights reserved.
//

#include "RogHIDUserClient.h"
#include "ROG_HID_Driver.h"

bool ROGHIDUserClient::init() {
    DBGLOG("Userclient init");

    return super::init();
}

void ROGHIDUserClient::free() {
    DBGLOG("Userclient free");
    super::free();
}

kern_return_t IMPL(ROGHIDUserClient, Start) {
    DBGLOG("Userclient start");
    
    return kIOReturnSuccess;
}

kern_return_t IMPL(ROGHIDUserClient, Stop) {
    DBGLOG("Userclient stop");
    return kIOReturnSuccess;
}

kern_return_t ROGHIDUserClient::ExternalMethod(uint64_t selector, IOUserClientMethodArguments* arguments, const IOUserClientMethodDispatch* dispatch, OSObject* target, void* reference) {
    DBGLOG("Userclient external method");
    return kIOReturnSuccess;
}
