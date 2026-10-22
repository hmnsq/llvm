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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/LEB128.h"

ନାମାକାଶ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t ମୂଲ୍ୟ) {
  unsigned Size = 0;
  କର {
    ମୂଲ୍ୟ >>= 7;
    Size += ଆକାର(int8_t);
  } ଯେତେବେଳେ (ମୂଲ୍ୟ);
  ଫେରନ୍ତୁ Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t ମୂଲ୍ୟ) {
  unsigned Size = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Sign = ମୂଲ୍ୟ >> (8 * ଆକାର(ମୂଲ୍ୟ) - 1);
  ତାର୍କିକ IsMore;

  କର {
    unsigned Byte = ମୂଲ୍ୟ & 0x7f;
    ମୂଲ୍ୟ >>= 7;
    IsMore = ମୂଲ୍ୟ != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += ଆକାର(int8_t);
  } ଯେତେବେଳେ (IsMore);
  ଫେରନ୍ତୁ Size;
}

}  // namespace llvm
