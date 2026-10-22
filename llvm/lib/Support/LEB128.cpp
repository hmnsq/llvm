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
بینښه get_ULEB128_اندازه(uint64_t ارزښت) {
  بینښه اندازه = 0;
  افعل {
    ارزښت >>= 7;
    اندازه += حجم(int8_t);
  } طالما (ارزښت);
  أرجع اندازه;
}

/// Utility function to get the size of the SLEB128-encoded value.
بینښه get_SLEB128_اندازه(int64_t ارزښت) {
  بینښه اندازه = 0;
  صحيح Sign = ارزښت >> (8 * حجم(ارزښت) - 1);
  منطقي Is_نور;

  افعل {
    بینښه Byte = ارزښت & 0x7f;
    ارزښت >>= 7;
    Is_نور = ارزښت != Sign || ((Byte ^ Sign) & 0x40) != 0;
    اندازه += حجم(int8_t);
  } طالما (Is_نور);
  أرجع اندازه;
}

}  // namespace llvm
