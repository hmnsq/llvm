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

#அந்தர்பாவ "llvm/Support/LEB128.h"

நாமாகாஶ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t மதிப்பு) {
  unsigned Size = 0;
  செய் {
    மதிப்பு >>= 7;
    Size += அளவு(int8_t);
  } போது (மதிப்பு);
  திரும்பு Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t மதிப்பு) {
  unsigned Size = 0;
  முழு Sign = மதிப்பு >> (8 * அளவு(மதிப்பு) - 1);
  தருக்க IsMore;

  செய் {
    unsigned Byte = மதிப்பு & 0x7f;
    மதிப்பு >>= 7;
    IsMore = மதிப்பு != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += அளவு(int8_t);
  } போது (IsMore);
  திரும்பு Size;
}

}  // namespace llvm
