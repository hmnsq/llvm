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

#അന്തര്ഭാവ "llvm/Support/circular_raw_ostream.h"
#അന്തര്ഭാവ <algorithm>
using നാമാകാശ llvm;

ശൂന്യം circular_raw_ostream::write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  എങ്കിൽ (BufferSize == 0) {
    TheStream->ലിഖ(Ptr, Size);
    തിരികെ;
  }

  // Write into the buffer, wrapping if necessary.
  ആകുമ്പോൾ (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    എങ്കിൽ (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = സത്യം;
    }
  }
}

ശൂന്യം circular_raw_ostream::flushBufferWithBanner() {
  എങ്കിൽ (BufferSize != 0) {
    // Write out the buffer
    TheStream->ലിഖ(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
