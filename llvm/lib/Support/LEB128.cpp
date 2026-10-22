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

#અન્તર્ભાવ "llvm/Support/LEB128.h"

નામાકાશ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t કિંમત) {
  unsigned Size = 0;
  કરો {
    કિંમત >>= 7;
    Size += કદ(int8_t);
  } જ્યારે (કિંમત);
  પાછા Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t કિંમત) {
  unsigned Size = 0;
  પૂર્ણાંક Sign = કિંમત >> (8 * કદ(કિંમત) - 1);
  તાર્કિક IsMore;

  કરો {
    unsigned Byte = કિંમત & 0x7f;
    કિંમત >>= 7;
    IsMore = કિંમત != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += કદ(int8_t);
  } જ્યારે (IsMore);
  પાછા Size;
}

}  // namespace llvm
