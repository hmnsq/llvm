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

#अन्तर्भाव "llvm/Support/LEB128.h"

नामाकाश llvm {

/// Utility function to get the size of the ULEB128-encoded value.
अचिह्नित get_ULEB128_आकार(uint64_t मानम्) {
  अचिह्नित आकार = 0;
  कुरु {
    मानम् >>= 7;
    आकार += परिमाण(int8_t);
  } यावत् (मानम्);
  प्रत्यागम आकार;
}

/// Utility function to get the size of the SLEB128-encoded value.
अचिह्नित get_SLEB128_आकार(int64_t मानम्) {
  अचिह्नित आकार = 0;
  पूर्णाङ्क Sign = मानम् >> (8 * परिमाण(मानम्) - 1);
  तार्किक Is_अधिकम्;

  कुरु {
    अचिह्नित Byte = मानम् & 0x7f;
    मानम् >>= 7;
    Is_अधिकम् = मानम् != Sign || ((Byte ^ Sign) & 0x40) != 0;
    आकार += परिमाण(int8_t);
  } यावत् (Is_अधिकम्);
  प्रत्यागम आकार;
}

}  // namespace llvm
