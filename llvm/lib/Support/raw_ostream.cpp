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

#إذا defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#إذا defined(__CYGWIN__)
#ضمن <io.h>
#endif

#إذا defined(_MSC_VER)
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

  إذا (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ثابت {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  إرجاع (16 * 1024);
#وإلا
  // BUFSIZ is intended to be a reasonable default.
  إرجاع BUFSIZ;
#endif
}

فراغ raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  إذا (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  وإلا
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
  أكد(Get_رقم_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  إذا (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  أكد(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*هذا, N, HexPrintStyle::Lower);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  إذا (C == Colors::RESET)
    resetColor();
  وإلا
    changeColor(C);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::write_uuid(ثابت uuid_t UUID) {
  لكل (صحيح Idx = 0; Idx < 16; ++Idx) {
    *هذا << format("%02" PRIX32, UUID[Idx]);
    إذا (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *هذا << "-";
  }
  إرجاع *هذا;
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
      إذا (isPrint(c)) {
        *هذا << c;
        اكسر;
      }

      // Write out the escaped representation.
      إذا (UseHexEscapes) {
        *هذا << '\\' << 'x';
        *هذا << hexdigit((c >> 4) & 0xF);
        *هذا << hexdigit((c >> 0) & 0xF);
      } وإلا {
        // Always use a full 3-character octal escape.
        *هذا << '\\';
        *هذا << حرف('0' + ((c >> 6) & 7));
        *هذا << حرف('0' + ((c >> 3) & 7));
        *هذا << حرف('0' + ((c >> 0) & 7));
      }
    }
  }

  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت فراغ *P) {
  llvm::write_hex(*هذا, (uintptr_t)P, HexPrintStyle::PrefixLower);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(مزدوج N) {
  llvm::write_double(*هذا, N, FloatStyle::Exponent);
  إرجاع *هذا;
}

فراغ raw_ostream::flush_nonempty() {
  أكد(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::اكتب(unsigned حرف C) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    إذا (LLVM_UNLIKELY(!OutBufStart)) {
      إذا (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<حرف *>(&C), 1);
        إرجاع *هذا;
      }
      // Set up a buffer and start over.
      SetBuffered();
      إرجاع اكتب(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  إرجاع *هذا;
}

raw_ostream &raw_ostream::اكتب(ثابت حرف *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    إذا (LLVM_UNLIKELY(!OutBufStart)) {
      إذا (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        إرجاع *هذا;
      }
      // Set up a buffer and start over.
      SetBuffered();
      إرجاع اكتب(Ptr, Size);
    }

    size_t رقم_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    إذا (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      أكد(رقم_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % رقم_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      إذا (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        إرجاع اكتب(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      إرجاع *هذا;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, رقم_Bytes);
    flush_nonempty();
    إرجاع اكتب(Ptr + رقم_Bytes, Size - رقم_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  إرجاع *هذا;
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
  size_t Buffer_Bytes_يسار = OutBufEnd - OutBufCur;
  إذا (Buffer_Bytes_يسار > 3) {
    صحيح N = Snprint(OutBufCur, Buffer_Bytes_يسار);
    إذا (N < 0)
      إرجاع *هذا;
    // Common case is that we have plenty of space.
    إذا (size_t(N) < Buffer_Bytes_يسار) {
      OutBufCur += N;
      إرجاع *هذا;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<حرف, 128> V;
  لكل (;;) {
    V.resize(Size);
    صحيح N = Snprint(V.data(), Size);
    إذا (N < 0)
      إرجاع *هذا;
    إذا (size_t(N) < Size)
      إرجاع اكتب(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ثابت formatv_object_base &Obj) {
  Obj.format(*هذا);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedString &FS) {
  unsigned يسار_Indent = 0;
  unsigned يمين_Indent = 0;
  ثابت ssize_t Difference = FS.Width - FS.Str.size();
  إذا (Difference > 0) {
    بدل (FS.Justify) {
    حالة FormattedString::JustifyNone:
      اكسر;
    حالة FormattedString::Justify_يسار:
      يمين_Indent = Difference;
      اكسر;
    حالة FormattedString::Justify_يمين:
      يسار_Indent = Difference;
      اكسر;
    حالة FormattedString::JustifyCenter:
      يسار_Indent = Difference / 2;
      يمين_Indent = Difference - يسار_Indent;
      اكسر;
    }
  }
  indent(يسار_Indent);
  (*هذا) << FS.Str;
  indent(يمين_Indent);
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت Formatted_العدد &FN) {
  إذا (FN.Hex) {
    HexPrintStyle Style;
    إذا (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    وإلا إذا (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    وإلا إذا (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    وإلا
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*هذا, FN.Hex_قيمة, Style, FN.Width);
  } وإلا {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_قيمة, 0, IntegerStyle::Integer);
    إذا (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*هذا) << Buffer;
  }
  إرجاع *هذا;
}

raw_ostream &raw_ostream::operator<<(ثابت FormattedBytes &FB) {
  إذا (FB.Bytes.empty())
    إرجاع *هذا;

  size_t سطر_Index = 0;
  auto Bytes = FB.Bytes;
  ثابت size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  إذا (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t أسطر = Size / FB.رقم_Per_سطر;
    uint64_t MaxOffset = *FB.FirstByteOffset + أسطر * FB.رقم_Per_سطر;
    unsigned Power = 0;
    إذا (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned رقم_Byte_Groups =
      alignTo(FB.رقم_Per_سطر, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.رقم_Per_سطر * 2 + رقم_Byte_Groups - 1;

  بينما (!Bytes.empty()) {
    indent(FB.IndentLevel);

    إذا (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*هذا, Offset + سطر_Index, HPS, OffsetWidth);
      *هذا << ": ";
    }

    auto سطر = Bytes.take_front(FB.رقم_Per_سطر);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    لكل (size_t I = 0; I < سطر.size(); ++I, CharsPrinted += 2) {
      إذا (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *هذا << " ";
      }
      llvm::write_hex(*هذا, سطر[I], HPS, 2);
    }

    إذا (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *هذا << "|";

      // Print the ASCII char values for each byte on this line
      لكل (uint8_t Byte : سطر) {
        إذا (isPrint(Byte))
          *هذا << static_cast<حرف>(Byte);
        وإلا
          *هذا << '.';
      }
      *هذا << '|';
    }

    Bytes = Bytes.drop_front(سطر.size());
    سطر_Index += سطر.size();
    إذا (سطر_Index < Size)
      *هذا << '\n';
  }
  إرجاع *هذا;
}

template <حرف C>
ساكن raw_ostream &write_padding(raw_ostream &OS, unsigned رقم_Chars) {
  ساكن ثابت حرف Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  إذا (رقم_Chars < std::size(Chars))
    إرجاع OS.اكتب(Chars, رقم_Chars);

  بينما (رقم_Chars) {
    unsigned رقم_To_Write = std::min(رقم_Chars, (unsigned)std::size(Chars) - 1);
    OS.اكتب(Chars, رقم_To_Write);
    رقم_Chars -= رقم_To_Write;
  }
  إرجاع OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned رقم_Spaces) {
  إرجاع write_padding<' '>(*هذا, رقم_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned رقم_Zeros) {
  إرجاع write_padding<'\0'>(*هذا, رقم_Zeros);
}

منطقي raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  إذا (!ColorEnabled)
    إرجاع خطأ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  إذا (sys::Process::ColorNeedsFlush() && !is_displayed())
    إرجاع خطأ;

  إذا (sys::Process::ColorNeedsFlush())
    flush();

  إرجاع true;
}

raw_ostream &raw_ostream::changeColor(تعداد Colors colors, منطقي bold, منطقي bg) {
  إذا (!prepare_colors())
    إرجاع *هذا;

  ثابت حرف *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<حرف>(colors), bold, bg);
  إذا (colorcode)
    اكتب(colorcode, strlen(colorcode));
  إرجاع *هذا;
}

raw_ostream &raw_ostream::resetColor() {
  إذا (!prepare_colors())
    إرجاع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, strlen(colorcode));
  إرجاع *هذا;
}

raw_ostream &raw_ostream::reverseColor() {
  إذا (!prepare_colors())
    إرجاع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::OutputReverse())
    اكتب(colorcode, strlen(colorcode));
  إرجاع *هذا;
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
  إذا (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    إرجاع STDOUT_FILENO;
  }

  صحيح FD;
  إذا (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  وإلا
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  إذا (EC)
    إرجاع -1;

  إرجاع FD;
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
raw_fd_ostream::raw_fd_ostream(صحيح fd, منطقي shouldClose, منطقي unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  إذا (FD < 0 ) {
    ShouldClose = خطأ;
    إرجاع;
  }

  enable_colors(true);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  إذا (FD <= STDERR_FILENO)
    ShouldClose = خطأ;

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
#وإلا
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  إذا (!SupportsSeeking)
    pos = 0;
  وإلا
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  إذا (FD >= 0) {
    flush();
    إذا (ShouldClose) {
      إذا (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  إذا (FD == 2) إرجاع;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  إذا (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#إذا defined(_WIN32)
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
  إذا (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    إرجاع خطأ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  إذا (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  افعل {
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
    إذا (!Success)
      إرجاع خطأ;

    WCharsWritten += ActuallyWritten;
  } بينما (WCharsWritten != WideText.size());
  إرجاع true;
}
#endif

فراغ raw_fd_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  إذا (TiedStream)
    TiedStream->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += Size;

#إذا defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  إذا (IsWindowsConsole)
    إذا (write_console_impl(FD, StringRef(Ptr, Size)))
      إرجاع;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#إذا defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  افعل {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::اكتب(FD, Ptr, ChunkSize);

    إذا (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      إذا (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        استمر;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      إذا (WinLastError == ERROR_BROKEN_PIPE ||
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
  } بينما (Size > 0);
}

فراغ raw_fd_ostream::close() {
  أكد(ShouldClose);
  ShouldClose = خطأ;
  flush();
  إذا (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#وإلا
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  إذا (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  إرجاع pos;
}

فراغ raw_fd_ostream::pwrite_impl(ثابت حرف *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  اكتب(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ثابت {
#إذا defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  إذا (IsWindowsConsole)
    إرجاع 0;
  إرجاع raw_ostream::preferred_buffer_size();
#وإلاإذا defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  إرجاع raw_ostream::preferred_buffer_size();
#وإلا
  أكد(FD >= 0 && "File not yet open!");
  بنية stat statbuf;
  إذا (fstat(FD, &statbuf) != 0)
    إرجاع 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  إذا (S_ISCHR(statbuf.st_mode) && is_displayed())
    إرجاع 0;
  // Return the preferred block size.
  إرجاع statbuf.st_blksize;
#endif
}

منطقي raw_fd_ostream::is_displayed() ثابت {
  إرجاع sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقي raw_fd_ostream::has_colors() ثابت {
  إذا (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  إرجاع *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  إذا (!EC)
    إرجاع sys::fs::FileLocker(FD);
  إرجاع errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ثابت& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  إذا (!EC)
    إرجاع sys::fs::FileLocker(FD);
  إرجاع errorCodeToError(EC);
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
  إرجاع S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ساكن std::error_code EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (فراغ)EC;

  // Set standard error to be unbuffered.
  ساكن raw_fd_ostream S(STDERR_FILENO, خطأ, true);
  إرجاع S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ساكن raw_null_ostream S;
  إرجاع S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     true, خطأ, OStreamKind::OK_FDStream) {
  إذا (EC)
    إرجاع;

  إذا (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(صحيح fd, منطقي shouldClose)
    : raw_fd_ostream(fd, shouldClose, خطأ, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::اقرأ(حرف *Ptr, size_t Size) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (فراغ *)Ptr, Size);
  إذا (Ret >= 0)
    inc_pos(Ret);
  وإلا
    error_detected(errnoAsErrorCode());
  إرجاع Ret;
}

منطقي raw_fd_stream::classof(ثابت raw_ostream *OS) {
  إرجاع OS->get_kind() == OStreamKind::OK_FDStream;
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

uint64_t raw_svector_ostream::current_pos() ثابت { إرجاع OS.size(); }

فراغ raw_svector_ostream::write_impl(ثابت حرف *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

فراغ raw_svector_ostream::pwrite_impl(ثابت حرف *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

منطقي raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  إرجاع OS->get_kind() == OStreamKind::OK_SVecStream;
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
  إرجاع 0;
}

فراغ raw_null_ostream::pwrite_impl(ثابت حرف *Ptr, size_t Size,
                                   uint64_t Offset) {}

فراغ raw_pwrite_stream::anchor() {}

فراغ buffer_ostream::anchor() {}

فراغ buffer_unique_ostream::anchor() {}

ساكن منطقي isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  إذا (OutputFileName.equals_insensitive("NUL"))
    إرجاع true;
#endif
  إرجاع OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  إذا (OutputFileName == "-")
    إرجاع Write(outs());

  إذا (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    إرجاع Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  إذا (!Temp)
    إرجاع createFileError(OutputFileName, Temp.takeError());

#إذا defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  إذا (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    إذا (EC != std::errc::no_such_file_or_directory)
      إرجاع createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, خطأ);

  إذا (Error E = Write(Out)) {
    إذا (Error DiscardError = Temp->discard())
      إرجاع joinErrors(std::move(E), std::move(DiscardError));
    إرجاع E;
  }
  Out.flush();

  إرجاع Temp->keep(OutputFileName);
}
