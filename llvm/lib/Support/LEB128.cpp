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

#অন্তর্ভাব "llvm/Support/LEB128.h"

নামাকাশ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t মান) {
  unsigned Size = 0;
  কুরু {
    মান >>= 7;
    Size += পরিমাণ(int8_t);
  } যাবত্ (মান);
  প্রত্যাগম Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t মান) {
  unsigned Size = 0;
  পূর্ণাঙ্ক Sign = মান >> (8 * পরিমাণ(মান) - 1);
  তার্কিক IsMore;

  কুরু {
    unsigned Byte = মান & 0x7f;
    মান >>= 7;
    IsMore = মান != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += পরিমাণ(int8_t);
  } যাবত্ (IsMore);
  প্রত্যাগম Size;
}

}  // namespace llvm
