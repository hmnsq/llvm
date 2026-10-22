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

#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ "llvm/ADT/StringExtras.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Support/AutoConvert.h"
#অন্তর্ভাৱ "llvm/Support/Compiler.h"
#অন্তর্ভাৱ "llvm/Support/Duration.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/Format.h"
#অন্তর্ভাৱ "llvm/Support/FormatVariadic.h"
#অন্তর্ভাৱ "llvm/Support/IOSandbox.h"
#অন্তর্ভাৱ "llvm/Support/MathExtras.h"
#অন্তর্ভাৱ "llvm/Support/NativeFormatting.h"
#অন্তর্ভাৱ "llvm/Support/Process.h"
#অন্তর্ভাৱ "llvm/Support/Program.h"
#অন্তর্ভাৱ <algorithm>
#অন্তর্ভাৱ <cerrno>
#অন্তর্ভাৱ <cstdio>
#অন্তর্ভাৱ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#যদি defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#যদি defined(__CYGWIN__)
#অন্তর্ভাৱ <io.h>
#endif

#যদি defined(_MSC_VER)
#অন্তর্ভাৱ <io.h>
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
#অন্তর্ভাৱ "llvm/Support/ConvertUTF.h"
#অন্তর্ভাৱ "llvm/Support/Signals.h"
#অন্তর্ভাৱ "llvm/Support/Windows/WindowsSupport.h"
#endif

