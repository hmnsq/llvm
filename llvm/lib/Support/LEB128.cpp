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
unsigned getULEB128Size(uint64_t بەها) {
  unsigned Size = 0;
  افعل {
    بەها >>= 7;
    Size += حجم(int8_t);
  } طالما (بەها);
  أرجع Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t بەها) {
  unsigned Size = 0;
  صحيح Sign = بەها >> (8 * حجم(بەها) - 1);
  منطقي IsMore;

  افعل {
    unsigned Byte = بەها & 0x7f;
    بەها >>= 7;
    IsMore = بەها != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += حجم(int8_t);
  } طالما (IsMore);
  أرجع Size;
}

}  // namespace llvm
