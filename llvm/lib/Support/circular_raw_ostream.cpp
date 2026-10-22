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

#ضمن "llvm/Support/circular_raw_ostream.h"
#ضمن <algorithm>
using نطاق llvm;

فراغ circular_raw_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  𐤀𐤌 (BufferSize == 0) {
    TheStream->اكتب(Ptr, Size);
    أرجع;
  }

  // Write into the buffer, wrapping if necessary.
  طالما (Size != 0) {
    unsigned Bytes =
      std::min(unsigned(Size), unsigned(BufferSize - (Cur - BufferArray)));
    memcpy(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    𐤀𐤌 (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = true;
    }
  }
}

فراغ circular_raw_ostream::flushBufferWithBanner() {
  𐤀𐤌 (BufferSize != 0) {
    // Write out the buffer
    TheStream->اكتب(Banner, std::strlen(Banner));
    flushBuffer();
  }
}