using নামাকাশ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  প্রতিজ্ঞা(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  যদি (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ধ্রুৱ {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ঘূৰাই (16 * 1024);
#অন্যথা
  // BUFSIZ is intended to be a reasonable default.
  ঘূৰাই BUFSIZ;
#endif
}

শূন্য raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  যদি (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  অন্যথা
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

শূন্য raw_ostream::SetBufferAndMode(ৱর্ণ *BufferStart, size_t Size,
                                   BufferKind Mode) {
  প্রতিজ্ঞা(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  প্রতিজ্ঞা(Get_অঙ্ক_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  যদি (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  প্রতিজ্ঞা(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*এতত্, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*এতত্, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*এতত্, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*এতত্, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*এতত্, N, HexPrintStyle::Lower);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  যদি (C == Colors::RESET)
    resetColor();
  অন্যথা
    changeColor(C);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::write_uuid(ধ্রুৱ uuid_t UUID) {
  বাবে (পূর্ণাঙ্ক Idx = 0; Idx < 16; ++Idx) {
    *এতত্ << format("%02" PRIX32, UUID[Idx]);
    যদি (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *এতত্ << "-";
  }
  ঘূৰাই *এতত্;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        তার্কিক UseHexEscapes) {
  বাবে (unsigned ৱর্ণ c : Str) {
    ৱিকল্প (c) {
    পক্ষ '\\':
      *এতত্ << '\\' << '\\';
      ভঙ্গ;
    পক্ষ '\t':
      *এতত্ << '\\' << 't';
      ভঙ্গ;
    পক্ষ '\n':
      *এতত্ << '\\' << 'n';
      ভঙ্গ;
    পক্ষ '"':
      *এতত্ << '\\' << '"';
      ভঙ্গ;
    স্ৱভাৱ:
      যদি (isPrint(c)) {
        *এতত্ << c;
        ভঙ্গ;
      }

      // Write out the escaped representation.
      যদি (UseHexEscapes) {
        *এতত্ << '\\' << 'x';
        *এতত্ << hexdigit((c >> 4) & 0xF);
        *এতত্ << hexdigit((c >> 0) & 0xF);
      } অন্যথা {
        // Always use a full 3-character octal escape.
        *এতত্ << '\\';
        *এতত্ << ৱর্ণ('0' + ((c >> 6) & 7));
        *এতত্ << ৱর্ণ('0' + ((c >> 3) & 7));
        *এতত্ << ৱর্ণ('0' + ((c >> 0) & 7));
      }
    }
  }

  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(ধ্রুৱ শূন্য *P) {
  llvm::write_hex(*এতত্, (uintptr_t)P, HexPrintStyle::PrefixLower);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(দ্ৱিগুণ N) {
  llvm::write_double(*এতত্, N, FloatStyle::Exponent);
  ঘূৰাই *এতত্;
}

শূন্য raw_ostream::flush_nonempty() {
  প্রতিজ্ঞা(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::লিখ(unsigned ৱর্ণ C) {
  // Group exceptional cases into a single branch.
  যদি (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    যদি (LLVM_UNLIKELY(!OutBufStart)) {
      যদি (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<ৱর্ণ *>(&C), 1);
        ঘূৰাই *এতত্;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ঘূৰাই লিখ(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::লিখ(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  যদি (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    যদি (LLVM_UNLIKELY(!OutBufStart)) {
      যদি (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        ঘূৰাই *এতত্;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ঘূৰাই লিখ(Ptr, Size);
    }

    size_t অঙ্ক_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    যদি (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      প্রতিজ্ঞা(অঙ্ক_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % অঙ্ক_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      যদি (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        ঘূৰাই লিখ(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      ঘূৰাই *এতত্;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, অঙ্ক_Bytes);
    flush_nonempty();
    ঘূৰাই লিখ(Ptr + অঙ্ক_Bytes, Size - অঙ্ক_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  ঘূৰাই *এতত্;
}

শূন্য raw_ostream::copy_to_buffer(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
  প্রতিজ্ঞা(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  ৱিকল্প (Size) {
  পক্ষ 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  পক্ষ 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  পক্ষ 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  পক্ষ 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  পক্ষ 0: ভঙ্গ;
  স্ৱভাৱ:
    memcpy(OutBufCur, Ptr, Size);
    ভঙ্গ;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<পূর্ণাঙ্ক(ৱর্ণ *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_বাম = OutBufEnd - OutBufCur;
  যদি (Buffer_Bytes_বাম > 3) {
    পূর্ণাঙ্ক N = Snprint(OutBufCur, Buffer_Bytes_বাম);
    যদি (N < 0)
      ঘূৰাই *এতত্;
    // Common case is that we have plenty of space.
    যদি (size_t(N) < Buffer_Bytes_বাম) {
      OutBufCur += N;
      ঘূৰাই *এতত্;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<ৱর্ণ, 128> V;
  বাবে (;;) {
    V.resize(Size);
    পূর্ণাঙ্ক N = Snprint(V.data(), Size);
    যদি (N < 0)
      ঘূৰাই *এতত্;
    যদি (size_t(N) < Size)
      ঘূৰাই লিখ(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ধ্রুৱ formatv_object_base &Obj) {
  Obj.format(*এতত্);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(ধ্রুৱ FormattedString &FS) {
  unsigned বাম_Indent = 0;
  unsigned ডান_Indent = 0;
  ধ্রুৱ ssize_t Difference = FS.Width - FS.Str.size();
  যদি (Difference > 0) {
    ৱিকল্প (FS.Justify) {
    পক্ষ FormattedString::JustifyNone:
      ভঙ্গ;
    পক্ষ FormattedString::Justify_বাম:
      ডান_Indent = Difference;
      ভঙ্গ;
    পক্ষ FormattedString::Justify_ডান:
      বাম_Indent = Difference;
      ভঙ্গ;
    পক্ষ FormattedString::JustifyCenter:
      বাম_Indent = Difference / 2;
      ডান_Indent = Difference - বাম_Indent;
      ভঙ্গ;
    }
  }
  indent(বাম_Indent);
  (*এতত্) << FS.Str;
  indent(ডান_Indent);
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(ধ্রুৱ Formatted_সংখ্যা &FN) {
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
    llvm::write_hex(*এতত্, FN.Hex_মান, Style, FN.Width);
  } অন্যথা {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_মান, 0, IntegerStyle::Integer);
    যদি (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*এতত্) << Buffer;
  }
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::operator<<(ধ্রুৱ FormattedBytes &FB) {
  যদি (FB.Bytes.empty())
    ঘূৰাই *এতত্;

  size_t সাৰি_Index = 0;
  auto Bytes = FB.Bytes;
  ধ্রুৱ size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  যদি (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t সাৰিগুলি = Size / FB.অঙ্ক_Per_সাৰি;
    uint64_t MaxOffset = *FB.FirstByteOffset + সাৰিগুলি * FB.অঙ্ক_Per_সাৰি;
    unsigned Power = 0;
    যদি (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned অঙ্ক_Byte_Groups =
      alignTo(FB.অঙ্ক_Per_সাৰি, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.অঙ্ক_Per_সাৰি * 2 + অঙ্ক_Byte_Groups - 1;

  যেতিয়ালৈ (!Bytes.empty()) {
    indent(FB.IndentLevel);

    যদি (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*এতত্, Offset + সাৰি_Index, HPS, OffsetWidth);
      *এতত্ << ": ";
    }

    auto সাৰি = Bytes.take_front(FB.অঙ্ক_Per_সাৰি);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    বাবে (size_t I = 0; I < সাৰি.size(); ++I, CharsPrinted += 2) {
      যদি (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *এতত্ << " ";
      }
      llvm::write_hex(*এতত্, সাৰি[I], HPS, 2);
    }

    যদি (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      প্রতিজ্ঞা(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *এতত্ << "|";

      // Print the ASCII char values for each byte on this line
      বাবে (uint8_t Byte : সাৰি) {
        যদি (isPrint(Byte))
          *এতত্ << static_cast<ৱর্ণ>(Byte);
        অন্যথা
          *এতত্ << '.';
      }
      *এতত্ << '|';
    }

    Bytes = Bytes.drop_front(সাৰি.size());
    সাৰি_Index += সাৰি.size();
    যদি (সাৰি_Index < Size)
      *এতত্ << '\n';
  }
  ঘূৰাই *এতত্;
}

template <ৱর্ণ C>
স্থির raw_ostream &write_padding(raw_ostream &OS, unsigned অঙ্ক_Chars) {
  স্থির ধ্রুৱ ৱর্ণ Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  যদি (অঙ্ক_Chars < std::size(Chars))
    ঘূৰাই OS.লিখ(Chars, অঙ্ক_Chars);

  যেতিয়ালৈ (অঙ্ক_Chars) {
    unsigned অঙ্ক_To_Write = std::min(অঙ্ক_Chars, (unsigned)std::size(Chars) - 1);
    OS.লিখ(Chars, অঙ্ক_To_Write);
    অঙ্ক_Chars -= অঙ্ক_To_Write;
  }
  ঘূৰাই OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned অঙ্ক_Spaces) {
  ঘূৰাই write_padding<' '>(*এতত্, অঙ্ক_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned অঙ্ক_Zeros) {
  ঘূৰাই write_padding<'\0'>(*এতত্, অঙ্ক_Zeros);
}

তার্কিক raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  যদি (!ColorEnabled)
    ঘূৰাই মিছা;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  যদি (sys::Process::ColorNeedsFlush() && !is_displayed())
    ঘূৰাই মিছা;

  যদি (sys::Process::ColorNeedsFlush())
    flush();

  ঘূৰাই সঁচা;
}

raw_ostream &raw_ostream::changeColor(গণনা Colors colors, তার্কিক bold, তার্কিক bg) {
  যদি (!prepare_colors())
    ঘূৰাই *এতত্;

  ধ্রুৱ ৱর্ণ *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<ৱর্ণ>(colors), bold, bg);
  যদি (colorcode)
    লিখ(colorcode, strlen(colorcode));
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::resetColor() {
  যদি (!prepare_colors())
    ঘূৰাই *এতত্;

  যদি (ধ্রুৱ ৱর্ণ *colorcode = sys::Process::ResetColor())
    লিখ(colorcode, strlen(colorcode));
  ঘূৰাই *এতত্;
}

raw_ostream &raw_ostream::reverseColor() {
  যদি (!prepare_colors())
    ঘূৰাই *এতত্;

  যদি (ধ্রুৱ ৱর্ণ *colorcode = sys::Process::OutputReverse())
    লিখ(colorcode, strlen(colorcode));
  ঘূৰাই *এতত্;
}

শূন্য raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

স্থির পূর্ণাঙ্ক getFD(StringRef Filename, std::error_code &EC,
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
    ঘূৰাই STDOUT_FILENO;
  }

  পূর্ণাঙ্ক FD;
  যদি (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  অন্যথা
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  যদি (EC)
    ঘূৰাই -1;

  ঘূৰাই FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), সঁচা) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(পূর্ণাঙ্ক fd, তার্কিক shouldClose, তার্কিক unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  যদি (FD < 0 ) {
    ShouldClose = মিছা;
    ঘূৰাই;
  }

  enable_colors(সঁচা);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  যদি (FD <= STDERR_FILENO)
    ShouldClose = মিছা;

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
  যদি (FD == 2) ঘূৰাই;
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
স্থির তার্কিক write_console_impl(পূর্ণাঙ্ক FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  যদি (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    ঘূৰাই মিছা;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  যদি (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  কৰা {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    তার্কিক Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    যদি (!Success)
      ঘূৰাই মিছা;

    WCharsWritten += ActuallyWritten;
  } যেতিয়ালৈ (WCharsWritten != WideText.size());
  ঘূৰাই সঁচা;
}
#endif

শূন্য raw_fd_ostream::write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
  যদি (TiedStream)
    TiedStream->flush();

  প্রতিজ্ঞা(FD >= 0 && "File already closed.");
  pos += Size;

#যদি defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  যদি (IsWindowsConsole)
    যদি (write_console_impl(FD, StringRef(Ptr, Size)))
      ঘূৰাই;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#যদি defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  কৰা {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::লিখ(FD, Ptr, ChunkSize);

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
        অনুৱর্ত;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      যদি (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      ভঙ্গ;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } যেতিয়ালৈ (Size > 0);
}

শূন্য raw_fd_ostream::close() {
  প্রতিজ্ঞা(ShouldClose);
  ShouldClose = মিছা;
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
  ঘূৰাই pos;
}

শূন্য raw_fd_ostream::pwrite_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  লিখ(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ধ্রুৱ {
#যদি defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  যদি (IsWindowsConsole)
    ঘূৰাই 0;
  ঘূৰাই raw_ostream::preferred_buffer_size();
#নোচেদ্যদি defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ঘূৰাই raw_ostream::preferred_buffer_size();
#অন্যথা
  প্রতিজ্ঞা(FD >= 0 && "File not yet open!");
  সংরচনা stat statbuf;
  যদি (fstat(FD, &statbuf) != 0)
    ঘূৰাই 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  যদি (S_ISCHR(statbuf.st_mode) && is_displayed())
    ঘূৰাই 0;
  // Return the preferred block size.
  ঘূৰাই statbuf.st_blksize;
#endif
}

তার্কিক raw_fd_ostream::is_displayed() ধ্রুৱ {
  ঘূৰাই sys::Process::FileDescriptorIsDisplayed(FD);
}

তার্কিক raw_fd_ostream::has_colors() ধ্রুৱ {
  যদি (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ঘূৰাই *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  যদি (!EC)
    ঘূৰাই sys::fs::FileLocker(FD);
  ঘূৰাই errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ধ্রুৱ& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  যদি (!EC)
    ঘূৰাই sys::fs::FileLocker(FD);
  ঘূৰাই errorCodeToError(EC);
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
  ঘূৰাই S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  স্থির std::error_code EC = enableAutoConversion(STDERR_FILENO);
  প্রতিজ্ঞা(!EC);
  (শূন্য)EC;

  // Set standard error to be unbuffered.
  স্থির raw_fd_ostream S(STDERR_FILENO, মিছা, সঁচা);
  ঘূৰাই S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  স্থির raw_null_ostream S;
  ঘূৰাই S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     সঁচা, মিছা, OStreamKind::OK_FDStream) {
  যদি (EC)
    ঘূৰাই;

  যদি (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(পূর্ণাঙ্ক fd, তার্কিক shouldClose)
    : raw_fd_ostream(fd, shouldClose, মিছা, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::পঠ(ৱর্ণ *Ptr, size_t Size) {
  প্রতিজ্ঞা(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::পঠ(get_fd(), (শূন্য *)Ptr, Size);
  যদি (Ret >= 0)
    inc_pos(Ret);
  অন্যথা
    error_detected(errnoAsErrorCode());
  ঘূৰাই Ret;
}

তার্কিক raw_fd_stream::classof(ধ্রুৱ raw_ostream *OS) {
  ঘূৰাই OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

শূন্য raw_string_ostream::write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ধ্রুৱ { ঘূৰাই OS.size(); }

শূন্য raw_svector_ostream::write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

শূন্য raw_svector_ostream::pwrite_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

তার্কিক raw_svector_ostream::classof(ধ্রুৱ raw_ostream *OS) {
  ঘূৰাই OS->get_kind() == OStreamKind::OK_SVecStream;
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

শূন্য raw_null_ostream::write_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() ধ্রুৱ {
  ঘূৰাই 0;
}

শূন্য raw_null_ostream::pwrite_impl(ধ্রুৱ ৱর্ণ *Ptr, size_t Size,
                                   uint64_t Offset) {}

শূন্য raw_pwrite_stream::anchor() {}

শূন্য buffer_ostream::anchor() {}

শূন্য buffer_unique_ostream::anchor() {}

স্থির তার্কিক isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  যদি (OutputFileName.equals_insensitive("NUL"))
    ঘূৰাই সঁচা;
#endif
  ঘূৰাই OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  যদি (OutputFileName == "-")
    ঘূৰাই Write(outs());

  যদি (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    ঘূৰাই Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  যদি (!Temp)
    ঘূৰাই createFileError(OutputFileName, Temp.takeError());

#যদি defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  যদি (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    যদি (EC != std::errc::no_such_file_or_directory)
      ঘূৰাই createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, মিছা);

  যদি (Error E = Write(Out)) {
    যদি (Error DiscardError = Temp->discard())
      ঘূৰাই joinErrors(std::move(E), std::move(DiscardError));
    ঘূৰাই E;
  }
  Out.flush();

  ঘূৰাই Temp->keep(OutputFileName);
}
