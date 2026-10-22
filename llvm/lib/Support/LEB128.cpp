//===- LEB128.cpp - LEB128 utility functions implementation -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements some utility functions for encoding SLEB128 and
// ULEB128 values.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/LEB128.h"

نامخلا llvm {

/// Utility function to get the size of the ULEB128-encoded value.
غیرعلامتی get_ULEB128_سائز(uint64_t قدر) {
  غیرعلامتی سائز = 0;
  کرو {
    قدر >>= 7;
    سائز += حجم(int8_t);
  } جب (قدر);
  واپس سائز;
}

/// Utility function to get the size of the SLEB128-encoded value.
غیرعلامتی get_SLEB128_سائز(int64_t قدر) {
  غیرعلامتی سائز = 0;
  صحیح Sign = قدر >> (8 * حجم(قدر) - 1);
  منطقی Is_مزید;

  کرو {
    غیرعلامتی Byte = قدر & 0x7f;
    قدر >>= 7;
    Is_مزید = قدر != Sign || ((Byte ^ Sign) & 0x40) != 0;
    سائز += حجم(int8_t);
  } جب (Is_مزید);
  واپس سائز;
}

}  // namespace llvm
