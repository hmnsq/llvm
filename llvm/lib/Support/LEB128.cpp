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

#అన్తర్భావ "llvm/Support/LEB128.h"

నామాకాశ llvm {

/// Utility function to get the size of the ULEB128-encoded value.
గుర్తులేని get_ULEB128_కొలత(uint64_t విలువ) {
  గుర్తులేని కొలత = 0;
  చేయి {
    విలువ >>= 7;
    కొలత += పరిమాణం(int8_t);
  } అయితే (విలువ);
  తిరిగి కొలత;
}

/// Utility function to get the size of the SLEB128-encoded value.
గుర్తులేని get_SLEB128_కొలత(int64_t విలువ) {
  గుర్తులేని కొలత = 0;
  పూర్ణాంకం Sign = విలువ >> (8 * పరిమాణం(విలువ) - 1);
  తార్కిక Is_మరిన్ని;

  చేయి {
    గుర్తులేని Byte = విలువ & 0x7f;
    విలువ >>= 7;
    Is_మరిన్ని = విలువ != Sign || ((Byte ^ Sign) & 0x40) != 0;
    కొలత += పరిమాణం(int8_t);
  } అయితే (Is_మరిన్ని);
  తిరిగి కొలత;
}

}  // namespace llvm
