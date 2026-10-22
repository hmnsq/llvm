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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/AutoConvert.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Duration.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FormatVariadic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MathExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/NativeFormatting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Process.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Program.h"
#ଅନ୍ତର୍ଭାଵ <algorithm>
#ଅନ୍ତର୍ଭାଵ <cerrno>
#ଅନ୍ତର୍ଭାଵ <cstdio>
#ଅନ୍ତର୍ଭାଵ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ଯଦି defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#ଯଦି defined(__CYGWIN__)
#ଅନ୍ତର୍ଭାଵ <io.h>
#endif

#ଯଦି defined(_MSC_VER)
#ଅନ୍ତର୍ଭାଵ <io.h>
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
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ConvertUTF.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Windows/WindowsSupport.h"
#endif

using ନାମାକାଶ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  ପ୍ରତିଜ୍ଞା(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  ଯଦି (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ଅଚଳ {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ଫେରନ୍ତୁ (16 * 1024);
#ଅନ୍ୟଥା
  // BUFSIZ is intended to be a reasonable default.
  ଫେରନ୍ତୁ BUFSIZ;
#endif
}

ଶୂନ୍ୟ raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ଯଦି (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  ଅନ୍ୟଥା
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

ଶୂନ୍ୟ raw_ostream::SetBufferAndMode(ଅକ୍ଷର *BufferStart, size_t Size,
                                   BufferKind Mode) {
  ପ୍ରତିଜ୍ଞା(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  ପ୍ରତିଜ୍ଞା(Get_ଅଙ୍କ_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  ଯଦି (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  ପ୍ରତିଜ୍ଞା(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*ଏହି, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*ଏହି, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*ଏହି, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*ଏହି, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*ଏହି, N, HexPrintStyle::Lower);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  ଯଦି (C == Colors::RESET)
    resetColor();
  ଅନ୍ୟଥା
    changeColor(C);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::write_uuid(ଅଚଳ uuid_t UUID) {
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା Idx = 0; Idx < 16; ++Idx) {
    *ଏହି << format("%02" PRIX32, UUID[Idx]);
    ଯଦି (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *ଏହି << "-";
  }
  ଫେରନ୍ତୁ *ଏହି;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        ତାର୍କିକ UseHexEscapes) {
  ପାଇଁ (unsigned ଅକ୍ଷର c : Str) {
    ବାଛ (c) {
    ସ୍ଥିତି '\\':
      *ଏହି << '\\' << '\\';
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '\t':
      *ଏହି << '\\' << 't';
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '\n':
      *ଏହି << '\\' << 'n';
      ଭାଙ୍ଗ;
    ସ୍ଥିତି '"':
      *ଏହି << '\\' << '"';
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଯଦି (isPrint(c)) {
        *ଏହି << c;
        ଭାଙ୍ଗ;
      }

      // Write out the escaped representation.
      ଯଦି (UseHexEscapes) {
        *ଏହି << '\\' << 'x';
        *ଏହି << hexdigit((c >> 4) & 0xF);
        *ଏହି << hexdigit((c >> 0) & 0xF);
      } ଅନ୍ୟଥା {
        // Always use a full 3-character octal escape.
        *ଏହି << '\\';
        *ଏହି << ଅକ୍ଷର('0' + ((c >> 6) & 7));
        *ଏହି << ଅକ୍ଷର('0' + ((c >> 3) & 7));
        *ଏହି << ଅକ୍ଷର('0' + ((c >> 0) & 7));
      }
    }
  }

  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(ଅଚଳ ଶୂନ୍ୟ *P) {
  llvm::write_hex(*ଏହି, (uintptr_t)P, HexPrintStyle::PrefixLower);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(ଦ୍ୱିଗୁଣ N) {
  llvm::write_double(*ଏହି, N, FloatStyle::Exponent);
  ଫେରନ୍ତୁ *ଏହି;
}

ଶୂନ୍ୟ raw_ostream::flush_nonempty() {
  ପ୍ରତିଜ୍ଞା(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::ଲିଖ(unsigned ଅକ୍ଷର C) {
  // Group exceptional cases into a single branch.
  ଯଦି (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    ଯଦି (LLVM_UNLIKELY(!OutBufStart)) {
      ଯଦି (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<ଅକ୍ଷର *>(&C), 1);
        ଫେରନ୍ତୁ *ଏହି;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ଫେରନ୍ତୁ ଲିଖ(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::ଲିଖ(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  ଯଦି (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    ଯଦି (LLVM_UNLIKELY(!OutBufStart)) {
      ଯଦି (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        ଫେରନ୍ତୁ *ଏହି;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ଫେରନ୍ତୁ ଲିଖ(Ptr, Size);
    }

    size_t ଅଙ୍କ_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ଯଦି (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      ପ୍ରତିଜ୍ଞା(ଅଙ୍କ_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % ଅଙ୍କ_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      ଯଦି (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        ଫେରନ୍ତୁ ଲିଖ(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      ଫେରନ୍ତୁ *ଏହି;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, ଅଙ୍କ_Bytes);
    flush_nonempty();
    ଫେରନ୍ତୁ ଲିଖ(Ptr + ଅଙ୍କ_Bytes, Size - ଅଙ୍କ_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  ଫେରନ୍ତୁ *ଏହି;
}

ଶୂନ୍ୟ raw_ostream::copy_to_buffer(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  ପ୍ରତିଜ୍ଞା(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  ବାଛ (Size) {
  ସ୍ଥିତି 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  ସ୍ଥିତି 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  ସ୍ଥିତି 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  ସ୍ଥିତି 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  ସ୍ଥିତି 0: ଭାଙ୍ଗ;
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    memcpy(OutBufCur, Ptr, Size);
    ଭାଙ୍ଗ;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<ପୂର୍ଣ୍ଣସଂଖ୍ୟା(ଅକ୍ଷର *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_ବାମ = OutBufEnd - OutBufCur;
  ଯଦି (Buffer_Bytes_ବାମ > 3) {
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା N = Snprint(OutBufCur, Buffer_Bytes_ବାମ);
    ଯଦି (N < 0)
      ଫେରନ୍ତୁ *ଏହି;
    // Common case is that we have plenty of space.
    ଯଦି (size_t(N) < Buffer_Bytes_ବାମ) {
      OutBufCur += N;
      ଫେରନ୍ତୁ *ଏହି;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<ଅକ୍ଷର, 128> V;
  ପାଇଁ (;;) {
    V.resize(Size);
    ପୂର୍ଣ୍ଣସଂଖ୍ୟା N = Snprint(V.data(), Size);
    ଯଦି (N < 0)
      ଫେରନ୍ତୁ *ଏହି;
    ଯଦି (size_t(N) < Size)
      ଫେରନ୍ତୁ ଲିଖ(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ଅଚଳ formatv_object_base &Obj) {
  Obj.format(*ଏହି);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(ଅଚଳ FormattedString &FS) {
  unsigned ବାମ_Indent = 0;
  unsigned ଡାହାଣ_Indent = 0;
  ଅଚଳ ssize_t Difference = FS.Width - FS.Str.size();
  ଯଦି (Difference > 0) {
    ବାଛ (FS.Justify) {
    ସ୍ଥିତି FormattedString::JustifyNone:
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FormattedString::Justify_ବାମ:
      ଡାହାଣ_Indent = Difference;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FormattedString::Justify_ଡାହାଣ:
      ବାମ_Indent = Difference;
      ଭାଙ୍ଗ;
    ସ୍ଥିତି FormattedString::JustifyCenter:
      ବାମ_Indent = Difference / 2;
      ଡାହାଣ_Indent = Difference - ବାମ_Indent;
      ଭାଙ୍ଗ;
    }
  }
  indent(ବାମ_Indent);
  (*ଏହି) << FS.Str;
  indent(ଡାହାଣ_Indent);
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(ଅଚଳ Formatted_ସଂଖ୍ୟା &FN) {
  ଯଦି (FN.Hex) {
    HexPrintStyle Style;
    ଯଦି (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    ଅନ୍ୟଥା ଯଦି (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    ଅନ୍ୟଥା ଯଦି (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    ଅନ୍ୟଥା
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*ଏହି, FN.Hex_ମୂଲ୍ୟ, Style, FN.Width);
  } ଅନ୍ୟଥା {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_ମୂଲ୍ୟ, 0, IntegerStyle::Integer);
    ଯଦି (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*ଏହି) << Buffer;
  }
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::operator<<(ଅଚଳ FormattedBytes &FB) {
  ଯଦି (FB.Bytes.empty())
    ଫେରନ୍ତୁ *ଏହି;

  size_t ଧାଡ଼ି_Index = 0;
  auto Bytes = FB.Bytes;
  ଅଚଳ size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  ଯଦି (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t ଧାଡ଼ିଗୁଡ଼ିକ = Size / FB.ଅଙ୍କ_Per_ଧାଡ଼ି;
    uint64_t MaxOffset = *FB.FirstByteOffset + ଧାଡ଼ିଗୁଡ଼ିକ * FB.ଅଙ୍କ_Per_ଧାଡ଼ି;
    unsigned Power = 0;
    ଯଦି (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned ଅଙ୍କ_Byte_Groups =
      alignTo(FB.ଅଙ୍କ_Per_ଧାଡ଼ି, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.ଅଙ୍କ_Per_ଧାଡ଼ି * 2 + ଅଙ୍କ_Byte_Groups - 1;

  ଯେତେବେଳେ (!Bytes.empty()) {
    indent(FB.IndentLevel);

    ଯଦି (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*ଏହି, Offset + ଧାଡ଼ି_Index, HPS, OffsetWidth);
      *ଏହି << ": ";
    }

    auto ଧାଡ଼ି = Bytes.take_front(FB.ଅଙ୍କ_Per_ଧାଡ଼ି);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    ପାଇଁ (size_t I = 0; I < ଧାଡ଼ି.size(); ++I, CharsPrinted += 2) {
      ଯଦି (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *ଏହି << " ";
      }
      llvm::write_hex(*ଏହି, ଧାଡ଼ି[I], HPS, 2);
    }

    ଯଦି (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      ପ୍ରତିଜ୍ଞା(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *ଏହି << "|";

      // Print the ASCII char values for each byte on this line
      ପାଇଁ (uint8_t Byte : ଧାଡ଼ି) {
        ଯଦି (isPrint(Byte))
          *ଏହି << static_cast<ଅକ୍ଷର>(Byte);
        ଅନ୍ୟଥା
          *ଏହି << '.';
      }
      *ଏହି << '|';
    }

    Bytes = Bytes.drop_front(ଧାଡ଼ି.size());
    ଧାଡ଼ି_Index += ଧାଡ଼ି.size();
    ଯଦି (ଧାଡ଼ି_Index < Size)
      *ଏହି << '\n';
  }
  ଫେରନ୍ତୁ *ଏହି;
}

template <ଅକ୍ଷର C>
ସ୍ଥିର raw_ostream &write_padding(raw_ostream &OS, unsigned ଅଙ୍କ_Chars) {
  ସ୍ଥିର ଅଚଳ ଅକ୍ଷର Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ଯଦି (ଅଙ୍କ_Chars < std::size(Chars))
    ଫେରନ୍ତୁ OS.ଲିଖ(Chars, ଅଙ୍କ_Chars);

  ଯେତେବେଳେ (ଅଙ୍କ_Chars) {
    unsigned ଅଙ୍କ_To_Write = std::min(ଅଙ୍କ_Chars, (unsigned)std::size(Chars) - 1);
    OS.ଲିଖ(Chars, ଅଙ୍କ_To_Write);
    ଅଙ୍କ_Chars -= ଅଙ୍କ_To_Write;
  }
  ଫେରନ୍ତୁ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned ଅଙ୍କ_Spaces) {
  ଫେରନ୍ତୁ write_padding<' '>(*ଏହି, ଅଙ୍କ_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned ଅଙ୍କ_Zeros) {
  ଫେରନ୍ତୁ write_padding<'\0'>(*ଏହି, ଅଙ୍କ_Zeros);
}

ତାର୍କିକ raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ଯଦି (!ColorEnabled)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ଯଦି (sys::Process::ColorNeedsFlush() && !is_displayed())
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  ଯଦି (sys::Process::ColorNeedsFlush())
    flush();

  ଫେରନ୍ତୁ ସତ୍ୟ;
}

raw_ostream &raw_ostream::changeColor(ଗଣନା Colors colors, ତାର୍କିକ bold, ତାର୍କିକ bg) {
  ଯଦି (!prepare_colors())
    ଫେରନ୍ତୁ *ଏହି;

  ଅଚଳ ଅକ୍ଷର *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<ଅକ୍ଷର>(colors), bold, bg);
  ଯଦି (colorcode)
    ଲିଖ(colorcode, strlen(colorcode));
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::resetColor() {
  ଯଦି (!prepare_colors())
    ଫେରନ୍ତୁ *ଏହି;

  ଯଦି (ଅଚଳ ଅକ୍ଷର *colorcode = sys::Process::ResetColor())
    ଲିଖ(colorcode, strlen(colorcode));
  ଫେରନ୍ତୁ *ଏହି;
}

raw_ostream &raw_ostream::reverseColor() {
  ଯଦି (!prepare_colors())
    ଫେରନ୍ତୁ *ଏହି;

  ଯଦି (ଅଚଳ ଅକ୍ଷର *colorcode = sys::Process::OutputReverse())
    ଲିଖ(colorcode, strlen(colorcode));
  ଫେରନ୍ତୁ *ଏହି;
}

ଶୂନ୍ୟ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ପ୍ରତିଜ୍ଞା((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ଯଦି (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    ଫେରନ୍ତୁ STDOUT_FILENO;
  }

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD;
  ଯଦି (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  ଅନ୍ୟଥା
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ଯଦି (EC)
    ଫେରନ୍ତୁ -1;

  ଫେରନ୍ତୁ FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), ସତ୍ୟ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ତାର୍କିକ shouldClose, ତାର୍କିକ unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ଯଦି (FD < 0 ) {
    ShouldClose = ମିଥ୍ୟା;
    ଫେରନ୍ତୁ;
  }

  enable_colors(ସତ୍ୟ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ଯଦି (FD <= STDERR_FILENO)
    ShouldClose = ମିଥ୍ୟା;

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
#ଅନ୍ୟଥା
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  ଯଦି (!SupportsSeeking)
    pos = 0;
  ଅନ୍ୟଥା
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ଯଦି (FD >= 0) {
    flush();
    ଯଦି (ShouldClose) {
      ଯଦି (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ଯଦି (FD == 2) ଫେରନ୍ତୁ;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ଯଦି (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#ଯଦି defined(_WIN32)
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
ସ୍ଥିର ତାର୍କିକ write_console_impl(ପୂର୍ଣ୍ଣସଂଖ୍ୟା FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  ଯଦି (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  ଯଦି (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  କର {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    ତାର୍କିକ Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ଯଦି (!Success)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    WCharsWritten += ActuallyWritten;
  } ଯେତେବେଳେ (WCharsWritten != WideText.size());
  ଫେରନ୍ତୁ ସତ୍ୟ;
}
#endif

ଶୂନ୍ୟ raw_fd_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  ଯଦି (TiedStream)
    TiedStream->flush();

  ପ୍ରତିଜ୍ଞା(FD >= 0 && "File already closed.");
  pos += Size;

#ଯଦି defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ଯଦି (IsWindowsConsole)
    ଯଦି (write_console_impl(FD, StringRef(Ptr, Size)))
      ଫେରନ୍ତୁ;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#ଯଦି defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  କର {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::ଲିଖ(FD, Ptr, ChunkSize);

    ଯଦି (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ଯଦି (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        ଜାରି;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      ଯଦି (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      ଭାଙ୍ଗ;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } ଯେତେବେଳେ (Size > 0);
}

ଶୂନ୍ୟ raw_fd_ostream::close() {
  ପ୍ରତିଜ୍ଞା(ShouldClose);
  ShouldClose = ମିଥ୍ୟା;
  flush();
  ଯଦି (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  ପ୍ରତିଜ୍ଞା(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#ଅନ୍ୟଥା
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  ଯଦି (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  ଫେରନ୍ତୁ pos;
}

ଶୂନ୍ୟ raw_fd_ostream::pwrite_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  ଲିଖ(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ଅଚଳ {
#ଯଦି defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ଯଦି (IsWindowsConsole)
    ଫେରନ୍ତୁ 0;
  ଫେରନ୍ତୁ raw_ostream::preferred_buffer_size();
#ନଚେତଯଦି defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ଫେରନ୍ତୁ raw_ostream::preferred_buffer_size();
#ଅନ୍ୟଥା
  ପ୍ରତିଜ୍ଞା(FD >= 0 && "File not yet open!");
  ସଂରଚନା stat statbuf;
  ଯଦି (fstat(FD, &statbuf) != 0)
    ଫେରନ୍ତୁ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ଯଦି (S_ISCHR(statbuf.st_mode) && is_displayed())
    ଫେରନ୍ତୁ 0;
  // Return the preferred block size.
  ଫେରନ୍ତୁ statbuf.st_blksize;
#endif
}

ତାର୍କିକ raw_fd_ostream::is_displayed() ଅଚଳ {
  ଫେରନ୍ତୁ sys::Process::FileDescriptorIsDisplayed(FD);
}

ତାର୍କିକ raw_fd_ostream::has_colors() ଅଚଳ {
  ଯଦି (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ଫେରନ୍ତୁ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  ଯଦି (!EC)
    ଫେରନ୍ତୁ sys::fs::FileLocker(FD);
  ଫେରନ୍ତୁ errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ଅଚଳ& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  ଯଦି (!EC)
    ଫେରନ୍ତୁ sys::fs::FileLocker(FD);
  ଫେରନ୍ତୁ errorCodeToError(EC);
}

ଶୂନ୍ୟ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  ସ୍ଥିର std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  ପ୍ରତିଜ୍ଞା(!EC1);
  (ଶୂନ୍ୟ)EC1;

  ସ୍ଥିର raw_fd_ostream S("-", EC, sys::fs::OF_None);
  ପ୍ରତିଜ୍ଞା(!EC);
  ଫେରନ୍ତୁ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ସ୍ଥିର std::error_code EC = enableAutoConversion(STDERR_FILENO);
  ପ୍ରତିଜ୍ଞା(!EC);
  (ଶୂନ୍ୟ)EC;

  // Set standard error to be unbuffered.
  ସ୍ଥିର raw_fd_ostream S(STDERR_FILENO, ମିଥ୍ୟା, ସତ୍ୟ);
  ଫେରନ୍ତୁ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ସ୍ଥିର raw_null_ostream S;
  ଫେରନ୍ତୁ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     ସତ୍ୟ, ମିଥ୍ୟା, OStreamKind::OK_FDStream) {
  ଯଦି (EC)
    ଫେରନ୍ତୁ;

  ଯଦି (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ତାର୍କିକ shouldClose)
    : raw_fd_ostream(fd, shouldClose, ମିଥ୍ୟା, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::ପଠ(ଅକ୍ଷର *Ptr, size_t Size) {
  ପ୍ରତିଜ୍ଞା(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::ପଠ(get_fd(), (ଶୂନ୍ୟ *)Ptr, Size);
  ଯଦି (Ret >= 0)
    inc_pos(Ret);
  ଅନ୍ୟଥା
    error_detected(errnoAsErrorCode());
  ଫେରନ୍ତୁ Ret;
}

ତାର୍କିକ raw_fd_stream::classof(ଅଚଳ raw_ostream *OS) {
  ଫେରନ୍ତୁ OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

ଶୂନ୍ୟ raw_string_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ଅଚଳ { ଫେରନ୍ତୁ OS.size(); }

ଶୂନ୍ୟ raw_svector_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

ଶୂନ୍ୟ raw_svector_ostream::pwrite_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

ତାର୍କିକ raw_svector_ostream::classof(ଅଚଳ raw_ostream *OS) {
  ଫେରନ୍ତୁ OS->get_kind() == OStreamKind::OK_SVecStream;
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

ଶୂନ୍ୟ raw_null_ostream::write_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() ଅଚଳ {
  ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ raw_null_ostream::pwrite_impl(ଅଚଳ ଅକ୍ଷର *Ptr, size_t Size,
                                   uint64_t Offset) {}

ଶୂନ୍ୟ raw_pwrite_stream::anchor() {}

ଶୂନ୍ୟ buffer_ostream::anchor() {}

ଶୂନ୍ୟ buffer_unique_ostream::anchor() {}

ସ୍ଥିର ତାର୍କିକ isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  ଯଦି (OutputFileName.equals_insensitive("NUL"))
    ଫେରନ୍ତୁ ସତ୍ୟ;
#endif
  ଫେରନ୍ତୁ OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ଯଦି (OutputFileName == "-")
    ଫେରନ୍ତୁ Write(outs());

  ଯଦି (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    ଫେରନ୍ତୁ Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  ଯଦି (!Temp)
    ଫେରନ୍ତୁ createFileError(OutputFileName, Temp.takeError());

#ଯଦି defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ଯଦି (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    ଯଦି (EC != std::errc::no_such_file_or_directory)
      ଫେରନ୍ତୁ createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, ମିଥ୍ୟା);

  ଯଦି (Error E = Write(Out)) {
    ଯଦି (Error DiscardError = Temp->discard())
      ଫେରନ୍ତୁ joinErrors(std::move(E), std::move(DiscardError));
    ଫେରନ୍ତୁ E;
  }
  Out.flush();

  ଫେରନ୍ତୁ Temp->keep(OutputFileName);
}
