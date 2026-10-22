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

using ನಾಮಾಕಾಶ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
ಶೂನ್ಯ formatted_raw_ostream::UpdatePosition(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &ಸಾಲು = Position.second;

  auto ProcessUTF8CodePoint = [&ಸಾಲು, &Column](StringRef CP) {
    ಪೂರ್ಣಾಙ್ಕ Width = sys::unicode::columnWidthUTF8(CP);
    ಯದಿ (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    ಯದಿ (CP.size() > 1)
      ಪ್ರತ್ಯಾಗಮ;

    ವಿಕಲ್ಪ (CP[0]) {
    ಪಕ್ಷ '\n':
      ಸಾಲು += 1;
      [[fallthrough]];
    ಪಕ್ಷ '\r':
      Column = 0;
      ಭಙ್ಗ;
    ಪಕ್ಷ '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      ಭಙ್ಗ;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  ಯದಿ (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_ಅಂಕೆ_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    ಯದಿ (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      ಪ್ರತ್ಯಾಗಮ;
    } ಅನ್ಯಥಾ {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, BytesFromBuffer));
      ProcessUTF8CodePoint(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += BytesFromBuffer;
      Size -= BytesFromBuffer;
    }
  }

  // Now scan the rest of the buffer.
  unsigned ಅಂಕೆ_Bytes;
  ಪ್ರತಿ (ಧ್ರುವ ವರ್ಣ *End = Ptr + Size; Ptr < End; Ptr += ಅಂಕೆ_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    ಯದಿ (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      ಅಂಕೆ_Bytes = 1;
      ++Column;
      ಅನುವರ್ತ;
    }

    ಅಂಕೆ_Bytes = get_ಅಂಕೆ_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    ಯದಿ ((unsigned)(End - Ptr) < ಅಂಕೆ_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      ಪ್ರತ್ಯಾಗಮ;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, ಅಂಕೆ_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
ಶೂನ್ಯ formatted_raw_ostream::ComputePosition(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  ಯದಿ (DisableScan)
    ಪ್ರತ್ಯಾಗಮ;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  ಯದಿ (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  ಅನ್ಯಥಾ
    UpdatePosition(Ptr, Size);

  // Update the scanning pointer.
  Scanned = Ptr + Size;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::PadToColumn(unsigned NewCol) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(getBufferStart(), Get_ಅಂಕೆ_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(ಪೂರ್ಣಾಙ್ಕ(NewCol - getColumn()), 1));
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ಶೂನ್ಯ formatted_raw_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->ಲಿಖ(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  ಸ್ಥಿರ formatted_raw_ostream S(outs());
  ಪ್ರತ್ಯಾಗಮ S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  ಸ್ಥಿರ formatted_raw_ostream S(errs());
  ಪ್ರತ್ಯಾಗಮ S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  ಸ್ಥಿರ formatted_raw_ostream S(dbgs());
  ಪ್ರತ್ಯಾಗಮ S;
}
