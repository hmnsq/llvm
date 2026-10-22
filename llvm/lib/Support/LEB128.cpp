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
अचिन्हित getULEB128Size(uint64_t मूल्य) {
  अचिन्हित Size = 0;
  करा {
    मूल्य >>= 7;
    Size += आकार(int8_t);
  } पर्यंत (मूल्य);
  परत Size;
}

/// Utility function to get the size of the SLEB128-encoded value.
अचिन्हित getSLEB128Size(int64_t मूल्य) {
  अचिन्हित Size = 0;
  पूर्णांक Sign = मूल्य >> (8 * आकार(मूल्य) - 1);
  तार्किक IsMore;

  करा {
    अचिन्हित Byte = मूल्य & 0x7f;
    मूल्य >>= 7;
    IsMore = मूल्य != Sign || ((Byte ^ Sign) & 0x40) != 0;
    Size += आकार(int8_t);
  } पर्यंत (IsMore);
  परत Size;
}

}  // namespace llvm
