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

#അന്തര്ഭാവ "llvm/Support/FormattedStream.h"
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/Unicode.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <algorithm>

using നാമാകാശ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
ശൂന്യം formatted_raw_ostream::UpdatePosition(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &വരി = Position.second;

  auto ProcessUTF8CodePoint = [&വരി, &Column](StringRef CP) {
    പൂർണ്ണസംഖ്യ Width = sys::unicode::columnWidthUTF8(CP);
    എങ്കിൽ (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    എങ്കിൽ (CP.size() > 1)
      തിരികെ;

    തിരഞ്ഞെടുപ്പ് (CP[0]) {
    അവസ്ഥ '\n':
      വരി += 1;
      [[fallthrough]];
    അവസ്ഥ '\r':
      Column = 0;
      നിർത്തുക;
    അവസ്ഥ '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      നിർത്തുക;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  എങ്കിൽ (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_അക്കം_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    എങ്കിൽ (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      തിരികെ;
    } അല്ലെങ്കിൽ {
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
  unsigned അക്കം_Bytes;
  വേണ്ടി (സ്ഥിരം അക്ഷരം *End = Ptr + Size; Ptr < End; Ptr += അക്കം_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    എങ്കിൽ (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      അക്കം_Bytes = 1;
      ++Column;
      തുടരുക;
    }

    അക്കം_Bytes = get_അക്കം_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    എങ്കിൽ ((unsigned)(End - Ptr) < അക്കം_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      തിരികെ;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, അക്കം_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
ശൂന്യം formatted_raw_ostream::ComputePosition(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  എങ്കിൽ (DisableScan)
    തിരികെ;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  എങ്കിൽ (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  അല്ലെങ്കിൽ
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
  ComputePosition(getBufferStart(), Get_അക്കം_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(പൂർണ്ണസംഖ്യ(NewCol - getColumn()), 1));
  തിരികെ *ഇത്;
}

ശൂന്യം formatted_raw_ostream::write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->ലിഖ(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  സ്ഥിതം formatted_raw_ostream S(outs());
  തിരികെ S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  സ്ഥിതം formatted_raw_ostream S(errs());
  തിരികെ S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  സ്ഥിതം formatted_raw_ostream S(dbgs());
  തിരികെ S;
}
