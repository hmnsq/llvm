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
unsigned get_ULEB128_আকার(uint64_t মান) {
  unsigned আকার_2 = 0;
  করো {
    মান >>= 7;
    আকার_2 += আকার(int8_t);
  } যখন (মান);
  ফেরত আকার_2;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned get_SLEB128_আকার(int64_t মান) {
  unsigned আকার_2 = 0;
  পূর্ণসংখ্যা Sign = মান >> (8 * আকার(মান) - 1);
  যৌক্তিক IsMore;

  করো {
    unsigned Byte = মান & 0x7f;
    মান >>= 7;
    IsMore = মান != Sign || ((Byte ^ Sign) & 0x40) != 0;
    আকার_2 += আকার(int8_t);
  } যখন (IsMore);
  ফেরত আকার_2;
}

}  // namespace llvm
