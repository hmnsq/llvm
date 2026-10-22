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

#অন্তর্ভাব "llvm/Support/FormattedStream.h"
#অন্তর্ভাব "llvm/Support/ConvertUTF.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/Unicode.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব <algorithm>

using নামাকাশ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
শূন্য formatted_raw_ostream::UpdatePosition(ধ্রুব বর্ণ *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &সারি = Position.second;

  auto ProcessUTF8CodePoint = [&সারি, &Column](StringRef CP) {
    পূর্ণাঙ্ক Width = sys::unicode::columnWidthUTF8(CP);
    যদি (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    যদি (CP.size() > 1)
      প্রত্যাগম;

    বিকল্প (CP[0]) {
    পক্ষ '\n':
      সারি += 1;
      [[fallthrough]];
    পক্ষ '\r':
      Column = 0;
      ভঙ্গ;
    পক্ষ '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      ভঙ্গ;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  যদি (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_অঙ্ক_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    যদি (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      প্রত্যাগম;
    } অন্যথা {
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
  unsigned অঙ্ক_Bytes;
  প্রতি (ধ্রুব বর্ণ *End = Ptr + Size; Ptr < End; Ptr += অঙ্ক_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    যদি (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      অঙ্ক_Bytes = 1;
      ++Column;
      অনুবর্ত;
    }

    অঙ্ক_Bytes = get_অঙ্ক_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    যদি ((unsigned)(End - Ptr) < অঙ্ক_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      প্রত্যাগম;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, অঙ্ক_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
শূন্য formatted_raw_ostream::ComputePosition(ধ্রুব বর্ণ *Ptr, size_t Size) {
  যদি (DisableScan)
    প্রত্যাগম;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  যদি (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  অন্যথা
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
  ComputePosition(getBufferStart(), Get_অঙ্ক_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(পূর্ণাঙ্ক(NewCol - getColumn()), 1));
  প্রত্যাগম *এতত্;
}

শূন্য formatted_raw_ostream::write_impl(ধ্রুব বর্ণ *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->লিখ(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  স্থির formatted_raw_ostream S(outs());
  প্রত্যাগম S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  স্থির formatted_raw_ostream S(errs());
  প্রত্যাগম S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  স্থির formatted_raw_ostream S(dbgs());
  প্রত্যাগম S;
}
