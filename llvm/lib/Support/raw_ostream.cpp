//===--- raw_ostream.cpp - Implement the raw_ostream classes --------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This implements support for bulk buffered stream output.
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ "llvm/ADT/StringExtras.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/AutoConvert.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/Duration.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/FormatVariadic.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/MathExtras.h"
#అన్తర్భావ "llvm/Support/NativeFormatting.h"
#అన్తర్భావ "llvm/Support/Process.h"
#అన్తర్భావ "llvm/Support/Program.h"
#అన్తర్భావ <algorithm>
#అన్తర్భావ <cerrno>
#అన్తర్భావ <cstdio>
#అన్తర్భావ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ఒకవేళ defined(HAVE_UNISTD_H)
# include <unistd.h>
#అయితే_ముగింపు

#ఒకవేళ defined(__CYGWIN__)
#అన్తర్భావ <io.h>
#అయితే_ముగింపు

#ఒకవేళ defined(_MSC_VER)
#అన్తర్భావ <io.h>
#నిర్వచితం_కాకపోతే STDIN_FILENO
# define STDIN_FILENO 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే STDOUT_FILENO
# define STDOUT_FILENO 1
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే STDERR_FILENO
# define STDERR_FILENO 2
#అయితే_ముగింపు
#అయితే_ముగింపు

#నిర్వచితమైతే _WIN32
#అన్తర్భావ "llvm/Support/ConvertUTF.h"
#అన్తర్భావ "llvm/Support/Signals.h"
#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  ప్రతిజ్ఞా(బహిర్గతం_Buf_Cur == బహిర్గతం_Buf_ప్రారంభం &&
         "raw_ostream destructor called with non-empty buffer!");

  ఒకవేళ (బఫరు_బహుళకం == బఫరు_రకం::అంతర్గత_బఫరు)
    తొలగించు [] బహిర్గతం_Buf_ప్రారంభం;
}

size_t raw_ostream::preferred_బఫరు_కొలత() స్థిరరాశి {
#నిర్వచితమైతే _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  తిరిగి (16 * 1024);
#లేకపోతే
  // BUFSIZ is intended to be a reasonable default.
  తిరిగి BUFSIZ;
#అయితే_ముగింపు
}

శూన్య raw_ostream::సమితి_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ఒకవేళ (size_t కొలత = preferred_బఫరు_కొలత())
    సమితి_బఫరు_కొలత(కొలత);
  లేకపోతే
    // It may return 0, meaning this stream should be unbuffered.
    సమితి_Unbuffered();
}

