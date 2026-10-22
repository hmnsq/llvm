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

#شامل "llvm/Support/FormattedStream.h"
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/Unicode.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <algorithm>

استعمال نامخلا llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
خالی formatted_raw_ostream::UpdatePosition(ثابت حرف *Ptr, size_t سائز) {
  غیرعلامتی &عمود = Position.پہلا_2;
  غیرعلامتی &سطر = Position.دوسرا_2;

  خودکار Process_UTF8_رمز_نقطہ = [&سطر, &عمود](StringRef CP) {
    صحیح چوڑائی = sys::unicode::عمود_چوڑائی_UTF8(CP);
    اگر (چوڑائی != sys::unicode::ErrorNonPrintableCharacter)
      عمود += چوڑائی;

    // The only special whitespace characters we care about are single-byte.
    اگر (CP.سائز_2() > 1)
      واپس;

    انتخاب (CP[0]) {
    حالت '\n':
      سطر += 1;
      [[fallthrough]];
    حالت '\r':
      عمود = 0;
      توڑو;
    حالت '\t':
      // Assumes tab stop = 8 characters.
      عمود += (8 - (عمود & 0x7)) & 0x7;
      توڑو;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  اگر (PartialUTF8Char.سائز_2()) {
    size_t بائٹیں_From_بفر =
        get_عدد_بائٹیں_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.سائز_2();
    اگر (سائز < بائٹیں_From_بفر) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, سائز));
      واپس;
    } ورنہ {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, بائٹیں_From_بفر));
      Process_UTF8_رمز_نقطہ(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += بائٹیں_From_بفر;
      سائز -= بائٹیں_From_بفر;
    }
  }

  // Now scan the rest of the buffer.
  غیرعلامتی عدد_بائٹیں;
  برائے (ثابت حرف *انتہا_2 = Ptr + سائز; Ptr < انتہا_2; Ptr += عدد_بائٹیں) {
    // Fast path for printable ASCII characters without special handling.
    اگر (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      عدد_بائٹیں = 1;
      ++عمود;
      جاری;
    }

    عدد_بائٹیں = get_عدد_بائٹیں_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    اگر ((غیرعلامتی)(انتہا_2 - Ptr) < عدد_بائٹیں) {
      PartialUTF8Char = StringRef(Ptr, انتہا_2 - Ptr);
      واپس;
    }

    Process_UTF8_رمز_نقطہ(StringRef(Ptr, عدد_بائٹیں));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
خالی formatted_raw_ostream::ComputePosition(ثابت حرف *Ptr, size_t سائز) {
  اگر (Disable_جائزہ)
    واپس;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  اگر (Ptr <= Scanned && Scanned <= Ptr + سائز)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, سائز - (Scanned - Ptr));
  ورنہ
    UpdatePosition(Ptr, سائز);

  // Update the scanning pointer.
  Scanned = Ptr + سائز;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_مقصد_عمود(غیرعلامتی New_ستون) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_بفر_آغاز(), Get_عدد_بائٹیں_اندر_بفر());

  // Output spaces until we reach the desired column.
  indent(std::max(صحیح(New_ستون - get_عمود()), 1));
  واپس *یہ;
}

خالی formatted_raw_ostream::write_impl(ثابت حرف *Ptr, size_t سائز) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, سائز);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_دھارا->اكتب(Ptr, سائز);

  // Reset the scanning pointer.
  Scanned = خالی_اشاریہ;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  ساکن formatted_raw_ostream S(outs());
  واپس S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  ساکن formatted_raw_ostream S(errs());
  واپس S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  ساکن formatted_raw_ostream S(dbgs());
  واپس S;
}
