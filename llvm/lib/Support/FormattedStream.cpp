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

#ضمن "llvm/Support/FormattedStream.h"
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/Unicode.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <algorithm>

using نطاق llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
فراغ formatted_raw_ostream::UpdatePosition(ثابت حرف *Ptr, size_t Size) {
  unsigned &Column = Position.first;
  unsigned &سطر = Position.second;

  auto ProcessUTF8CodePoint = [&سطر, &Column](StringRef CP) {
    صحيح Width = sys::unicode::columnWidthUTF8(CP);
    إذا (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    إذا (CP.size() > 1)
      أرجع;

    بدل (CP[0]) {
    حالة '\n':
      سطر += 1;
      [[fallthrough]];
    حالة '\r':
      Column = 0;
      اكسر;
    حالة '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      اكسر;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  إذا (PartialUTF8Char.size()) {
    size_t BytesFromBuffer =
        get_رقم_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.size();
    إذا (Size < BytesFromBuffer) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, Size));
      أرجع;
    } وإلا {
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
  unsigned رقم_Bytes;
  لكل (ثابت حرف *End = Ptr + Size; Ptr < End; Ptr += رقم_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    إذا (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      رقم_Bytes = 1;
      ++Column;
      استمر;
    }

    رقم_Bytes = get_رقم_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    إذا ((unsigned)(End - Ptr) < رقم_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      أرجع;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, رقم_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
فراغ formatted_raw_ostream::ComputePosition(ثابت حرف *Ptr, size_t Size) {
  إذا (DisableScan)
    أرجع;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  إذا (Ptr <= Scanned && Scanned <= Ptr + Size)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, Size - (Scanned - Ptr));
  وإلا
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
  ComputePosition(getBufferStart(), Get_رقم_Bytes_In_Buffer());

  // Output spaces until we reach the desired column.
  indent(std::max(صحيح(NewCol - getColumn()), 1));
  أرجع *هذا;
}

فراغ formatted_raw_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, Size);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->اكتب(Ptr, Size);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  ساكن formatted_raw_ostream S(outs());
  أرجع S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  ساكن formatted_raw_ostream S(errs());
  أرجع S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  ساكن formatted_raw_ostream S(dbgs());
  أرجع S;
}
