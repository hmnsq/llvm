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

#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Duration.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/NativeFormatting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cerrno>
#ಅನ್ತರ್ಭಾವ <cstdio>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ಯದಿ defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#ಯದಿ defined(__CYGWIN__)
#ಅನ್ತರ್ಭಾವ <io.h>
#endif

#ಯದಿ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <io.h>
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
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
#endif

using ನಾಮಾಕಾಶ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  ಪ್ರತಿಜ್ಞಾ(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  ಯದಿ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ಧ್ರುವ {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ಪ್ರತ್ಯಾಗಮ (16 * 1024);
#ಅನ್ಯಥಾ
  // BUFSIZ is intended to be a reasonable default.
  ಪ್ರತ್ಯಾಗಮ BUFSIZ;
#endif
}

ಶೂನ್ಯ raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ಯದಿ (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  ಅನ್ಯಥಾ
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

ಶೂನ್ಯ raw_ostream::SetBufferAndMode(ವರ್ಣ *BufferStart, size_t Size,
                                   BufferKind Mode) {
  ಪ್ರತಿಜ್ಞಾ(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  ಪ್ರತಿಜ್ಞಾ(Get_ಅಂಕೆ_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  ಯದಿ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  ಪ್ರತಿಜ್ಞಾ(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*ಏತತ್, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*ಏತತ್, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*ಏತತ್, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*ಏತತ್, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*ಏತತ್, N, HexPrintStyle::Lower);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  ಯದಿ (C == Colors::RESET)
    resetColor();
  ಅನ್ಯಥಾ
    changeColor(C);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::write_uuid(ಧ್ರುವ uuid_t UUID) {
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ Idx = 0; Idx < 16; ++Idx) {
    *ಏತತ್ << format("%02" PRIX32, UUID[Idx]);
    ಯದಿ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *ಏತತ್ << "-";
  }
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        ತಾರ್ಕಿಕ UseHexEscapes) {
  ಪ್ರತಿ (unsigned ವರ್ಣ c : Str) {
    ವಿಕಲ್ಪ (c) {
    ಪಕ್ಷ '\\':
      *ಏತತ್ << '\\' << '\\';
      ಭಙ್ಗ;
    ಪಕ್ಷ '\t':
      *ಏತತ್ << '\\' << 't';
      ಭಙ್ಗ;
    ಪಕ್ಷ '\n':
      *ಏತತ್ << '\\' << 'n';
      ಭಙ್ಗ;
    ಪಕ್ಷ '"':
      *ಏತತ್ << '\\' << '"';
      ಭಙ್ಗ;
    ಸ್ವಭಾವ:
      ಯದಿ (isPrint(c)) {
        *ಏತತ್ << c;
        ಭಙ್ಗ;
      }

      // Write out the escaped representation.
      ಯದಿ (UseHexEscapes) {
        *ಏತತ್ << '\\' << 'x';
        *ಏತತ್ << hexdigit((c >> 4) & 0xF);
        *ಏತತ್ << hexdigit((c >> 0) & 0xF);
      } ಅನ್ಯಥಾ {
        // Always use a full 3-character octal escape.
        *ಏತತ್ << '\\';
        *ಏತತ್ << ವರ್ಣ('0' + ((c >> 6) & 7));
        *ಏತತ್ << ವರ್ಣ('0' + ((c >> 3) & 7));
        *ಏತತ್ << ವರ್ಣ('0' + ((c >> 0) & 7));
      }
    }
  }

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(ಧ್ರುವ ಶೂನ್ಯ *P) {
  llvm::write_hex(*ಏತತ್, (uintptr_t)P, HexPrintStyle::PrefixLower);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(ದ್ವಿಗುಣ N) {
  llvm::write_double(*ಏತತ್, N, FloatStyle::Exponent);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ಶೂನ್ಯ raw_ostream::flush_nonempty() {
  ಪ್ರತಿಜ್ಞಾ(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::ಲಿಖ(unsigned ವರ್ಣ C) {
  // Group exceptional cases into a single branch.
  ಯದಿ (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    ಯದಿ (LLVM_UNLIKELY(!OutBufStart)) {
      ಯದಿ (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<ವರ್ಣ *>(&C), 1);
        ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ಪ್ರತ್ಯಾಗಮ ಲಿಖ(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::ಲಿಖ(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  ಯದಿ (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    ಯದಿ (LLVM_UNLIKELY(!OutBufStart)) {
      ಯದಿ (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ಪ್ರತ್ಯಾಗಮ ಲಿಖ(Ptr, Size);
    }

    size_t ಅಂಕೆ_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ಯದಿ (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % ಅಂಕೆ_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      ಯದಿ (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        ಪ್ರತ್ಯಾಗಮ ಲಿಖ(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, ಅಂಕೆ_Bytes);
    flush_nonempty();
    ಪ್ರತ್ಯಾಗಮ ಲಿಖ(Ptr + ಅಂಕೆ_Bytes, Size - ಅಂಕೆ_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ಶೂನ್ಯ raw_ostream::copy_to_buffer(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  ಪ್ರತಿಜ್ಞಾ(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  ವಿಕಲ್ಪ (Size) {
  ಪಕ್ಷ 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  ಪಕ್ಷ 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  ಪಕ್ಷ 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  ಪಕ್ಷ 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  ಪಕ್ಷ 0: ಭಙ್ಗ;
  ಸ್ವಭಾವ:
    memcpy(OutBufCur, Ptr, Size);
    ಭಙ್ಗ;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<ಪೂರ್ಣಾಙ್ಕ(ವರ್ಣ *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_ಎಡ = OutBufEnd - OutBufCur;
  ಯದಿ (Buffer_Bytes_ಎಡ > 3) {
    ಪೂರ್ಣಾಙ್ಕ N = Snprint(OutBufCur, Buffer_Bytes_ಎಡ);
    ಯದಿ (N < 0)
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    // Common case is that we have plenty of space.
    ಯದಿ (size_t(N) < Buffer_Bytes_ಎಡ) {
      OutBufCur += N;
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<ವರ್ಣ, 128> V;
  ಪ್ರತಿ (;;) {
    V.resize(Size);
    ಪೂರ್ಣಾಙ್ಕ N = Snprint(V.data(), Size);
    ಯದಿ (N < 0)
      ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
    ಯದಿ (size_t(N) < Size)
      ಪ್ರತ್ಯಾಗಮ ಲಿಖ(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ಧ್ರುವ formatv_object_base &Obj) {
  Obj.format(*ಏತತ್);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(ಧ್ರುವ FormattedString &FS) {
  unsigned ಎಡ_Indent = 0;
  unsigned ಬಲ_Indent = 0;
  ಧ್ರುವ ssize_t Difference = FS.Width - FS.Str.size();
  ಯದಿ (Difference > 0) {
    ವಿಕಲ್ಪ (FS.Justify) {
    ಪಕ್ಷ FormattedString::JustifyNone:
      ಭಙ್ಗ;
    ಪಕ್ಷ FormattedString::Justify_ಎಡ:
      ಬಲ_Indent = Difference;
      ಭಙ್ಗ;
    ಪಕ್ಷ FormattedString::Justify_ಬಲ:
      ಎಡ_Indent = Difference;
      ಭಙ್ಗ;
    ಪಕ್ಷ FormattedString::JustifyCenter:
      ಎಡ_Indent = Difference / 2;
      ಬಲ_Indent = Difference - ಎಡ_Indent;
      ಭಙ್ಗ;
    }
  }
  indent(ಎಡ_Indent);
  (*ಏತತ್) << FS.Str;
  indent(ಬಲ_Indent);
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(ಧ್ರುವ Formatted_ಸಂಖ್ಯೆ &FN) {
  ಯದಿ (FN.Hex) {
    HexPrintStyle Style;
    ಯದಿ (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    ಅನ್ಯಥಾ ಯದಿ (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    ಅನ್ಯಥಾ ಯದಿ (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    ಅನ್ಯಥಾ
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*ಏತತ್, FN.Hex_ಮೌಲ್ಯ, Style, FN.Width);
  } ಅನ್ಯಥಾ {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_ಮೌಲ್ಯ, 0, IntegerStyle::Integer);
    ಯದಿ (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*ಏತತ್) << Buffer;
  }
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::operator<<(ಧ್ರುವ FormattedBytes &FB) {
  ಯದಿ (FB.Bytes.empty())
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  size_t ಸಾಲು_Index = 0;
  auto Bytes = FB.Bytes;
  ಧ್ರುವ size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  ಯದಿ (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t ಸಾಲುಗಳು = Size / FB.ಅಂಕೆ_Per_ಸಾಲು;
    uint64_t MaxOffset = *FB.FirstByteOffset + ಸಾಲುಗಳು * FB.ಅಂಕೆ_Per_ಸಾಲು;
    unsigned Power = 0;
    ಯದಿ (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned ಅಂಕೆ_Byte_Groups =
      alignTo(FB.ಅಂಕೆ_Per_ಸಾಲು, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.ಅಂಕೆ_Per_ಸಾಲು * 2 + ಅಂಕೆ_Byte_Groups - 1;

  ಯಾವತ್ (!Bytes.empty()) {
    indent(FB.IndentLevel);

    ಯದಿ (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*ಏತತ್, Offset + ಸಾಲು_Index, HPS, OffsetWidth);
      *ಏತತ್ << ": ";
    }

    auto ಸಾಲು = Bytes.take_front(FB.ಅಂಕೆ_Per_ಸಾಲು);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    ಪ್ರತಿ (size_t I = 0; I < ಸಾಲು.size(); ++I, CharsPrinted += 2) {
      ಯದಿ (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *ಏತತ್ << " ";
      }
      llvm::write_hex(*ಏತತ್, ಸಾಲು[I], HPS, 2);
    }

    ಯದಿ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      ಪ್ರತಿಜ್ಞಾ(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *ಏತತ್ << "|";

      // Print the ASCII char values for each byte on this line
      ಪ್ರತಿ (uint8_t Byte : ಸಾಲು) {
        ಯದಿ (isPrint(Byte))
          *ಏತತ್ << static_cast<ವರ್ಣ>(Byte);
        ಅನ್ಯಥಾ
          *ಏತತ್ << '.';
      }
      *ಏತತ್ << '|';
    }

    Bytes = Bytes.drop_front(ಸಾಲು.size());
    ಸಾಲು_Index += ಸಾಲು.size();
    ಯದಿ (ಸಾಲು_Index < Size)
      *ಏತತ್ << '\n';
  }
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

template <ವರ್ಣ C>
ಸ್ಥಿರ raw_ostream &write_padding(raw_ostream &OS, unsigned ಅಂಕೆ_Chars) {
  ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ಯದಿ (ಅಂಕೆ_Chars < std::size(Chars))
    ಪ್ರತ್ಯಾಗಮ OS.ಲಿಖ(Chars, ಅಂಕೆ_Chars);

  ಯಾವತ್ (ಅಂಕೆ_Chars) {
    unsigned ಅಂಕೆ_To_Write = std::min(ಅಂಕೆ_Chars, (unsigned)std::size(Chars) - 1);
    OS.ಲಿಖ(Chars, ಅಂಕೆ_To_Write);
    ಅಂಕೆ_Chars -= ಅಂಕೆ_To_Write;
  }
  ಪ್ರತ್ಯಾಗಮ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned ಅಂಕೆ_Spaces) {
  ಪ್ರತ್ಯಾಗಮ write_padding<' '>(*ಏತತ್, ಅಂಕೆ_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned ಅಂಕೆ_Zeros) {
  ಪ್ರತ್ಯಾಗಮ write_padding<'\0'>(*ಏತತ್, ಅಂಕೆ_Zeros);
}

ತಾರ್ಕಿಕ raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ಯದಿ (!ColorEnabled)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ಯದಿ (sys::Process::ColorNeedsFlush() && !is_displayed())
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  ಯದಿ (sys::Process::ColorNeedsFlush())
    flush();

  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

raw_ostream &raw_ostream::changeColor(ಗಣನಾ Colors colors, ತಾರ್ಕಿಕ bold, ತಾರ್ಕಿಕ bg) {
  ಯದಿ (!prepare_colors())
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  ಧ್ರುವ ವರ್ಣ *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<ವರ್ಣ>(colors), bold, bg);
  ಯದಿ (colorcode)
    ಲಿಖ(colorcode, strlen(colorcode));
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::resetColor() {
  ಯದಿ (!prepare_colors())
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  ಯದಿ (ಧ್ರುವ ವರ್ಣ *colorcode = sys::Process::ResetColor())
    ಲಿಖ(colorcode, strlen(colorcode));
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

raw_ostream &raw_ostream::reverseColor() {
  ಯದಿ (!prepare_colors())
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;

  ಯದಿ (ಧ್ರುವ ವರ್ಣ *colorcode = sys::Process::OutputReverse())
    ಲಿಖ(colorcode, strlen(colorcode));
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

ಶೂನ್ಯ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ಯದಿ (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    ಪ್ರತ್ಯಾಗಮ STDOUT_FILENO;
  }

  ಪೂರ್ಣಾಙ್ಕ FD;
  ಯದಿ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  ಅನ್ಯಥಾ
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ -1;

  ಪ್ರತ್ಯಾಗಮ FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), ಸತ್ಯ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(ಪೂರ್ಣಾಙ್ಕ fd, ತಾರ್ಕಿಕ shouldClose, ತಾರ್ಕಿಕ unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ಯದಿ (FD < 0 ) {
    ShouldClose = ಅಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ;
  }

  enable_colors(ಸತ್ಯ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ಯದಿ (FD <= STDERR_FILENO)
    ShouldClose = ಅಸತ್ಯ;

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
#ಅನ್ಯಥಾ
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  ಯದಿ (!SupportsSeeking)
    pos = 0;
  ಅನ್ಯಥಾ
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ಯದಿ (FD >= 0) {
    flush();
    ಯದಿ (ShouldClose) {
      ಯದಿ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ಯದಿ (FD == 2) ಪ್ರತ್ಯಾಗಮ;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ಯದಿ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#ಯದಿ defined(_WIN32)
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
ಸ್ಥಿರ ತಾರ್ಕಿಕ write_console_impl(ಪೂರ್ಣಾಙ್ಕ FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  ಯದಿ (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  ಯದಿ (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  ಕುರು {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    ತಾರ್ಕಿಕ Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ಯದಿ (!Success)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    WCharsWritten += ActuallyWritten;
  } ಯಾವತ್ (WCharsWritten != WideText.size());
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}
#endif

ಶೂನ್ಯ raw_fd_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  ಯದಿ (TiedStream)
    TiedStream->flush();

  ಪ್ರತಿಜ್ಞಾ(FD >= 0 && "File already closed.");
  pos += Size;

#ಯದಿ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ಯದಿ (IsWindowsConsole)
    ಯದಿ (write_console_impl(FD, StringRef(Ptr, Size)))
      ಪ್ರತ್ಯಾಗಮ;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#ಯದಿ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  ಕುರು {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::ಲಿಖ(FD, Ptr, ChunkSize);

    ಯದಿ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ಯದಿ (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        ಅನುವರ್ತ;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      ಯದಿ (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      ಭಙ್ಗ;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } ಯಾವತ್ (Size > 0);
}

ಶೂನ್ಯ raw_fd_ostream::close() {
  ಪ್ರತಿಜ್ಞಾ(ShouldClose);
  ShouldClose = ಅಸತ್ಯ;
  flush();
  ಯದಿ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  ಪ್ರತಿಜ್ಞಾ(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#ಅನ್ಯಥಾ
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  ಯದಿ (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  ಪ್ರತ್ಯಾಗಮ pos;
}

ಶೂನ್ಯ raw_fd_ostream::pwrite_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  ಲಿಖ(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ಧ್ರುವ {
#ಯದಿ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ಯದಿ (IsWindowsConsole)
    ಪ್ರತ್ಯಾಗಮ 0;
  ಪ್ರತ್ಯಾಗಮ raw_ostream::preferred_buffer_size();
#ನೋಚೇದ್ಯದಿ defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ಪ್ರತ್ಯಾಗಮ raw_ostream::preferred_buffer_size();
#ಅನ್ಯಥಾ
  ಪ್ರತಿಜ್ಞಾ(FD >= 0 && "File not yet open!");
  ಸಂರಚನಾ stat statbuf;
  ಯದಿ (fstat(FD, &statbuf) != 0)
    ಪ್ರತ್ಯಾಗಮ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ಯದಿ (S_ISCHR(statbuf.st_mode) && is_displayed())
    ಪ್ರತ್ಯಾಗಮ 0;
  // Return the preferred block size.
  ಪ್ರತ್ಯಾಗಮ statbuf.st_blksize;
#endif
}

ತಾರ್ಕಿಕ raw_fd_ostream::is_displayed() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ sys::Process::FileDescriptorIsDisplayed(FD);
}

ತಾರ್ಕಿಕ raw_fd_ostream::has_colors() ಧ್ರುವ {
  ಯದಿ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ಪ್ರತ್ಯಾಗಮ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  ಯದಿ (!EC)
    ಪ್ರತ್ಯಾಗಮ sys::fs::FileLocker(FD);
  ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ಧ್ರುವ& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  ಯದಿ (!EC)
    ಪ್ರತ್ಯಾಗಮ sys::fs::FileLocker(FD);
  ಪ್ರತ್ಯಾಗಮ errorCodeToError(EC);
}

ಶೂನ್ಯ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  ಸ್ಥಿರ std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  ಪ್ರತಿಜ್ಞಾ(!EC1);
  (ಶೂನ್ಯ)EC1;

  ಸ್ಥಿರ raw_fd_ostream S("-", EC, sys::fs::OF_None);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  ಪ್ರತ್ಯಾಗಮ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ಸ್ಥಿರ std::error_code EC = enableAutoConversion(STDERR_FILENO);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  // Set standard error to be unbuffered.
  ಸ್ಥಿರ raw_fd_ostream S(STDERR_FILENO, ಅಸತ್ಯ, ಸತ್ಯ);
  ಪ್ರತ್ಯಾಗಮ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ಸ್ಥಿರ raw_null_ostream S;
  ಪ್ರತ್ಯಾಗಮ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     ಸತ್ಯ, ಅಸತ್ಯ, OStreamKind::OK_FDStream) {
  ಯದಿ (EC)
    ಪ್ರತ್ಯಾಗಮ;

  ಯದಿ (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(ಪೂರ್ಣಾಙ್ಕ fd, ತಾರ್ಕಿಕ shouldClose)
    : raw_fd_ostream(fd, shouldClose, ಅಸತ್ಯ, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::ಪಠ(ವರ್ಣ *Ptr, size_t Size) {
  ಪ್ರತಿಜ್ಞಾ(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::ಪಠ(get_fd(), (ಶೂನ್ಯ *)Ptr, Size);
  ಯದಿ (Ret >= 0)
    inc_pos(Ret);
  ಅನ್ಯಥಾ
    error_detected(errnoAsErrorCode());
  ಪ್ರತ್ಯಾಗಮ Ret;
}

ತಾರ್ಕಿಕ raw_fd_stream::classof(ಧ್ರುವ raw_ostream *OS) {
  ಪ್ರತ್ಯಾಗಮ OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ raw_string_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ OS.size(); }

ಶೂನ್ಯ raw_svector_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

ಶೂನ್ಯ raw_svector_ostream::pwrite_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

ತಾರ್ಕಿಕ raw_svector_ostream::classof(ಧ್ರುವ raw_ostream *OS) {
  ಪ್ರತ್ಯಾಗಮ OS->get_kind() == OStreamKind::OK_SVecStream;
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

ಶೂನ್ಯ raw_null_ostream::write_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ raw_null_ostream::pwrite_impl(ಧ್ರುವ ವರ್ಣ *Ptr, size_t Size,
                                   uint64_t Offset) {}

ಶೂನ್ಯ raw_pwrite_stream::anchor() {}

ಶೂನ್ಯ buffer_ostream::anchor() {}

ಶೂನ್ಯ buffer_unique_ostream::anchor() {}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  ಯದಿ (OutputFileName.equals_insensitive("NUL"))
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
#endif
  ಪ್ರತ್ಯಾಗಮ OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ಯದಿ (OutputFileName == "-")
    ಪ್ರತ್ಯಾಗಮ Write(outs());

  ಯದಿ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    ಪ್ರತ್ಯಾಗಮ Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  ಯದಿ (!Temp)
    ಪ್ರತ್ಯಾಗಮ createFileError(OutputFileName, Temp.takeError());

#ಯದಿ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ಯದಿ (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    ಯದಿ (EC != std::errc::no_such_file_or_directory)
      ಪ್ರತ್ಯಾಗಮ createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, ಅಸತ್ಯ);

  ಯದಿ (Error E = Write(Out)) {
    ಯದಿ (Error DiscardError = Temp->discard())
      ಪ್ರತ್ಯಾಗಮ joinErrors(std::move(E), std::move(DiscardError));
    ಪ್ರತ್ಯಾಗಮ E;
  }
  Out.flush();

  ಪ್ರತ್ಯಾಗಮ Temp->keep(OutputFileName);
}
