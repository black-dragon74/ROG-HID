//
//  ROG_HID_Driver.cpp
//  ROG-HID-Driver
//
//  Created by Nick on 7/20/20.
//  Copyright © 2020 Nick. All rights reserved.
//

#include <os/log.h>

#include <HIDDriverKit/HIDDriverKit.h>

#include "ROG_HID_Driver.h"
#include "HIDUsageTables.h"
#include "IOBufferMemoryDescriptorUtility.h"

#undef super
#define super IOUserHIDEventDriver

struct ROG_HID_Driver_IVars
{
    IOHIDInterface* hid_interface       { nullptr };
    OSArray* customKeyboardElements     { nullptr };
    uint8_t kbdLux                      { 3 };
    uint8_t kbdFunction                 { 0 };
};

#define _hid_interface              ivars->hid_interface
#define _custom_keyboard_elements   ivars->customKeyboardElements
#define _current_lux                ivars->kbdLux
#define _kbd_function               ivars->kbdFunction

bool ROG_HID_Driver::init()
{
    DBGLOG("Init");
    
    if (!super::init())
        return false;
    
    ivars = IONewZero(ROG_HID_Driver_IVars, 1);
    
    if (ivars == nullptr)
        return false;
    
    _current_lux = 3;
    
    return true;
}

kern_return_t IMPL(ROG_HID_Driver, Start)
{
    DBGLOG("Start");
    kern_return_t ret;
    ret = Start(provider, SUPERDISPATCH);
    
    if (ret != kIOReturnSuccess)
    {
        OSLOG("Start failed %d", ret);
        return ret;
    }
    
    IOHIDInterface* interface = OSDynamicCast(IOHIDInterface, provider);
    if (!interface)
    {
        OSLOG("Failure in casting provider to IOHIDInterface");
        return kIOReturnError;
    }
    
    _hid_interface = interface;

    OSArray *elements = _hid_interface->getElements();
    elements->retain();
    if (elements)
        parseKeyboardElementsHook(elements);
    elements->release();

    // Now we init the keyboard
    asusKbdInit();
    
    // Read the keyboard functions
    asusKbdGetFunctions();
    
    // Reflect the intital value on lux
    DBGLOG("Trying to set initial kbd lux to 0x%x", _current_lux);
    asusKbdBacklightSet(_current_lux);

    // And register ourselves with the system
    DBGLOG("Register service");
    RegisterService();
    
    return ret;
}

void IMPL(ROG_HID_Driver, parseKeyboardElementsHook)
{
    DBGLOG("Parse keyboard element hook called");
    _custom_keyboard_elements = OSArray::withCapacity(4);
    uint32_t count, index;
    for (index = 0, count = elements->getCount(); index < count; index++)
    {
        IOHIDElement* element = OSDynamicCast(IOHIDElement, elements->getObject(index));
        if (!element || element->getUsage() == 0)
            continue;
        if (element->getType() == kIOHIDElementTypeCollection)
            continue;

        uint32_t usage, usagePage;
        usage = element->getUsage();
        usagePage = element->getUsagePage();
        bool store = false;

        switch (usagePage)
        {
            case kHIDPage_AsusVendor:
                switch (usage)
                {
                    case kHIDUsage_AsusVendor_BrightnessUp:
                    case kHIDUsage_AsusVendor_BrightnessDown:
                    case kHIDUsage_AsusVendor_DisplayOff:
                    case kHIDUsage_AsusVendor_ROG:
                    case kHIDUsage_AsusVendor_Power4Gear:
                    case kHIDUsage_AsusVendor_TouchpadToggle:
                    case kHIDUsage_AsusVendor_Sleep:
                    case kHIDUsage_AsusVendor_MicMute:
                    case kHIDUsage_AsusVendor_Camera:
                    case kHIDUsage_AsusVendor_RFKill:
                    case kHIDUsage_AsusVendor_Fan:
                    case kHIDUsage_AsusVendor_Calc:
                    case kHIDUsage_AsusVendor_Splendid:
                    case kHIDUsage_AsusVendor_IlluminationUp:
                    case kHIDUsage_AsusVendor_IlluminationDown:
                        store = true;
                        break;
                }
                break;
            case kHIDPage_MicrosoftVendor:
                switch (usage)
                {
                    case kHIDUsage_MicrosoftVendor_WLAN:
                    case kHIDUsage_MicrosoftVendor_BrightnessDown:
                    case kHIDUsage_MicrosoftVendor_BrightnessUp:
                    case kHIDUsage_MicrosoftVendor_DisplayOff:
                    case kHIDUsage_MicrosoftVendor_Camera:
                    case kHIDUsage_MicrosoftVendor_ROG:
                        store = true;
                        break;
                }
                break;
        }
        if (store)
            _custom_keyboard_elements->setObject(element);
    }
}

