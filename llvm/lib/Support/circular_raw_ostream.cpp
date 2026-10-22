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

#समावेश "llvm/Support/circular_raw_ostream.h"
#समावेश <algorithm>
using namespace llvm;

व्योम circular_raw_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  अगर (बफर_आकार == 0) {
    TheStream->ग_लिखो(Ptr, आकार);
    वापस;
  }

  // Write into the buffer, wrapping if necessary.
  जबतक (आकार != 0) {
    अचिन्हित Bytes =
      std::min(अचिन्हित(आकार), अचिन्हित(बफर_आकार - (Cur - बफर_Array)));
    स_नकल(Cur, Ptr, Bytes);
    आकार -= Bytes;
    Cur += Bytes;
    अगर (Cur == बफर_Array + बफर_आकार) {
      // Reset the output pointer to the start of the buffer.
      Cur = बफर_Array;
      Filled = true;
    }
  }
}

व्योम circular_raw_ostream::flush_बफर_With_Banner() {
  अगर (बफर_आकार != 0) {
    // Write out the buffer
    TheStream->ग_लिखो(Banner, std::म_माप(Banner));
    flush_बफर();
  }
}
