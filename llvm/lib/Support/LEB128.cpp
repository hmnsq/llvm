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
بیعلامت get_ULEB128_اندازه(uint64_t مقدار) {
  بیعلامت اندازه = 0;
  افعل {
    مقدار >>= 7;
    اندازه += حجم(int8_t);
  } طالما (مقدار);
  أرجع اندازه;
}

/// Utility function to get the size of the SLEB128-encoded value.
بیعلامت get_SLEB128_اندازه(int64_t مقدار) {
  بیعلامت اندازه = 0;
  صحيح Sign = مقدار >> (8 * حجم(مقدار) - 1);
  منطقي Is_بیشتر;

  افعل {
    بیعلامت Byte = مقدار & 0x7f;
    مقدار >>= 7;
    Is_بیشتر = مقدار != Sign || ((Byte ^ Sign) & 0x40) != 0;
    اندازه += حجم(int8_t);
  } طالما (Is_بیشتر);
  أرجع اندازه;
}

}  // namespace llvm