void ROG_HID_Driver::handleReport(uint64_t timestamp, uint8_t *report, uint32_t reportLength, IOHIDReportType type, uint32_t reportID)
{
    // We will just handle the keyboard related reports and ignore the rest (pointer, digitizer etc.)
    handleKeyboardReport(timestamp, reportID);
}

void ROG_HID_Driver::handleKeyboardReport(uint64_t timestamp, uint32_t reportID)
{
    for (unsigned int i = 0; i < _custom_keyboard_elements->getCount(); i++)
    {
        IOHIDElement* element { nullptr };
        uint64_t elementTimeStamp;
        uint32_t usagePage, usage, value, preValue;

        element = OSDynamicCast(IOHIDElement, _custom_keyboard_elements->getObject(i));

        if (!element || element->getReportID() != reportID)
            continue;

        elementTimeStamp = element->getTimeStamp();
        if (timestamp != elementTimeStamp)
            continue;
        
        usagePage = element->getUsagePage();
        usage = element->getUsage();

        preValue = element->getValue(kIOHIDValueOptionsFlagPrevious) != 0;
        
        // Fix for double reports of KBD illumination
        if (usagePage == kHIDPage_AsusVendor && (usage == kHIDUsage_AsusVendor_IlluminationUp || usage == kHIDUsage_AsusVendor_IlluminationDown))
        {
            value = element->getValue(kIOHIDValueOptionsFlagRelativeSimple) != 0;
        }
        else
        {
            value = element->getValue(0) != 0;
        }

        if (value == preValue)
            continue;
        
        DBGLOG("Handle Key Report - usage: 0x%x, page: 0x%x, val: 0x%x", usage, usagePage, value);
        dispatchKeyboardEvent(timestamp, usagePage, usage, value, 0, true);
    }
    
    // Pass the rest of the reports to super for handling
    super::handleKeyboardReport(timestamp, reportID);
}

kern_return_t ROG_HID_Driver::dispatchKeyboardEvent(uint64_t timeStamp, uint32_t usagePage, uint32_t usage, uint32_t value, IOOptionBits options, bool repeat)
{
    if (usagePage == kHIDPage_AsusVendor)
    {
        switch (usage) {
            case kHIDUsage_AsusVendor_BrightnessDown:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessDown;
                break;
            case kHIDUsage_AsusVendor_BrightnessUp:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessUp;
                break;
            case kHIDUsage_AsusVendor_IlluminationDown:
                setKbdLux(luxDown);
                break;
            case kHIDUsage_AsusVendor_IlluminationUp:
                setKbdLux(luxUp);
                break;
        }
    }
    if (usagePage == kHIDPage_MicrosoftVendor)
    {
        switch (usage) {
            case kHIDUsage_MicrosoftVendor_BrightnessDown:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessDown;
                break;
            case kHIDUsage_MicrosoftVendor_BrightnessUp:
                usagePage = kHIDPage_AppleVendorTopCase;
                usage = kHIDUsage_AV_TopCase_BrightnessUp;
                break;
        }
    }
    
    // Fix erratic caps lock key
    if (usage == kHIDUsage_KeyboardCapsLock)
        IOSleep(80);
    
    DBGLOG("Dispatch Event - usage: 0x%x, page: 0x%x, val: 0x%x", usage, usagePage, value);
    return super::dispatchKeyboardEvent(timeStamp, usagePage, usage, value, options, repeat);
}

void IMPL(ROG_HID_Driver, setKbdLux)
{
    if (luxEvent == luxUp)
    {
        if (_current_lux == 3)
            return;
        asusKbdBacklightSet(++_current_lux);
        return;
    }
    
    if (luxEvent == luxDown)
    {
        if (_current_lux == 0)
            return;
        asusKbdBacklightSet(--_current_lux);
        return;
    }
}

