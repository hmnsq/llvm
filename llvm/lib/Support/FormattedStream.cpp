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

#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Unicode.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>

using namespace llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
void formatted_raw_ostream::UpdatePosition(const char *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &Line = Position.second;

  auto ProcessUTF8CodePoint = [&Line, &Column](StringRef CP) {
    整数 Width = sys::unicode::columnWidthUTF8(CP);
    もし (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    もし (CP.size() > 1)
      戻る;

    switch (CP[0]) {
    case '\n':
      Line += 1;
      [[fallthrough]];
    case '\r':
      Column = 0;
      break;
    case '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      break;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  もし (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        getNumBytesForUTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    もし (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      戻る;
    } 否则 {
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
  unsigned NumBytes;
  ために (const char *End = Ptr + Size; Ptr < End; Ptr += NumBytes) {
    // Fast path for printable ASCII characters without special handling.
    もし (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      NumBytes = 1;
      ++Column;
      continue;
    }

    NumBytes = getNumBytesForUTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    もし ((unsigned)(End - Ptr) < NumBytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      戻る;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, NumBytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
void formatted_raw_ostream::ComputePosition(const char *Ptr, size_t Size) {
  もし (DisableScan)
    戻る;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  もし (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  否则
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
  ComputePosition(getBufferStart(), GetNumBytesInBuffer());

  // Output spaces until we reach the desired column.
  indent(std::max(整数(NewCol - getColumn()), 1));
  戻る *this;
}

void formatted_raw_ostream::write_impl(const char *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->write(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  static formatted_raw_ostream S(outs());
  戻る S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  static formatted_raw_ostream S(errs());
  戻る S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  static formatted_raw_ostream S(dbgs());
  戻る S;
}
