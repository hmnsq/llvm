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

#அந்தர்பாவ "llvm/Support/FormattedStream.h"
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/Unicode.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <algorithm>

using நாமாகாஶ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
வெறுமை formatted_raw_ostream::UpdatePosition(மாறிலி எழுத்து *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &வரி = Position.second;

  auto ProcessUTF8CodePoint = [&வரி, &Column](StringRef CP) {
    முழு Width = sys::unicode::columnWidthUTF8(CP);
    எனில் (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    எனில் (CP.size() > 1)
      திரும்பு;

    தேர்வு (CP[0]) {
    நிலை '\n':
      வரி += 1;
      [[fallthrough]];
    நிலை '\r':
      Column = 0;
      நிறுத்து;
    நிலை '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      நிறுத்து;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  எனில் (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_இலக்கம்_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    எனில் (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      திரும்பு;
    } இல்லையெனில் {
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
  unsigned இலக்கம்_Bytes;
  க்கு (மாறிலி எழுத்து *End = Ptr + Size; Ptr < End; Ptr += இலக்கம்_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    எனில் (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      இலக்கம்_Bytes = 1;
      ++Column;
      தொடர்;
    }

    இலக்கம்_Bytes = get_இலக்கம்_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    எனில் ((unsigned)(End - Ptr) < இலக்கம்_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      திரும்பு;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, இலக்கம்_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
வெறுமை formatted_raw_ostream::ComputePosition(மாறிலி எழுத்து *Ptr, size_t Size) {
  எனில் (DisableScan)
    திரும்பு;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  எனில் (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  இல்லையெனில்
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
  ComputePosition(getBufferStart(), Get_இலக்கம்_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(முழு(NewCol - getColumn()), 1));
  திரும்பு *இது;
}

வெறுமை formatted_raw_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->லிக(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  நிலையான formatted_raw_ostream S(outs());
  திரும்பு S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  நிலையான formatted_raw_ostream S(errs());
  திரும்பு S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  நிலையான formatted_raw_ostream S(dbgs());
  திரும்பு S;
}
