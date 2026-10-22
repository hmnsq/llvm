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

#అన్తర్భావ "llvm/Support/FormattedStream.h"
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/Unicode.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <algorithm>

ఉపయోగించి నామాకాశ llvm;

/// UpdatePosition - Examine the given char sequence and figure out which
/// column we end up in after output, and how many line breaks are contained.
/// This assumes that the input string is well-formed UTF-8, and takes into
/// account Unicode characters which render as multiple columns wide.
శూన్య formatted_raw_ostream::UpdatePosition(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  గుర్తులేని &స్తంభం = Position.మొదటి_2;
  గుర్తులేని &పంక్తి = Position.రెండవ_2;

  స్వయంచాలక Process_UTF8_సంకేతం_బిందువు = [&పంక్తి, &స్తంభం](StringRef CP) {
    పూర్ణాంకం వెడల్పు = sys::unicode::స్తంభం_వెడల్పు_UTF8(CP);
    ఒకవేళ (వెడల్పు != sys::unicode::ErrorNonPrintableCharacter)
      స్తంభం += వెడల్పు;

    // The only special whitespace characters we care about are single-byte.
    ఒకవేళ (CP.కొలత_2() > 1)
      తిరిగి;

    ఎంపిక (CP[0]) {
    స్థితి '\n':
      పంక్తి += 1;
      [[fallthrough]];
    స్థితి '\r':
      స్తంభం = 0;
      ఆపు;
    స్థితి '\t':
      // Assumes tab stop = 8 characters.
      స్తంభం += (8 - (స్తంభం & 0x7)) & 0x7;
      ఆపు;
    }
  };

  // If we have a partial UTF-8 sequence from the previous buffer, check that
  // first.
  ఒకవేళ (PartialUTF8Char.కొలత_2()) {
    size_t బైట్లు_From_బఫరు =
        get_అంకె_బైట్లు_For_UTF8(PartialUTF8Char[0]) - PartialUTF8Char.కొలత_2();
    ఒకవేళ (కొలత < బైట్లు_From_బఫరు) {
      // If we still don't have enough bytes for a complete code point, just
      // append what we have.
      PartialUTF8Char.append(StringRef(Ptr, కొలత));
      తిరిగి;
    } లేకపోతే {
      // The first few bytes from the buffer will complete the code point.
      // Concatenate them and process their effect on the line and column
      // numbers.
      PartialUTF8Char.append(StringRef(Ptr, బైట్లు_From_బఫరు));
      Process_UTF8_సంకేతం_బిందువు(PartialUTF8Char);
      PartialUTF8Char.clear();
      Ptr += బైట్లు_From_బఫరు;
      కొలత -= బైట్లు_From_బఫరు;
    }
  }

  // Now scan the rest of the buffer.
  గుర్తులేని అంకె_బైట్లు;
  కొరకు (స్థిరరాశి అక్షరం *చివర_2 = Ptr + కొలత; Ptr < చివర_2; Ptr += అంకె_బైట్లు) {
    // Fast path for printable ASCII characters without special handling.
    ఒకవేళ (*Ptr >= 0x20 && *Ptr <= 0x7e) {
      అంకె_బైట్లు = 1;
      ++స్తంభం;
      కొనసాగు;
    }

    అంకె_బైట్లు = get_అంకె_బైట్లు_For_UTF8(*Ptr);

    // The buffer might end part way through a UTF-8 code unit sequence for a
    // Unicode scalar value if it got flushed. If this happens, we can't know
    // the display width until we see the rest of the code point. Stash the
    // bytes we do have, so that we can reconstruct the whole code point later,
    // even if the buffer is being flushed.
    ఒకవేళ ((గుర్తులేని)(చివర_2 - Ptr) < అంకె_బైట్లు) {
      PartialUTF8Char = StringRef(Ptr, చివర_2 - Ptr);
      తిరిగి;
    }

    Process_UTF8_సంకేతం_బిందువు(StringRef(Ptr, అంకె_బైట్లు));
  }
}

/// ComputePosition - Examine the current output and update line and column
/// counts.
శూన్య formatted_raw_ostream::ComputePosition(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  ఒకవేళ (Disable_పరిశీలన)
    తిరిగి;

  // If our previous scan pointer is inside the buffer, assume we already
  // scanned those bytes. This depends on raw_ostream to not change our buffer
  // in unexpected ways.
  ఒకవేళ (Ptr <= Scanned && Scanned <= Ptr + కొలత)
    // Scan all characters added since our last scan to determine the new
    // column.
    UpdatePosition(Scanned, కొలత - (Scanned - Ptr));
  లేకపోతే
    UpdatePosition(Ptr, కొలత);

  // Update the scanning pointer.
  Scanned = Ptr + కొలత;
}

/// PadToColumn - Align the output to some column number.
///
/// \param NewCol - The column to move to.
///
formatted_raw_ostream &formatted_raw_ostream::Pad_గమ్యం_స్తంభం(గుర్తులేని New_నిలువు) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(get_బఫరు_ప్రారంభం(), Get_అంకె_బైట్లు_లోపల_బఫరు());

  // Output spaces until we reach the desired column.
  indent(std::max(పూర్ణాంకం(New_నిలువు - get_స్తంభం()), 1));
  తిరిగి *ఇది;
}

శూన్య formatted_raw_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  // Figure out what's in the buffer and add it to the column count.
  ComputePosition(Ptr, కొలత);

  // Write the data to the underlying stream (which is unbuffered, so
  // the data will be immediately written out).
  The_ప్రవాహం->లిఖ(Ptr, కొలత);

  // Reset the scanning pointer.
  Scanned = శూన్యసూచిక;
}

/// fouts() - This returns a reference to a formatted_raw_ostream for
/// standard output.  Use it like: fouts() << "foo" << "bar";
formatted_raw_ostream &llvm::fouts() {
  స్థిర formatted_raw_ostream S(outs());
  తిరిగి S;
}

/// ferrs() - This returns a reference to a formatted_raw_ostream for
/// standard error.  Use it like: ferrs() << "foo" << "bar";
formatted_raw_ostream &llvm::ferrs() {
  స్థిర formatted_raw_ostream S(errs());
  తిరిగి S;
}

/// fdbgs() - This returns a reference to a formatted_raw_ostream for
/// the debug stream.  Use it like: fdbgs() << "foo" << "bar";
formatted_raw_ostream &llvm::fdbgs() {
  స్థిర formatted_raw_ostream S(dbgs());
  తిరిగి S;
}
