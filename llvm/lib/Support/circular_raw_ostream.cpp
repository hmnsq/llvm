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
ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

ಶೂನ್ಯ circular_raw_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  ಆದರೆ (ಬಫರ್_ಗಾತ್ರ == 0) {
    The_ಹರಿವು->ಲಿಖ(Ptr, ಗಾತ್ರ_2);
    ಹಿಂತಿರುಗಿ;
  }

  // Write into the buffer, wrapping if necessary.
  ಆಗ (ಗಾತ್ರ_2 != 0) {
    ಚಿಹ್ನೆರಹಿತ ಬೈಟುಗಳು_2 =
      std::min(ಚಿಹ್ನೆರಹಿತ(ಗಾತ್ರ_2), ಚಿಹ್ನೆರಹಿತ(ಬಫರ್_ಗಾತ್ರ - (Cur - ಬಫರ್_Array)));
    ಸ್ಮರಣೆ_ನಕಲು(Cur, Ptr, ಬೈಟುಗಳು_2);
    ಗಾತ್ರ_2 -= ಬೈಟುಗಳು_2;
    Cur += ಬೈಟುಗಳು_2;
    ಆದರೆ (Cur == ಬಫರ್_Array + ಬಫರ್_ಗಾತ್ರ) {
      // Reset the output pointer to the start of the buffer.
      Cur = ಬಫರ್_Array;
      Filled = ನಿಜ;
    }
  }
}

ಶೂನ್ಯ circular_raw_ostream::flush_ಬಫರ್_With_Banner() {
  ಆದರೆ (ಬಫರ್_ಗಾತ್ರ != 0) {
    // Write out the buffer
    The_ಹರಿವು->ಲಿಖ(Banner, std::ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(Banner));
    flush_ಬಫರ್();
  }
}
