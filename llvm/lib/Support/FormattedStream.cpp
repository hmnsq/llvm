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

ورتوں نطاق llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
فراغ formatted_raw_ostream::UpdatePosition(ثابت حرف *Ptr, size_t ناپ) {
  بنانشانی &کالم = Position.پہلا_2;
  بنانشانی &سطر = Position.دوجا_2;

  خودکار Process_UTF8_کوڈ_نقطہ = [&سطر, &کالم](StringRef CP) {
    صحيح چوڑائی = sys::unicode::کالم_چوڑائی_UTF8(CP);
    إذا (چوڑائی != sys::unicode::ErrorNonPrintableCharacter)
      کالم += چوڑائی;

    // The only special whitespace characters we care about are single-byte.
    إذا (CP.ناپ_2() > 1)
      أرجع;

    بدل (CP[0]) {
    حالة '\n':
      سطر += 1;
      [[fallthrough]];
    حالة '\r':
      کالم = 0;
      اكسر;
    حالة '\t':
      // Assumes tab stop = 8 characters.
      کالم += (8 - (کالم & 0x7)) & 0x7;
      اكسر;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  إذا (PartialUTF8Char.ناپ_2()) {
    size_t بائٹاں_From_بفر =
        get_عدد_بائٹاں_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.ناپ_2();
    إذا (ناپ < بائٹاں_From_بفر) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, ناپ));
      أرجع;
    } وإلا {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, بائٹاں_From_بفر));
      Process_UTF8_کوڈ_نقطہ(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += بائٹاں_From_بفر;
      ناپ -= بائٹاں_From_بفر;
    }
  }

  // Now scan the rest of the buffer.
  بنانشانی عدد_بائٹاں;
  لكل (ثابت حرف *سرا_2 = Ptr + ناپ; Ptr < سرا_2; Ptr += عدد_بائٹاں) {
    // Fast path for printable ASCII characters without special handling.
    إذا (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      عدد_بائٹاں = 1;
      ++کالم;
      استمر;
    }

    عدد_بائٹاں = get_عدد_بائٹاں_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    إذا ((بنانشانی)(سرا_2 - Ptr) < عدد_بائٹاں) {
      PartialUTF8Char = StringRef(Ptr, سرا_2 - Ptr);
      أرجع;
    }

    Process_UTF8_کوڈ_نقطہ(StringRef(Ptr, عدد_بائٹاں));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
فراغ formatted_raw_ostream::ComputePosition(ثابت حرف *Ptr, size_t ناپ) {
  إذا (Disable_جانچ)
    أرجع;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  إذا (Ptr <= Scanned && Scanned <= Ptr + ناپ)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, ناپ - (Scanned - Ptr));
  وإلا
    UpdatePosition(Ptr, ناپ);

  // Update the scanning pointer.
  Scanned = Ptr + ناپ;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_منزل_کالم(بنانشانی New_خانہ) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_بفر_شروع(), Get_عدد_بائٹاں_وچ_بفر());

  // Output spaces until we reach the desired column.
  indent(std::max(صحيح(New_خانہ - get_کالم()), 1));
  أرجع *هذا;
}

فراغ formatted_raw_ostream::write_impl(ثابت حرف *Ptr, size_t ناپ) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, ناپ);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_دھارا->اكتب(Ptr, ناپ);

  // Reset the scanning pointer.
  Scanned = خالی_سوچک;
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
