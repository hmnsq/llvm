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

#अन्तर्भाव "llvm/Support/circular_raw_ostream.h"
#अन्तर्भाव <algorithm>
using नामाकाश llvm;

शून्य circular_raw_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t Size) {
  सचे (BufferSize == 0) {
    TheStream->लिख(Ptr, Size);
    पटिनिवत्त;
  }

  // Write into the buffer, wrapping if necessary.
  याव (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    सचे (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = सच्च;
    }
  }
}

शून्य circular_raw_ostream::flushBufferWithBanner() {
  सचे (BufferSize != 0) {
    // Write out the buffer
    TheStream->लिख(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
