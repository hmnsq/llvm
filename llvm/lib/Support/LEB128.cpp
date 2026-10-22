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

#ضمن "llvm/Support/LEB128.h"

نطاق llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t نيلاي) {
  unsigned Size = 0;
  افعل {
    نيلاي >>= 7;
    Size += حجم(int8_t);
  } طالما (نيلاي);
  أرجع Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t نيلاي) {
  unsigned Size = 0;
  صحيح Sign = نيلاي >> (8 * حجم(نيلاي) - 1);
  منطقي IsMore;

  افعل {
    unsigned Byte = نيلاي & 0x7f;
    نيلاي >>= 7;
    IsMore = نيلاي != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += حجم(int8_t);
  } طالما (IsMore);
  أرجع Size;
}

}  // namespace llvm
