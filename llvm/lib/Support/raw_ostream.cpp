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

#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/AutoConvert.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/Duration.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/MathExtras.h"
#अन्तर्भाव "llvm/Support/NativeFormatting.h"
#अन्तर्भाव "llvm/Support/Process.h"
#अन्तर्भाव "llvm/Support/Program.h"
#अन्तर्भाव <algorithm>
#अन्तर्भाव <cerrno>
#अन्तर्भाव <cstdio>
#अन्तर्भाव <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#जइ defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#जइ defined(__CYGWIN__)
#अन्तर्भाव <io.h>
#endif

#जइ defined(_MSC_VER)
#अन्तर्भाव <io.h>
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
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#endif

using नामाकाश llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  प्रतिज्ञा(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  जइ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

size_t raw_ostream::preferred_buffer_size() ध्रुव {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  प्रत्यागम (16 * 1024);
#अण्णहा
  // BUFSIZ is intended to be a reasonable default.
  प्रत्यागम BUFSIZ;
#endif
}

शून्य raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  जइ (size_t Size = preferred_buffer_size())
    SetBufferSize(Size);
  अण्णहा
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

शून्य raw_ostream::SetBufferAndMode(वर्ण *BufferStart, size_t Size,
                                   BufferKind Mode) {
  प्रतिज्ञा(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  प्रतिज्ञा(Get_अङ्कः_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  जइ (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  प्रतिज्ञा(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::operator<<(unsigned long N) {
  write_integer(*एतत्, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(long N) {
  write_integer(*एतत्, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(unsigned long long N) {
  write_integer(*एतत्, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(long long N) {
  write_integer(*एतत्, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::write_hex(unsigned long long N) {
  llvm::write_hex(*एतत्, N, HexPrintStyle::Lower);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(Colors C) {
  जइ (C == Colors::RESET)
    resetColor();
  अण्णहा
    changeColor(C);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::write_uuid(ध्रुव uuid_t UUID) {
  प्रति (पूर्णाङ्क Idx = 0; Idx < 16; ++Idx) {
    *एतत् << format("%02" PRIX32, UUID[Idx]);
    जइ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *एतत् << "-";
  }
  प्रत्यागम *एतत्;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        तार्किक UseHexEscapes) {
  प्रति (unsigned वर्ण c : Str) {
    विकल्प (c) {
    पक्ष '\\':
      *एतत् << '\\' << '\\';
      भङ्ग;
    पक्ष '\t':
      *एतत् << '\\' << 't';
      भङ्ग;
    पक्ष '\n':
      *एतत् << '\\' << 'n';
      भङ्ग;
    पक्ष '"':
      *एतत् << '\\' << '"';
      भङ्ग;
    स्वभाव:
      जइ (isPrint(c)) {
        *एतत् << c;
        भङ्ग;
      }

      // Write out the escaped representation.
      जइ (UseHexEscapes) {
        *एतत् << '\\' << 'x';
        *एतत् << hexdigit((c >> 4) & 0xF);
        *एतत् << hexdigit((c >> 0) & 0xF);
      } अण्णहा {
        // Always use a full 3-character octal escape.
        *एतत् << '\\';
        *एतत् << वर्ण('0' + ((c >> 6) & 7));
        *एतत् << वर्ण('0' + ((c >> 3) & 7));
        *एतत् << वर्ण('0' + ((c >> 0) & 7));
      }
    }
  }

  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(ध्रुव शून्य *P) {
  llvm::write_hex(*एतत्, (uintptr_t)P, HexPrintStyle::PrefixLower);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(द्विगुण N) {
  llvm::write_double(*एतत्, N, FloatStyle::Exponent);
  प्रत्यागम *एतत्;
}

शून्य raw_ostream::flush_nonempty() {
  प्रतिज्ञा(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  size_t Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::लिख(unsigned वर्ण C) {
  // Group exceptional cases into a single branch.
  जइ (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    जइ (LLVM_UNLIKELY(!OutBufStart)) {
      जइ (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<वर्ण *>(&C), 1);
        प्रत्यागम *एतत्;
      }
      // Set up a buffer and start over.
      SetBuffered();
      प्रत्यागम लिख(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::लिख(ध्रुव वर्ण *Ptr, size_t Size) {
  // Group exceptional cases into a single branch.
  जइ (LLVM_UNLIKELY(size_t(OutBufEnd - OutBufCur) < Size)) {
    जइ (LLVM_UNLIKELY(!OutBufStart)) {
      जइ (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        प्रत्यागम *एतत्;
      }
      // Set up a buffer and start over.
      SetBuffered();
      प्रत्यागम लिख(Ptr, Size);
    }

    size_t अङ्कः_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    जइ (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      प्रतिज्ञा(अङ्कः_Bytes != 0 && "undefined behavior");
      size_t BytesToWrite = Size - (Size % अङ्कः_Bytes);
      write_impl(Ptr, BytesToWrite);
      size_t BytesRemaining = Size - BytesToWrite;
      जइ (BytesRemaining > size_t(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        प्रत्यागम लिख(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      प्रत्यागम *एतत्;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, अङ्कः_Bytes);
    flush_nonempty();
    प्रत्यागम लिख(Ptr + अङ्कः_Bytes, Size - अङ्कः_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  प्रत्यागम *एतत्;
}

शून्य raw_ostream::copy_to_buffer(ध्रुव वर्ण *Ptr, size_t Size) {
  प्रतिज्ञा(Size <= size_t(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  विकल्प (Size) {
  पक्ष 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  पक्ष 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  पक्ष 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  पक्ष 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  पक्ष 0: भङ्ग;
  स्वभाव:
    memcpy(OutBufCur, Ptr, Size);
    भङ्ग;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::operator<<(function_ref<पूर्णाङ्क(वर्ण *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t Size = 128;
  size_t Buffer_Bytes_वामः = OutBufEnd - OutBufCur;
  जइ (Buffer_Bytes_वामः > 3) {
    पूर्णाङ्क N = Snprint(OutBufCur, Buffer_Bytes_वामः);
    जइ (N < 0)
      प्रत्यागम *एतत्;
    // Common case is that we have plenty of space.
    जइ (size_t(N) < Buffer_Bytes_वामः) {
      OutBufCur += N;
      प्रत्यागम *एतत्;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<वर्ण, 128> V;
  प्रति (;;) {
    V.resize(Size);
    पूर्णाङ्क N = Snprint(V.data(), Size);
    जइ (N < 0)
      प्रत्यागम *एतत्;
    जइ (size_t(N) < Size)
      प्रत्यागम लिख(V.data(), N);
    Size = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::operator<<(ध्रुव formatv_object_base &Obj) {
  Obj.format(*एतत्);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(ध्रुव FormattedString &FS) {
  unsigned वामः_Indent = 0;
  unsigned दक्षिणः_Indent = 0;
  ध्रुव ssize_t Difference = FS.Width - FS.Str.size();
  जइ (Difference > 0) {
    विकल्प (FS.Justify) {
    पक्ष FormattedString::JustifyNone:
      भङ्ग;
    पक्ष FormattedString::Justify_वामः:
      दक्षिणः_Indent = Difference;
      भङ्ग;
    पक्ष FormattedString::Justify_दक्षिणः:
      वामः_Indent = Difference;
      भङ्ग;
    पक्ष FormattedString::JustifyCenter:
      वामः_Indent = Difference / 2;
      दक्षिणः_Indent = Difference - वामः_Indent;
      भङ्ग;
    }
  }
  indent(वामः_Indent);
  (*एतत्) << FS.Str;
  indent(दक्षिणः_Indent);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(ध्रुव Formatted_सङ्ख्या &FN) {
  जइ (FN.Hex) {
    HexPrintStyle Style;
    जइ (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    अण्णहा जइ (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    अण्णहा जइ (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    अण्णहा
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*एतत्, FN.Hex_मानम्, Style, FN.Width);
  } अण्णहा {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_मानम्, 0, IntegerStyle::Integer);
    जइ (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*एतत्) << Buffer;
  }
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::operator<<(ध्रुव FormattedBytes &FB) {
  जइ (FB.Bytes.empty())
    प्रत्यागम *एतत्;

  size_t पङ्क्तिः_Index = 0;
  auto Bytes = FB.Bytes;
  ध्रुव size_t Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  जइ (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t पङ्क्तयः = Size / FB.अङ्कः_Per_पङ्क्तिः;
    uint64_t MaxOffset = *FB.FirstByteOffset + पङ्क्तयः * FB.अङ्कः_Per_पङ्क्तिः;
    unsigned Power = 0;
    जइ (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  unsigned अङ्कः_Byte_Groups =
      alignTo(FB.अङ्कः_Per_पङ्क्तिः, FB.ByteGroupSize) / FB.ByteGroupSize;
  unsigned BlockCharWidth = FB.अङ्कः_Per_पङ्क्तिः * 2 + अङ्कः_Byte_Groups - 1;

  जाव (!Bytes.empty()) {
    indent(FB.IndentLevel);

    जइ (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*एतत्, Offset + पङ्क्तिः_Index, HPS, OffsetWidth);
      *एतत् << ": ";
    }

    auto पङ्क्तिः = Bytes.take_front(FB.अङ्कः_Per_पङ्क्तिः);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    प्रति (size_t I = 0; I < पङ्क्तिः.size(); ++I, CharsPrinted += 2) {
      जइ (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *एतत् << " ";
      }
      llvm::write_hex(*एतत्, पङ्क्तिः[I], HPS, 2);
    }

    जइ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      प्रतिज्ञा(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *एतत् << "|";

      // Print the ASCII char values for each byte on this line
      प्रति (uint8_t Byte : पङ्क्तिः) {
        जइ (isPrint(Byte))
          *एतत् << static_cast<वर्ण>(Byte);
        अण्णहा
          *एतत् << '.';
      }
      *एतत् << '|';
    }

    Bytes = Bytes.drop_front(पङ्क्तिः.size());
    पङ्क्तिः_Index += पङ्क्तिः.size();
    जइ (पङ्क्तिः_Index < Size)
      *एतत् << '\n';
  }
  प्रत्यागम *एतत्;
}

template <वर्ण C>
स्थिर raw_ostream &write_padding(raw_ostream &OS, unsigned अङ्कः_Chars) {
  स्थिर ध्रुव वर्ण Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  जइ (अङ्कः_Chars < std::size(Chars))
    प्रत्यागम OS.लिख(Chars, अङ्कः_Chars);

  जाव (अङ्कः_Chars) {
    unsigned अङ्कः_To_Write = std::min(अङ्कः_Chars, (unsigned)std::size(Chars) - 1);
    OS.लिख(Chars, अङ्कः_To_Write);
    अङ्कः_Chars -= अङ्कः_To_Write;
  }
  प्रत्यागम OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(unsigned अङ्कः_Spaces) {
  प्रत्यागम write_padding<' '>(*एतत्, अङ्कः_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(unsigned अङ्कः_Zeros) {
  प्रत्यागम write_padding<'\0'>(*एतत्, अङ्कः_Zeros);
}

तार्किक raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  जइ (!ColorEnabled)
    प्रत्यागम मिच्छ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  जइ (sys::Process::ColorNeedsFlush() && !is_displayed())
    प्रत्यागम मिच्छ;

  जइ (sys::Process::ColorNeedsFlush())
    flush();

  प्रत्यागम सच्च;
}

raw_ostream &raw_ostream::changeColor(गणना Colors colors, तार्किक bold, तार्किक bg) {
  जइ (!prepare_colors())
    प्रत्यागम *एतत्;

  ध्रुव वर्ण *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<वर्ण>(colors), bold, bg);
  जइ (colorcode)
    लिख(colorcode, strlen(colorcode));
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::resetColor() {
  जइ (!prepare_colors())
    प्रत्यागम *एतत्;

  जइ (ध्रुव वर्ण *colorcode = sys::Process::ResetColor())
    लिख(colorcode, strlen(colorcode));
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::reverseColor() {
  जइ (!prepare_colors())
    प्रत्यागम *एतत्;

  जइ (ध्रुव वर्ण *colorcode = sys::Process::OutputReverse())
    लिख(colorcode, strlen(colorcode));
  प्रत्यागम *एतत्;
}

शून्य raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

स्थिर पूर्णाङ्क getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  जइ (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    प्रत्यागम STDOUT_FILENO;
  }

  पूर्णाङ्क FD;
  जइ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  अण्णहा
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  जइ (EC)
    प्रत्यागम -1;

  प्रत्यागम FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), सच्च) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(पूर्णाङ्क fd, तार्किक shouldClose, तार्किक unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  जइ (FD < 0 ) {
    ShouldClose = मिच्छ;
    प्रत्यागम;
  }

  enable_colors(सच्च);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  जइ (FD <= STDERR_FILENO)
    ShouldClose = मिच्छ;

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
#अण्णहा
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  जइ (!SupportsSeeking)
    pos = 0;
  अण्णहा
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  जइ (FD >= 0) {
    flush();
    जइ (ShouldClose) {
      जइ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  जइ (FD == 2) प्रत्यागम;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  जइ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#जइ defined(_WIN32)
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
स्थिर तार्किक write_console_impl(पूर्णाङ्क FD, StringRef Data) {
  SmallVector<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  जइ (auto EC = sys::windows::UTF8ToUTF16(Data, WideText))
    प्रत्यागम मिच्छ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t MaxWriteSize = WideText.size();
  जइ (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  size_t WCharsWritten = 0;
  करेहि {
    size_t WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    तार्किक Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    जइ (!Success)
      प्रत्यागम मिच्छ;

    WCharsWritten += ActuallyWritten;
  } जाव (WCharsWritten != WideText.size());
  प्रत्यागम सच्च;
}
#endif

शून्य raw_fd_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t Size) {
  जइ (TiedStream)
    TiedStream->flush();

  प्रतिज्ञा(FD >= 0 && "File already closed.");
  pos += Size;

#जइ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  जइ (IsWindowsConsole)
    जइ (write_console_impl(FD, StringRef(Ptr, Size)))
      प्रत्यागम;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t MaxWriteSize = INT32_MAX;

#जइ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  करेहि {
    size_t ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::लिख(FD, Ptr, ChunkSize);

    जइ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      जइ (errno == EINTR || errno == EAGAIN
#ifdef EWOULDBLOCK
          || errno == EWOULDBLOCK
#endif
          )
        अनुवर्त;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      जइ (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      भङ्ग;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } जाव (Size > 0);
}

शून्य raw_fd_ostream::close() {
  प्रतिज्ञा(ShouldClose);
  ShouldClose = मिच्छ;
  flush();
  जइ (auto EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  प्रतिज्ञा(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#अण्णहा
  pos = ::lseek(FD, off, SEEK_SET);
#endif
  जइ (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  प्रत्यागम pos;
}

शून्य raw_fd_ostream::pwrite_impl(ध्रुव वर्ण *Ptr, size_t Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  लिख(Ptr, Size);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_buffer_size() ध्रुव {
#जइ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  जइ (IsWindowsConsole)
    प्रत्यागम 0;
  प्रत्यागम raw_ostream::preferred_buffer_size();
#नोचेद्यदि defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  प्रत्यागम raw_ostream::preferred_buffer_size();
#अण्णहा
  प्रतिज्ञा(FD >= 0 && "File not yet open!");
  संरचना stat statbuf;
  जइ (fstat(FD, &statbuf) != 0)
    प्रत्यागम 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  जइ (S_ISCHR(statbuf.st_mode) && is_displayed())
    प्रत्यागम 0;
  // Return the preferred block size.
  प्रत्यागम statbuf.st_blksize;
#endif
}

तार्किक raw_fd_ostream::is_displayed() ध्रुव {
  प्रत्यागम sys::Process::FileDescriptorIsDisplayed(FD);
}

तार्किक raw_fd_ostream::has_colors() ध्रुव {
  जइ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  प्रत्यागम *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  जइ (!EC)
    प्रत्यागम sys::fs::FileLocker(FD);
  प्रत्यागम errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration ध्रुव& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  जइ (!EC)
    प्रत्यागम sys::fs::FileLocker(FD);
  प्रत्यागम errorCodeToError(EC);
}

शून्य raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  स्थिर std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  प्रतिज्ञा(!EC1);
  (शून्य)EC1;

  स्थिर raw_fd_ostream S("-", EC, sys::fs::OF_None);
  प्रतिज्ञा(!EC);
  प्रत्यागम S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  स्थिर std::error_code EC = enableAutoConversion(STDERR_FILENO);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  // Set standard error to be unbuffered.
  स्थिर raw_fd_ostream S(STDERR_FILENO, मिच्छ, सच्च);
  प्रत्यागम S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  स्थिर raw_null_ostream S;
  प्रत्यागम S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     सच्च, मिच्छ, OStreamKind::OK_FDStream) {
  जइ (EC)
    प्रत्यागम;

  जइ (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(पूर्णाङ्क fd, तार्किक shouldClose)
    : raw_fd_ostream(fd, shouldClose, मिच्छ, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::पठ(वर्ण *Ptr, size_t Size) {
  प्रतिज्ञा(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::पठ(get_fd(), (शून्य *)Ptr, Size);
  जइ (Ret >= 0)
    inc_pos(Ret);
  अण्णहा
    error_detected(errnoAsErrorCode());
  प्रत्यागम Ret;
}

तार्किक raw_fd_stream::classof(ध्रुव raw_ostream *OS) {
  प्रत्यागम OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

शून्य raw_string_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ध्रुव { प्रत्यागम OS.size(); }

शून्य raw_svector_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t Size) {
  OS.append(Ptr, Ptr + Size);
}

शून्य raw_svector_ostream::pwrite_impl(ध्रुव वर्ण *Ptr, size_t Size,
                                      uint64_t Offset) {
  memcpy(OS.data() + Offset, Ptr, Size);
}

तार्किक raw_svector_ostream::classof(ध्रुव raw_ostream *OS) {
  प्रत्यागम OS->get_kind() == OStreamKind::OK_SVecStream;
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

शून्य raw_null_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t Size) {
}

uint64_t raw_null_ostream::current_pos() ध्रुव {
  प्रत्यागम 0;
}

शून्य raw_null_ostream::pwrite_impl(ध्रुव वर्ण *Ptr, size_t Size,
                                   uint64_t Offset) {}

शून्य raw_pwrite_stream::anchor() {}

शून्य buffer_ostream::anchor() {}

शून्य buffer_unique_ostream::anchor() {}

स्थिर तार्किक isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  जइ (OutputFileName.equals_insensitive("NUL"))
    प्रत्यागम सच्च;
#endif
  प्रत्यागम OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  जइ (OutputFileName == "-")
    प्रत्यागम Write(outs());

  जइ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    प्रत्यागम Write(Out);
  }

  unsigned Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  जइ (!Temp)
    प्रत्यागम createFileError(OutputFileName, Temp.takeError());

#जइ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  जइ (auto EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    जइ (EC != std::errc::no_such_file_or_directory)
      प्रत्यागम createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, मिच्छ);

  जइ (Error E = Write(Out)) {
    जइ (Error DiscardError = Temp->discard())
      प्रत्यागम joinErrors(std::move(E), std::move(DiscardError));
    प्रत्यागम E;
  }
  Out.flush();

  प्रत्यागम Temp->keep(OutputFileName);
}
