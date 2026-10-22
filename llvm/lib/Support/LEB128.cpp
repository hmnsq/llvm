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
بنانشانی get_ULEB128_ناپ(uint64_t قدر) {
  بنانشانی ناپ = 0;
  افعل {
    قدر >>= 7;
    ناپ += حجم(int8_t);
  } طالما (قدر);
  أرجع ناپ;
}

/// Utility function to get the size of the SLEB128-encoded value.
بنانشانی get_SLEB128_ناپ(int64_t قدر) {
  بنانشانی ناپ = 0;
  صحيح Sign = قدر >> (8 * حجم(قدر) - 1);
  منطقي Is_ہور;

  افعل {
    بنانشانی Byte = قدر & 0x7f;
    قدر >>= 7;
    Is_ہور = قدر != Sign || ((Byte ^ Sign) & 0x40) != 0;
    ناپ += حجم(int8_t);
  } طالما (Is_ہور);
  أرجع ناپ;
}

}  // namespace llvm
