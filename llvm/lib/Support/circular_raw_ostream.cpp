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

#شامل "llvm/Support/circular_raw_ostream.h"
#شامل <algorithm>
using فضاینام llvm;

تهی circular_raw_ostream::write_impl(ثابت نویسه *Ptr, size_t Size) {
  اگر (BufferSize == 0) {
    TheStream->اكتب(Ptr, Size);
    بازگشت;
  }

  // Write into the buffer, wrapping if necessary.
  وقتی (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    اگر (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = درست;
    }
  }
}

تهی circular_raw_ostream::flushBufferWithBanner() {
  اگر (BufferSize != 0) {
    // Write out the buffer
    TheStream->اكتب(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
