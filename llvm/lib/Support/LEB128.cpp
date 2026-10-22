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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/LEB128.h"

ਨਾਮਾਕਾਸ਼ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
ਬਿਨਾਨਿਸ਼ਾਨੀ get_ULEB128_ਮਾਪ(uint64_t ਮੁੱਲ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = 0;
  ਕਰੋ {
    ਮੁੱਲ >>= 7;
    ਮਾਪ += ਆਕਾਰ(int8_t);
  } ਜਦੋਂ (ਮੁੱਲ);
  ਵਾਪਸ ਮਾਪ;
}

/// Utility function to get the size of the SLEB128-encoded value.
ਬਿਨਾਨਿਸ਼ਾਨੀ get_SLEB128_ਮਾਪ(int64_t ਮੁੱਲ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਮਾਪ = 0;
  ਪੂਰਨ Sign = ਮੁੱਲ >> (8 * ਆਕਾਰ(ਮੁੱਲ) - 1);
  ਤਾਰਕਿਕ Is_ਹੋਰ;

  ਕਰੋ {
    ਬਿਨਾਨਿਸ਼ਾਨੀ Byte = ਮੁੱਲ & 0x7f;
    ਮੁੱਲ >>= 7;
    Is_ਹੋਰ = ਮੁੱਲ != Sign || ((Byte ^ Sign) & 0x40) != 0;
    ਮਾਪ += ਆਕਾਰ(int8_t);
  } ਜਦੋਂ (Is_ਹੋਰ);
  ਵਾਪਸ ਮਾਪ;
}

}  // namespace llvm
