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

#include "llvm/Support/LEB128.h"

namespace llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t हालागा) {
  unsigned Size = 0;
  do {
    हालागा >>= 7;
    Size += sizeof(int8_t);
  } while (हालागा);
  return Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t हालागा) {
  unsigned Size = 0;
  int Sign = हालागा >> (8 * sizeof(हालागा) - 1);
  bool IsMore;

  do {
    unsigned Byte = हालागा & 0x7f;
    हालागा >>= 7;
    IsMore = हालागा != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += sizeof(int8_t);
  } while (IsMore);
  return Size;
}

}  // namespace llvm
