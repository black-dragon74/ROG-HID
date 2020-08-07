# ROG-HID

(c) 2020 black.dragon74 aka Nick

macOS HID driver based on DriverKit for Asus ROG notebook built-in keyboard.  Tested on 0b05:1869. Should support 0b05:1866, 0b05:1854, 0b05:1837, 0b05:1822.

For enhanced functionality, use with [ROGSwitch](https://github.com/black-dragon74/ROGSwitch).

## Features
- Supports setting display brightness using Fn Keys
- Supports setting keyboard backlight brightness using Fn Keys
- Fixes erratic behavior of CAPS Lock key on keyboards with CAPS Lock LED.
- Automatically turns off keyboard backlight after 5 minutes of inactivity

Many features such as touchpad toggle, display off, sleep and airplane mode require use of IOKit. Currently, there is no documentation of interfacing DriverKit with IOKit. These features are on my TODO and will be implemented in a future release.

## Installing

Download the appropriate version from the [Releases](https://github.com/black-dragon74/ROG-HID/releases) and copy `ROG-HID.app` to `/Applications` folder.

Now open the `ROG-HID.app` from Launchpad, click on `Activate ROG-HID Extension` and follow the instructions.

The driver won't attach right away as DriverKit drivers do not attach to pre-enumerated devices. You can either reboot or use the `reenumerate` utility found inside Utils folder. To use this utlity, you need to know the VendorID and ProductID of your Keyboard. You can re-enumerate a USB device like:

```sh
#for vendor-id 0b05 and product-id 1869
reenumerate -v 0x0b05,0x1869
```

<details>
  <summary>Build sign and install by yourself</summary>
    
  In order to build and use this driver, make sure your SIP is disabled and you have a free Apple developer account along with Xcode.

  Then, you need to change the `codesign.sh` file to reflect your own developer identity. Follow the steps below to find and update your developer identity.

  ```sh
  # Find the code signing identity
  security find-identity -p codesigning -v
  ```

  Copy the identity you get and then open the `codesign.sh` file. Replace the existing identity with the new one.

  Now we need to enable DriverKit development mode. Run `systemextensionsctl developer on` in Terminal.

  Now run the following commands in Terminal to build and install.

  ```sh
  make
  make install
  ```
    
</details>

## Configuring

You can enable or disable additonal features and fixes in this driver by editing the Info.plist file of the dext. Currently configurable options are:

Setting value to `0` disables and `1` enables the feature/fix.

- `FixCapsLockLED` - Fixes erratic behavior of Caps Lock LED on some keyboards. If your LED didn't work under macOS without this driver. You should disable this feature.
- `BacklightAutoTurnOff` - Automatically turns off the keyboard backlight after 5 mins of inactivity. Recommended to disable it on devices where there are multiple `IOHIDInterface` nubs for a single device, like `0b05:1866`.

You need to edit ONLY the entry for your vendor and device id. Like for `0b05:1869` I would edit `0b05_1869` entry under `IOKitPersonalities` dict.

## Building

### From GitHub:

Install Xcode, clone the GitHub repo and enter the top-level directory.  Then:

```sh
make build
```

## Credits

- Apple for no documentation of DriverKit
- Linux for hid-asus.c

