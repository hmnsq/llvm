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

#समावेश "llvm/Support/FormattedStream.h"
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/Unicode.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <algorithm>

using namespace llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
व्योम formatted_raw_ostream::UpdatePosition(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  अचिन्हित &Column = Position.first;
  अचिन्हित &पंक्ति_2 = Position.second;

  स्वतः ProcessUTF8CodePoint = [&पंक्ति_2, &Column](StringRef CP) {
    पूर्णांक Width = sys::unicode::columnWidthUTF8(CP);
    अगर (Width != sys::unicode::ErrorNonPrintableCharacter)
      Column += Width;

    // The only special whitespace characters we care about are single-byte.
    अगर (CP.आकार_2() > 1)
      वापस;

    चयन (CP[0]) {
    हाल '\n':
      पंक्ति_2 += 1;
      [[fallthrough]];
    हाल '\r':
      Column = 0;
      अवरोध;
    हाल '\t':
      // Assumes tab stop = 8 characters.
      Column += (8 - (Column & 0x7)) & 0x7;
      अवरोध;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  अगर (PartialUTF8Char.आकार_2()) {
    माप_प्रकार Bytes_From_बफर =
        get_अंक_Bytes_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.आकार_2();
    अगर (आकार < Bytes_From_बफर) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, आकार));
      वापस;
    } अन्यथा {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, Bytes_From_बफर));
      ProcessUTF8CodePoint(PartialUTF8Char);
      PartialUTF8Char.सफा();
      Ptr += Bytes_From_बफर;
      आकार -= Bytes_From_बफर;
    }
  }

  // Now scan the rest of the buffer.
  अचिन्हित अंक_Bytes;
  क्रम (स्थिर अक्षर *End = Ptr + आकार; Ptr < End; Ptr += अंक_Bytes) {
    // Fast path for printable ASCII characters without special handling.
    अगर (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      अंक_Bytes = 1;
      ++Column;
      जारी;
    }

    अंक_Bytes = get_अंक_Bytes_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    अगर ((अचिन्हित)(End - Ptr) < अंक_Bytes) {
      PartialUTF8Char = StringRef(Ptr, End - Ptr);
      वापस;
    }

    ProcessUTF8CodePoint(StringRef(Ptr, अंक_Bytes));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
व्योम formatted_raw_ostream::ComputePosition(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  अगर (DisableScan)
    वापस;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  अगर (Ptr <= Scanned && Scanned <= Ptr + आकार)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, आकार - (Scanned - Ptr));
  अन्यथा
    UpdatePosition(Ptr, आकार);

  // Update the scanning pointer.
  Scanned = Ptr + आकार;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::PadToColumn(अचिन्हित NewCol) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_बफर_Start(), Get_अंक_Bytes_In_बफर());

  // Output spaces until we reach the desired column.
  indent(std::max(पूर्णांक(NewCol - getColumn()), 1));
  वापस *यह;
}

व्योम formatted_raw_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, आकार);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  TheStream->ग_लिखो(Ptr, आकार);

  // Reset the scanning pointer.
  Scanned = nullptr;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  अटल formatted_raw_ostream S(म_दो());
  वापस S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  अटल formatted_raw_ostream S(errs());
  वापस S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  अटल formatted_raw_ostream S(dbgs());
  वापस S;
}
