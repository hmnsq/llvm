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
unsigned getULEB128Size(uint64_t درج) {
  unsigned Size = 0;
  افعل {
    درج >>= 7;
    Size += حجم(int8_t);
  } طالما (درج);
  أرجع Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t درج) {
  unsigned Size = 0;
  صحيح Sign = درج >> (8 * حجم(درج) - 1);
  منطقي IsMore;

  افعل {
    unsigned Byte = درج & 0x7f;
    درج >>= 7;
    IsMore = درج != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += حجم(int8_t);
  } طالما (IsMore);
  أرجع Size;
}

}  // namespace llvm
