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

استعمال نطاق llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
فراغ formatted_raw_ostream::UpdatePosition(ثابت حرف *Ptr, size_t سائيز) {
  بنانشان &ڪالم = Position.پهريون_2;
  بنانشان &سٽ = Position.ٻيو_2;

  خودڪار Process_UTF8_ڪوڊ_نقطو = [&سٽ, &ڪالم](StringRef CP) {
    صحيح ويڪر = sys::unicode::ڪالم_ويڪر_UTF8(CP);
    إذا (ويڪر != sys::unicode::ErrorNonPrintableCharacter)
      ڪالم += ويڪر;

    // The only special whitespace characters we care about are single-byte.
    إذا (CP.سائيز_2() > 1)
      أرجع;

    بدل (CP[0]) {
    حالة '\n':
      سٽ += 1;
      [[fallthrough]];
    حالة '\r':
      ڪالم = 0;
      اكسر;
    حالة '\t':
      // Assumes tab stop = 8 characters.
      ڪالم += (8 - (ڪالم & 0x7)) & 0x7;
      اكسر;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  إذا (PartialUTF8Char.سائيز_2()) {
    size_t بائيٽون_From_بفر =
        get_انگ_بائيٽون_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.سائيز_2();
    إذا (سائيز < بائيٽون_From_بفر) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, سائيز));
      أرجع;
    } وإلا {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, بائيٽون_From_بفر));
      Process_UTF8_ڪوڊ_نقطو(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += بائيٽون_From_بفر;
      سائيز -= بائيٽون_From_بفر;
    }
  }

  // Now scan the rest of the buffer.
  بنانشان انگ_بائيٽون;
  لكل (ثابت حرف *پڇاڙي_2 = Ptr + سائيز; Ptr < پڇاڙي_2; Ptr += انگ_بائيٽون) {
    // Fast path for printable ASCII characters without special handling.
    إذا (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      انگ_بائيٽون = 1;
      ++ڪالم;
      استمر;
    }

    انگ_بائيٽون = get_انگ_بائيٽون_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    إذا ((بنانشان)(پڇاڙي_2 - Ptr) < انگ_بائيٽون) {
      PartialUTF8Char = StringRef(Ptr, پڇاڙي_2 - Ptr);
      أرجع;
    }

    Process_UTF8_ڪوڊ_نقطو(StringRef(Ptr, انگ_بائيٽون));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
فراغ formatted_raw_ostream::ComputePosition(ثابت حرف *Ptr, size_t سائيز) {
  إذا (Disable_جاچ)
    أرجع;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  إذا (Ptr <= Scanned && Scanned <= Ptr + سائيز)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, سائيز - (Scanned - Ptr));
  وإلا
    UpdatePosition(Ptr, سائيز);

  // Update the scanning pointer.
  Scanned = Ptr + سائيز;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_منزل_ڪالم(بنانشان New_کانو) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_بفر_شروعات(), Get_انگ_بائيٽون_اندريون_بفر());

  // Output spaces until we reach the desired column.
  indent(std::max(صحيح(New_کانو - get_ڪالم()), 1));
  أرجع *هذا;
}

فراغ formatted_raw_ostream::write_impl(ثابت حرف *Ptr, size_t سائيز) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, سائيز);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_وهڪرو->اكتب(Ptr, سائيز);

  // Reset the scanning pointer.
  Scanned = خالي_اشارو;
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
