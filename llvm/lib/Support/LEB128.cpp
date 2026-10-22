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
بنانشان get_ULEB128_سائيز(uint64_t قدر) {
  بنانشان سائيز = 0;
  افعل {
    قدر >>= 7;
    سائيز += حجم(int8_t);
  } طالما (قدر);
  أرجع سائيز;
}

/// Utility function to get the size of the SLEB128-encoded value.
بنانشان get_SLEB128_سائيز(int64_t قدر) {
  بنانشان سائيز = 0;
  صحيح Sign = قدر >> (8 * حجم(قدر) - 1);
  منطقي Is_وڌيڪ;

  افعل {
    بنانشان Byte = قدر & 0x7f;
    قدر >>= 7;
    Is_وڌيڪ = قدر != Sign || ((Byte ^ Sign) & 0x40) != 0;
    سائيز += حجم(int8_t);
  } طالما (Is_وڌيڪ);
  أرجع سائيز;
}

}  // namespace llvm
