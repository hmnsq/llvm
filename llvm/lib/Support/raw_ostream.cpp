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

#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/AutoConvert.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/Duration.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/FormatVariadic.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/MathExtras.h"
#அந்தர்பாவ "llvm/Support/NativeFormatting.h"
#அந்தர்பாவ "llvm/Support/Process.h"
#அந்தர்பாவ "llvm/Support/Program.h"
#அந்தர்பாவ <algorithm>
#அந்தர்பாவ <cerrno>
#அந்தர்பாவ <cstdio>
#அந்தர்பாவ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#எனில் defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#எனில் defined(__CYGWIN__)
#அந்தர்பாவ <io.h>
#endif

#எனில் defined(_MSC_VER)
#அந்தர்பாவ <io.h>
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
#அந்தர்பாவ "llvm/Support/ConvertUTF.h"
#அந்தர்பாவ "llvm/Support/Signals.h"
#அந்தர்பாவ "llvm/Support/Windows/WindowsSupport.h"
#endif

using நாமாகாஶ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  ப்ரதிஜ்ஞா(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  எனில் (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() மாறிலி {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  திரும்பு (16 * 1024);
#இல்லையெனில்
  // BUFSIZ is intended to be a reasonable default.
  திரும்பு BUFSIZ;
#endif
}

வெறுமை raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  எனில் (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  இல்லையெனில்
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

வெறுமை raw_ostream::SetBufferAndMode(எழுத்து *BufferStart, size_t Size,
                                   BufferKind Mode) {
  ப்ரதிஜ்ஞா(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  ப்ரதிஜ்ஞா(Get_இலக்கம்_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  எனில் (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  ப்ரதிஜ்ஞா(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*இது, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*இது, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*இது, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*இது, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*இது, N, HexPrintStyle::Lower);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  எனில் (C == Colors::RESET)
    resetColor();
  இல்லையெனில்
    changeColor(C);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::write_uuid(மாறிலி uuid_t UUID) {
  க்கு (முழு Idx = 0; Idx < 16; ++Idx) {
    *இது << format("%02" PRIX32, UUID[Idx]);
    எனில் (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *இது << "-";
  }
  திரும்பு *இது;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        தருக்க UseHexEscapes) {
  க்கு (unsigned எழுத்து c : Str) {
    தேர்வு (c) {
    நிலை '\\':
      *இது << '\\' << '\\';
      நிறுத்து;
    நிலை '\t':
      *இது << '\\' << 't';
      நிறுத்து;
    நிலை '\n':
      *இது << '\\' << 'n';
      நிறுத்து;
    நிலை '"':
      *இது << '\\' << '"';
      நிறுத்து;
    இயல்பு:
      எனில் (isPrint(c)) {
        *இது << c;
        நிறுத்து;
      }

      // Write out the escaped representation.
      எனில் (UseHexEscapes) {
        *இது << '\\' << 'x';
        *இது << hexdigit((c >> 4) & 0xF);
        *இது << hexdigit((c >> 0) & 0xF);
      } இல்லையெனில் {
        // Always use a full 3-character octal escape.
        *இது << '\\';
        *இது << எழுத்து('0' + ((c >> 6) & 7));
        *இது << எழுத்து('0' + ((c >> 3) & 7));
        *இது << எழுத்து('0' + ((c >> 0) & 7));
      }
    }
  }

  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(மாறிலி வெறுமை *P) {
  llvm::write_hex(*இது, (uintptr_t)P, HexPrintStyle::PrefixLower);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(இரட்டை N) {
  llvm::write_double(*இது, N, FloatStyle::Exponent);
  திரும்பு *இது;
}

வெறுமை raw_ostream::flush_nonempty() {
  ப்ரதிஜ்ஞா(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::லிக(unsigned எழுத்து C) {
  // Group exceptional cases into a single branch.
  எனில் (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    எனில் (LLVM_UNLIKELY(!OutBufStart)) {
      எனில் (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<எழுத்து *>(&C), 1);
        திரும்பு *இது;
      }
      // Set up a buffer and start over.
      SetBuffered();
      திரும்பு லிக(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  திரும்பு *இது;
}

raw_ostream &raw_ostream::லிக(மாறிலி எழுத்து *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  எனில் (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    எனில் (LLVM_UNLIKELY(!OutBufStart)) {
      எனில் (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        திரும்பு *இது;
      }
      // Set up a buffer and start over.
      SetBuffered();
      திரும்பு லிக(Ptr, Size);
    }

    size_t இலக்கம்_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    எனில் (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      ப்ரதிஜ்ஞா(இலக்கம்_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % இலக்கம்_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      எனில் (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        திரும்பு லிக(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      திரும்பு *இது;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, இலக்கம்_Bytes);
    flush_nonempty();
    திரும்பு லிக(Ptr + இலக்கம்_Bytes, Size - இலக்கம்_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  திரும்பு *இது;
}

வெறுமை raw_ostream::copy_to_buffer(மாறிலி எழுத்து *Ptr, size_t Size) {
  ப்ரதிஜ்ஞா(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  தேர்வு (Size) {
  நிலை 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  நிலை 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  நிலை 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  நிலை 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  நிலை 0: நிறுத்து;
  இயல்பு:
    memcpy(OutBufCur, Ptr, Size);
    நிறுத்து;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<முழு(எழுத்து *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_இடது = OutBufEnd - OutBufCur;
  எனில் (Buffer_Bytes_இடது > 3) {
    முழு N = Snprint(OutBufCur, Buffer_Bytes_இடது);
    எனில் (N < 0)
      திரும்பு *இது;
    // Common case is that we have plenty of space.
    எனில் (size_t(N) < Buffer_Bytes_இடது) {
      OutBufCur += N;
      திரும்பு *இது;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<எழுத்து, 128> V;
  க்கு (;;) {
    V.resize(Size);
    முழு N = Snprint(V.data(), Size);
    எனில் (N < 0)
      திரும்பு *இது;
    எனில் (size_t(N) < Size)
      திரும்பு லிக(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(மாறிலி formatv_object_base &Obj) {
  Obj.format(*இது);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(மாறிலி FormattedString &FS) {
  unsigned இடது_Indent = 0;
  unsigned வலது_Indent = 0;
  மாறிலி ssize_t Difference = FS.Width - FS.Str.size();
  எனில் (Difference > 0) {
    தேர்வு (FS.Justify) {
    நிலை FormattedString::JustifyNone:
      நிறுத்து;
    நிலை FormattedString::Justify_இடது:
      வலது_Indent = Difference;
      நிறுத்து;
    நிலை FormattedString::Justify_வலது:
      இடது_Indent = Difference;
      நிறுத்து;
    நிலை FormattedString::JustifyCenter:
      இடது_Indent = Difference / 2;
      வலது_Indent = Difference - இடது_Indent;
      நிறுத்து;
    }
  }
  indent(இடது_Indent);
  (*இது) << FS.Str;
  indent(வலது_Indent);
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(மாறிலி Formatted_எண் &FN) {
  எனில் (FN.Hex) {
    HexPrintStyle Style;
    எனில் (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    இல்லையெனில் எனில் (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    இல்லையெனில் எனில் (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    இல்லையெனில்
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*இது, FN.Hex_மதிப்பு, Style, FN.Width);
  } இல்லையெனில் {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_மதிப்பு, 0, IntegerStyle::Integer);
    எனில் (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*இது) << Buffer;
  }
  திரும்பு *இது;
}

raw_ostream &raw_ostream::operator<<(மாறிலி FormattedBytes &FB) {
  எனில் (FB.Bytes.empty())
    திரும்பு *இது;

  size_t வரி_Index = 0;
  auto Bytes = FB.Bytes;
  மாறிலி size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  எனில் (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t வரிகள் = Size / FB.இலக்கம்_Per_வரி;
    uint64_t MaxOffset = *FB.FirstByteOffset + வரிகள் * FB.இலக்கம்_Per_வரி;
    unsigned Power = 0;
    எனில் (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned இலக்கம்_Byte_Groups =
      alignTo(FB.இலக்கம்_Per_வரி, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.இலக்கம்_Per_வரி * 2 + இலக்கம்_Byte_Groups - 1;

  போது (!Bytes.empty()) {
    indent(FB.IndentLevel);

    எனில் (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*இது, Offset + வரி_Index, HPS, OffsetWidth);
      *இது << ": ";
    }

    auto வரி = Bytes.take_front(FB.இலக்கம்_Per_வரி);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    க்கு (size_t I = 0; I < வரி.size(); ++I, CharsPrinted += 2) {
      எனில் (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *இது << " ";
      }
      llvm::write_hex(*இது, வரி[I], HPS, 2);
    }

    எனில் (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      ப்ரதிஜ்ஞா(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *இது << "|";

      // Print the ASCII char values for each byte on this line
      க்கு (uint8_t Byte : வரி) {
        எனில் (isPrint(Byte))
          *இது << static_cast<எழுத்து>(Byte);
        இல்லையெனில்
          *இது << '.';
      }
      *இது << '|';
    }

    Bytes = Bytes.drop_front(வரி.size());
    வரி_Index += வரி.size();
    எனில் (வரி_Index < Size)
      *இது << '\n';
  }
  திரும்பு *இது;
}

template <எழுத்து C>
நிலையான raw_ostream &write_padding(raw_ostream &OS, unsigned இலக்கம்_Chars) {
  நிலையான மாறிலி எழுத்து Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  எனில் (இலக்கம்_Chars < std::size(Chars))
    திரும்பு OS.லிக(Chars, இலக்கம்_Chars);

  போது (இலக்கம்_Chars) {
    unsigned இலக்கம்_To_Write = std::min(இலக்கம்_Chars, (unsigned)std::size(Chars) - 1);
    OS.லிக(Chars, இலக்கம்_To_Write);
    இலக்கம்_Chars -= இலக்கம்_To_Write;
  }
  திரும்பு OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned இலக்கம்_Spaces) {
  திரும்பு write_padding<' '>(*இது, இலக்கம்_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned இலக்கம்_Zeros) {
  திரும்பு write_padding<'\0'>(*இது, இலக்கம்_Zeros);
}

தருக்க raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  எனில் (!ColorEnabled)
    திரும்பு பொய்;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  எனில் (sys::Process::ColorNeedsFlush() && !is_displayed())
    திரும்பு பொய்;

  எனில் (sys::Process::ColorNeedsFlush())
    flush();

  திரும்பு உண்மை;
}

raw_ostream &raw_ostream::changeColor(எண்ணிக்கை Colors colors, தருக்க bold, தருக்க bg) {
  எனில் (!prepare_colors())
    திரும்பு *இது;

  மாறிலி எழுத்து *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<எழுத்து>(colors), bold, bg);
  எனில் (colorcode)
    லிக(colorcode, strlen(colorcode));
  திரும்பு *இது;
}

raw_ostream &raw_ostream::resetColor() {
  எனில் (!prepare_colors())
    திரும்பு *இது;

  எனில் (மாறிலி எழுத்து *colorcode = sys::Process::ResetColor())
    லிக(colorcode, strlen(colorcode));
  திரும்பு *இது;
}

raw_ostream &raw_ostream::reverseColor() {
  எனில் (!prepare_colors())
    திரும்பு *இது;

  எனில் (மாறிலி எழுத்து *colorcode = sys::Process::OutputReverse())
    லிக(colorcode, strlen(colorcode));
  திரும்பு *இது;
}

வெறுமை raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

நிலையான முழு getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  ப்ரதிஜ்ஞா((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  எனில் (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    திரும்பு STDOUT_FILENO;
  }

  முழு FD;
  எனில் (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  இல்லையெனில்
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  எனில் (EC)
    திரும்பு -1;

  திரும்பு FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), உண்மை) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(முழு fd, தருக்க shouldClose, தருக்க unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  எனில் (FD < 0 ) {
    ShouldClose = பொய்;
    திரும்பு;
  }

  enable_colors(உண்மை);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  எனில் (FD <= STDERR_FILENO)
    ShouldClose = பொய்;

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
#இல்லையெனில்
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  எனில் (!SupportsSeeking)
    pos = 0;
  இல்லையெனில்
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  எனில் (FD >= 0) {
    flush();
    எனில் (ShouldClose) {
      எனில் (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  எனில் (FD == 2) திரும்பு;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  எனில் (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#எனில் defined(_WIN32)
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
நிலையான தருக்க write_console_impl(முழு FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  எனில் (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    திரும்பு பொய்;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  எனில் (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  செய் {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    தருக்க Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    எனில் (!Success)
      திரும்பு பொய்;

    WCharsWritten += ActuallyWritten;
  } போது (WCharsWritten != WideText.size());
  திரும்பு உண்மை;
}
#endif

வெறுமை raw_fd_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
  எனில் (TiedStream)
    TiedStream->flush();

  ப்ரதிஜ்ஞா(FD >= 0 && "File already closed.");
  pos += Size;

#எனில் defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  எனில் (IsWindowsConsole)
    எனில் (write_console_impl(FD, StringRef(Ptr, Size)))
      திரும்பு;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#எனில் defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  செய் {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::லிக(FD, Ptr, ChunkSize);

    எனில் (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      எனில் (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        தொடர்;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      எனில் (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      நிறுத்து;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } போது (Size > 0);
}

வெறுமை raw_fd_ostream::close() {
  ப்ரதிஜ்ஞா(ShouldClose);
  ShouldClose = பொய்;
  flush();
  எனில் (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  ப்ரதிஜ்ஞா(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#இல்லையெனில்
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  எனில் (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  திரும்பு pos;
}

வெறுமை raw_fd_ostream::pwrite_impl(மாறிலி எழுத்து *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  லிக(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() மாறிலி {
#எனில் defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  எனில் (IsWindowsConsole)
    திரும்பு 0;
  திரும்பு raw_ostream::preferred_buffer_size();
#அல்லதெனில் defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  திரும்பு raw_ostream::preferred_buffer_size();
#இல்லையெனில்
  ப்ரதிஜ்ஞா(FD >= 0 && "File not yet open!");
  கட்டமைப்பு stat statbuf;
  எனில் (fstat(FD, &statbuf) != 0)
    திரும்பு 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  எனில் (S_ISCHR(statbuf.st_mode) && is_displayed())
    திரும்பு 0;
  // Return the preferred block size.
  திரும்பு statbuf.st_blksize;
#endif
}

தருக்க raw_fd_ostream::is_displayed() மாறிலி {
  திரும்பு sys::Process::FileDescriptorIsDisplayed(FD);
}

தருக்க raw_fd_ostream::has_colors() மாறிலி {
  எனில் (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  திரும்பு *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  எனில் (!EC)
    திரும்பு sys::fs::FileLocker(FD);
  திரும்பு errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration மாறிலி& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  எனில் (!EC)
    திரும்பு sys::fs::FileLocker(FD);
  திரும்பு errorCodeToError(EC);
}

வெறுமை raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  நிலையான std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  ப்ரதிஜ்ஞா(!EC1);
  (வெறுமை)EC1;

  நிலையான raw_fd_ostream S("-", EC, sys::fs::OF_None);
  ப்ரதிஜ்ஞா(!EC);
  திரும்பு S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  நிலையான std::error_code EC = enableAutoConversion(STDERR_FILENO);
  ப்ரதிஜ்ஞா(!EC);
  (வெறுமை)EC;

  // Set standard error to be unbuffered.
  நிலையான raw_fd_ostream S(STDERR_FILENO, பொய், உண்மை);
  திரும்பு S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  நிலையான raw_null_ostream S;
  திரும்பு S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     உண்மை, பொய், OStreamKind::OK_FDStream) {
  எனில் (EC)
    திரும்பு;

  எனில் (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(முழு fd, தருக்க shouldClose)
    : raw_fd_ostream(fd, shouldClose, பொய், OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::பட(எழுத்து *Ptr, size_t Size) {
  ப்ரதிஜ்ஞா(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::பட(get_fd(), (வெறுமை *)Ptr, Size);
  எனில் (Ret >= 0)
    inc_pos(Ret);
  இல்லையெனில்
    error_detected(errnoAsErrorCode());
  திரும்பு Ret;
}

தருக்க raw_fd_stream::classof(மாறிலி raw_ostream *OS) {
  திரும்பு OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

வெறுமை raw_string_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() மாறிலி { திரும்பு OS.size(); }

வெறுமை raw_svector_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

வெறுமை raw_svector_ostream::pwrite_impl(மாறிலி எழுத்து *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

தருக்க raw_svector_ostream::classof(மாறிலி raw_ostream *OS) {
  திரும்பு OS->get_kind() == OStreamKind::OK_SVecStream;
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

வெறுமை raw_null_ostream::write_impl(மாறிலி எழுத்து *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() மாறிலி {
  திரும்பு 0;
}

வெறுமை raw_null_ostream::pwrite_impl(மாறிலி எழுத்து *Ptr, size_t Size,
                                   uint64_t Offset) {}

வெறுமை raw_pwrite_stream::anchor() {}

வெறுமை buffer_ostream::anchor() {}

வெறுமை buffer_unique_ostream::anchor() {}

நிலையான தருக்க isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  எனில் (OutputFileName.equals_insensitive("NUL"))
    திரும்பு உண்மை;
#endif
  திரும்பு OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  எனில் (OutputFileName == "-")
    திரும்பு Write(outs());

  எனில் (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    திரும்பு Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  எனில் (!Temp)
    திரும்பு createFileError(OutputFileName, Temp.takeError());

#எனில் defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  எனில் (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    எனில் (EC != std::errc::no_such_file_or_directory)
      திரும்பு createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, பொய்);

  எனில் (Error E = Write(Out)) {
    எனில் (Error DiscardError = Temp->discard())
      திரும்பு joinErrors(std::move(E), std::move(DiscardError));
    திரும்பு E;
  }
  Out.flush();

  திரும்பு Temp->keep(OutputFileName);
}