kern_return_t IMPL(ROG_HID_Driver, Stop)
{
    DBGLOG("Stop");
    return Stop(provider, SUPERDISPATCH);
}

void ROG_HID_Driver::free()
{
    DBGLOG("Free");
    if (ivars != nullptr)
    {
        OSSafeReleaseNULL(_hid_interface);
        OSSafeReleaseNULL(_custom_keyboard_elements);
    }
    
    IOSafeDeleteNULL(ivars, ROG_HID_Driver_IVars, 1);
    super::free();
}

#pragma mark
#pragma mark Ported from hid-asus.c
#pragma mark

void IMPL(ROG_HID_Driver, asusKbdInit)
{
    DBGLOG("Sending keyboard init report");
    uint8_t buf[] = { KBD_FEATURE_REPORT_ID, 0x41, 0x53, 0x55, 0x53, 0x20, 0x54, 0x65, 0x63, 0x68, 0x2e, 0x49, 0x6e, 0x63, 0x2e, 0x00 };
    OSData* data = OSData::withBytes(buf, KBD_FEATURE_REPORT_SIZE);
    IOMemoryDescriptor* report = nullptr;
    auto kr = IOBufferMemoryDescriptorUtility::createWithBytes(data->getBytesNoCopy(0, KBD_FEATURE_REPORT_SIZE), data->getLength(), &report);
    if (kr != kIOReturnSuccess)
    {
        OSLOG("Error allocating memory for init report");
        report->release();
        data->release();
        return;
    }
    _hid_interface->SetReport(report, kIOHIDReportTypeFeature, KBD_FEATURE_REPORT_ID, 0);

    data->release();
    report->release();
}

void IMPL(ROG_HID_Driver, asusKbdBacklightSet)
{
    if (!(SUPPORT_KEYBOARD_BACKLIGHT & _kbd_function)) {
        DBGLOG("Setting keyboard baclikght is not supported on this device");
        return;
    }
    
    DBGLOG("Setting keyboard Lux to: %d", val);
    uint8_t buf[] = { KBD_FEATURE_REPORT_ID, 0xba, 0xc5, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    buf[4] = val;
    OSData* data = OSData::withBytes(buf, KBD_FEATURE_REPORT_SIZE);
    IOMemoryDescriptor* report;
    auto ret = IOBufferMemoryDescriptorUtility::createWithBytes(data->getBytesNoCopy(0, KBD_FEATURE_REPORT_SIZE), data->getLength(), &report);
    if (ret != kIOReturnSuccess)
    {
        OSLOG("Error allocating memory for backlightSet report");
        report->release();
        data->release();
        return;
    }
    _hid_interface->SetReport(report, kIOHIDReportTypeFeature, KBD_FEATURE_REPORT_ID, 0);

    data->release();
    report->release();
}

void IMPL(ROG_HID_Driver, asusKbdGetFunctions)
{
    DBGLOG("Getting keyboard functions");
    uint8_t buf[] = { KBD_FEATURE_REPORT_ID, 0x05, 0x20, 0x31, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    OSData* data = OSData::withBytes(buf, KBD_FEATURE_REPORT_SIZE);
    uint8_t readBuffer[KBD_FEATURE_REPORT_SIZE] {};
    
    IOMemoryDescriptor* report;
    auto ret = IOBufferMemoryDescriptorUtility::createWithBytes(data->getBytesNoCopy(0, KBD_FEATURE_REPORT_SIZE), data->getLength(), &report);
    
    if (ret != kIOReturnSuccess) {
        OSLOG("Error creating report for kbd feature");
        goto exit;
    }
    
    _hid_interface->SetReport(report, kIOHIDReportTypeFeature, KBD_FEATURE_REPORT_ID, 0);
    _hid_interface->GetReport(report, kIOHIDReportTypeFeature, KBD_FEATURE_REPORT_ID, 0);
    
    ret = IOBufferMemoryDescriptorUtility::readBytes(readBuffer, KBD_FEATURE_REPORT_SIZE, &report);
    
    if (ret != kIOReturnSuccess) {
        OSLOG("Error in reading feature report");
        goto exit;
    }
    
    _kbd_function = readBuffer[6];
    DBGLOG("Keyboard feature report is %02x", _kbd_function);
    
exit:
    report->release();
    data->release();
    return;
}
