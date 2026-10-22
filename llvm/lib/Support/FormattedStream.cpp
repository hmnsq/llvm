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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FormattedStream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Unicode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
ਖਾਲੀ formatted_raw_ostream::UpdatePosition(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ &ਕਾਲਮ = Position.ਪਹਿਲਾ_2;
  ਬਿਨਾਨਿਸ਼ਾਨੀ &ਸਤਰ = Position.ਦੂਜਾ_2;

  ਸਵੈਚਾਲਿਤ Process_UTF8_ਕੋਡ_ਬਿੰਦੂ = [&ਸਤਰ, &ਕਾਲਮ](StringRef CP) {
    ਪੂਰਨ ਚੌੜਾਈ = sys::unicode::ਕਾਲਮ_ਚੌੜਾਈ_UTF8(CP);
    ਜੇ (ਚੌੜਾਈ != sys::unicode::ErrorNonPrintableCharacter)
      ਕਾਲਮ += ਚੌੜਾਈ;

    // The only special whitespace characters we care about are single-byte.
    ਜੇ (CP.ਮਾਪ_2() > 1)
      ਵਾਪਸ;

    ਚੋਣ (CP[0]) {
    ਹਾਲਤ '\n':
      ਸਤਰ += 1;
      [[fallthrough]];
    ਹਾਲਤ '\r':
      ਕਾਲਮ = 0;
      ਰੋਕੋ;
    ਹਾਲਤ '\t':
      // Assumes tab stop = 8 characters.
      ਕਾਲਮ += (8 - (ਕਾਲਮ & 0x7)) & 0x7;
      ਰੋਕੋ;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  ਜੇ (PartialUTF8Char.ਮਾਪ_2()) {
    size_t ਬਾਈਟਾਂ_From_ਬਫਰ =
        get_ਅੰਕ_ਬਾਈਟਾਂ_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.ਮਾਪ_2();
    ਜੇ (ਮਾਪ < ਬਾਈਟਾਂ_From_ਬਫਰ) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, ਮਾਪ));
      ਵਾਪਸ;
    } ਨਹੀਂਤਾਂ {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, ਬਾਈਟਾਂ_From_ਬਫਰ));
      Process_UTF8_ਕੋਡ_ਬਿੰਦੂ(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += ਬਾਈਟਾਂ_From_ਬਫਰ;
      ਮਾਪ -= ਬਾਈਟਾਂ_From_ਬਫਰ;
    }
  }

  // Now scan the rest of the buffer.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਬਾਈਟਾਂ;
  ਲਈ (ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_2 = Ptr + ਮਾਪ; Ptr < ਸਿਰਾ_2; Ptr += ਅੰਕ_ਬਾਈਟਾਂ) {
    // Fast path for printable ASCII characters without special handling.
    ਜੇ (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      ਅੰਕ_ਬਾਈਟਾਂ = 1;
      ++ਕਾਲਮ;
      ਜਾਰੀ;
    }

    ਅੰਕ_ਬਾਈਟਾਂ = get_ਅੰਕ_ਬਾਈਟਾਂ_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(ਸਿਰਾ_2 - Ptr) < ਅੰਕ_ਬਾਈਟਾਂ) {
      PartialUTF8Char = StringRef(Ptr, ਸਿਰਾ_2 - Ptr);
      ਵਾਪਸ;
    }

    Process_UTF8_ਕੋਡ_ਬਿੰਦੂ(StringRef(Ptr, ਅੰਕ_ਬਾਈਟਾਂ));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
ਖਾਲੀ formatted_raw_ostream::ComputePosition(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  ਜੇ (Disable_ਜਾਂਚ)
    ਵਾਪਸ;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  ਜੇ (Ptr <= Scanned && Scanned <= Ptr + ਮਾਪ)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, ਮਾਪ - (Scanned - Ptr));
  ਨਹੀਂਤਾਂ
    UpdatePosition(Ptr, ਮਾਪ);

  // Update the scanning pointer.
  Scanned = Ptr + ਮਾਪ;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_ਮੰਜ਼ਿਲ_ਕਾਲਮ(ਬਿਨਾਨਿਸ਼ਾਨੀ New_ਖਾਨਾ) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_ਬਫਰ_ਸ਼ੁਰੂ(), Get_ਅੰਕ_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ());

  // Output spaces until we reach the desired column.
  indent(std::max(ਪੂਰਨ(New_ਖਾਨਾ - get_ਕਾਲਮ()), 1));
  ਵਾਪਸ *ਇਹ;
}

ਖਾਲੀ formatted_raw_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, ਮਾਪ);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_ਧਾਰਾ->ਲਿਖ(Ptr, ਮਾਪ);

  // Reset the scanning pointer.
  Scanned = ਖਾਲੀ_ਸੂਚਕ;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  ਸਥਿਰ formatted_raw_ostream S(outs());
  ਵਾਪਸ S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  ਸਥਿਰ formatted_raw_ostream S(errs());
  ਵਾਪਸ S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  ਸਥਿਰ formatted_raw_ostream S(dbgs());
  ਵਾਪਸ S;
}
