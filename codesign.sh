#!/bin/bash

#  codesign.sh
#  ROG-HID
#
#  Created by Nick on 7/20/20.
#  Copyright © 2020 Nick. All rights reserved.

readonly CODE_SIGN_IDENTITY=2FB6A1000899D1646EB5ECA45606F1522A05EFF9
echo "Using signing identity: $CODE_SIGN_IDENTITY"

security  unlock-keychain -p '{}' /Users/${USER}/Library/Keychains/login.keychain

echo "Signing Dext..."
codesign --sign $CODE_SIGN_IDENTITY \
    --entitlements ROG-HID-Driver/ROG_HID_Driver.entitlements \
    --options runtime --verbose --force \
    build/Release/ROG-HID.app/Contents/Library/SystemExtensions/com.black-dragon74.ROG-HID-Driver.dext &>/dev/null || exit

echo "Verifying Dext..."
codesign --verify --verbose \
    build/Release/ROG-HID.app/Contents/Library/SystemExtensions/com.black-dragon74.ROG-HID-Driver.dext &>/dev/null || exit

echo "Signing App..."
codesign --sign $CODE_SIGN_IDENTITY \
    --entitlements ROG-HID/ROG_HID.entitlements \
    --options runtime --verbose --force \
    build/Release/ROG-HID.app &>/dev/null || exit

echo "Verifying App..."
codesign \
    --verify --verbose \
    build/Release/ROG-HID.app &>/dev/null || exit

echo
echo "** SIGNING SUCCEEDED **"
echo
