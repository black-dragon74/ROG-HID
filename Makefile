# vim: noexpandtab tabstop=8


.PHONY: build

all:

build: clean
	xcodebuild -configuration Release SYMROOT="$(CURDIR)/build"
	./codesign.sh

clean:
	rm -rf build

xcode:
	open ROG-HID.xcodeproj

open: 
	open build/Release/ROG-HID.app

install:
	rsync -a --delete build/Release/ROG-HID.app /Applications

run: install
	open /Applications/ROG-HID.app

systemextensionsctl-list:
	systemextensionsctl list

log-show-extension-manager:
	log show --predicate 'sender == "sysextd" or sender CONTAINS "com.black-dragon74.ROG-HID-Driver"' --info --debug --last 1h

