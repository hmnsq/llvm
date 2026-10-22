//===- circular_raw_ostream.cpp - Implement circular_raw_ostream ----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements support for circular buffered streams.
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/circular_raw_ostream.h"
#include <algorithm>
using namespace llvm;

void circular_raw_ostream::write_impl(const var_na *Ptr, size_t Size) {
  ᬬᬾᬦ᭄ (BufferSize == 0) {
    TheStream->write(Ptr, Size);
    ᬯᬮᬶ;
  }

  // Write into the buffer, wrapping if necessary.
  ᬲᭂᬤᭂᬮ (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    ᬬᬾᬦ᭄ (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = true;
    }
  }
}

void circular_raw_ostream::flushBufferWithBanner() {
  ᬬᬾᬦ᭄ (BufferSize != 0) {
    // Write out the buffer
    TheStream->write(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
