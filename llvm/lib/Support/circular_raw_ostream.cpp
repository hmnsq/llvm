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
उपयुज्य नामाकाश llvm;

शून्य circular_raw_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t आकार) {
  यदि (बफर_आकार == 0) {
    The_प्रवाहः->लिख(Ptr, आकार);
    प्रत्यागम;
  }

  // Write into the buffer, wrapping if necessary.
  यावत् (आकार != 0) {
    अचिह्नित अष्टकानि_2 =
      std::min(अचिह्नित(आकार), अचिह्नित(बफर_आकार - (Cur - बफर_Array)));
    स्मृतिं_प्रतिलिख(Cur, Ptr, अष्टकानि_2);
    आकार -= अष्टकानि_2;
    Cur += अष्टकानि_2;
    यदि (Cur == बफर_Array + बफर_आकार) {
      // Reset the output pointer to the start of the buffer.
      Cur = बफर_Array;
      Filled = सत्य;
    }
  }
}

शून्य circular_raw_ostream::flush_बफर_With_Banner() {
  यदि (बफर_आकार != 0) {
    // Write out the buffer
    The_प्रवाहः->लिख(Banner, std::वाक्यदैर्घ्यम्(Banner));
    flush_बफर();
  }
}
