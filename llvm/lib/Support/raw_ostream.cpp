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

#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ "llvm/ADT/StringExtras.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/AutoConvert.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/Duration.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/FormatVariadic.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/MathExtras.h"
#അന്തര്ഭാവ "llvm/Support/NativeFormatting.h"
#അന്തര്ഭാവ "llvm/Support/Process.h"
#അന്തര്ഭാവ "llvm/Support/Program.h"
#അന്തര്ഭാവ <algorithm>
#അന്തര്ഭാവ <cerrno>
#അന്തര്ഭാവ <cstdio>
#അന്തര്ഭാവ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#എങ്കിൽ defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#എങ്കിൽ defined(__CYGWIN__)
#അന്തര്ഭാവ <io.h>
#endif

#എങ്കിൽ defined(_MSC_VER)
#അന്തര്ഭാവ <io.h>
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
#അന്തര്ഭാവ "llvm/Support/ConvertUTF.h"
#അന്തര്ഭാവ "llvm/Support/Signals.h"
#അന്തര്ഭാവ "llvm/Support/Windows/WindowsSupport.h"
#endif

using നാമാകാശ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  പ്രതിജ്ഞാ(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  എങ്കിൽ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() സ്ഥിരം {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  തിരികെ (16 * 1024);
#അല്ലെങ്കിൽ
  // BUFSIZ is intended to be a reasonable default.
  തിരികെ BUFSIZ;
#endif
}

ശൂന്യം raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  എങ്കിൽ (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  അല്ലെങ്കിൽ
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

ശൂന്യം raw_ostream::SetBufferAndMode(അക്ഷരം *BufferStart, size_t Size,
                                   BufferKind Mode) {
  പ്രതിജ്ഞാ(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  പ്രതിജ്ഞാ(Get_അക്കം_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  എങ്കിൽ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  പ്രതിജ്ഞാ(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*ഇത്, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*ഇത്, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*ഇത്, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*ഇത്, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*ഇത്, N, HexPrintStyle::Lower);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  എങ്കിൽ (C == Colors::RESET)
    resetColor();
  അല്ലെങ്കിൽ
    changeColor(C);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::write_uuid(സ്ഥിരം uuid_t UUID) {
  വേണ്ടി (പൂർണ്ണസംഖ്യ Idx = 0; Idx < 16; ++Idx) {
    *ഇത് << format("%02" PRIX32, UUID[Idx]);
    എങ്കിൽ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *ഇത് << "-";
  }
  തിരികെ *ഇത്;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        തർക്കം UseHexEscapes) {
  വേണ്ടി (unsigned അക്ഷരം c : Str) {
    തിരഞ്ഞെടുപ്പ് (c) {
    അവസ്ഥ '\\':
      *ഇത് << '\\' << '\\';
      നിർത്തുക;
    അവസ്ഥ '\t':
      *ഇത് << '\\' << 't';
      നിർത്തുക;
    അവസ്ഥ '\n':
      *ഇത് << '\\' << 'n';
      നിർത്തുക;
    അവസ്ഥ '"':
      *ഇത് << '\\' << '"';
      നിർത്തുക;
    സ്ഥിരസ്ഥിതി:
      എങ്കിൽ (isPrint(c)) {
        *ഇത് << c;
        നിർത്തുക;
      }

      // Write out the escaped representation.
      എങ്കിൽ (UseHexEscapes) {
        *ഇത് << '\\' << 'x';
        *ഇത് << hexdigit((c >> 4) & 0xF);
        *ഇത് << hexdigit((c >> 0) & 0xF);
      } അല്ലെങ്കിൽ {
        // Always use a full 3-character octal escape.
        *ഇത് << '\\';
        *ഇത് << അക്ഷരം('0' + ((c >> 6) & 7));
        *ഇത് << അക്ഷരം('0' + ((c >> 3) & 7));
        *ഇത് << അക്ഷരം('0' + ((c >> 0) & 7));
      }
    }
  }

  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(സ്ഥിരം ശൂന്യം *P) {
  llvm::write_hex(*ഇത്, (uintptr_t)P, HexPrintStyle::PrefixLower);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(ഇരട്ട N) {
  llvm::write_double(*ഇത്, N, FloatStyle::Exponent);
  തിരികെ *ഇത്;
}

ശൂന്യം raw_ostream::flush_nonempty() {
  പ്രതിജ്ഞാ(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::ലിഖ(unsigned അക്ഷരം C) {
  // Group exceptional cases into a single branch.
  എങ്കിൽ (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    എങ്കിൽ (LLVM_UNLIKELY(!OutBufStart)) {
      എങ്കിൽ (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<അക്ഷരം *>(&C), 1);
        തിരികെ *ഇത്;
      }
      // Set up a buffer and start over.
      SetBuffered();
      തിരികെ ലിഖ(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::ലിഖ(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  എങ്കിൽ (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    എങ്കിൽ (LLVM_UNLIKELY(!OutBufStart)) {
      എങ്കിൽ (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        തിരികെ *ഇത്;
      }
      // Set up a buffer and start over.
      SetBuffered();
      തിരികെ ലിഖ(Ptr, Size);
    }

    size_t അക്കം_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    എങ്കിൽ (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      പ്രതിജ്ഞാ(അക്കം_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % അക്കം_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      എങ്കിൽ (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        തിരികെ ലിഖ(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      തിരികെ *ഇത്;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, അക്കം_Bytes);
    flush_nonempty();
    തിരികെ ലിഖ(Ptr + അക്കം_Bytes, Size - അക്കം_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  തിരികെ *ഇത്;
}

ശൂന്യം raw_ostream::copy_to_buffer(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  പ്രതിജ്ഞാ(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  തിരഞ്ഞെടുപ്പ് (Size) {
  അവസ്ഥ 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  അവസ്ഥ 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  അവസ്ഥ 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  അവസ്ഥ 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  അവസ്ഥ 0: നിർത്തുക;
  സ്ഥിരസ്ഥിതി:
    memcpy(OutBufCur, Ptr, Size);
    നിർത്തുക;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<പൂർണ്ണസംഖ്യ(അക്ഷരം *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_ഇടത് = OutBufEnd - OutBufCur;
  എങ്കിൽ (Buffer_Bytes_ഇടത് > 3) {
    പൂർണ്ണസംഖ്യ N = Snprint(OutBufCur, Buffer_Bytes_ഇടത്);
    എങ്കിൽ (N < 0)
      തിരികെ *ഇത്;
    // Common case is that we have plenty of space.
    എങ്കിൽ (size_t(N) < Buffer_Bytes_ഇടത്) {
      OutBufCur += N;
      തിരികെ *ഇത്;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<അക്ഷരം, 128> V;
  വേണ്ടി (;;) {
    V.resize(Size);
    പൂർണ്ണസംഖ്യ N = Snprint(V.data(), Size);
    എങ്കിൽ (N < 0)
      തിരികെ *ഇത്;
    എങ്കിൽ (size_t(N) < Size)
      തിരികെ ലിഖ(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(സ്ഥിരം formatv_object_base &Obj) {
  Obj.format(*ഇത്);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(സ്ഥിരം FormattedString &FS) {
  unsigned ഇടത്_Indent = 0;
  unsigned വലത്_Indent = 0;
  സ്ഥിരം ssize_t Difference = FS.Width - FS.Str.size();
  എങ്കിൽ (Difference > 0) {
    തിരഞ്ഞെടുപ്പ് (FS.Justify) {
    അവസ്ഥ FormattedString::JustifyNone:
      നിർത്തുക;
    അവസ്ഥ FormattedString::Justify_ഇടത്:
      വലത്_Indent = Difference;
      നിർത്തുക;
    അവസ്ഥ FormattedString::Justify_വലത്:
      ഇടത്_Indent = Difference;
      നിർത്തുക;
    അവസ്ഥ FormattedString::JustifyCenter:
      ഇടത്_Indent = Difference / 2;
      വലത്_Indent = Difference - ഇടത്_Indent;
      നിർത്തുക;
    }
  }
  indent(ഇടത്_Indent);
  (*ഇത്) << FS.Str;
  indent(വലത്_Indent);
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(സ്ഥിരം Formatted_സംഖ്യ &FN) {
  എങ്കിൽ (FN.Hex) {
    HexPrintStyle Style;
    എങ്കിൽ (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    അല്ലെങ്കിൽ എങ്കിൽ (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    അല്ലെങ്കിൽ എങ്കിൽ (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    അല്ലെങ്കിൽ
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*ഇത്, FN.Hex_മൂല്യം, Style, FN.Width);
  } അല്ലെങ്കിൽ {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_മൂല്യം, 0, IntegerStyle::Integer);
    എങ്കിൽ (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*ഇത്) << Buffer;
  }
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::operator<<(സ്ഥിരം FormattedBytes &FB) {
  എങ്കിൽ (FB.Bytes.empty())
    തിരികെ *ഇത്;

  size_t വരി_Index = 0;
  auto Bytes = FB.Bytes;
  സ്ഥിരം size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  എങ്കിൽ (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t വരികൾ = Size / FB.അക്കം_Per_വരി;
    uint64_t MaxOffset = *FB.FirstByteOffset + വരികൾ * FB.അക്കം_Per_വരി;
    unsigned Power = 0;
    എങ്കിൽ (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned അക്കം_Byte_Groups =
      alignTo(FB.അക്കം_Per_വരി, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.അക്കം_Per_വരി * 2 + അക്കം_Byte_Groups - 1;

  ആകുമ്പോൾ (!Bytes.empty()) {
    indent(FB.IndentLevel);

    എങ്കിൽ (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*ഇത്, Offset + വരി_Index, HPS, OffsetWidth);
      *ഇത് << ": ";
    }

    auto വരി = Bytes.take_front(FB.അക്കം_Per_വരി);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    വേണ്ടി (size_t I = 0; I < വരി.size(); ++I, CharsPrinted += 2) {
      എങ്കിൽ (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *ഇത് << " ";
      }
      llvm::write_hex(*ഇത്, വരി[I], HPS, 2);
    }

    എങ്കിൽ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      പ്രതിജ്ഞാ(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *ഇത് << "|";

      // Print the ASCII char values for each byte on this line
      വേണ്ടി (uint8_t Byte : വരി) {
        എങ്കിൽ (isPrint(Byte))
          *ഇത് << static_cast<അക്ഷരം>(Byte);
        അല്ലെങ്കിൽ
          *ഇത് << '.';
      }
      *ഇത് << '|';
    }

    Bytes = Bytes.drop_front(വരി.size());
    വരി_Index += വരി.size();
    എങ്കിൽ (വരി_Index < Size)
      *ഇത് << '\n';
  }
  തിരികെ *ഇത്;
}

template <അക്ഷരം C>
സ്ഥിതം raw_ostream &write_padding(raw_ostream &OS, unsigned അക്കം_Chars) {
  സ്ഥിതം സ്ഥിരം അക്ഷരം Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  എങ്കിൽ (അക്കം_Chars < std::size(Chars))
    തിരികെ OS.ലിഖ(Chars, അക്കം_Chars);

  ആകുമ്പോൾ (അക്കം_Chars) {
    unsigned അക്കം_To_Write = std::min(അക്കം_Chars, (unsigned)std::size(Chars) - 1);
    OS.ലിഖ(Chars, അക്കം_To_Write);
    അക്കം_Chars -= അക്കം_To_Write;
  }
  തിരികെ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned അക്കം_Spaces) {
  തിരികെ write_padding<' '>(*ഇത്, അക്കം_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned അക്കം_Zeros) {
  തിരികെ write_padding<'\0'>(*ഇത്, അക്കം_Zeros);
}

തർക്കം raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  എങ്കിൽ (!ColorEnabled)
    തിരികെ അസത്യം;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  എങ്കിൽ (sys::Process::ColorNeedsFlush() && !is_displayed())
    തിരികെ അസത്യം;

  എങ്കിൽ (sys::Process::ColorNeedsFlush())
    flush();

  തിരികെ സത്യം;
}

raw_ostream &raw_ostream::changeColor(എണ്ണൽ Colors colors, തർക്കം bold, തർക്കം bg) {
  എങ്കിൽ (!prepare_colors())
    തിരികെ *ഇത്;

  സ്ഥിരം അക്ഷരം *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<അക്ഷരം>(colors), bold, bg);
  എങ്കിൽ (colorcode)
    ലിഖ(colorcode, strlen(colorcode));
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::resetColor() {
  എങ്കിൽ (!prepare_colors())
    തിരികെ *ഇത്;

  എങ്കിൽ (സ്ഥിരം അക്ഷരം *colorcode = sys::Process::ResetColor())
    ലിഖ(colorcode, strlen(colorcode));
  തിരികെ *ഇത്;
}

raw_ostream &raw_ostream::reverseColor() {
  എങ്കിൽ (!prepare_colors())
    തിരികെ *ഇത്;

  എങ്കിൽ (സ്ഥിരം അക്ഷരം *colorcode = sys::Process::OutputReverse())
    ലിഖ(colorcode, strlen(colorcode));
  തിരികെ *ഇത്;
}

ശൂന്യം raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

സ്ഥിതം പൂർണ്ണസംഖ്യ getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  പ്രതിജ്ഞാ((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  എങ്കിൽ (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    തിരികെ STDOUT_FILENO;
  }

  പൂർണ്ണസംഖ്യ FD;
  എങ്കിൽ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  അല്ലെങ്കിൽ
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  എങ്കിൽ (EC)
    തിരികെ -1;

  തിരികെ FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), സത്യം) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(പൂർണ്ണസംഖ്യ fd, തർക്കം shouldClose, തർക്കം unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  എങ്കിൽ (FD < 0 ) {
    ShouldClose = അസത്യം;
    തിരികെ;
  }

  enable_colors(സത്യം);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  എങ്കിൽ (FD <= STDERR_FILENO)
    ShouldClose = അസത്യം;

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
#അല്ലെങ്കിൽ
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  എങ്കിൽ (!SupportsSeeking)
    pos = 0;
  അല്ലെങ്കിൽ
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  എങ്കിൽ (FD >= 0) {
    flush();
    എങ്കിൽ (ShouldClose) {
      എങ്കിൽ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  എങ്കിൽ (FD == 2) തിരികെ;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  എങ്കിൽ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#എങ്കിൽ defined(_WIN32)
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
സ്ഥിതം തർക്കം write_console_impl(പൂർണ്ണസംഖ്യ FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  എങ്കിൽ (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    തിരികെ അസത്യം;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  എങ്കിൽ (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  ചെയ്യുക {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    തർക്കം Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    എങ്കിൽ (!Success)
      തിരികെ അസത്യം;

    WCharsWritten += ActuallyWritten;
  } ആകുമ്പോൾ (WCharsWritten != WideText.size());
  തിരികെ സത്യം;
}
#endif

ശൂന്യം raw_fd_ostream::write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  എങ്കിൽ (TiedStream)
    TiedStream->flush();

  പ്രതിജ്ഞാ(FD >= 0 && "File already closed.");
  pos += Size;

#എങ്കിൽ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  എങ്കിൽ (IsWindowsConsole)
    എങ്കിൽ (write_console_impl(FD, StringRef(Ptr, Size)))
      തിരികെ;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#എങ്കിൽ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  ചെയ്യുക {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::ലിഖ(FD, Ptr, ChunkSize);

    എങ്കിൽ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      എങ്കിൽ (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        തുടരുക;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      എങ്കിൽ (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      നിർത്തുക;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } ആകുമ്പോൾ (Size > 0);
}

ശൂന്യം raw_fd_ostream::close() {
  പ്രതിജ്ഞാ(ShouldClose);
  ShouldClose = അസത്യം;
  flush();
  എങ്കിൽ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  പ്രതിജ്ഞാ(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#അല്ലെങ്കിൽ
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  എങ്കിൽ (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  തിരികെ pos;
}

ശൂന്യം raw_fd_ostream::pwrite_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  ലിഖ(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() സ്ഥിരം {
#എങ്കിൽ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  എങ്കിൽ (IsWindowsConsole)
    തിരികെ 0;
  തിരികെ raw_ostream::preferred_buffer_size();
#അല്ലെങ്കിലെങ്കിൽ defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  തിരികെ raw_ostream::preferred_buffer_size();
#അല്ലെങ്കിൽ
  പ്രതിജ്ഞാ(FD >= 0 && "File not yet open!");
  ഘടന stat statbuf;
  എങ്കിൽ (fstat(FD, &statbuf) != 0)
    തിരികെ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  എങ്കിൽ (S_ISCHR(statbuf.st_mode) && is_displayed())
    തിരികെ 0;
  // Return the preferred block size.
  തിരികെ statbuf.st_blksize;
#endif
}

തർക്കം raw_fd_ostream::is_displayed() സ്ഥിരം {
  തിരികെ sys::Process::FileDescriptorIsDisplayed(FD);
}

തർക്കം raw_fd_ostream::has_colors() സ്ഥിരം {
  എങ്കിൽ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  തിരികെ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  എങ്കിൽ (!EC)
    തിരികെ sys::fs::FileLocker(FD);
  തിരികെ errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration സ്ഥിരം& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  എങ്കിൽ (!EC)
    തിരികെ sys::fs::FileLocker(FD);
  തിരികെ errorCodeToError(EC);
}

ശൂന്യം raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  സ്ഥിതം std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  പ്രതിജ്ഞാ(!EC1);
  (ശൂന്യം)EC1;

  സ്ഥിതം raw_fd_ostream S("-", EC, sys::fs::OF_None);
  പ്രതിജ്ഞാ(!EC);
  തിരികെ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  സ്ഥിതം std::error_code EC = enableAutoConversion(STDERR_FILENO);
  പ്രതിജ്ഞാ(!EC);
  (ശൂന്യം)EC;

  // Set standard error to be unbuffered.
  സ്ഥിതം raw_fd_ostream S(STDERR_FILENO, അസത്യം, സത്യം);
  തിരികെ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  സ്ഥിതം raw_null_ostream S;
  തിരികെ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     സത്യം, അസത്യം, OStreamKind::OK_FDStream) {
  എങ്കിൽ (EC)
    തിരികെ;

  എങ്കിൽ (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(പൂർണ്ണസംഖ്യ fd, തർക്കം shouldClose)
    : raw_fd_ostream(fd, shouldClose, അസത്യം, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::പഠ(അക്ഷരം *Ptr, size_t Size) {
  പ്രതിജ്ഞാ(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::പഠ(get_fd(), (ശൂന്യം *)Ptr, Size);
  എങ്കിൽ (Ret >= 0)
    inc_pos(Ret);
  അല്ലെങ്കിൽ
    error_detected(errnoAsErrorCode());
  തിരികെ Ret;
}

തർക്കം raw_fd_stream::classof(സ്ഥിരം raw_ostream *OS) {
  തിരികെ OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

ശൂന്യം raw_string_ostream::write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() സ്ഥിരം { തിരികെ OS.size(); }

ശൂന്യം raw_svector_ostream::write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

ശൂന്യം raw_svector_ostream::pwrite_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

തർക്കം raw_svector_ostream::classof(സ്ഥിരം raw_ostream *OS) {
  തിരികെ OS->get_kind() == OStreamKind::OK_SVecStream;
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

ശൂന്യം raw_null_ostream::write_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() സ്ഥിരം {
  തിരികെ 0;
}

ശൂന്യം raw_null_ostream::pwrite_impl(സ്ഥിരം അക്ഷരം *Ptr, size_t Size,
                                   uint64_t Offset) {}

ശൂന്യം raw_pwrite_stream::anchor() {}

ശൂന്യം buffer_ostream::anchor() {}

ശൂന്യം buffer_unique_ostream::anchor() {}

സ്ഥിതം തർക്കം isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  എങ്കിൽ (OutputFileName.equals_insensitive("NUL"))
    തിരികെ സത്യം;
#endif
  തിരികെ OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  എങ്കിൽ (OutputFileName == "-")
    തിരികെ Write(outs());

  എങ്കിൽ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    തിരികെ Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  എങ്കിൽ (!Temp)
    തിരികെ createFileError(OutputFileName, Temp.takeError());

#എങ്കിൽ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  എങ്കിൽ (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    എങ്കിൽ (EC != std::errc::no_such_file_or_directory)
      തിരികെ createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, അസത്യം);

  എങ്കിൽ (Error E = Write(Out)) {
    എങ്കിൽ (Error DiscardError = Temp->discard())
      തിരികെ joinErrors(std::move(E), std::move(DiscardError));
    തിരികെ E;
  }
  Out.flush();

  തിരികെ Temp->keep(OutputFileName);
}
