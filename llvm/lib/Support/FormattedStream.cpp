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

#अन्तर्भाव "llvm/Support/FormattedStream.h"
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/Unicode.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <algorithm>

using नामाकाश llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
शून्य formatted_raw_ostream::UpdatePosition(ध्रुव वर्ण *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &पंक्ति = Position.second;

  auto ProcessUTF8CodePoint = [&पंक्ति, &Column](StringRef CP) {
    पूर्णाङ्क Width = sys::unicode::columnWidthUTF8(CP);
    यदि (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    यदि (CP.size() > 1)
      प्रत्यागम;

    विकल्प (CP[0]) {
    पक्ष '\n':
      पंक्ति += 1;
      [[fallthrough]];
    पक्ष '\r':
      Column = 0;
      भङ्ग;
    पक्ष '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      भङ्ग;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  यदि (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_अंक_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    यदि (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      प्रत्यागम;
    } अन्यथा {
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
  unsigned अंक_Bytes;
  प्रति (ध्रुव वर्ण *End = Ptr + Size; Ptr < End; Ptr += अंक_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    यदि (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      अंक_Bytes = 1;
      ++Column;
      अनुवर्त;
    }

    अंक_Bytes = get_अंक_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    यदि ((unsigned)(End - Ptr) < अंक_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      प्रत्यागम;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, अंक_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
शून्य formatted_raw_ostream::ComputePosition(ध्रुव वर्ण *Ptr, size_t Size) {
  यदि (DisableScan)
    प्रत्यागम;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  यदि (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  अन्यथा
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
  ComputePosition(getBufferStart(), Get_अंक_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(पूर्णाङ्क(NewCol - getColumn()), 1));
  प्रत्यागम *एतत्;
}

शून्य formatted_raw_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->लिख(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  स्थिर formatted_raw_ostream S(outs());
  प्रत्यागम S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  स्थिर formatted_raw_ostream S(errs());
  प्रत्यागम S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  स्थिर formatted_raw_ostream S(dbgs());
  प्रत्यागम S;
}
