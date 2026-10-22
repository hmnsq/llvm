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

#समावेश "llvm/Support/LEB128.h"

namespace llvm {

/// Utility function to get the size of the ULEB128-encoded value.
अचिन्हित get_ULEB128_आकार(uint64_t मूल्य) {
  अचिन्हित आकार = 0;
  करो {
    मूल्य >>= 7;
    आकार += माप(int8_t);
  } जबतक (मूल्य);
  वापस आकार;
}

/// Utility function to get the size of the SLEB128-encoded value.
अचिन्हित get_SLEB128_आकार(int64_t मूल्य) {
  अचिन्हित आकार = 0;
  पूर्णांक Sign = मूल्य >> (8 * माप(मूल्य) - 1);
  bool IsMore;

  करो {
    अचिन्हित Byte = मूल्य & 0x7f;
    मूल्य >>= 7;
    IsMore = मूल्य != Sign || ((Byte ^ Sign) & 0x40) != 0;
    आकार += माप(int8_t);
  } जबतक (IsMore);
  वापस आकार;
}

}  // namespace llvm
