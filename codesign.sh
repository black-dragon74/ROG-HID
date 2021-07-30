#!/bin/bash

#  codesign.sh
#  ROG-HID
#
#  Created by Nick on 7/20/20.
#  Copyright © 2020 Nick. All rights reserved.

readonly CODE_SIGN_IDENTITY=414EFE82A4174FBF050EC0BF31A3F9A191A4B228

flavor="Release"

if [[ $1 == "Debug" ]];
then
    echo "Using debug flavor..."
    flavor="Debug"
fi

echo "Using signing identity: $CODE_SIGN_IDENTITY"

security  unlock-keychain -p '{}' /Users/${USER}/Library/Keychains/login.keychain

echo "Signing Dext..."
codesign --sign $CODE_SIGN_IDENTITY \
    --entitlements ROG-HID-Driver/ROG_HID_Driver.entitlements \
    --options runtime --verbose --force \
    build/$flavor/ROG-HID.app/Contents/Library/SystemExtensions/com.black-dragon74.ROG-HID-Driver.dext || exit

echo "Verifying Dext..."
codesign --verify --verbose \
    build/$flavor/ROG-HID.app/Contents/Library/SystemExtensions/com.black-dragon74.ROG-HID-Driver.dext || exit

echo "Signing App..."
codesign --sign $CODE_SIGN_IDENTITY \
    --entitlements ROG-HID/ROG_HID.entitlements \
    --options runtime --verbose --force \
    build/$flavor/ROG-HID.app || exit

echo "Verifying App..."
codesign \
    --verify --verbose \
    build/$flavor/ROG-HID.app || exit

echo
echo "** SIGNING SUCCEEDED **"
echo
