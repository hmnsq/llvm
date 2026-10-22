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

#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/AutoConvert.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/Duration.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/FormatVariadic.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/MathExtras.h"
#અન્તર્ભાવ "llvm/Support/NativeFormatting.h"
#અન્તર્ભાવ "llvm/Support/Process.h"
#અન્તર્ભાવ "llvm/Support/Program.h"
#અન્તર્ભાવ <algorithm>
#અન્તર્ભાવ <cerrno>
#અન્તર્ભાવ <cstdio>
#અન્તર્ભાવ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#જો defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#જો defined(__CYGWIN__)
#અન્તર્ભાવ <io.h>
#endif

#જો defined(_MSC_VER)
#અન્તર્ભાવ <io.h>
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
#અન્તર્ભાવ "llvm/Support/ConvertUTF.h"
#અન્તર્ભાવ "llvm/Support/Signals.h"
#અન્તર્ભાવ "llvm/Support/Windows/WindowsSupport.h"
#endif

using નામાકાશ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  પ્રતિજ્ઞા(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  જો (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() અચલ {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  પાછા (16 * 1024);
#નહીંતર
  // BUFSIZ is intended to be a reasonable default.
  પાછા BUFSIZ;
#endif
}

શૂન્ય raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  જો (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  નહીંતર
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

શૂન્ય raw_ostream::SetBufferAndMode(અક્ષર *BufferStart, size_t Size,
                                   BufferKind Mode) {
  પ્રતિજ્ઞા(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  પ્રતિજ્ઞા(Get_અંક_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  જો (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  પ્રતિજ્ઞા(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*આ, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*આ, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*આ, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*આ, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  પાછા *આ;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*આ, N, HexPrintStyle::Lower);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  જો (C == Colors::RESET)
    resetColor();
  નહીંતર
    changeColor(C);
  પાછા *આ;
}

raw_ostream &raw_ostream::write_uuid(અચલ uuid_t UUID) {
  માટે (પૂર્ણાંક Idx = 0; Idx < 16; ++Idx) {
    *આ << format("%02" PRIX32, UUID[Idx]);
    જો (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *આ << "-";
  }
  પાછા *આ;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        તાર્કિક UseHexEscapes) {
  માટે (unsigned અક્ષર c : Str) {
    પસંદગી (c) {
    સ્થિતિ '\\':
      *આ << '\\' << '\\';
      તોડો;
    સ્થિતિ '\t':
      *આ << '\\' << 't';
      તોડો;
    સ્થિતિ '\n':
      *આ << '\\' << 'n';
      તોડો;
    સ્થિતિ '"':
      *આ << '\\' << '"';
      તોડો;
    મૂળભૂત:
      જો (isPrint(c)) {
        *આ << c;
        તોડો;
      }

      // Write out the escaped representation.
      જો (UseHexEscapes) {
        *આ << '\\' << 'x';
        *આ << hexdigit((c >> 4) & 0xF);
        *આ << hexdigit((c >> 0) & 0xF);
      } નહીંતર {
        // Always use a full 3-character octal escape.
        *આ << '\\';
        *આ << અક્ષર('0' + ((c >> 6) & 7));
        *આ << અક્ષર('0' + ((c >> 3) & 7));
        *આ << અક્ષર('0' + ((c >> 0) & 7));
      }
    }
  }

  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(અચલ શૂન્ય *P) {
  llvm::write_hex(*આ, (uintptr_t)P, HexPrintStyle::PrefixLower);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(દ્વિગુણ N) {
  llvm::write_double(*આ, N, FloatStyle::Exponent);
  પાછા *આ;
}

શૂન્ય raw_ostream::flush_nonempty() {
  પ્રતિજ્ઞા(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::લિખ(unsigned અક્ષર C) {
  // Group exceptional cases into a single branch.
  જો (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    જો (LLVM_UNLIKELY(!OutBufStart)) {
      જો (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<અક્ષર *>(&C), 1);
        પાછા *આ;
      }
      // Set up a buffer and start over.
      SetBuffered();
      પાછા લિખ(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  પાછા *આ;
}

raw_ostream &raw_ostream::લિખ(અચલ અક્ષર *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  જો (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    જો (LLVM_UNLIKELY(!OutBufStart)) {
      જો (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        પાછા *આ;
      }
      // Set up a buffer and start over.
      SetBuffered();
      પાછા લિખ(Ptr, Size);
    }

    size_t અંક_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    જો (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      પ્રતિજ્ઞા(અંક_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % અંક_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      જો (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        પાછા લિખ(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      પાછા *આ;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, અંક_Bytes);
    flush_nonempty();
    પાછા લિખ(Ptr + અંક_Bytes, Size - અંક_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  પાછા *આ;
}

શૂન્ય raw_ostream::copy_to_buffer(અચલ અક્ષર *Ptr, size_t Size) {
  પ્રતિજ્ઞા(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  પસંદગી (Size) {
  સ્થિતિ 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  સ્થિતિ 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  સ્થિતિ 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  સ્થિતિ 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  સ્થિતિ 0: તોડો;
  મૂળભૂત:
    memcpy(OutBufCur, Ptr, Size);
    તોડો;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<પૂર્ણાંક(અક્ષર *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_ડાબું = OutBufEnd - OutBufCur;
  જો (Buffer_Bytes_ડાબું > 3) {
    પૂર્ણાંક N = Snprint(OutBufCur, Buffer_Bytes_ડાબું);
    જો (N < 0)
      પાછા *આ;
    // Common case is that we have plenty of space.
    જો (size_t(N) < Buffer_Bytes_ડાબું) {
      OutBufCur += N;
      પાછા *આ;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<અક્ષર, 128> V;
  માટે (;;) {
    V.resize(Size);
    પૂર્ણાંક N = Snprint(V.data(), Size);
    જો (N < 0)
      પાછા *આ;
    જો (size_t(N) < Size)
      પાછા લિખ(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(અચલ formatv_object_base &Obj) {
  Obj.format(*આ);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(અચલ FormattedString &FS) {
  unsigned ડાબું_Indent = 0;
  unsigned જમણું_Indent = 0;
  અચલ ssize_t Difference = FS.Width - FS.Str.size();
  જો (Difference > 0) {
    પસંદગી (FS.Justify) {
    સ્થિતિ FormattedString::JustifyNone:
      તોડો;
    સ્થિતિ FormattedString::Justify_ડાબું:
      જમણું_Indent = Difference;
      તોડો;
    સ્થિતિ FormattedString::Justify_જમણું:
      ડાબું_Indent = Difference;
      તોડો;
    સ્થિતિ FormattedString::JustifyCenter:
      ડાબું_Indent = Difference / 2;
      જમણું_Indent = Difference - ડાબું_Indent;
      તોડો;
    }
  }
  indent(ડાબું_Indent);
  (*આ) << FS.Str;
  indent(જમણું_Indent);
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(અચલ Formatted_સંખ્યા &FN) {
  જો (FN.Hex) {
    HexPrintStyle Style;
    જો (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    નહીંતર જો (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    નહીંતર જો (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    નહીંતર
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*આ, FN.Hex_કિંમત, Style, FN.Width);
  } નહીંતર {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_કિંમત, 0, IntegerStyle::Integer);
    જો (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*આ) << Buffer;
  }
  પાછા *આ;
}

raw_ostream &raw_ostream::operator<<(અચલ FormattedBytes &FB) {
  જો (FB.Bytes.empty())
    પાછા *આ;

  size_t લીટી_Index = 0;
  auto Bytes = FB.Bytes;
  અચલ size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  જો (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t લીટીઓ = Size / FB.અંક_Per_લીટી;
    uint64_t MaxOffset = *FB.FirstByteOffset + લીટીઓ * FB.અંક_Per_લીટી;
    unsigned Power = 0;
    જો (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned અંક_Byte_Groups =
      alignTo(FB.અંક_Per_લીટી, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.અંક_Per_લીટી * 2 + અંક_Byte_Groups - 1;

  જ્યારે (!Bytes.empty()) {
    indent(FB.IndentLevel);

    જો (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*આ, Offset + લીટી_Index, HPS, OffsetWidth);
      *આ << ": ";
    }

    auto લીટી = Bytes.take_front(FB.અંક_Per_લીટી);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    માટે (size_t I = 0; I < લીટી.size(); ++I, CharsPrinted += 2) {
      જો (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *આ << " ";
      }
      llvm::write_hex(*આ, લીટી[I], HPS, 2);
    }

    જો (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      પ્રતિજ્ઞા(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *આ << "|";

      // Print the ASCII char values for each byte on this line
      માટે (uint8_t Byte : લીટી) {
        જો (isPrint(Byte))
          *આ << static_cast<અક્ષર>(Byte);
        નહીંતર
          *આ << '.';
      }
      *આ << '|';
    }

    Bytes = Bytes.drop_front(લીટી.size());
    લીટી_Index += લીટી.size();
    જો (લીટી_Index < Size)
      *આ << '\n';
  }
  પાછા *આ;
}

template <અક્ષર C>
સ્થિર raw_ostream &write_padding(raw_ostream &OS, unsigned અંક_Chars) {
  સ્થિર અચલ અક્ષર Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  જો (અંક_Chars < std::size(Chars))
    પાછા OS.લિખ(Chars, અંક_Chars);

  જ્યારે (અંક_Chars) {
    unsigned અંક_To_Write = std::min(અંક_Chars, (unsigned)std::size(Chars) - 1);
    OS.લિખ(Chars, અંક_To_Write);
    અંક_Chars -= અંક_To_Write;
  }
  પાછા OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned અંક_Spaces) {
  પાછા write_padding<' '>(*આ, અંક_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned અંક_Zeros) {
  પાછા write_padding<'\0'>(*આ, અંક_Zeros);
}

તાર્કિક raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  જો (!ColorEnabled)
    પાછા ખોટું;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  જો (sys::Process::ColorNeedsFlush() && !is_displayed())
    પાછા ખોટું;

  જો (sys::Process::ColorNeedsFlush())
    flush();

  પાછા સાચું;
}

raw_ostream &raw_ostream::changeColor(ગણના Colors colors, તાર્કિક bold, તાર્કિક bg) {
  જો (!prepare_colors())
    પાછા *આ;

  અચલ અક્ષર *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<અક્ષર>(colors), bold, bg);
  જો (colorcode)
    લિખ(colorcode, strlen(colorcode));
  પાછા *આ;
}

raw_ostream &raw_ostream::resetColor() {
  જો (!prepare_colors())
    પાછા *આ;

  જો (અચલ અક્ષર *colorcode = sys::Process::ResetColor())
    લિખ(colorcode, strlen(colorcode));
  પાછા *આ;
}

raw_ostream &raw_ostream::reverseColor() {
  જો (!prepare_colors())
    પાછા *આ;

  જો (અચલ અક્ષર *colorcode = sys::Process::OutputReverse())
    લિખ(colorcode, strlen(colorcode));
  પાછા *આ;
}

શૂન્ય raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

સ્થિર પૂર્ણાંક getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  પ્રતિજ્ઞા((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  જો (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    પાછા STDOUT_FILENO;
  }

  પૂર્ણાંક FD;
  જો (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  નહીંતર
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  જો (EC)
    પાછા -1;

  પાછા FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), સાચું) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(પૂર્ણાંક fd, તાર્કિક shouldClose, તાર્કિક unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  જો (FD < 0 ) {
    ShouldClose = ખોટું;
    પાછા;
  }

  enable_colors(સાચું);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  જો (FD <= STDERR_FILENO)
    ShouldClose = ખોટું;

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
#નહીંતર
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  જો (!SupportsSeeking)
    pos = 0;
  નહીંતર
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  જો (FD >= 0) {
    flush();
    જો (ShouldClose) {
      જો (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  જો (FD == 2) પાછા;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  જો (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#જો defined(_WIN32)
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
સ્થિર તાર્કિક write_console_impl(પૂર્ણાંક FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  જો (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    પાછા ખોટું;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  જો (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  કરો {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    તાર્કિક Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    જો (!Success)
      પાછા ખોટું;

    WCharsWritten += ActuallyWritten;
  } જ્યારે (WCharsWritten != WideText.size());
  પાછા સાચું;
}
#endif

શૂન્ય raw_fd_ostream::write_impl(અચલ અક્ષર *Ptr, size_t Size) {
  જો (TiedStream)
    TiedStream->flush();

  પ્રતિજ્ઞા(FD >= 0 && "File already closed.");
  pos += Size;

#જો defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  જો (IsWindowsConsole)
    જો (write_console_impl(FD, StringRef(Ptr, Size)))
      પાછા;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#જો defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  કરો {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::લિખ(FD, Ptr, ChunkSize);

    જો (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      જો (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        ચાલુ;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      જો (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      તોડો;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } જ્યારે (Size > 0);
}

શૂન્ય raw_fd_ostream::close() {
  પ્રતિજ્ઞા(ShouldClose);
  ShouldClose = ખોટું;
  flush();
  જો (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  પ્રતિજ્ઞા(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#નહીંતર
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  જો (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  પાછા pos;
}

શૂન્ય raw_fd_ostream::pwrite_impl(અચલ અક્ષર *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  લિખ(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() અચલ {
#જો defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  જો (IsWindowsConsole)
    પાછા 0;
  પાછા raw_ostream::preferred_buffer_size();
#અથવાજો defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  પાછા raw_ostream::preferred_buffer_size();
#નહીંતર
  પ્રતિજ્ઞા(FD >= 0 && "File not yet open!");
  સંરચના stat statbuf;
  જો (fstat(FD, &statbuf) != 0)
    પાછા 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  જો (S_ISCHR(statbuf.st_mode) && is_displayed())
    પાછા 0;
  // Return the preferred block size.
  પાછા statbuf.st_blksize;
#endif
}

તાર્કિક raw_fd_ostream::is_displayed() અચલ {
  પાછા sys::Process::FileDescriptorIsDisplayed(FD);
}

તાર્કિક raw_fd_ostream::has_colors() અચલ {
  જો (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  પાછા *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  જો (!EC)
    પાછા sys::fs::FileLocker(FD);
  પાછા errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration અચલ& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  જો (!EC)
    પાછા sys::fs::FileLocker(FD);
  પાછા errorCodeToError(EC);
}

શૂન્ય raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  સ્થિર std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  પ્રતિજ્ઞા(!EC1);
  (શૂન્ય)EC1;

  સ્થિર raw_fd_ostream S("-", EC, sys::fs::OF_None);
  પ્રતિજ્ઞા(!EC);
  પાછા S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  સ્થિર std::error_code EC = enableAutoConversion(STDERR_FILENO);
  પ્રતિજ્ઞા(!EC);
  (શૂન્ય)EC;

  // Set standard error to be unbuffered.
  સ્થિર raw_fd_ostream S(STDERR_FILENO, ખોટું, સાચું);
  પાછા S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  સ્થિર raw_null_ostream S;
  પાછા S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     સાચું, ખોટું, OStreamKind::OK_FDStream) {
  જો (EC)
    પાછા;

  જો (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(પૂર્ણાંક fd, તાર્કિક shouldClose)
    : raw_fd_ostream(fd, shouldClose, ખોટું, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::પઠ(અક્ષર *Ptr, size_t Size) {
  પ્રતિજ્ઞા(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::પઠ(get_fd(), (શૂન્ય *)Ptr, Size);
  જો (Ret >= 0)
    inc_pos(Ret);
  નહીંતર
    error_detected(errnoAsErrorCode());
  પાછા Ret;
}

તાર્કિક raw_fd_stream::classof(અચલ raw_ostream *OS) {
  પાછા OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

શૂન્ય raw_string_ostream::write_impl(અચલ અક્ષર *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() અચલ { પાછા OS.size(); }

શૂન્ય raw_svector_ostream::write_impl(અચલ અક્ષર *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

શૂન્ય raw_svector_ostream::pwrite_impl(અચલ અક્ષર *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

તાર્કિક raw_svector_ostream::classof(અચલ raw_ostream *OS) {
  પાછા OS->get_kind() == OStreamKind::OK_SVecStream;
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

શૂન્ય raw_null_ostream::write_impl(અચલ અક્ષર *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() અચલ {
  પાછા 0;
}

શૂન્ય raw_null_ostream::pwrite_impl(અચલ અક્ષર *Ptr, size_t Size,
                                   uint64_t Offset) {}

શૂન્ય raw_pwrite_stream::anchor() {}

શૂન્ય buffer_ostream::anchor() {}

શૂન્ય buffer_unique_ostream::anchor() {}

સ્થિર તાર્કિક isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  જો (OutputFileName.equals_insensitive("NUL"))
    પાછા સાચું;
#endif
  પાછા OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  જો (OutputFileName == "-")
    પાછા Write(outs());

  જો (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    પાછા Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  જો (!Temp)
    પાછા createFileError(OutputFileName, Temp.takeError());

#જો defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  જો (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    જો (EC != std::errc::no_such_file_or_directory)
      પાછા createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, ખોટું);

  જો (Error E = Write(Out)) {
    જો (Error DiscardError = Temp->discard())
      પાછા joinErrors(std::move(E), std::move(DiscardError));
    પાછા E;
  }
  Out.flush();

  પાછા Temp->keep(OutputFileName);
}
