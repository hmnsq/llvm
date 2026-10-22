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

शून्य circular_raw_ostream::write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) {
  यदि (BufferSize == 0) {
    TheStream->लिख(Ptr, Size);
    फर्काऊ;
  }

  // Write into the buffer, wrapping if necessary.
  जबसम्म (Size != 0) {
    अचिन्हित Bytes =
      std::min(अचिन्हित(Size), अचिन्हित(BufferSize - (Cur - BufferArray)));
    स_नकल(Cur, Ptr, Bytes);
    Size -= Bytes;
    Cur += Bytes;
    यदि (Cur == BufferArray + BufferSize) {
      // Reset the output pointer to the start of the buffer.
      Cur = BufferArray;
      Filled = सत्य;
    }
  }
}

शून्य circular_raw_ostream::flushBufferWithBanner() {
  यदि (BufferSize != 0) {
    // Write out the buffer
    TheStream->लिख(Banner, std::म_माप(Banner));
    flushBuffer();
  }
}