శూన్య raw_ostream::సమితి_బఫరు_And_బహుళకం(అక్షరం *బఫరు_ప్రారంభం, size_t కొలత,
                                   బఫరు_రకం బహుళకం) {
  ప్రతిజ్ఞా(((బహుళకం == బఫరు_రకం::Unbuffered && !బఫరు_ప్రారంభం && కొలత == 0) ||
          (బహుళకం != బఫరు_రకం::Unbuffered && బఫరు_ప్రారంభం && కొలత != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  ప్రతిజ్ఞా(Get_అంకె_బైట్లు_లోపల_బఫరు() == 0 && "Current buffer is non-empty!");

  ఒకవేళ (బఫరు_బహుళకం == బఫరు_రకం::అంతర్గత_బఫరు)
    తొలగించు [] బహిర్గతం_Buf_ప్రారంభం;
  బహిర్గతం_Buf_ప్రారంభం = బఫరు_ప్రారంభం;
  బహిర్గతం_Buf_చివర = బహిర్గతం_Buf_ప్రారంభం+కొలత;
  బహిర్గతం_Buf_Cur = బహిర్గతం_Buf_ప్రారంభం;
  బఫరు_బహుళకం = బహుళకం;

  ప్రతిజ్ఞా(బహిర్గతం_Buf_ప్రారంభం <= బహిర్గతం_Buf_చివర && "Invalid size!");
}

raw_ostream &raw_ostream::క్రియాకారకం<<(గుర్తులేని దీర్ఘ N) {
  write_integer(*ఇది, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(దీర్ఘ N) {
  write_integer(*ఇది, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(గుర్తులేని దీర్ఘ దీర్ఘ N) {
  write_integer(*ఇది, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(దీర్ఘ దీర్ఘ N) {
  write_integer(*ఇది, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::write_hex(గుర్తులేని దీర్ఘ దీర్ఘ N) {
  llvm::write_hex(*ఇది, N, HexPrintStyle::క్రింది_2);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(Colors C) {
  ఒకవేళ (C == Colors::RESET)
    resetColor();
  లేకపోతే
    changeColor(C);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::write_uuid(స్థిరరాశి uuid_t UUID) {
  కొరకు (పూర్ణాంకం Idx = 0; Idx < 16; ++Idx) {
    *ఇది << format("%02" PRIX32, UUID[Idx]);
    ఒకవేళ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *ఇది << "-";
  }
  తిరిగి *ఇది;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        తార్కిక UseHexEscapes) {
  కొరకు (గుర్తులేని అక్షరం c : Str) {
    ఎంపిక (c) {
    స్థితి '\\':
      *ఇది << '\\' << '\\';
      ఆపు;
    స్థితి '\t':
      *ఇది << '\\' << 't';
      ఆపు;
    స్థితి '\n':
      *ఇది << '\\' << 'n';
      ఆపు;
    స్థితి '"':
      *ఇది << '\\' << '"';
      ఆపు;
    అప్రమేయ:
      ఒకవేళ (isPrint(c)) {
        *ఇది << c;
        ఆపు;
      }

      // Write out the escaped representation.
      ఒకవేళ (UseHexEscapes) {
        *ఇది << '\\' << 'x';
        *ఇది << hexdigit((c >> 4) & 0xF);
        *ఇది << hexdigit((c >> 0) & 0xF);
      } లేకపోతే {
        // Always use a full 3-character octal escape.
        *ఇది << '\\';
        *ఇది << అక్షరం('0' + ((c >> 6) & 7));
        *ఇది << అక్షరం('0' + ((c >> 3) & 7));
        *ఇది << అక్షరం('0' + ((c >> 0) & 7));
      }
    }
  }

  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(స్థిరరాశి శూన్య *P) {
  llvm::write_hex(*ఇది, (uintptr_t)P, HexPrintStyle::ఉపసర్గ_క్రింది);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(ద్విగుణ N) {
  llvm::write_double(*ఇది, N, FloatStyle::Exponent);
  తిరిగి *ఇది;
}

శూన్య raw_ostream::flush_nonempty() {
  ప్రతిజ్ఞా(బహిర్గతం_Buf_Cur > బహిర్గతం_Buf_ప్రారంభం && "Invalid call to flush_nonempty.");
  size_t నిడివి = బహిర్గతం_Buf_Cur - బహిర్గతం_Buf_ప్రారంభం;
  బహిర్గతం_Buf_Cur = బహిర్గతం_Buf_ప్రారంభం;
  write_impl(బహిర్గతం_Buf_ప్రారంభం, నిడివి);
}

raw_ostream &raw_ostream::లిఖ(గుర్తులేని అక్షరం C) {
  // Group exceptional cases into a single branch.
  ఒకవేళ (LLVM_UNLIKELY(బహిర్గతం_Buf_Cur >= బహిర్గతం_Buf_చివర)) {
    ఒకవేళ (LLVM_UNLIKELY(!బహిర్గతం_Buf_ప్రారంభం)) {
      ఒకవేళ (బఫరు_బహుళకం == బఫరు_రకం::Unbuffered) {
        write_impl(reinterpret_cast<అక్షరం *>(&C), 1);
        తిరిగి *ఇది;
      }
      // Set up a buffer and start over.
      సమితి_Buffered();
      తిరిగి లిఖ(C);
    }

    flush_nonempty();
  }

  *బహిర్గతం_Buf_Cur++ = C;
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::లిఖ(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  // Group exceptional cases into a single branch.
  ఒకవేళ (LLVM_UNLIKELY(size_t(బహిర్గతం_Buf_చివర - బహిర్గతం_Buf_Cur) < కొలత)) {
    ఒకవేళ (LLVM_UNLIKELY(!బహిర్గతం_Buf_ప్రారంభం)) {
      ఒకవేళ (బఫరు_బహుళకం == బఫరు_రకం::Unbuffered) {
        write_impl(Ptr, కొలత);
        తిరిగి *ఇది;
      }
      // Set up a buffer and start over.
      సమితి_Buffered();
      తిరిగి లిఖ(Ptr, కొలత);
    }

    size_t అంకె_బైట్లు = బహిర్గతం_Buf_చివర - బహిర్గతం_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ఒకవేళ (LLVM_UNLIKELY(బహిర్గతం_Buf_Cur == బహిర్గతం_Buf_ప్రారంభం)) {
      ప్రతిజ్ఞా(అంకె_బైట్లు != 0 && "undefined behavior");
      size_t బైట్లు_గమ్యం_Write_2 = కొలత - (కొలత % అంకె_బైట్లు);
      write_impl(Ptr, బైట్లు_గమ్యం_Write_2);
      size_t బైట్లు_మిగిలిన_2 = కొలత - బైట్లు_గమ్యం_Write_2;
      ఒకవేళ (బైట్లు_మిగిలిన_2 > size_t(బహిర్గతం_Buf_చివర - బహిర్గతం_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        తిరిగి లిఖ(Ptr + బైట్లు_గమ్యం_Write_2, బైట్లు_మిగిలిన_2);
      }
      copy_గమ్యం_బఫరు(Ptr + బైట్లు_గమ్యం_Write_2, బైట్లు_మిగిలిన_2);
      తిరిగి *ఇది;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_గమ్యం_బఫరు(Ptr, అంకె_బైట్లు);
    flush_nonempty();
    తిరిగి లిఖ(Ptr + అంకె_బైట్లు, కొలత - అంకె_బైట్లు);
  }

  copy_గమ్యం_బఫరు(Ptr, కొలత);

  తిరిగి *ఇది;
}

శూన్య raw_ostream::copy_గమ్యం_బఫరు(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  ప్రతిజ్ఞా(కొలత <= size_t(బహిర్గతం_Buf_చివర - బహిర్గతం_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  ఎంపిక (కొలత) {
  స్థితి 4: బహిర్గతం_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  స్థితి 3: బహిర్గతం_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  స్థితి 2: బహిర్గతం_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  స్థితి 1: బహిర్గతం_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  స్థితి 0: ఆపు;
  అప్రమేయ:
    జ్ఞాపకం_నకలు(బహిర్గతం_Buf_Cur, Ptr, కొలత);
    ఆపు;
  }

  బహిర్గతం_Buf_Cur += కొలత;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::క్రియాకారకం<<(function_ref<పూర్ణాంకం(అక్షరం *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t కొలత = 128;
  size_t బఫరు_బైట్లు_ఎడమ = బహిర్గతం_Buf_చివర - బహిర్గతం_Buf_Cur;
  ఒకవేళ (బఫరు_బైట్లు_ఎడమ > 3) {
    పూర్ణాంకం N = Snprint(బహిర్గతం_Buf_Cur, బఫరు_బైట్లు_ఎడమ);
    ఒకవేళ (N < 0)
      తిరిగి *ఇది;
    // Common case is that we have plenty of space.
    ఒకవేళ (size_t(N) < బఫరు_బైట్లు_ఎడమ) {
      బహిర్గతం_Buf_Cur += N;
      తిరిగి *ఇది;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    కొలత = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_సదిశ<అక్షరం, 128> V;
  కొరకు (;;) {
    V.resize(కొలత);
    పూర్ణాంకం N = Snprint(V.దత్తాంశం(), కొలత);
    ఒకవేళ (N < 0)
      తిరిగి *ఇది;
    ఒకవేళ (size_t(N) < కొలత)
      తిరిగి లిఖ(V.దత్తాంశం(), N);
    కొలత = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::క్రియాకారకం<<(స్థిరరాశి formatv_object_పునాది &Obj) {
  Obj.format(*ఇది);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(స్థిరరాశి FormattedString &FS) {
  గుర్తులేని ఎడమ_Indent = 0;
  గుర్తులేని కుడి_Indent = 0;
  స్థిరరాశి ssize_t Difference = FS.వెడల్పు - FS.Str.కొలత_2();
  ఒకవేళ (Difference > 0) {
    ఎంపిక (FS.Justify) {
    స్థితి FormattedString::JustifyNone:
      ఆపు;
    స్థితి FormattedString::Justify_ఎడమ:
      కుడి_Indent = Difference;
      ఆపు;
    స్థితి FormattedString::Justify_కుడి:
      ఎడమ_Indent = Difference;
      ఆపు;
    స్థితి FormattedString::JustifyCenter:
      ఎడమ_Indent = Difference / 2;
      కుడి_Indent = Difference - ఎడమ_Indent;
      ఆపు;
    }
  }
  indent(ఎడమ_Indent);
  (*ఇది) << FS.Str;
  indent(కుడి_Indent);
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(స్థిరరాశి Formatted_సంఖ్య &FN) {
  ఒకవేళ (FN.Hex) {
    HexPrintStyle Style;
    ఒకవేళ (FN.పై_2 && FN.Hex_ఉపసర్గ)
      Style = HexPrintStyle::ఉపసర్గ_పై;
    లేకపోతే ఒకవేళ (FN.పై_2 && !FN.Hex_ఉపసర్గ)
      Style = HexPrintStyle::పై_2;
    లేకపోతే ఒకవేళ (!FN.పై_2 && FN.Hex_ఉపసర్గ)
      Style = HexPrintStyle::ఉపసర్గ_క్రింది;
    లేకపోతే
      Style = HexPrintStyle::క్రింది_2;
    llvm::write_hex(*ఇది, FN.Hex_విలువ, Style, FN.వెడల్పు);
  } లేకపోతే {
    llvm::SmallString<16> బఫరు;
    llvm::raw_svector_ostream ప్రవాహం(బఫరు);
    llvm::write_integer(ప్రవాహం, FN.Dec_విలువ, 0, IntegerStyle::Integer);
    ఒకవేళ (బఫరు.కొలత_2() < FN.వెడల్పు)
      indent(FN.వెడల్పు - బఫరు.కొలత_2());
    (*ఇది) << బఫరు;
  }
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::క్రియాకారకం<<(స్థిరరాశి Formatted_బైట్లు &FB) {
  ఒకవేళ (FB.బైట్లు_2.ఖాళీలు())
    తిరిగి *ఇది;

  size_t పంక్తి_సూచీ = 0;
  స్వయంచాలక బైట్లు_2 = FB.బైట్లు_2;
  స్థిరరాశి size_t కొలత = బైట్లు_2.కొలత_2();
  HexPrintStyle HPS = FB.పై_2 ? HexPrintStyle::పై_2 : HexPrintStyle::క్రింది_2;
  uint64_t స్థానభ్రంశం_వెడల్పు = 0;
  ఒకవేళ (FB.మొదటి_Byte_స్థానభ్రంశం) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t పంక్తులు = కొలత / FB.అంకె_చొప్పున_పంక్తి;
    uint64_t Max_స్థానభ్రంశం = *FB.మొదటి_Byte_స్థానభ్రంశం + పంక్తులు * FB.అంకె_చొప్పున_పంక్తి;
    గుర్తులేని Power = 0;
    ఒకవేళ (Max_స్థానభ్రంశం > 0)
      Power = llvm::Log2_64_Ceil(Max_స్థానభ్రంశం);
    స్థానభ్రంశం_వెడల్పు = std::max<uint64_t>(4, llvm::align_గమ్యం(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  గుర్తులేని అంకె_Byte_Groups =
      align_గమ్యం(FB.అంకె_చొప్పున_పంక్తి, FB.Byte_సమూహం_కొలత) / FB.Byte_సమూహం_కొలత;
  గుర్తులేని ఖండం_Char_వెడల్పు = FB.అంకె_చొప్పున_పంక్తి * 2 + అంకె_Byte_Groups - 1;

  అయితే (!బైట్లు_2.ఖాళీలు()) {
    indent(FB.IndentLevel);

    ఒకవేళ (FB.మొదటి_Byte_స్థానభ్రంశం) {
      uint64_t స్థానభ్రంశం = *FB.మొదటి_Byte_స్థానభ్రంశం;
      llvm::write_hex(*ఇది, స్థానభ్రంశం + పంక్తి_సూచీ, HPS, స్థానభ్రంశం_వెడల్పు);
      *ఇది << ": ";
    }

    స్వయంచాలక పంక్తి = బైట్లు_2.take_front(FB.అంకె_చొప్పున_పంక్తి);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    కొరకు (size_t I = 0; I < పంక్తి.కొలత_2(); ++I, CharsPrinted += 2) {
      ఒకవేళ (I && (I % FB.Byte_సమూహం_కొలత) == 0) {
        ++CharsPrinted;
        *ఇది << " ";
      }
      llvm::write_hex(*ఇది, పంక్తి[I], HPS, 2);
    }

    ఒకవేళ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      ప్రతిజ్ఞా(ఖండం_Char_వెడల్పు >= CharsPrinted);
      indent(ఖండం_Char_వెడల్పు - CharsPrinted + 2);
      *ఇది << "|";

      // Print the ASCII char values for each byte on this line
      కొరకు (uint8_t Byte : పంక్తి) {
        ఒకవేళ (isPrint(Byte))
          *ఇది << static_cast<అక్షరం>(Byte);
        లేకపోతే
          *ఇది << '.';
      }
      *ఇది << '|';
    }

    బైట్లు_2 = బైట్లు_2.drop_front(పంక్తి.కొలత_2());
    పంక్తి_సూచీ += పంక్తి.కొలత_2();
    ఒకవేళ (పంక్తి_సూచీ < కొలత)
      *ఇది << '\n';
  }
  తిరిగి *ఇది;
}

మూస <అక్షరం C>
స్థిర raw_ostream &write_padding(raw_ostream &OS, గుర్తులేని అంకె_Chars) {
  స్థిర స్థిరరాశి అక్షరం Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ఒకవేళ (అంకె_Chars < std::కొలత_2(Chars))
    తిరిగి OS.లిఖ(Chars, అంకె_Chars);

  అయితే (అంకె_Chars) {
    గుర్తులేని అంకె_గమ్యం_Write = std::min(అంకె_Chars, (గుర్తులేని)std::కొలత_2(Chars) - 1);
    OS.లిఖ(Chars, అంకె_గమ్యం_Write);
    అంకె_Chars -= అంకె_గమ్యం_Write;
  }
  తిరిగి OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(గుర్తులేని అంకె_Spaces) {
  తిరిగి write_padding<' '>(*ఇది, అంకె_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(గుర్తులేని అంకె_Zeros) {
  తిరిగి write_padding<'\0'>(*ఇది, అంకె_Zeros);
}

తార్కిక raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ఒకవేళ (!ColorEnabled)
    తిరిగి అబద్ధం;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ఒకవేళ (sys::Process::ColorNeedsFlush() && !is_displayed())
    తిరిగి అబద్ధం;

  ఒకవేళ (sys::Process::ColorNeedsFlush())
    flush();

  తిరిగి నిజం;
}

raw_ostream &raw_ostream::changeColor(గణన Colors colors, తార్కిక bold, తార్కిక bg) {
  ఒకవేళ (!prepare_colors())
    తిరిగి *ఇది;

  స్థిరరాశి అక్షరం *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<అక్షరం>(colors), bold, bg);
  ఒకవేళ (colorcode)
    లిఖ(colorcode, అక్షరశ్రేణి_పొడవు(colorcode));
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::resetColor() {
  ఒకవేళ (!prepare_colors())
    తిరిగి *ఇది;

  ఒకవేళ (స్థిరరాశి అక్షరం *colorcode = sys::Process::ResetColor())
    లిఖ(colorcode, అక్షరశ్రేణి_పొడవు(colorcode));
  తిరిగి *ఇది;
}

raw_ostream &raw_ostream::తిరగేయి_Color() {
  ఒకవేళ (!prepare_colors())
    తిరిగి *ఇది;

  ఒకవేళ (స్థిరరాశి అక్షరం *colorcode = sys::Process::Output_తిరగేయి())
    లిఖ(colorcode, అక్షరశ్రేణి_పొడవు(colorcode));
  తిరిగి *ఇది;
}

శూన్య raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

స్థిర పూర్ణాంకం getFD(StringRef Filename, std::error_సంకేతం &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ప్రతిజ్ఞా((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ఒకవేళ (Filename == "-") {
    EC = std::error_సంకేతం();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_బహుళకం(Flags);
    తిరిగి STDOUT_FILENO;
  }

  పూర్ణాంకం FD;
  ఒకవేళ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  లేకపోతే
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ఒకవేళ (EC)
    తిరిగి -1;

  తిరిగి FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_సంకేతం &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::యొక్క_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_సంకేతం &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::యొక్క_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_సంకేతం &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::యొక్క_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_సంకేతం &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_సంకేతం &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), నిజం) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(పూర్ణాంకం fd, తార్కిక shouldClose, తార్కిక unbuffered,
                               OStream_రకం K)
    : raw_pwrite_ప్రవాహం(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  ఒకవేళ (FD < 0 ) {
    ShouldClose = అబద్ధం;
    తిరిగి;
  }

  enable_colors(నిజం);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ఒకవేళ (FD <= STDERR_FILENO)
    ShouldClose = అబద్ధం;

#నిర్వచితమైతే _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#అయితే_ముగింపు

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_స్థితి స్థితి_2;
  std::error_సంకేతం EC = స్థితి_3(FD, స్థితి_2);
  IsRegularFile = స్థితి_2.type() == sys::fs::file_type::regular_file;
#నిర్వచితమైతే _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#లేకపోతే
  SupportsSeeking = !EC && loc != (off_t)-1;
#అయితే_ముగింపు
  ఒకవేళ (!SupportsSeeking)
    pos = 0;
  లేకపోతే
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ఒకవేళ (FD >= 0) {
    flush();
    ఒకవేళ (ShouldClose) {
      ఒకవేళ (స్వయంచాలక EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#నిర్వచితమైతే __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ఒకవేళ (FD == 2) తిరిగి;
#అయితే_ముగింపు

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ఒకవేళ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().సందేశం_2());
}

#ఒకవేళ defined(_WIN32)
// The most reliable way to print unicode in a Windows console is with
// WriteConsoleW. To use that, first transcode from UTF-8 to UTF-16. This
// assumes that LLVM programs always print valid UTF-8 to the console. The data
// might not be UTF-8 for two major reasons:
// 1. The program is printing binary (-filetype=obj -o -), in which case it
// would have been gibberish anyway.
// 2. The program is printing text in a semi-ascii compatible codepage like
// shift-jis or cp1252.
//
// Most LLVM programs don't produce non-ascii text unless they are quoting
// user source input. A well-behaved LLVM program should either validate that
// the input is UTF-8 or transcode from the local codepage to UTF-8 before
// quoting it. If they don't, this may mess up the encoding, but this is still
// probably the best compromise we can make.
స్థిర తార్కిక write_console_impl(పూర్ణాంకం FD, StringRef దత్తాంశం_2) {
  Small_సదిశ<wchar_t, 256> Wide_పాఠ్యం;

  // Fall back to ::write if it wasn't valid UTF-8.
  ఒకవేళ (స్వయంచాలక EC = sys::windows::UTF8_గమ్యం_UTF16(దత్తాంశం_2, Wide_పాఠ్యం))
    తిరిగి అబద్ధం;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_కొలత = Wide_పాఠ్యం.కొలత_2();
  ఒకవేళ (!RunningWindows8OrGreater())
    Max_Write_కొలత = 32767;

  size_t WCharsWritten = 0;
  చేయి {
    size_t WChars_గమ్యం_Write =
        std::min(Max_Write_కొలత, Wide_పాఠ్యం.కొలత_2() - WCharsWritten);
    DWORD ActuallyWritten;
    తార్కిక Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_పాఠ్యం[WCharsWritten],
                        WChars_గమ్యం_Write, &ActuallyWritten,
                        /*Reserved=*/శూన్యసూచిక);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ఒకవేళ (!Success)
      తిరిగి అబద్ధం;

    WCharsWritten += ActuallyWritten;
  } అయితే (WCharsWritten != Wide_పాఠ్యం.కొలత_2());
  తిరిగి నిజం;
}
#అయితే_ముగింపు

శూన్య raw_fd_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  ఒకవేళ (Tied_ప్రవాహం)
    Tied_ప్రవాహం->flush();

  ప్రతిజ్ఞా(FD >= 0 && "File already closed.");
  pos += కొలత;

#ఒకవేళ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ఒకవేళ (IsWindowsConsole)
    ఒకవేళ (write_console_impl(FD, StringRef(Ptr, కొలత)))
      తిరిగి;
#అయితే_ముగింపు

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_కొలత = INT32_MAX;

#ఒకవేళ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_కొలత = 1024 * 1024 * 1024;
#అయితే_ముగింపు

  చేయి {
    size_t Chunk_కొలత = std::min(కొలత, Max_Write_కొలత);
    ssize_t ret = ::లిఖ(FD, Ptr, Chunk_కొలత);

    ఒకవేళ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ఒకవేళ (errno == EINTR || errno == EAGAIN
#నిర్వచితమైతే EWOULDBLOCK
          || errno == EWOULDBLOCK
#అయితే_ముగింపు
          )
        కొనసాగు;

#నిర్వచితమైతే _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_చివరి_Error = Get_చివరి_Error();
      ఒకవేళ (Win_చివరి_Error == ERROR_BROKEN_PIPE ||
          (Win_చివరి_Error == ERROR_NO_దత్తాంశం && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#అయితే_ముగింపు
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_సంకేతం());
      ఆపు;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    కొలత -= ret;
  } అయితే (కొలత > 0);
}

శూన్య raw_fd_ostream::close() {
  ప్రతిజ్ఞా(ShouldClose);
  ShouldClose = అబద్ధం;
  flush();
  ఒకవేళ (స్వయంచాలక EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  ప్రతిజ్ఞా(SupportsSeeking && "Stream does not support seeking!");
  flush();
#నిర్వచితమైతే _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#లేకపోతే
  pos = ::lseek(FD, off, SEEK_SET);
#అయితే_ముగింపు
  ఒకవేళ (pos == (uint64_t)-1)
    error_detected(errno_As_Error_సంకేతం());
  తిరిగి pos;
}

శూన్య raw_fd_ostream::pwrite_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత,
                                 uint64_t స్థానభ్రంశం) {
  uint64_t Pos = tell();
  seek(స్థానభ్రంశం);
  లిఖ(Ptr, కొలత);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_బఫరు_కొలత() స్థిరరాశి {
#ఒకవేళ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ఒకవేళ (IsWindowsConsole)
    తిరిగి 0;
  తిరిగి raw_ostream::preferred_బఫరు_కొలత();
#లేదావేళ defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  తిరిగి raw_ostream::preferred_బఫరు_కొలత();
#లేకపోతే
  ప్రతిజ్ఞా(FD >= 0 && "File not yet open!");
  నిర్మాణం stat statbuf;
  ఒకవేళ (fstat(FD, &statbuf) != 0)
    తిరిగి 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ఒకవేళ (S_ISCHR(statbuf.st_బహుళకం) && is_displayed())
    తిరిగి 0;
  // Return the preferred block size.
  తిరిగి statbuf.st_blksize;
#అయితే_ముగింపు
}

తార్కిక raw_fd_ostream::is_displayed() స్థిరరాశి {
  తిరిగి sys::Process::FileDescriptorIsDisplayed(FD);
}

తార్కిక raw_fd_ostream::has_colors() స్థిరరాశి {
  ఒకవేళ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  తిరిగి *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::తాళం_2() {
  std::error_సంకేతం EC = sys::fs::తాళం_File(FD);
  ఒకవేళ (!EC)
    తిరిగి sys::fs::FileLocker(FD);
  తిరిగి error_సంకేతం_గమ్యం_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_తాళం_For(Duration స్థిరరాశి& Timeout) {
  std::error_సంకేతం EC = sys::fs::try_తాళం_File(FD, Timeout.getDuration());
  ఒకవేళ (!EC)
    తిరిగి sys::fs::FileLocker(FD);
  తిరిగి error_సంకేతం_గమ్యం_Error(EC);
}

శూన్య raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_సంకేతం EC;

  // On z/OS we need to enable auto conversion
  స్థిర std::error_సంకేతం EC1 = enableAutoConversion(STDOUT_FILENO);
  ప్రతిజ్ఞా(!EC1);
  (శూన్య)EC1;

  స్థిర raw_fd_ostream S("-", EC, sys::fs::యొక్క_None);
  ప్రతిజ్ఞా(!EC);
  తిరిగి S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  స్థిర std::error_సంకేతం EC = enableAutoConversion(STDERR_FILENO);
  ప్రతిజ్ఞా(!EC);
  (శూన్య)EC;

  // Set standard error to be unbuffered.
  స్థిర raw_fd_ostream S(STDERR_FILENO, అబద్ధం, నిజం);
  తిరిగి S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  స్థిర raw_null_ostream S;
  తిరిగి S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_ప్రవాహం::raw_fd_ప్రవాహం(StringRef Filename, std::error_సంకేతం &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::యొక్క_None),
                     నిజం, అబద్ధం, OStream_రకం::OK_FDStream) {
  ఒకవేళ (EC)
    తిరిగి;

  ఒకవేళ (!isRegularFile())
    EC = std::make_error_సంకేతం(std::errc::invalid_వాదం);
}

raw_fd_ప్రవాహం::raw_fd_ప్రవాహం(పూర్ణాంకం fd, తార్కిక shouldClose)
    : raw_fd_ostream(fd, shouldClose, అబద్ధం, OStream_రకం::OK_FDStream) {}

ssize_t raw_fd_ప్రవాహం::పఠ(అక్షరం *Ptr, size_t కొలత) {
  ప్రతిజ్ఞా(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::పఠ(get_fd(), (శూన్య *)Ptr, కొలత);
  ఒకవేళ (Ret >= 0)
    inc_pos(Ret);
  లేకపోతే
    error_detected(errno_As_Error_సంకేతం());
  తిరిగి Ret;
}

తార్కిక raw_fd_ప్రవాహం::classof(స్థిరరాశి raw_ostream *OS) {
  తిరిగి OS->get_రకం_2() == OStream_రకం::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

శూన్య raw_string_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  OS.append(Ptr, కొలత);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() స్థిరరాశి { తిరిగి OS.కొలత_2(); }

శూన్య raw_svector_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
  OS.append(Ptr, Ptr + కొలత);
}

శూన్య raw_svector_ostream::pwrite_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత,
                                      uint64_t స్థానభ్రంశం) {
  జ్ఞాపకం_నకలు(OS.దత్తాంశం() + స్థానభ్రంశం, Ptr, కొలత);
}

తార్కిక raw_svector_ostream::classof(స్థిరరాశి raw_ostream *OS) {
  తిరిగి OS->get_రకం_2() == OStream_రకం::OK_SVec_ప్రవాహం;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#నిర్వచితం_కాకపోతే NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#అయితే_ముగింపు
}

శూన్య raw_null_ostream::write_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత) {
}

uint64_t raw_null_ostream::current_pos() స్థిరరాశి {
  తిరిగి 0;
}

శూన్య raw_null_ostream::pwrite_impl(స్థిరరాశి అక్షరం *Ptr, size_t కొలత,
                                   uint64_t స్థానభ్రంశం) {}

శూన్య raw_pwrite_ప్రవాహం::anchor() {}

శూన్య బఫరు_ostream::anchor() {}

శూన్య బఫరు_unique_ostream::anchor() {}

స్థిర తార్కిక isNullDeviceName(StringRef OutputFileName) {
#నిర్వచితమైతే _WIN32
  ఒకవేళ (OutputFileName.equals_insensitive("NUL"))
    తిరిగి నిజం;
#అయితే_ముగింపు
  తిరిగి OutputFileName == "/dev/null";
}

Error llvm::write_గమ్యం_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ఒకవేళ (OutputFileName == "-")
    తిరిగి Write(outs());

  ఒకవేళ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream బహిర్గతం;
    తిరిగి Write(బహిర్గతం);
  }

  గుర్తులేని బహుళకం = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::తాత్కాలిక_File> తాత్కాలిక =
      sys::fs::తాత్కాలిక_File::create(OutputFileName + ".temp-stream-%%%%%%", బహుళకం);
  ఒకవేళ (!తాత్కాలిక)
    తిరిగి createFileError(OutputFileName, తాత్కాలిక.takeError());

#ఒకవేళ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ఒకవేళ (స్వయంచాలక EC = llvm::copy_File_ట్యాగ్_లక్షణాలు(OutputFileName.str(), తాత్కాలిక->FD)) {
    ఒకవేళ (EC != std::errc::no_such_file_or_directory)
      తిరిగి createFileError(OutputFileName, EC);
  }
#అయితే_ముగింపు

  raw_fd_ostream బహిర్గతం(తాత్కాలిక->FD, అబద్ధం);

  ఒకవేళ (Error E = Write(బహిర్గతం)) {
    ఒకవేళ (Error DiscardError = తాత్కాలిక->discard())
      తిరిగి joinErrors(std::move(E), std::move(DiscardError));
    తిరిగి E;
  }
  బహిర్గతం.flush();

  తిరిగి తాత్కాలిక->keep(OutputFileName);
}
