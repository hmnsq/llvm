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

کارول نطاق llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
فراغ formatted_raw_ostream::UpdatePosition(ثابت حرف *Ptr, size_t اندازه) {
  بینښه &ستنه = Position.لومړی_2;
  بینښه &کرښه = Position.دوهم_2;

  خپلکاری Process_UTF8_کوډ_ټکی = [&کرښه, &ستنه](StringRef CP) {
    صحيح پلنوالی = sys::unicode::ستنه_پلنوالی_UTF8(CP);
    إذا (پلنوالی != sys::unicode::ErrorNonPrintableCharacter)
      ستنه += پلنوالی;

    // The only special whitespace characters we care about are single-byte.
    إذا (CP.اندازه_2() > 1)
      أرجع;

    بدل (CP[0]) {
    حالة '\n':
      کرښه += 1;
      [[fallthrough]];
    حالة '\r':
      ستنه = 0;
      اكسر;
    حالة '\t':
      // Assumes tab stop = 8 characters.
      ستنه += (8 - (ستنه & 0x7)) & 0x7;
      اكسر;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  إذا (PartialUTF8Char.اندازه_2()) {
    size_t بایټونه_From_بفر =
        get_عدد_بایټونه_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.اندازه_2();
    إذا (اندازه < بایټونه_From_بفر) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, اندازه));
      أرجع;
    } وإلا {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, بایټونه_From_بفر));
      Process_UTF8_کوډ_ټکی(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += بایټونه_From_بفر;
      اندازه -= بایټونه_From_بفر;
    }
  }

  // Now scan the rest of the buffer.
  بینښه عدد_بایټونه;
  لكل (ثابت حرف *پای_ټکی_2 = Ptr + اندازه; Ptr < پای_ټکی_2; Ptr += عدد_بایټونه) {
    // Fast path for printable ASCII characters without special handling.
    إذا (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      عدد_بایټونه = 1;
      ++ستنه;
      استمر;
    }

    عدد_بایټونه = get_عدد_بایټونه_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    إذا ((بینښه)(پای_ټکی_2 - Ptr) < عدد_بایټونه) {
      PartialUTF8Char = StringRef(Ptr, پای_ټکی_2 - Ptr);
      أرجع;
    }

    Process_UTF8_کوډ_ټکی(StringRef(Ptr, عدد_بایټونه));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
فراغ formatted_raw_ostream::ComputePosition(ثابت حرف *Ptr, size_t اندازه) {
  إذا (Disable_لټون)
    أرجع;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  إذا (Ptr <= Scanned && Scanned <= Ptr + اندازه)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, اندازه - (Scanned - Ptr));
  وإلا
    UpdatePosition(Ptr, اندازه);

  // Update the scanning pointer.
  Scanned = Ptr + اندازه;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_منزل_ستنه(بینښه New_ستون) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_بفر_پیل(), Get_عدد_بایټونه_دننه_بفر());

  // Output spaces until we reach the desired column.
  indent(std::max(صحيح(New_ستون - get_ستنه()), 1));
  أرجع *هذا;
}

فراغ formatted_raw_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, اندازه);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_بهیر->اكتب(Ptr, اندازه);

  // Reset the scanning pointer.
  Scanned = تش_اشاره;
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
