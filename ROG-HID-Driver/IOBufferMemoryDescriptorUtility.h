//
//  IOBufferMemoryDescriptorUtility.h
//  ROG-HID-Driver
//
//  Created by Nick on 7/20/20.
//  Copyright © 2020 Nick. All rights reserved.
//

#ifndef IOBufferMemoryDescriptorUtility_h
#define IOBufferMemoryDescriptorUtility_h

#pragma once

#include <DriverKit/DriverKit.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#include <DriverKit/OSCollections.h>

namespace IOBufferMemoryDescriptorUtility {

inline kern_return_t createWithBytes(const void* bytes, size_t length, IOMemoryDescriptor** memory) {
  if (!bytes || !memory) {
    return kIOReturnBadArgument;
  }

  IOBufferMemoryDescriptor* m = nullptr;
  auto kr = IOBufferMemoryDescriptor::Create(kIOMemoryDirectionInOut, length, 0, &m);
  if (kr == kIOReturnSuccess) {
    uint64_t address;
    uint64_t len;
    m->Map(0, 0, 0, 0, &address, &len);

    if (length != len) {
      kr = kIOReturnNoMemory;
      goto error;
    }

    memcpy(reinterpret_cast<void*>(address), bytes, length);
  }

  *memory = m;

  return kr;

error:
  if (m) {
    OSSafeReleaseNULL(m);
  }

  return kr;
}

} // namespace IOBufferMemoryDescriptorUtility

#endif /* IOBufferMemoryDescriptorUtility_h */
