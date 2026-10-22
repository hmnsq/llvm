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

#ಅನ್ತರ್ಭಾವ "llvm/Support/LEB128.h"

ನಾಮಾಕಾಶ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
unsigned getULEB128Size(uint64_t ಮೌಲ್ಯ) {
  unsigned Size = 0;
  ಕುರು {
    ಮೌಲ್ಯ >>= 7;
    Size += ಪರಿಮಾಣ(int8_t);
  } ಯಾವತ್ (ಮೌಲ್ಯ);
  ಪ್ರತ್ಯಾಗಮ Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
unsigned getSLEB128Size(int64_t ಮೌಲ್ಯ) {
  unsigned Size = 0;
  ಪೂರ್ಣಾಙ್ಕ Sign = ಮೌಲ್ಯ >> (8 * ಪರಿಮಾಣ(ಮೌಲ್ಯ) - 1);
  ತಾರ್ಕಿಕ IsMore;

  ಕುರು {
    unsigned Byte = ಮೌಲ್ಯ & 0x7f;
    ಮೌಲ್ಯ >>= 7;
    IsMore = ಮೌಲ್ಯ != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += ಪರಿಮಾಣ(int8_t);
  } ಯಾವತ್ (IsMore);
  ಪ್ರತ್ಯಾಗಮ Size;
}

}  // namespace llvm
