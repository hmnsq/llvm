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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/FormattedStream.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Unicode.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>

using ନାମାକାଶ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
ଶୂନ୍ୟ formatted_raw_ostream::UpdatePosition(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &ଧାଡ଼ି = Position.second;

  auto ProcessUTF8CodePoint = [&ଧାଡ଼ି, &Column](StringRef CP) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା Width = sys::unicode::columnWidthUTF8(CP);
    ଯଦି (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    ଯଦି (CP.size() > 1)
      ଫେରନ୍ତୁ;

    ବାଛ (CP[0]) {
    ସ୍ଥିତି '\n':
      ଧାଡ଼ି += 1;
      [[fallthrough]];
    ସ୍ଥିତି '\r':
      Column = 0;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      ଭାଙ୍ଗ;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  ଯଦି (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_ଅଙ୍କ_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    ଯଦି (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      ଫେରନ୍ତୁ;
    } ଅନ୍ୟଥା {
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
  unsigned ଅଙ୍କ_Bytes;
  ପାଇଁ (ଅଚଳ ଅକ୍ଷର *End = Ptr + Size; Ptr < End; Ptr += ଅଙ୍କ_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    ଯଦି (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      ଅଙ୍କ_Bytes = 1;
      ++Column;
      ଜାରି;
    }

    ଅଙ୍କ_Bytes = get_ଅଙ୍କ_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    ଯଦି ((unsigned)(End - Ptr) < ଅଙ୍କ_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      ଫେରନ୍ତୁ;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, ଅଙ୍କ_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
ଶୂନ୍ୟ formatted_raw_ostream::ComputePosition(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  ଯଦି (DisableScan)
    ଫେରନ୍ତୁ;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  ଯଦି (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  ଅନ୍ୟଥା
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
  ComputePosition(getBufferStart(), Get_ଅଙ୍କ_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(ପୂର୍ଣ୍ଣସଂଖ୍ୟା(NewCol - getColumn()), 1));
  ଫେରନ୍ତୁ *ଏହି;
}

ଶୂନ୍ୟ formatted_raw_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->ଲିଖ(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  ସ୍ଥିର formatted_raw_ostream S(outs());
  ଫେରନ୍ତୁ S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  ସ୍ଥିର formatted_raw_ostream S(errs());
  ଫେରନ୍ତୁ S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  ସ୍ଥିର formatted_raw_ostream S(dbgs());
  ଫେରନ୍ତୁ S;
}
