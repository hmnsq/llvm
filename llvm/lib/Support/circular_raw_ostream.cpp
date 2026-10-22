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

#ಅನ್ತರ್ಭಾವ "llvm/Support/circular_raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
using ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ circular_raw_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  ಯದಿ (BufferSize == 0) {
    TheStream->ಲಿಖ(Ptr, Size);
    ಪ್ರತ್ಯಾಗಮ;
  }

  // Write into the buffer, wrapping if necessary.
  ಯಾವತ್ (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    ಯದಿ (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = ಸತ್ಯ;
    }
  }
}

ಶೂನ್ಯ circular_raw_ostream::flushBufferWithBanner() {
  ಯದಿ (BufferSize != 0) {
    // Write out the buffer
    TheStream->ಲಿಖ(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
