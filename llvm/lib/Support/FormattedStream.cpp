//===-- llvm/Support/FormattedStream.cpp - Formatted streams ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of formatted_raw_ostream.
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/FormattedStream.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Unicode.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <algorithm>

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
ಶೂನ್ಯ formatted_raw_ostream::UpdatePosition(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  ಚಿಹ್ನೆರಹಿತ &ಸ್ತಂಭ = Position.ಮೊದಲ_2;
  ಚಿಹ್ನೆರಹಿತ &ಸಾಲು = Position.ಎರಡನೇ_2;

  ಸ್ವಯಂಚಾಲಿತ Process_UTF8_ಸಂಕೇತ_ಬಿಂದು = [&ಸಾಲು, &ಸ್ತಂಭ](StringRef CP) {
    ಪೂರ್ಣಾಂಕ ಅಗಲ = sys::unicode::ಸ್ತಂಭ_ಅಗಲ_UTF8(CP);
    ಆದರೆ (ಅಗಲ != sys::unicode::ErrorNonPrintableCharacter)
      ಸ್ತಂಭ += ಅಗಲ;

    // The only special whitespace characters we care about are single-byte.
    ಆದರೆ (CP.ಗಾತ್ರ_3() > 1)
      ಹಿಂತಿರುಗಿ;

    ಆಯ್ಕೆ (CP[0]) {
    ಸ್ಥಿತಿ '\n':
      ಸಾಲು += 1;
      [[fallthrough]];
    ಸ್ಥಿತಿ '\r':
      ಸ್ತಂಭ = 0;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\t':
      // Assumes tab stop = 8 characters.
      ಸ್ತಂಭ += (8 - (ಸ್ತಂಭ & 0x7)) & 0x7;
      ನಿಲ್ಲಿಸು;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  ಆದರೆ (PartialUTF8Char.ಗಾತ್ರ_3()) {
    size_t ಬೈಟುಗಳು_From_ಬಫರ್ =
        get_ಅಂಕೆ_ಬೈಟುಗಳು_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.ಗಾತ್ರ_3();
    ಆದರೆ (ಗಾತ್ರ_2 < ಬೈಟುಗಳು_From_ಬಫರ್) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, ಗಾತ್ರ_2));
      ಹಿಂತಿರುಗಿ;
    } ಇಲ್ಲದಿದ್ದರೆ {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, ಬೈಟುಗಳು_From_ಬಫರ್));
      Process_UTF8_ಸಂಕೇತ_ಬಿಂದು(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += ಬೈಟುಗಳು_From_ಬಫರ್;
      ಗಾತ್ರ_2 -= ಬೈಟುಗಳು_From_ಬಫರ್;
    }
  }

  // Now scan the rest of the buffer.
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಬೈಟುಗಳು;
  ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ_2 = Ptr + ಗಾತ್ರ_2; Ptr < ಕೊನೆ_2; Ptr += ಅಂಕೆ_ಬೈಟುಗಳು) {
    // Fast path for printable ASCII characters without special handling.
    ಆದರೆ (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      ಅಂಕೆ_ಬೈಟುಗಳು = 1;
      ++ಸ್ತಂಭ;
      ಮುಂದುವರಿ;
    }

    ಅಂಕೆ_ಬೈಟುಗಳು = get_ಅಂಕೆ_ಬೈಟುಗಳು_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)(ಕೊನೆ_2 - Ptr) < ಅಂಕೆ_ಬೈಟುಗಳು) {
      PartialUTF8Char = StringRef(Ptr, ಕೊನೆ_2 - Ptr);
      ಹಿಂತಿರುಗಿ;
    }

    Process_UTF8_ಸಂಕೇತ_ಬಿಂದು(StringRef(Ptr, ಅಂಕೆ_ಬೈಟುಗಳು));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
ಶೂನ್ಯ formatted_raw_ostream::ComputePosition(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  ಆದರೆ (Disable_ಪರಿಶೀಲನೆ)
    ಹಿಂತಿರುಗಿ;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  ಆದರೆ (Ptr <= Scanned && Scanned <= Ptr + ಗಾತ್ರ_2)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, ಗಾತ್ರ_2 - (Scanned - Ptr));
  ಇಲ್ಲದಿದ್ದರೆ
    UpdatePosition(Ptr, ಗಾತ್ರ_2);

  // Update the scanning pointer.
  Scanned = Ptr + ಗಾತ್ರ_2;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_ಗಮ್ಯ_ಸ್ತಂಭ(ಚಿಹ್ನೆರಹಿತ New_ಲಂಬಸಾಲು) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_ಬಫರ್_ಆರಂಭ(), Get_ಅಂಕೆ_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್());

  // Output spaces until we reach the desired column.
  indent(std::max(ಪೂರ್ಣಾಂಕ(New_ಲಂಬಸಾಲು - get_ಸ್ತಂಭ()), 1));
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಶೂನ್ಯ formatted_raw_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, ಗಾತ್ರ_2);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_ಹರಿವು->ಲಿಖ(Ptr, ಗಾತ್ರ_2);

  // Reset the scanning pointer.
  Scanned = ಶೂನ್ಯಸೂಚಕ;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  ಸ್ಥಿರ formatted_raw_ostream S(outs());
  ಹಿಂತಿರುಗಿ S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  ಸ್ಥಿರ formatted_raw_ostream S(errs());
  ಹಿಂತಿರುಗಿ S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  ಸ್ಥಿರ formatted_raw_ostream S(dbgs());
  ಹಿಂತಿರುಗಿ S;
}
