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

#अन्तर्भाव "llvm/Support/LEB128.h"

नामाकाश llvm {

/// Utility function to get the size of the ULEB128-encoded value.
अचिन्हित getULEB128Size(uint64_t मान) {
  अचिन्हित Size = 0;
  गर {
    मान >>= 7;
    Size += परिमाण(int8_t);
  } जबसम्म (मान);
  फर्काऊ Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
अचिन्हित getSLEB128Size(int64_t मान) {
  अचिन्हित Size = 0;
  पूर्णाङ्क Sign = मान >> (8 * परिमाण(मान) - 1);
  तार्किक IsMore;

  गर {
    अचिन्हित Byte = मान & 0x7f;
    मान >>= 7;
    IsMore = मान != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += परिमाण(int8_t);
  } जबसम्म (IsMore);
  फर्काऊ Size;
}

}  // namespace llvm
