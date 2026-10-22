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

فضاینام llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t مقدار) {
  unsigned Size = 0;
  انجامبده {
    مقدار >>= 7;
    Size += اندازه(int8_t);
  } وقتی (مقدار);
  بازگشت Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t مقدار) {
  unsigned Size = 0;
  صحیح Sign = مقدار >> (8 * اندازه(مقدار) - 1);
  منطقی IsMore;

  انجامبده {
    unsigned Byte = مقدار & 0x7f;
    مقدار >>= 7;
    IsMore = مقدار != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += اندازه(int8_t);
  } وقتی (IsMore);
  بازگشت Size;
}

}  // namespace llvm
