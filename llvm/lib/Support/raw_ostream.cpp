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

#অন্তর্ভাব "llvm/Support/raw_ostream.h"
#অন্তর্ভাব "llvm/ADT/StringExtras.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/AutoConvert.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/Duration.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব "llvm/Support/FileSystem.h"
#অন্তর্ভাব "llvm/Support/Format.h"
#অন্তর্ভাব "llvm/Support/FormatVariadic.h"
#অন্তর্ভাব "llvm/Support/IOSandbox.h"
#অন্তর্ভাব "llvm/Support/MathExtras.h"
#অন্তর্ভাব "llvm/Support/NativeFormatting.h"
#অন্তর্ভাব "llvm/Support/Process.h"
#অন্তর্ভাব "llvm/Support/Program.h"
#অন্তর্ভাব <algorithm>
#অন্তর্ভাব <cerrno>
#অন্তর্ভাব <cstdio>
#অন্তর্ভাব <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#যদি defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#যদি defined(__CYGWIN__)
#অন্তর্ভাব <io.h>
#endif

#যদি defined(_MSC_VER)
#অন্তর্ভাব <io.h>
#ifndef STDIN_FILENO
# define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
# define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
# define STDERR_FILENO 2
#endif
#endif

#ifdef _WIN32
#অন্তর্ভাব "llvm/Support/ConvertUTF.h"
#অন্তর্ভাব "llvm/Support/Signals.h"
#অন্তর্ভাব "llvm/Support/Windows/WindowsSupport.h"
#endif

