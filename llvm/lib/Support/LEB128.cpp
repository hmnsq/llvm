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
unsigned getULEB128Size(uint64_t Value) {
  unsigned Size = 0;
  עשה {
    Value >>= 7;
    Size += حجم(int8_t);
  } כלעוד (Value);
  החזר Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t Value) {
  unsigned Size = 0;
  صحيح Sign = Value >> (8 * حجم(Value) - 1);
  منطقي IsMore;

  עשה {
    unsigned Byte = Value & 0x7f;
    Value >>= 7;
    IsMore = Value != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += حجم(int8_t);
  } כלעוד (IsMore);
  החזר Size;
}

}  // namespace llvm
