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

#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Config/config.h"
#include "llvm/Support/AutoConvert.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Duration.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/NativeFormatting.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/Program.h"
#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ако defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#ако defined(__CYGWIN__)
#include <io.h>
#endif

#ако defined(_MSC_VER)
#include <io.h>
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
#include "llvm/Support/ConvertUTF.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Windows/WindowsSupport.h"
#endif

using namespace llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  assert(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  ако (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() const {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  врати (16 * 1024);
#инако
  // BUFSIZ is intended to be a reasonable default.
  врати BUFSIZ;
#endif
}

void raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ако (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  инако
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

void raw_ostream::SetBufferAndMode(char *BufferStart, size_t Size,
                                   BufferKind Mode) {
  assert(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  assert(GetNumBytesInBuffer() == 0 && "Current buffer is non-empty!");

  ако (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  assert(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*this, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*this, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*this, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*this, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  врати *this;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*this, N, HexPrintStyle::Lower);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  ако (C == Colors::RESET)
    resetColor();
  инако
    changeColor(C);
  врати *this;
}

raw_ostream &raw_ostream::write_uuid(const uuid_t UUID) {
  за (цео Idx = 0; Idx < 16; ++Idx) {
    *this << format("%02" PRIX32, UUID[Idx]);
    ако (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *this << "-";
  }
  врати *this;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        bool UseHexEscapes) {
  за (unsigned char c : Str) {
    switch (c) {
    case '\\':
      *this << '\\' << '\\';
      break;
    case '\t':
      *this << '\\' << 't';
      break;
    case '\n':
      *this << '\\' << 'n';
      break;
    case '"':
      *this << '\\' << '"';
      break;
    default:
      ако (isPrint(c)) {
        *this << c;
        break;
      }

      // Write out the escaped representation.
      ако (UseHexEscapes) {
        *this << '\\' << 'x';
        *this << hexdigit((c >> 4) & 0xF);
        *this << hexdigit((c >> 0) & 0xF);
      } инако {
        // Always use a full 3-character octal escape.
        *this << '\\';
        *this << char('0' + ((c >> 6) & 7));
        *this << char('0' + ((c >> 3) & 7));
        *this << char('0' + ((c >> 0) & 7));
      }
    }
  }

  врати *this;
}

raw_ostream &raw_ostream::operator<<(const void *P) {
  llvm::write_hex(*this, (uintptr_t)P, HexPrintStyle::PrefixLower);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(double N) {
  llvm::write_double(*this, N, FloatStyle::Exponent);
  врати *this;
}

void raw_ostream::flush_nonempty() {
  assert(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::write(unsigned char C) {
  // Group exceptional cases into a single branch.
  ако (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    ако (LLVM_UNLIKELY(!OutBufStart)) {
      ако (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<char *>(&C), 1);
        врати *this;
      }
      // Set up a buffer and start over.
      SetBuffered();
      врати write(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  врати *this;
}

raw_ostream &raw_ostream::write(const char *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  ако (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    ако (LLVM_UNLIKELY(!OutBufStart)) {
      ако (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        врати *this;
      }
      // Set up a buffer and start over.
      SetBuffered();
      врати write(Ptr, Size);
    }

    size_t NumBytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ако (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      assert(NumBytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % NumBytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      ако (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        врати write(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      врати *this;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, NumBytes);
    flush_nonempty();
    врати write(Ptr + NumBytes, Size - NumBytes);
  }

  copy_to_buffer(Ptr, Size);

  врати *this;
}

void raw_ostream::copy_to_buffer(const char *Ptr, size_t Size) {
  assert(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  switch (Size) {
  case 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  case 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  case 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  case 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  case 0: break;
  default:
    memcpy(OutBufCur, Ptr, Size);
    break;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<цео(char *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t BufferBytesLeft = OutBufEnd - OutBufCur;
  ако (BufferBytesLeft > 3) {
    цео N = Snprint(OutBufCur, BufferBytesLeft);
    ако (N < 0)
      врати *this;
    // Common case is that we have plenty of space.
    ако (size_t(N) < BufferBytesLeft) {
      OutBufCur += N;
      врати *this;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<char, 128> V;
  за (;;) {
    V.resize(Size);
    цео N = Snprint(V.data(), Size);
    ако (N < 0)
      врати *this;
    ако (size_t(N) < Size)
      врати write(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(const formatv_object_base &Obj) {
  Obj.format(*this);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(const FormattedString &FS) {
  unsigned LeftIndent = 0;
  unsigned RightIndent = 0;
  const ssize_t Difference = FS.Width - FS.Str.size();
  ако (Difference > 0) {
    switch (FS.Justify) {
    case FormattedString::JustifyNone:
      break;
    case FormattedString::JustifyLeft:
      RightIndent = Difference;
      break;
    case FormattedString::JustifyRight:
      LeftIndent = Difference;
      break;
    case FormattedString::JustifyCenter:
      LeftIndent = Difference / 2;
      RightIndent = Difference - LeftIndent;
      break;
    }
  }
  indent(LeftIndent);
  (*this) << FS.Str;
  indent(RightIndent);
  врати *this;
}

raw_ostream &raw_ostream::operator<<(const FormattedNumber &FN) {
  ако (FN.Hex) {
    HexPrintStyle Style;
    ако (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    инако ако (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    инако ако (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    инако
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*this, FN.HexValue, Style, FN.Width);
  } инако {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.DecValue, 0, IntegerStyle::Integer);
    ако (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*this) << Buffer;
  }
  врати *this;
}

raw_ostream &raw_ostream::operator<<(const FormattedBytes &FB) {
  ако (FB.Bytes.empty())
    врати *this;

  size_t LineIndex = 0;
  auto Bytes = FB.Bytes;
  const size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  ако (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t Lines = Size / FB.NumPerLine;
    uint64_t MaxOffset = *FB.FirstByteOffset + Lines * FB.NumPerLine;
    unsigned Power = 0;
    ако (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned NumByteGroups =
      alignTo(FB.NumPerLine, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.NumPerLine * 2 + NumByteGroups - 1;

  док (!Bytes.empty()) {
    indent(FB.IndentLevel);

    ако (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*this, Offset + LineIndex, HPS, OffsetWidth);
      *this << ": ";
    }

    auto Line = Bytes.take_front(FB.NumPerLine);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    за (size_t I = 0; I < Line.size(); ++I, CharsPrinted += 2) {
      ако (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *this << " ";
      }
      llvm::write_hex(*this, Line[I], HPS, 2);
    }

    ако (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      assert(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *this << "|";

      // Print the ASCII char values for each byte on this line
      за (uint8_t Byte : Line) {
        ако (isPrint(Byte))
          *this << static_cast<char>(Byte);
        инако
          *this << '.';
      }
      *this << '|';
    }

    Bytes = Bytes.drop_front(Line.size());
    LineIndex += Line.size();
    ако (LineIndex < Size)
      *this << '\n';
  }
  врати *this;
}

template <char C>
static raw_ostream &write_padding(raw_ostream &OS, unsigned NumChars) {
  static const char Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ако (NumChars < std::size(Chars))
    врати OS.write(Chars, NumChars);

  док (NumChars) {
    unsigned NumToWrite = std::min(NumChars, (unsigned)std::size(Chars) - 1);
    OS.write(Chars, NumToWrite);
    NumChars -= NumToWrite;
  }
  врати OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned NumSpaces) {
  врати write_padding<' '>(*this, NumSpaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned NumZeros) {
  врати write_padding<'\0'>(*this, NumZeros);
}

bool raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ако (!ColorEnabled)
    врати false;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ако (sys::Process::ColorNeedsFlush() && !is_displayed())
    врати false;

  ако (sys::Process::ColorNeedsFlush())
    flush();

  врати true;
}

raw_ostream &raw_ostream::changeColor(enum Colors colors, bool bold, bool bg) {
  ако (!prepare_colors())
    врати *this;

  const char *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<char>(colors), bold, bg);
  ако (colorcode)
    write(colorcode, strlen(colorcode));
  врати *this;
}

raw_ostream &raw_ostream::resetColor() {
  ако (!prepare_colors())
    врати *this;

  ако (const char *colorcode = sys::Process::ResetColor())
    write(colorcode, strlen(colorcode));
  врати *this;
}

raw_ostream &raw_ostream::reverseColor() {
  ако (!prepare_colors())
    врати *this;

  ако (const char *colorcode = sys::Process::OutputReverse())
    write(colorcode, strlen(colorcode));
  врати *this;
}

void raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

static цео getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  assert((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ако (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    врати STDOUT_FILENO;
  }

  цео FD;
  ако (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  инако
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ако (EC)
    врати -1;

  врати FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), true) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(цео fd, bool shouldClose, bool unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ако (FD < 0 ) {
    ShouldClose = false;
    врати;
  }

  enable_colors(true);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ако (FD <= STDERR_FILENO)
    ShouldClose = false;

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
#инако
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  ако (!SupportsSeeking)
    pos = 0;
  инако
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ако (FD >= 0) {
    flush();
    ако (ShouldClose) {
      ако (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ако (FD == 2) врати;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ако (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#ако defined(_WIN32)
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
static bool write_console_impl(цео FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  ако (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    врати false;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  ако (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  do {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    bool Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ако (!Success)
      врати false;

    WCharsWritten += ActuallyWritten;
  } док (WCharsWritten != WideText.size());
  врати true;
}
#endif

void raw_fd_ostream::write_impl(const char *Ptr, size_t Size) {
  ако (TiedStream)
    TiedStream->flush();

  assert(FD >= 0 && "File already closed.");
  pos += Size;

#ако defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ако (IsWindowsConsole)
    ако (write_console_impl(FD, StringRef(Ptr, Size)))
      врати;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#ако defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  do {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::write(FD, Ptr, ChunkSize);

    ако (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ако (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        continue;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      ако (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      break;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } док (Size > 0);
}

void raw_fd_ostream::close() {
  assert(ShouldClose);
  ShouldClose = false;
  flush();
  ако (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  assert(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#инако
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  ако (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  врати pos;
}

void raw_fd_ostream::pwrite_impl(const char *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  write(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() const {
#ако defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ако (IsWindowsConsole)
    врати 0;
  врати raw_ostream::preferred_buffer_size();
#elif defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  врати raw_ostream::preferred_buffer_size();
#инако
  assert(FD >= 0 && "File not yet open!");
  struct stat statbuf;
  ако (fstat(FD, &statbuf) != 0)
    врати 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ако (S_ISCHR(statbuf.st_mode) && is_displayed())
    врати 0;
  // Return the preferred block size.
  врати statbuf.st_blksize;
#endif
}

bool raw_fd_ostream::is_displayed() const {
  врати sys::Process::FileDescriptorIsDisplayed(FD);
}

bool raw_fd_ostream::has_colors() const {
  ако (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  врати *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  ако (!EC)
    врати sys::fs::FileLocker(FD);
  врати errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration const& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  ако (!EC)
    врати sys::fs::FileLocker(FD);
  врати errorCodeToError(EC);
}

void raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  static std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  assert(!EC1);
  (void)EC1;

  static raw_fd_ostream S("-", EC, sys::fs::OF_None);
  assert(!EC);
  врати S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  static std::error_code EC = enableAutoConversion(STDERR_FILENO);
  assert(!EC);
  (void)EC;

  // Set standard error to be unbuffered.
  static raw_fd_ostream S(STDERR_FILENO, false, true);
  врати S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  static raw_null_ostream S;
  врати S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     true, false, OStreamKind::OK_FDStream) {
  ако (EC)
    врати;

  ако (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(цео fd, bool shouldClose)
    : raw_fd_ostream(fd, shouldClose, false, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::read(char *Ptr, size_t Size) {
  assert(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::read(get_fd(), (void *)Ptr, Size);
  ако (Ret >= 0)
    inc_pos(Ret);
  инако
    error_detected(errnoAsErrorCode());
  врати Ret;
}

bool raw_fd_stream::classof(const raw_ostream *OS) {
  врати OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

void raw_string_ostream::write_impl(const char *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() const { врати OS.size(); }

void raw_svector_ostream::write_impl(const char *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

void raw_svector_ostream::pwrite_impl(const char *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

bool raw_svector_ostream::classof(const raw_ostream *OS) {
  врати OS->get_kind() == OStreamKind::OK_SVecStream;
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

void raw_null_ostream::write_impl(const char *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() const {
  врати 0;
}

void raw_null_ostream::pwrite_impl(const char *Ptr, size_t Size,
                                   uint64_t Offset) {}

void raw_pwrite_stream::anchor() {}

void buffer_ostream::anchor() {}

void buffer_unique_ostream::anchor() {}

static bool isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  ако (OutputFileName.equals_insensitive("NUL"))
    врати true;
#endif
  врати OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ако (OutputFileName == "-")
    врати Write(outs());

  ако (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    врати Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  ако (!Temp)
    врати createFileError(OutputFileName, Temp.takeError());

#ако defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ако (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    ако (EC != std::errc::no_such_file_or_directory)
      врати createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, false);

  ако (Error E = Write(Out)) {
    ако (Error DiscardError = Temp->discard())
      врати joinErrors(std::move(E), std::move(DiscardError));
    врати E;
  }
  Out.flush();

  врати Temp->keep(OutputFileName);
}
