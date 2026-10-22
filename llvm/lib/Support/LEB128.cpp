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

#അന്തര്ഭാവ "llvm/Support/LEB128.h"

നാമാകാശ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t മൂല്യം) {
  unsigned Size = 0;
  ചെയ്യുക {
    മൂല്യം >>= 7;
    Size += വലുപ്പം(int8_t);
  } ആകുമ്പോൾ (മൂല്യം);
  തിരികെ Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t മൂല്യം) {
  unsigned Size = 0;
  പൂർണ്ണസംഖ്യ Sign = മൂല്യം >> (8 * വലുപ്പം(മൂല്യം) - 1);
  തർക്കം IsMore;

  ചെയ്യുക {
    unsigned Byte = മൂല്യം & 0x7f;
    മൂല്യം >>= 7;
    IsMore = മൂല്യം != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += വലുപ്പം(int8_t);
  } ആകുമ്പോൾ (IsMore);
  തിരികെ Size;
}

}  // namespace llvm