using নামাকাশ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  প্রতিজ্ঞা(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  যদি (বাফার_Mode == বাফার_Kind::Internal_বাফার)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_বাফার_আকার() ধ্রুবক {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ফেরত (16 * 1024);
#অন্যথা
  // BUFSIZ is intended to be a reasonable default.
  ফেরত BUFSIZ;
#endif
}

শূন্য raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  যদি (size_t আকার_2 = preferred_বাফার_আকার())
    Set_বাফার_আকার(আকার_2);
  অন্যথা
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

শূন্য raw_ostream::Set_বাফার_And_Mode(অক্ষর *বাফার_Start, size_t আকার_2,
                                   বাফার_Kind Mode) {
  প্রতিজ্ঞা(((Mode == বাফার_Kind::Unbuffered && !বাফার_Start && আকার_2 == 0) ||
          (Mode != বাফার_Kind::Unbuffered && বাফার_Start && আকার_2 != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  প্রতিজ্ঞা(Get_অঙ্ক_Bytes_In_বাফার() == 0 && "Current buffer is non-empty!");

  যদি (বাফার_Mode == বাফার_Kind::Internal_বাফার)
    delete [] OutBufStart;
  OutBufStart = বাফার_Start;
  OutBufEnd = OutBufStart+আকার_2;
  OutBufCur = OutBufStart;
  বাফার_Mode = Mode;

  প্রতিজ্ঞা(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*এই, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*এই, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*এই, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*এই, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ফেরত *এই;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*এই, N, HexPrintStyle::Lower);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  যদি (C == Colors::RESET)
    resetColor();
  অন্যথা
    changeColor(C);
  ফেরত *এই;
}

raw_ostream &raw_ostream::write_uuid(ধ্রুবক uuid_t UUID) {
  জন্য (পূর্ণসংখ্যা Idx = 0; Idx < 16; ++Idx) {
    *এই << format("%02" PRIX32, UUID[Idx]);
    যদি (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *এই << "-";
  }
  ফেরত *এই;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        যৌক্তিক UseHexEscapes) {
  জন্য (unsigned অক্ষর c : Str) {
    নির্বাচন (c) {
    ক্ষেত্র '\\':
      *এই << '\\' << '\\';
      বিরতি;
    ক্ষেত্র '\t':
      *এই << '\\' << 't';
      বিরতি;
    ক্ষেত্র '\n':
      *এই << '\\' << 'n';
      বিরতি;
    ক্ষেত্র '"':
      *এই << '\\' << '"';
      বিরতি;
    পূর্বনির্ধারিত:
      যদি (isPrint(c)) {
        *এই << c;
        বিরতি;
      }

      // Write out the escaped representation.
      যদি (UseHexEscapes) {
        *এই << '\\' << 'x';
        *এই << hexdigit((c >> 4) & 0xF);
        *এই << hexdigit((c >> 0) & 0xF);
      } অন্যথা {
        // Always use a full 3-character octal escape.
        *এই << '\\';
        *এই << অক্ষর('0' + ((c >> 6) & 7));
        *এই << অক্ষর('0' + ((c >> 3) & 7));
        *এই << অক্ষর('0' + ((c >> 0) & 7));
      }
    }
  }

  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(ধ্রুবক শূন্য *P) {
  llvm::write_hex(*এই, (uintptr_t)P, HexPrintStyle::PrefixLower);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(দ্বিগুণ N) {
  llvm::write_double(*এই, N, FloatStyle::Exponent);
  ফেরত *এই;
}

শূন্য raw_ostream::flush_nonempty() {
  প্রতিজ্ঞা(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t দৈর্ঘ্য = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, দৈর্ঘ্য);
}

raw_ostream &raw_ostream::লিখ(unsigned অক্ষর C) {
  // Group exceptional cases into a single branch.
  যদি (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    যদি (LLVM_UNLIKELY(!OutBufStart)) {
      যদি (বাফার_Mode == বাফার_Kind::Unbuffered) {
        write_impl(reinterpret_cast<অক্ষর *>(&C), 1);
        ফেরত *এই;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ফেরত লিখ(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  ফেরত *এই;
}

raw_ostream &raw_ostream::লিখ(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  // Group exceptional cases into a single branch.
  যদি (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < আকার_2)) {
    যদি (LLVM_UNLIKELY(!OutBufStart)) {
      যদি (বাফার_Mode == বাফার_Kind::Unbuffered) {
        write_impl(Ptr, আকার_2);
        ফেরত *এই;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ফেরত লিখ(Ptr, আকার_2);
    }

    size_t অঙ্ক_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    যদি (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      প্রতিজ্ঞা(অঙ্ক_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = আকার_2 - (আকার_2 % অঙ্ক_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = আকার_2 - BytesToWrite;
      যদি (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        ফেরত লিখ(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_বাফার(Ptr + BytesToWrite, BytesRemaining);
      ফেরত *এই;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_বাফার(Ptr, অঙ্ক_Bytes);
    flush_nonempty();
    ফেরত লিখ(Ptr + অঙ্ক_Bytes, আকার_2 - অঙ্ক_Bytes);
  }

  copy_to_বাফার(Ptr, আকার_2);

  ফেরত *এই;
}

শূন্য raw_ostream::copy_to_বাফার(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  প্রতিজ্ঞা(আকার_2 <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  নির্বাচন (আকার_2) {
  ক্ষেত্র 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  ক্ষেত্র 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  ক্ষেত্র 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  ক্ষেত্র 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  ক্ষেত্র 0: বিরতি;
  পূর্বনির্ধারিত:
    memcpy(OutBufCur, Ptr, আকার_2);
    বিরতি;
  }

  OutBufCur += আকার_2;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<পূর্ণসংখ্যা(অক্ষর *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t আকার_2 = 128;
  size_t বাফার_Bytes_বাম = OutBufEnd - OutBufCur;
  যদি (বাফার_Bytes_বাম > 3) {
    পূর্ণসংখ্যা N = Snprint(OutBufCur, বাফার_Bytes_বাম);
    যদি (N < 0)
      ফেরত *এই;
    // Common case is that we have plenty of space.
    যদি (size_t(N) < বাফার_Bytes_বাম) {
      OutBufCur += N;
      ফেরত *এই;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    আকার_2 = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<অক্ষর, 128> V;
  জন্য (;;) {
    V.resize(আকার_2);
    পূর্ণসংখ্যা N = Snprint(V.তথ্য(), আকার_2);
    যদি (N < 0)
      ফেরত *এই;
    যদি (size_t(N) < আকার_2)
      ফেরত লিখ(V.তথ্য(), N);
    আকার_2 = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ধ্রুবক formatv_object_base &Obj) {
  Obj.format(*এই);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(ধ্রুবক FormattedString &FS) {
  unsigned বাম_Indent = 0;
  unsigned ডান_Indent = 0;
  ধ্রুবক ssize_t Difference = FS.Width - FS.Str.আকার_3();
  যদি (Difference > 0) {
    নির্বাচন (FS.Justify) {
    ক্ষেত্র FormattedString::JustifyNone:
      বিরতি;
    ক্ষেত্র FormattedString::Justify_বাম:
      ডান_Indent = Difference;
      বিরতি;
    ক্ষেত্র FormattedString::Justify_ডান:
      বাম_Indent = Difference;
      বিরতি;
    ক্ষেত্র FormattedString::JustifyCenter:
      বাম_Indent = Difference / 2;
      ডান_Indent = Difference - বাম_Indent;
      বিরতি;
    }
  }
  indent(বাম_Indent);
  (*এই) << FS.Str;
  indent(ডান_Indent);
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(ধ্রুবক Formatted_সংখ্যা &FN) {
  যদি (FN.Hex) {
    HexPrintStyle Style;
    যদি (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    অন্যথা যদি (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    অন্যথা যদি (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    অন্যথা
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*এই, FN.Hex_মান, Style, FN.Width);
  } অন্যথা {
    llvm::SmallString<16> বাফার;
    llvm::raw_svector_ostream Stream(বাফার);
    llvm::write_integer(Stream, FN.Dec_মান, 0, IntegerStyle::Integer);
    যদি (বাফার.আকার_3() < FN.Width)
      indent(FN.Width - বাফার.আকার_3());
    (*এই) << বাফার;
  }
  ফেরত *এই;
}

raw_ostream &raw_ostream::operator<<(ধ্রুবক FormattedBytes &FB) {
  যদি (FB.Bytes.empty())
    ফেরত *এই;

  size_t সারি_সূচক = 0;
  auto Bytes = FB.Bytes;
  ধ্রুবক size_t আকার_2 = Bytes.আকার_3();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  যদি (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t সারিগুলি = আকার_2 / FB.অঙ্ক_Per_সারি;
    uint64_t MaxOffset = *FB.FirstByteOffset + সারিগুলি * FB.অঙ্ক_Per_সারি;
    unsigned Power = 0;
    যদি (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned অঙ্ক_Byte_Groups =
      alignTo(FB.অঙ্ক_Per_সারি, FB.Byte_Group_আকার) / FB.Byte_Group_আকার;
  unsigned BlockCharWidth = FB.অঙ্ক_Per_সারি * 2 + অঙ্ক_Byte_Groups - 1;

  যখন (!Bytes.empty()) {
    indent(FB.IndentLevel);

    যদি (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*এই, Offset + সারি_সূচক, HPS, OffsetWidth);
      *এই << ": ";
    }

    auto সারি = Bytes.take_front(FB.অঙ্ক_Per_সারি);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    জন্য (size_t I = 0; I < সারি.আকার_3(); ++I, CharsPrinted += 2) {
      যদি (I && (I % FB.Byte_Group_আকার) == 0) {
        ++CharsPrinted;
        *এই << " ";
      }
      llvm::write_hex(*এই, সারি[I], HPS, 2);
    }

    যদি (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      প্রতিজ্ঞা(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *এই << "|";

      // Print the ASCII char values for each byte on this line
      জন্য (uint8_t Byte : সারি) {
        যদি (isPrint(Byte))
          *এই << static_cast<অক্ষর>(Byte);
        অন্যথা
          *এই << '.';
      }
      *এই << '|';
    }

    Bytes = Bytes.drop_front(সারি.আকার_3());
    সারি_সূচক += সারি.আকার_3();
    যদি (সারি_সূচক < আকার_2)
      *এই << '\n';
  }
  ফেরত *এই;
}

template <অক্ষর C>
স্থির raw_ostream &write_padding(raw_ostream &OS, unsigned অঙ্ক_Chars) {
  স্থির ধ্রুবক অক্ষর Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  যদি (অঙ্ক_Chars < std::আকার_3(Chars))
    ফেরত OS.লিখ(Chars, অঙ্ক_Chars);

  যখন (অঙ্ক_Chars) {
    unsigned অঙ্ক_To_Write = std::min(অঙ্ক_Chars, (unsigned)std::আকার_3(Chars) - 1);
    OS.লিখ(Chars, অঙ্ক_To_Write);
    অঙ্ক_Chars -= অঙ্ক_To_Write;
  }
  ফেরত OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned অঙ্ক_Spaces) {
  ফেরত write_padding<' '>(*এই, অঙ্ক_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned অঙ্ক_Zeros) {
  ফেরত write_padding<'\0'>(*এই, অঙ্ক_Zeros);
}

যৌক্তিক raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  যদি (!ColorEnabled)
    ফেরত মিথ্যা;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  যদি (sys::Process::ColorNeedsFlush() && !is_displayed())
    ফেরত মিথ্যা;

  যদি (sys::Process::ColorNeedsFlush())
    flush();

  ফেরত সত্য;
}

raw_ostream &raw_ostream::changeColor(গণনা Colors colors, যৌক্তিক bold, যৌক্তিক bg) {
  যদি (!prepare_colors())
    ফেরত *এই;

  ধ্রুবক অক্ষর *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<অক্ষর>(colors), bold, bg);
  যদি (colorcode)
    লিখ(colorcode, strlen(colorcode));
  ফেরত *এই;
}

raw_ostream &raw_ostream::resetColor() {
  যদি (!prepare_colors())
    ফেরত *এই;

  যদি (ধ্রুবক অক্ষর *colorcode = sys::Process::ResetColor())
    লিখ(colorcode, strlen(colorcode));
  ফেরত *এই;
}

raw_ostream &raw_ostream::reverseColor() {
  যদি (!prepare_colors())
    ফেরত *এই;

  যদি (ধ্রুবক অক্ষর *colorcode = sys::Process::OutputReverse())
    লিখ(colorcode, strlen(colorcode));
  ফেরত *এই;
}

শূন্য raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

স্থির পূর্ণসংখ্যা getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  প্রতিজ্ঞা((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  যদি (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    ফেরত STDOUT_FILENO;
  }

  পূর্ণসংখ্যা FD;
  যদি (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  অন্যথা
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  যদি (EC)
    ফেরত -1;

  ফেরত FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::OF_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_code &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::OF_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_code &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::OF_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_code &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_code &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), সত্য) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(পূর্ণসংখ্যা fd, যৌক্তিক shouldClose, যৌক্তিক unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (FD < 0 ) {
    ShouldClose = মিথ্যা;
    ফেরত;
  }

  enable_colors(সত্য);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  যদি (FD <= STDERR_FILENO)
    ShouldClose = মিথ্যা;

#ifdef _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#endif

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_status Status;
  std::error_code EC = status(FD, Status);
  IsRegularFile = Status.type() == sys::fs::file_type::regular_file;
#ifdef _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#অন্যথা
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  যদি (!SupportsSeeking)
    pos = 0;
  অন্যথা
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  যদি (FD >= 0) {
    flush();
    যদি (ShouldClose) {
      যদি (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  যদি (FD == 2) ফেরত;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  যদি (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#যদি defined(_WIN32)
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
স্থির যৌক্তিক write_console_impl(পূর্ণসংখ্যা FD, StringRef তথ্য_2) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  যদি (auto EC = sys::windows::UTF8ToUTF16(তথ্য_2, WideText))
    ফেরত মিথ্যা;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_আকার = WideText.আকার_3();
  যদি (!RunningWindows8OrGreater())
    Max_Write_আকার = 32767;

  size_t WCharsWritten = 0;
  করো {
    size_t WCharsToWrite =
        std::min(Max_Write_আকার, WideText.আকার_3() - WCharsWritten);
    DWORD ActuallyWritten;
    যৌক্তিক Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    যদি (!Success)
      ফেরত মিথ্যা;

    WCharsWritten += ActuallyWritten;
  } যখন (WCharsWritten != WideText.আকার_3());
  ফেরত সত্য;
}
#endif

শূন্য raw_fd_ostream::write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  যদি (TiedStream)
    TiedStream->flush();

  প্রতিজ্ঞা(FD >= 0 && "File already closed.");
  pos += আকার_2;

#যদি defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  যদি (IsWindowsConsole)
    যদি (write_console_impl(FD, StringRef(Ptr, আকার_2)))
      ফেরত;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_আকার = INT32_MAX;

#যদি defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_আকার = 1024 * 1024 * 1024;
#endif

  করো {
    size_t Chunk_আকার = std::min(আকার_2, Max_Write_আকার);
    ssize_t ret = ::লিখ(FD, Ptr, Chunk_আকার);

    যদি (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      যদি (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        চালু;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      যদি (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_তথ্য && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      বিরতি;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    আকার_2 -= ret;
  } যখন (আকার_2 > 0);
}

শূন্য raw_fd_ostream::close() {
  প্রতিজ্ঞা(ShouldClose);
  ShouldClose = মিথ্যা;
  flush();
  যদি (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  প্রতিজ্ঞা(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#অন্যথা
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  যদি (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  ফেরত pos;
}

শূন্য raw_fd_ostream::pwrite_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  লিখ(Ptr, আকার_2);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_বাফার_আকার() ধ্রুবক {
#যদি defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  যদি (IsWindowsConsole)
    ফেরত 0;
  ফেরত raw_ostream::preferred_বাফার_আকার();
#নতুবা_যদি defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ফেরত raw_ostream::preferred_বাফার_আকার();
#অন্যথা
  প্রতিজ্ঞা(FD >= 0 && "File not yet open!");
  কাঠামো stat statbuf;
  যদি (fstat(FD, &statbuf) != 0)
    ফেরত 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  যদি (S_ISCHR(statbuf.st_mode) && is_displayed())
    ফেরত 0;
  // Return the preferred block size.
  ফেরত statbuf.st_blksize;
#endif
}

যৌক্তিক raw_fd_ostream::is_displayed() ধ্রুবক {
  ফেরত sys::Process::FileDescriptorIsDisplayed(FD);
}

যৌক্তিক raw_fd_ostream::has_colors() ধ্রুবক {
  যদি (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ফেরত *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  যদি (!EC)
    ফেরত sys::fs::FileLocker(FD);
  ফেরত errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ধ্রুবক& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  যদি (!EC)
    ফেরত sys::fs::FileLocker(FD);
  ফেরত errorCodeToError(EC);
}

শূন্য raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  স্থির std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  প্রতিজ্ঞা(!EC1);
  (শূন্য)EC1;

  স্থির raw_fd_ostream S("-", EC, sys::fs::OF_None);
  প্রতিজ্ঞা(!EC);
  ফেরত S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  স্থির std::error_code EC = enableAutoConversion(STDERR_FILENO);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  // Set standard error to be unbuffered.
  স্থির raw_fd_ostream S(STDERR_FILENO, মিথ্যা, সত্য);
  ফেরত S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  স্থির raw_null_ostream S;
  ফেরত S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     সত্য, মিথ্যা, OStreamKind::OK_FDStream) {
  যদি (EC)
    ফেরত;

  যদি (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(পূর্ণসংখ্যা fd, যৌক্তিক shouldClose)
    : raw_fd_ostream(fd, shouldClose, মিথ্যা, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::পঠ(অক্ষর *Ptr, size_t আকার_2) {
  প্রতিজ্ঞা(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::পঠ(get_fd(), (শূন্য *)Ptr, আকার_2);
  যদি (Ret >= 0)
    inc_pos(Ret);
  অন্যথা
    error_detected(errnoAsErrorCode());
  ফেরত Ret;
}

যৌক্তিক raw_fd_stream::classof(ধ্রুবক raw_ostream *OS) {
  ফেরত OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

শূন্য raw_string_ostream::write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  OS.append(Ptr, আকার_2);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ধ্রুবক { ফেরত OS.আকার_3(); }

শূন্য raw_svector_ostream::write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
  OS.append(Ptr, Ptr + আকার_2);
}

শূন্য raw_svector_ostream::pwrite_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2,
                                      uint64_t Offset) {
  memcpy(OS.তথ্য() + Offset, Ptr, আকার_2);
}

যৌক্তিক raw_svector_ostream::classof(ধ্রুবক raw_ostream *OS) {
  ফেরত OS->get_kind() == OStreamKind::OK_SVecStream;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#ifndef NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#endif
}

শূন্য raw_null_ostream::write_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2) {
}

uint64_t raw_null_ostream::current_pos() ধ্রুবক {
  ফেরত 0;
}

শূন্য raw_null_ostream::pwrite_impl(ধ্রুবক অক্ষর *Ptr, size_t আকার_2,
                                   uint64_t Offset) {}

শূন্য raw_pwrite_stream::anchor() {}

শূন্য বাফার_ostream::anchor() {}

শূন্য বাফার_unique_ostream::anchor() {}

স্থির যৌক্তিক isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  যদি (OutputFileName.equals_insensitive("NUL"))
    ফেরত সত্য;
#endif
  ফেরত OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  যদি (OutputFileName == "-")
    ফেরত Write(outs());

  যদি (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    ফেরত Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::অস্থায়ী_File> অস্থায়ী =
      sys::fs::অস্থায়ী_File::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  যদি (!অস্থায়ী)
    ফেরত createFileError(OutputFileName, অস্থায়ী.takeError());

#যদি defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  যদি (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), অস্থায়ী->FD)) {
    যদি (EC != std::errc::no_such_file_or_directory)
      ফেরত createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(অস্থায়ী->FD, মিথ্যা);

  যদি (Error E = Write(Out)) {
    যদি (Error DiscardError = অস্থায়ী->discard())
      ফেরত joinErrors(std::move(E), std::move(DiscardError));
    ফেরত E;
  }
  Out.flush();

  ফেরত অস্থায়ী->keep(OutputFileName);
}
