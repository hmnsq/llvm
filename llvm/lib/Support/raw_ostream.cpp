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

#شامل "llvm/Support/raw_ostream.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/AutoConvert.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/Duration.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/NativeFormatting.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Program.h"
#شامل <algorithm>
#شامل <cerrno>
#شامل <cstdio>
#شامل <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#اگر defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#اگر defined(__CYGWIN__)
#شامل <io.h>
#endif

#اگر defined(_MSC_VER)
#شامل <io.h>
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
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/Windows/WindowsSupport.h"
#endif

using فضاینام llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  أكد(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  اگر (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ثابت {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  بازگشت (16 * 1024);
#وگرنه
  // BUFSIZ is intended to be a reasonable default.
  بازگشت BUFSIZ;
#endif
}

تهی raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  اگر (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  وگرنه
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

تهی raw_ostream::SetBufferAndMode(نویسه *BufferStart, size_t Size,
                                   BufferKind Mode) {
  أكد(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  أكد(Get_رقم_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  اگر (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  أكد(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*این, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*این, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*این, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*این, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  بازگشت *این;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*این, N, HexPrintStyle::Lower);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  اگر (C == Colors::RESET)
    resetColor();
  وگرنه
    changeColor(C);
  بازگشت *این;
}

raw_ostream &raw_ostream::write_uuid(ثابت uuid_t UUID) {
  برای (صحیح Idx = 0; Idx < 16; ++Idx) {
    *این << format("%02" PRIX32, UUID[Idx]);
    اگر (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *این << "-";
  }
  بازگشت *این;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        منطقی UseHexEscapes) {
  برای (unsigned نویسه c : Str) {
    انتخاب (c) {
    حالت '\\':
      *این << '\\' << '\\';
      بشکن;
    حالت '\t':
      *این << '\\' << 't';
      بشکن;
    حالت '\n':
      *این << '\\' << 'n';
      بشکن;
    حالت '"':
      *این << '\\' << '"';
      بشکن;
    پیشفرض:
      اگر (isPrint(c)) {
        *این << c;
        بشکن;
      }

      // Write out the escaped representation.
      اگر (UseHexEscapes) {
        *این << '\\' << 'x';
        *این << hexdigit((c >> 4) & 0xF);
        *این << hexdigit((c >> 0) & 0xF);
      } وگرنه {
        // Always use a full 3-character octal escape.
        *این << '\\';
        *این << نویسه('0' + ((c >> 6) & 7));
        *این << نویسه('0' + ((c >> 3) & 7));
        *این << نویسه('0' + ((c >> 0) & 7));
      }
    }
  }

  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(ثابت تهی *P) {
  llvm::write_hex(*این, (uintptr_t)P, HexPrintStyle::PrefixLower);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(دوگانه N) {
  llvm::write_double(*این, N, FloatStyle::Exponent);
  بازگشت *این;
}

تهی raw_ostream::flush_nonempty() {
  أكد(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::اكتب(unsigned نویسه C) {
  // Group exceptional cases into a single branch.
  اگر (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    اگر (LLVM_UNLIKELY(!OutBufStart)) {
      اگر (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<نویسه *>(&C), 1);
        بازگشت *این;
      }
      // Set up a buffer and start over.
      SetBuffered();
      بازگشت اكتب(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  بازگشت *این;
}

raw_ostream &raw_ostream::اكتب(ثابت نویسه *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  اگر (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    اگر (LLVM_UNLIKELY(!OutBufStart)) {
      اگر (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        بازگشت *این;
      }
      // Set up a buffer and start over.
      SetBuffered();
      بازگشت اكتب(Ptr, Size);
    }

    size_t رقم_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    اگر (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      أكد(رقم_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % رقم_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      اگر (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        بازگشت اكتب(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      بازگشت *این;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, رقم_Bytes);
    flush_nonempty();
    بازگشت اكتب(Ptr + رقم_Bytes, Size - رقم_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  بازگشت *این;
}

تهی raw_ostream::copy_to_buffer(ثابت نویسه *Ptr, size_t Size) {
  أكد(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  انتخاب (Size) {
  حالت 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  حالت 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  حالت 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  حالت 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  حالت 0: بشکن;
  پیشفرض:
    memcpy(OutBufCur, Ptr, Size);
    بشکن;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<صحیح(نویسه *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_چپ = OutBufEnd - OutBufCur;
  اگر (Buffer_Bytes_چپ > 3) {
    صحیح N = Snprint(OutBufCur, Buffer_Bytes_چپ);
    اگر (N < 0)
      بازگشت *این;
    // Common case is that we have plenty of space.
    اگر (size_t(N) < Buffer_Bytes_چپ) {
      OutBufCur += N;
      بازگشت *این;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<نویسه, 128> V;
  برای (;;) {
    V.resize(Size);
    صحیح N = Snprint(V.data(), Size);
    اگر (N < 0)
      بازگشت *این;
    اگر (size_t(N) < Size)
      بازگشت اكتب(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ثابت formatv_object_base &Obj) {
  Obj.format(*این);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedString &FS) {
  unsigned چپ_Indent = 0;
  unsigned راست_Indent = 0;
  ثابت ssize_t Difference = FS.Width - FS.Str.size();
  اگر (Difference > 0) {
    انتخاب (FS.Justify) {
    حالت FormattedString::JustifyNone:
      بشکن;
    حالت FormattedString::Justify_چپ:
      راست_Indent = Difference;
      بشکن;
    حالت FormattedString::Justify_راست:
      چپ_Indent = Difference;
      بشکن;
    حالت FormattedString::JustifyCenter:
      چپ_Indent = Difference / 2;
      راست_Indent = Difference - چپ_Indent;
      بشکن;
    }
  }
  indent(چپ_Indent);
  (*این) << FS.Str;
  indent(راست_Indent);
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(ثابت Formatted_عدد &FN) {
  اگر (FN.Hex) {
    HexPrintStyle Style;
    اگر (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    وگرنه اگر (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    وگرنه اگر (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    وگرنه
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*این, FN.Hex_مقدار, Style, FN.Width);
  } وگرنه {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_مقدار, 0, IntegerStyle::Integer);
    اگر (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*این) << Buffer;
  }
  بازگشت *این;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedBytes &FB) {
  اگر (FB.Bytes.empty())
    بازگشت *این;

  size_t سطر_Index = 0;
  auto Bytes = FB.Bytes;
  ثابت size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  اگر (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t سطرها = Size / FB.رقم_Per_سطر;
    uint64_t MaxOffset = *FB.FirstByteOffset + سطرها * FB.رقم_Per_سطر;
    unsigned Power = 0;
    اگر (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned رقم_Byte_Groups =
      alignTo(FB.رقم_Per_سطر, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.رقم_Per_سطر * 2 + رقم_Byte_Groups - 1;

  وقتی (!Bytes.empty()) {
    indent(FB.IndentLevel);

    اگر (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*این, Offset + سطر_Index, HPS, OffsetWidth);
      *این << ": ";
    }

    auto سطر = Bytes.take_front(FB.رقم_Per_سطر);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    برای (size_t I = 0; I < سطر.size(); ++I, CharsPrinted += 2) {
      اگر (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *این << " ";
      }
      llvm::write_hex(*این, سطر[I], HPS, 2);
    }

    اگر (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *این << "|";

      // Print the ASCII char values for each byte on this line
      برای (uint8_t Byte : سطر) {
        اگر (isPrint(Byte))
          *این << static_cast<نویسه>(Byte);
        وگرنه
          *این << '.';
      }
      *این << '|';
    }

    Bytes = Bytes.drop_front(سطر.size());
    سطر_Index += سطر.size();
    اگر (سطر_Index < Size)
      *این << '\n';
  }
  بازگشت *این;
}

template <نویسه C>
ایستا raw_ostream &write_padding(raw_ostream &OS, unsigned رقم_Chars) {
  ایستا ثابت نویسه Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  اگر (رقم_Chars < std::size(Chars))
    بازگشت OS.اكتب(Chars, رقم_Chars);

  وقتی (رقم_Chars) {
    unsigned رقم_To_Write = std::min(رقم_Chars, (unsigned)std::size(Chars) - 1);
    OS.اكتب(Chars, رقم_To_Write);
    رقم_Chars -= رقم_To_Write;
  }
  بازگشت OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned رقم_Spaces) {
  بازگشت write_padding<' '>(*این, رقم_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned رقم_Zeros) {
  بازگشت write_padding<'\0'>(*این, رقم_Zeros);
}

منطقی raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  اگر (!ColorEnabled)
    بازگشت نادرست;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  اگر (sys::Process::ColorNeedsFlush() && !is_displayed())
    بازگشت نادرست;

  اگر (sys::Process::ColorNeedsFlush())
    flush();

  بازگشت درست;
}

raw_ostream &raw_ostream::changeColor(شمارش Colors colors, منطقی bold, منطقی bg) {
  اگر (!prepare_colors())
    بازگشت *این;

  ثابت نویسه *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<نویسه>(colors), bold, bg);
  اگر (colorcode)
    اكتب(colorcode, strlen(colorcode));
  بازگشت *این;
}

raw_ostream &raw_ostream::resetColor() {
  اگر (!prepare_colors())
    بازگشت *این;

  اگر (ثابت نویسه *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, strlen(colorcode));
  بازگشت *این;
}

raw_ostream &raw_ostream::reverseColor() {
  اگر (!prepare_colors())
    بازگشت *این;

  اگر (ثابت نویسه *colorcode = sys::Process::OutputReverse())
    اكتب(colorcode, strlen(colorcode));
  بازگشت *این;
}

تهی raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ایستا صحیح getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  أكد((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  اگر (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    بازگشت STDOUT_FILENO;
  }

  صحیح FD;
  اگر (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  وگرنه
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  اگر (EC)
    بازگشت -1;

  بازگشت FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), درست) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(صحیح fd, منطقی shouldClose, منطقی unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  اگر (FD < 0 ) {
    ShouldClose = نادرست;
    بازگشت;
  }

  enable_colors(درست);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  اگر (FD <= STDERR_FILENO)
    ShouldClose = نادرست;

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
#وگرنه
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  اگر (!SupportsSeeking)
    pos = 0;
  وگرنه
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  اگر (FD >= 0) {
    flush();
    اگر (ShouldClose) {
      اگر (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  اگر (FD == 2) بازگشت;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  اگر (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#اگر defined(_WIN32)
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
ایستا منطقی write_console_impl(صحیح FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  اگر (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    بازگشت نادرست;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  اگر (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  انجامبده {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    منطقی Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    اگر (!Success)
      بازگشت نادرست;

    WCharsWritten += ActuallyWritten;
  } وقتی (WCharsWritten != WideText.size());
  بازگشت درست;
}
#endif

تهی raw_fd_ostream::write_impl(ثابت نویسه *Ptr, size_t Size) {
  اگر (TiedStream)
    TiedStream->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += Size;

#اگر defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  اگر (IsWindowsConsole)
    اگر (write_console_impl(FD, StringRef(Ptr, Size)))
      بازگشت;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#اگر defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  انجامبده {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::اكتب(FD, Ptr, ChunkSize);

    اگر (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      اگر (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        ادامه;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      اگر (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      بشکن;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } وقتی (Size > 0);
}

تهی raw_fd_ostream::close() {
  أكد(ShouldClose);
  ShouldClose = نادرست;
  flush();
  اگر (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#وگرنه
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  اگر (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  بازگشت pos;
}

تهی raw_fd_ostream::pwrite_impl(ثابت نویسه *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  اكتب(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ثابت {
#اگر defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  اگر (IsWindowsConsole)
    بازگشت 0;
  بازگشت raw_ostream::preferred_buffer_size();
#وگرنهاگر defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  بازگشت raw_ostream::preferred_buffer_size();
#وگرنه
  أكد(FD >= 0 && "File not yet open!");
  ساختار stat statbuf;
  اگر (fstat(FD, &statbuf) != 0)
    بازگشت 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  اگر (S_ISCHR(statbuf.st_mode) && is_displayed())
    بازگشت 0;
  // Return the preferred block size.
  بازگشت statbuf.st_blksize;
#endif
}

منطقی raw_fd_ostream::is_displayed() ثابت {
  بازگشت sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقی raw_fd_ostream::has_colors() ثابت {
  اگر (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  بازگشت *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  اگر (!EC)
    بازگشت sys::fs::FileLocker(FD);
  بازگشت errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ثابت& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  اگر (!EC)
    بازگشت sys::fs::FileLocker(FD);
  بازگشت errorCodeToError(EC);
}

تهی raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  ایستا std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  أكد(!EC1);
  (تهی)EC1;

  ایستا raw_fd_ostream S("-", EC, sys::fs::OF_None);
  أكد(!EC);
  بازگشت S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ایستا std::error_code EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (تهی)EC;

  // Set standard error to be unbuffered.
  ایستا raw_fd_ostream S(STDERR_FILENO, نادرست, درست);
  بازگشت S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ایستا raw_null_ostream S;
  بازگشت S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     درست, نادرست, OStreamKind::OK_FDStream) {
  اگر (EC)
    بازگشت;

  اگر (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(صحیح fd, منطقی shouldClose)
    : raw_fd_ostream(fd, shouldClose, نادرست, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::اقرأ(نویسه *Ptr, size_t Size) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (تهی *)Ptr, Size);
  اگر (Ret >= 0)
    inc_pos(Ret);
  وگرنه
    error_detected(errnoAsErrorCode());
  بازگشت Ret;
}

منطقی raw_fd_stream::classof(ثابت raw_ostream *OS) {
  بازگشت OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

تهی raw_string_ostream::write_impl(ثابت نویسه *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ثابت { بازگشت OS.size(); }

تهی raw_svector_ostream::write_impl(ثابت نویسه *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

تهی raw_svector_ostream::pwrite_impl(ثابت نویسه *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

منطقی raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  بازگشت OS->get_kind() == OStreamKind::OK_SVecStream;
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

تهی raw_null_ostream::write_impl(ثابت نویسه *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() ثابت {
  بازگشت 0;
}

تهی raw_null_ostream::pwrite_impl(ثابت نویسه *Ptr, size_t Size,
                                   uint64_t Offset) {}

تهی raw_pwrite_stream::anchor() {}

تهی buffer_ostream::anchor() {}

تهی buffer_unique_ostream::anchor() {}

ایستا منطقی isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  اگر (OutputFileName.equals_insensitive("NUL"))
    بازگشت درست;
#endif
  بازگشت OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  اگر (OutputFileName == "-")
    بازگشت Write(outs());

  اگر (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    بازگشت Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  اگر (!Temp)
    بازگشت createFileError(OutputFileName, Temp.takeError());

#اگر defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  اگر (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    اگر (EC != std::errc::no_such_file_or_directory)
      بازگشت createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, نادرست);

  اگر (Error E = Write(Out)) {
    اگر (Error DiscardError = Temp->discard())
      بازگشت joinErrors(std::move(E), std::move(DiscardError));
    بازگشت E;
  }
  Out.flush();

  بازگشت Temp->keep(OutputFileName);
}
