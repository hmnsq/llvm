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

#ضمن "llvm/Support/raw_ostream.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/AutoConvert.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/Duration.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/MathExtras.h"
#ضمن "llvm/Support/NativeFormatting.h"
#ضمن "llvm/Support/Process.h"
#ضمن "llvm/Support/Program.h"
#ضمن <algorithm>
#ضمن <cerrno>
#ضمن <cstdio>
#ضمن <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ܐܢ defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#ܐܢ defined(__CYGWIN__)
#ضمن <io.h>
#endif

#ܐܢ defined(_MSC_VER)
#ضمن <io.h>
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
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#endif

using نطاق llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  أكد(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  ܐܢ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ثابت {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ܦܢܐ (16 * 1024);
#ܐܚܪܢܐ
  // BUFSIZ is intended to be a reasonable default.
  ܦܢܐ BUFSIZ;
#endif
}

فراغ raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ܐܢ (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  ܐܚܪܢܐ
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

فراغ raw_ostream::SetBufferAndMode(حرف *BufferStart, size_t Size,
                                   BufferKind Mode) {
  أكد(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  أكد(GetNumBytesInBuffer() == 0 && "Current buffer is non-empty!");

  ܐܢ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  أكد(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*هذا, N, HexPrintStyle::Lower);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  ܐܢ (C == Colors::RESET)
    resetColor();
  ܐܚܪܢܐ
    changeColor(C);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::write_uuid(ثابت uuid_t UUID) {
  لكل (صحيح Idx = 0; Idx < 16; ++Idx) {
    *هذا << format("%02" PRIX32, UUID[Idx]);
    ܐܢ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *هذا << "-";
  }
  ܦܢܐ *هذا;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        منطقي UseHexEscapes) {
  لكل (unsigned حرف c : Str) {
    بدل (c) {
    حالة '\\':
      *هذا << '\\' << '\\';
      اكسر;
    حالة '\t':
      *هذا << '\\' << 't';
      اكسر;
    حالة '\n':
      *هذا << '\\' << 'n';
      اكسر;
    حالة '"':
      *هذا << '\\' << '"';
      اكسر;
    افتراضي:
      ܐܢ (isPrint(c)) {
        *هذا << c;
        اكسر;
      }

      // Write out the escaped representation.
      ܐܢ (UseHexEscapes) {
        *هذا << '\\' << 'x';
        *هذا << hexdigit((c >> 4) & 0xF);
        *هذا << hexdigit((c >> 0) & 0xF);
      } ܐܚܪܢܐ {
        // Always use a full 3-character octal escape.
        *هذا << '\\';
        *هذا << حرف('0' + ((c >> 6) & 7));
        *هذا << حرف('0' + ((c >> 3) & 7));
        *هذا << حرف('0' + ((c >> 0) & 7));
      }
    }
  }

  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت فراغ *P) {
  llvm::write_hex(*هذا, (uintptr_t)P, HexPrintStyle::PrefixLower);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(مزدوج N) {
  llvm::write_double(*هذا, N, FloatStyle::Exponent);
  ܦܢܐ *هذا;
}

فراغ raw_ostream::flush_nonempty() {
  أكد(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::اكتب(unsigned حرف C) {
  // Group exceptional cases into a single branch.
  ܐܢ (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    ܐܢ (LLVM_UNLIKELY(!OutBufStart)) {
      ܐܢ (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<حرف *>(&C), 1);
        ܦܢܐ *هذا;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ܦܢܐ اكتب(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::اكتب(ثابت حرف *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  ܐܢ (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    ܐܢ (LLVM_UNLIKELY(!OutBufStart)) {
      ܐܢ (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        ܦܢܐ *هذا;
      }
      // Set up a buffer and start over.
      SetBuffered();
      ܦܢܐ اكتب(Ptr, Size);
    }

    size_t NumBytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ܐܢ (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      أكد(NumBytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % NumBytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      ܐܢ (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        ܦܢܐ اكتب(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      ܦܢܐ *هذا;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, NumBytes);
    flush_nonempty();
    ܦܢܐ اكتب(Ptr + NumBytes, Size - NumBytes);
  }

  copy_to_buffer(Ptr, Size);

  ܦܢܐ *هذا;
}

فراغ raw_ostream::copy_to_buffer(ثابت حرف *Ptr, size_t Size) {
  أكد(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  بدل (Size) {
  حالة 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  حالة 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  حالة 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  حالة 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  حالة 0: اكسر;
  افتراضي:
    memcpy(OutBufCur, Ptr, Size);
    اكسر;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<صحيح(حرف *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t BufferBytesLeft = OutBufEnd - OutBufCur;
  ܐܢ (BufferBytesLeft > 3) {
    صحيح N = Snprint(OutBufCur, BufferBytesLeft);
    ܐܢ (N < 0)
      ܦܢܐ *هذا;
    // Common case is that we have plenty of space.
    ܐܢ (size_t(N) < BufferBytesLeft) {
      OutBufCur += N;
      ܦܢܐ *هذا;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<حرف, 128> V;
  لكل (;;) {
    V.resize(Size);
    صحيح N = Snprint(V.data(), Size);
    ܐܢ (N < 0)
      ܦܢܐ *هذا;
    ܐܢ (size_t(N) < Size)
      ܦܢܐ اكتب(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ثابت formatv_object_base &Obj) {
  Obj.format(*هذا);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedString &FS) {
  unsigned LeftIndent = 0;
  unsigned RightIndent = 0;
  ثابت ssize_t Difference = FS.Width - FS.Str.size();
  ܐܢ (Difference > 0) {
    بدل (FS.Justify) {
    حالة FormattedString::JustifyNone:
      اكسر;
    حالة FormattedString::JustifyLeft:
      RightIndent = Difference;
      اكسر;
    حالة FormattedString::JustifyRight:
      LeftIndent = Difference;
      اكسر;
    حالة FormattedString::JustifyCenter:
      LeftIndent = Difference / 2;
      RightIndent = Difference - LeftIndent;
      اكسر;
    }
  }
  indent(LeftIndent);
  (*هذا) << FS.Str;
  indent(RightIndent);
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedNumber &FN) {
  ܐܢ (FN.Hex) {
    HexPrintStyle Style;
    ܐܢ (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    ܐܚܪܢܐ ܐܢ (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    ܐܚܪܢܐ ܐܢ (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    ܐܚܪܢܐ
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*هذا, FN.HexValue, Style, FN.Width);
  } ܐܚܪܢܐ {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.DecValue, 0, IntegerStyle::Integer);
    ܐܢ (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*هذا) << Buffer;
  }
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedBytes &FB) {
  ܐܢ (FB.Bytes.empty())
    ܦܢܐ *هذا;

  size_t LineIndex = 0;
  auto Bytes = FB.Bytes;
  ثابت size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  ܐܢ (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t Lines = Size / FB.NumPerLine;
    uint64_t MaxOffset = *FB.FirstByteOffset + Lines * FB.NumPerLine;
    unsigned Power = 0;
    ܐܢ (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned NumByteGroups =
      alignTo(FB.NumPerLine, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.NumPerLine * 2 + NumByteGroups - 1;

  ܟܡܐ (!Bytes.empty()) {
    indent(FB.IndentLevel);

    ܐܢ (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*هذا, Offset + LineIndex, HPS, OffsetWidth);
      *هذا << ": ";
    }

    auto Line = Bytes.take_front(FB.NumPerLine);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    لكل (size_t I = 0; I < Line.size(); ++I, CharsPrinted += 2) {
      ܐܢ (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *هذا << " ";
      }
      llvm::write_hex(*هذا, Line[I], HPS, 2);
    }

    ܐܢ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *هذا << "|";

      // Print the ASCII char values for each byte on this line
      لكل (uint8_t Byte : Line) {
        ܐܢ (isPrint(Byte))
          *هذا << static_cast<حرف>(Byte);
        ܐܚܪܢܐ
          *هذا << '.';
      }
      *هذا << '|';
    }

    Bytes = Bytes.drop_front(Line.size());
    LineIndex += Line.size();
    ܐܢ (LineIndex < Size)
      *هذا << '\n';
  }
  ܦܢܐ *هذا;
}

template <حرف C>
ساكن raw_ostream &write_padding(raw_ostream &OS, unsigned NumChars) {
  ساكن ثابت حرف Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ܐܢ (NumChars < std::size(Chars))
    ܦܢܐ OS.اكتب(Chars, NumChars);

  ܟܡܐ (NumChars) {
    unsigned NumToWrite = std::min(NumChars, (unsigned)std::size(Chars) - 1);
    OS.اكتب(Chars, NumToWrite);
    NumChars -= NumToWrite;
  }
  ܦܢܐ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned NumSpaces) {
  ܦܢܐ write_padding<' '>(*هذا, NumSpaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned NumZeros) {
  ܦܢܐ write_padding<'\0'>(*هذا, NumZeros);
}

منطقي raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ܐܢ (!ColorEnabled)
    ܦܢܐ ܕܓܠܐ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ܐܢ (sys::Process::ColorNeedsFlush() && !is_displayed())
    ܦܢܐ ܕܓܠܐ;

  ܐܢ (sys::Process::ColorNeedsFlush())
    flush();

  ܦܢܐ ܫܪܪܐ;
}

raw_ostream &raw_ostream::changeColor(تعداد Colors colors, منطقي bold, منطقي bg) {
  ܐܢ (!prepare_colors())
    ܦܢܐ *هذا;

  ثابت حرف *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<حرف>(colors), bold, bg);
  ܐܢ (colorcode)
    اكتب(colorcode, strlen(colorcode));
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::resetColor() {
  ܐܢ (!prepare_colors())
    ܦܢܐ *هذا;

  ܐܢ (ثابت حرف *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, strlen(colorcode));
  ܦܢܐ *هذا;
}

raw_ostream &raw_ostream::reverseColor() {
  ܐܢ (!prepare_colors())
    ܦܢܐ *هذا;

  ܐܢ (ثابت حرف *colorcode = sys::Process::OutputReverse())
    اكتب(colorcode, strlen(colorcode));
  ܦܢܐ *هذا;
}

فراغ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ساكن صحيح getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ܐܢ (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    ܦܢܐ STDOUT_FILENO;
  }

  صحيح FD;
  ܐܢ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  ܐܚܪܢܐ
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ܐܢ (EC)
    ܦܢܐ -1;

  ܦܢܐ FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), ܫܪܪܐ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(صحيح fd, منطقي shouldClose, منطقي unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ܐܢ (FD < 0 ) {
    ShouldClose = ܕܓܠܐ;
    ܦܢܐ;
  }

  enable_colors(ܫܪܪܐ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ܐܢ (FD <= STDERR_FILENO)
    ShouldClose = ܕܓܠܐ;

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
#ܐܚܪܢܐ
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  ܐܢ (!SupportsSeeking)
    pos = 0;
  ܐܚܪܢܐ
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ܐܢ (FD >= 0) {
    flush();
    ܐܢ (ShouldClose) {
      ܐܢ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ܐܢ (FD == 2) ܦܢܐ;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ܐܢ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#ܐܢ defined(_WIN32)
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
ساكن منطقي write_console_impl(صحيح FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  ܐܢ (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    ܦܢܐ ܕܓܠܐ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  ܐܢ (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  ܥܒܕ {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    منطقي Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ܐܢ (!Success)
      ܦܢܐ ܕܓܠܐ;

    WCharsWritten += ActuallyWritten;
  } ܟܡܐ (WCharsWritten != WideText.size());
  ܦܢܐ ܫܪܪܐ;
}
#endif

فراغ raw_fd_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  ܐܢ (TiedStream)
    TiedStream->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += Size;

#ܐܢ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ܐܢ (IsWindowsConsole)
    ܐܢ (write_console_impl(FD, StringRef(Ptr, Size)))
      ܦܢܐ;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#ܐܢ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  ܥܒܕ {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::اكتب(FD, Ptr, ChunkSize);

    ܐܢ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ܐܢ (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        استمر;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      ܐܢ (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      اكسر;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } ܟܡܐ (Size > 0);
}

فراغ raw_fd_ostream::close() {
  أكد(ShouldClose);
  ShouldClose = ܕܓܠܐ;
  flush();
  ܐܢ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#ܐܚܪܢܐ
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  ܐܢ (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  ܦܢܐ pos;
}

فراغ raw_fd_ostream::pwrite_impl(ثابت حرف *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  اكتب(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ثابت {
#ܐܢ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ܐܢ (IsWindowsConsole)
    ܦܢܐ 0;
  ܦܢܐ raw_ostream::preferred_buffer_size();
#وإلاإذا defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ܦܢܐ raw_ostream::preferred_buffer_size();
#ܐܚܪܢܐ
  أكد(FD >= 0 && "File not yet open!");
  بنية stat statbuf;
  ܐܢ (fstat(FD, &statbuf) != 0)
    ܦܢܐ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ܐܢ (S_ISCHR(statbuf.st_mode) && is_displayed())
    ܦܢܐ 0;
  // Return the preferred block size.
  ܦܢܐ statbuf.st_blksize;
#endif
}

منطقي raw_fd_ostream::is_displayed() ثابت {
  ܦܢܐ sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقي raw_fd_ostream::has_colors() ثابت {
  ܐܢ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ܦܢܐ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  ܐܢ (!EC)
    ܦܢܐ sys::fs::FileLocker(FD);
  ܦܢܐ errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ثابت& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  ܐܢ (!EC)
    ܦܢܐ sys::fs::FileLocker(FD);
  ܦܢܐ errorCodeToError(EC);
}

فراغ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  ساكن std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  أكد(!EC1);
  (فراغ)EC1;

  ساكن raw_fd_ostream S("-", EC, sys::fs::OF_None);
  أكد(!EC);
  ܦܢܐ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ساكن std::error_code EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (فراغ)EC;

  // Set standard error to be unbuffered.
  ساكن raw_fd_ostream S(STDERR_FILENO, ܕܓܠܐ, ܫܪܪܐ);
  ܦܢܐ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ساكن raw_null_ostream S;
  ܦܢܐ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     ܫܪܪܐ, ܕܓܠܐ, OStreamKind::OK_FDStream) {
  ܐܢ (EC)
    ܦܢܐ;

  ܐܢ (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(صحيح fd, منطقي shouldClose)
    : raw_fd_ostream(fd, shouldClose, ܕܓܠܐ, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::اقرأ(حرف *Ptr, size_t Size) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (فراغ *)Ptr, Size);
  ܐܢ (Ret >= 0)
    inc_pos(Ret);
  ܐܚܪܢܐ
    error_detected(errnoAsErrorCode());
  ܦܢܐ Ret;
}

منطقي raw_fd_stream::classof(ثابت raw_ostream *OS) {
  ܦܢܐ OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

فراغ raw_string_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ثابت { ܦܢܐ OS.size(); }

فراغ raw_svector_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

فراغ raw_svector_ostream::pwrite_impl(ثابت حرف *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

منطقي raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  ܦܢܐ OS->get_kind() == OStreamKind::OK_SVecStream;
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

فراغ raw_null_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() ثابت {
  ܦܢܐ 0;
}

فراغ raw_null_ostream::pwrite_impl(ثابت حرف *Ptr, size_t Size,
                                   uint64_t Offset) {}

فراغ raw_pwrite_stream::anchor() {}

فراغ buffer_ostream::anchor() {}

فراغ buffer_unique_ostream::anchor() {}

ساكن منطقي isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  ܐܢ (OutputFileName.equals_insensitive("NUL"))
    ܦܢܐ ܫܪܪܐ;
#endif
  ܦܢܐ OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ܐܢ (OutputFileName == "-")
    ܦܢܐ Write(outs());

  ܐܢ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    ܦܢܐ Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  ܐܢ (!Temp)
    ܦܢܐ createFileError(OutputFileName, Temp.takeError());

#ܐܢ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ܐܢ (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    ܐܢ (EC != std::errc::no_such_file_or_directory)
      ܦܢܐ createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, ܕܓܠܐ);

  ܐܢ (Error E = Write(Out)) {
    ܐܢ (Error DiscardError = Temp->discard())
      ܦܢܐ joinErrors(std::move(E), std::move(DiscardError));
    ܦܢܐ E;
  }
  Out.flush();

  ܦܢܐ Temp->keep(OutputFileName);
}
