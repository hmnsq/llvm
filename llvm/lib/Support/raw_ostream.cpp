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

#जर defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#जर defined(__CYGWIN__)
#अन्तर्भाव <io.h>
#endif

#जर defined(_MSC_VER)
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

  जर (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

माप_प्रकार raw_ostream::preferred_buffer_size() स्थिरांक {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  परत (16 * 1024);
#अन्यथा
  // BUFSIZ is intended to be a reasonable default.
  परत बफ_मान;
#endif
}

शून्य raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  जर (माप_प्रकार Size = preferred_buffer_size())
    SetBufferSize(Size);
  अन्यथा
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

शून्य raw_ostream::SetBufferAndMode(अक्षर *BufferStart, माप_प्रकार Size,
                                   BufferKind Mode) {
  प्रतिज्ञा(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  प्रतिज्ञा(Get_अंक_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  जर (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  प्रतिज्ञा(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::चालक<<(अचिन्हित दीर्घ N) {
  write_integer(*हे, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(दीर्घ N) {
  write_integer(*हे, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(अचिन्हित दीर्घ दीर्घ N) {
  write_integer(*हे, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(दीर्घ दीर्घ N) {
  write_integer(*हे, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  परत *हे;
}

raw_ostream &raw_ostream::write_hex(अचिन्हित दीर्घ दीर्घ N) {
  llvm::write_hex(*हे, N, HexPrintStyle::Lower);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(Colors C) {
  जर (C == Colors::RESET)
    resetColor();
  अन्यथा
    changeColor(C);
  परत *हे;
}

raw_ostream &raw_ostream::write_uuid(स्थिरांक uuid_t UUID) {
  साठी (पूर्णांक Idx = 0; Idx < 16; ++Idx) {
    *हे << format("%02" PRIX32, UUID[Idx]);
    जर (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *हे << "-";
  }
  परत *हे;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        तार्किक UseHexEscapes) {
  साठी (अचिन्हित अक्षर c : Str) {
    निवड (c) {
    स्थिती '\\':
      *हे << '\\' << '\\';
      थांबा;
    स्थिती '\t':
      *हे << '\\' << 't';
      थांबा;
    स्थिती '\n':
      *हे << '\\' << 'n';
      थांबा;
    स्थिती '"':
      *हे << '\\' << '"';
      थांबा;
    पूर्वनिर्धारित:
      जर (isPrint(c)) {
        *हे << c;
        थांबा;
      }

      // Write out the escaped representation.
      जर (UseHexEscapes) {
        *हे << '\\' << 'x';
        *हे << hexdigit((c >> 4) & 0xF);
        *हे << hexdigit((c >> 0) & 0xF);
      } अन्यथा {
        // Always use a full 3-character octal escape.
        *हे << '\\';
        *हे << अक्षर('0' + ((c >> 6) & 7));
        *हे << अक्षर('0' + ((c >> 3) & 7));
        *हे << अक्षर('0' + ((c >> 0) & 7));
      }
    }
  }

  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(स्थिरांक शून्य *P) {
  llvm::write_hex(*हे, (uintptr_t)P, HexPrintStyle::PrefixLower);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(द्विगुण N) {
  llvm::write_double(*हे, N, FloatStyle::Exponent);
  परत *हे;
}

शून्य raw_ostream::flush_nonempty() {
  प्रतिज्ञा(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  माप_प्रकार Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::लिख(अचिन्हित अक्षर C) {
  // Group exceptional cases into a single branch.
  जर (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    जर (LLVM_UNLIKELY(!OutBufStart)) {
      जर (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<अक्षर *>(&C), 1);
        परत *हे;
      }
      // Set up a buffer and start over.
      SetBuffered();
      परत लिख(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  परत *हे;
}

raw_ostream &raw_ostream::लिख(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
  // Group exceptional cases into a single branch.
  जर (LLVM_UNLIKELY(माप_प्रकार(OutBufEnd - OutBufCur) < Size)) {
    जर (LLVM_UNLIKELY(!OutBufStart)) {
      जर (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        परत *हे;
      }
      // Set up a buffer and start over.
      SetBuffered();
      परत लिख(Ptr, Size);
    }

    माप_प्रकार अंक_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    जर (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      प्रतिज्ञा(अंक_Bytes != 0 && "undefined behavior");
      माप_प्रकार BytesToWrite = Size - (Size % अंक_Bytes);
      write_impl(Ptr, BytesToWrite);
      माप_प्रकार BytesRemaining = Size - BytesToWrite;
      जर (BytesRemaining > माप_प्रकार(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        परत लिख(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      परत *हे;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, अंक_Bytes);
    flush_nonempty();
    परत लिख(Ptr + अंक_Bytes, Size - अंक_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  परत *हे;
}

शून्य raw_ostream::copy_to_buffer(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
  प्रतिज्ञा(Size <= माप_प्रकार(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  निवड (Size) {
  स्थिती 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  स्थिती 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  स्थिती 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  स्थिती 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  स्थिती 0: थांबा;
  पूर्वनिर्धारित:
    स_नकल(OutBufCur, Ptr, Size);
    थांबा;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::चालक<<(function_ref<पूर्णांक(अक्षर *, माप_प्रकार)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  माप_प्रकार Size = 128;
  माप_प्रकार Buffer_Bytes_डावा = OutBufEnd - OutBufCur;
  जर (Buffer_Bytes_डावा > 3) {
    पूर्णांक N = Snprint(OutBufCur, Buffer_Bytes_डावा);
    जर (N < 0)
      परत *हे;
    // Common case is that we have plenty of space.
    जर (माप_प्रकार(N) < Buffer_Bytes_डावा) {
      OutBufCur += N;
      परत *हे;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = माप_प्रकार(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<अक्षर, 128> V;
  साठी (;;) {
    V.resize(Size);
    पूर्णांक N = Snprint(V.data(), Size);
    जर (N < 0)
      परत *हे;
    जर (माप_प्रकार(N) < Size)
      परत लिख(V.data(), N);
    Size = माप_प्रकार(N) + 1;
  }
}

raw_ostream &raw_ostream::चालक<<(स्थिरांक formatv_object_base &Obj) {
  Obj.format(*हे);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(स्थिरांक FormattedString &FS) {
  अचिन्हित डावा_Indent = 0;
  अचिन्हित उजवा_Indent = 0;
  स्थिरांक ssize_t Difference = FS.Width - FS.Str.size();
  जर (Difference > 0) {
    निवड (FS.Justify) {
    स्थिती FormattedString::JustifyNone:
      थांबा;
    स्थिती FormattedString::Justify_डावा:
      उजवा_Indent = Difference;
      थांबा;
    स्थिती FormattedString::Justify_उजवा:
      डावा_Indent = Difference;
      थांबा;
    स्थिती FormattedString::JustifyCenter:
      डावा_Indent = Difference / 2;
      उजवा_Indent = Difference - डावा_Indent;
      थांबा;
    }
  }
  indent(डावा_Indent);
  (*हे) << FS.Str;
  indent(उजवा_Indent);
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(स्थिरांक Formatted_संख्या &FN) {
  जर (FN.Hex) {
    HexPrintStyle Style;
    जर (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    अन्यथा जर (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    अन्यथा जर (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    अन्यथा
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*हे, FN.Hex_मूल्य, Style, FN.Width);
  } अन्यथा {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_मूल्य, 0, IntegerStyle::Integer);
    जर (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*हे) << Buffer;
  }
  परत *हे;
}

raw_ostream &raw_ostream::चालक<<(स्थिरांक FormattedBytes &FB) {
  जर (FB.Bytes.empty())
    परत *हे;

  माप_प्रकार ओळ_Index = 0;
  स्वतः Bytes = FB.Bytes;
  स्थिरांक माप_प्रकार Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  जर (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    माप_प्रकार ओळी = Size / FB.अंक_Per_ओळ;
    uint64_t MaxOffset = *FB.FirstByteOffset + ओळी * FB.अंक_Per_ओळ;
    अचिन्हित Power = 0;
    जर (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  अचिन्हित अंक_Byte_Groups =
      alignTo(FB.अंक_Per_ओळ, FB.ByteGroupSize) / FB.ByteGroupSize;
  अचिन्हित BlockCharWidth = FB.अंक_Per_ओळ * 2 + अंक_Byte_Groups - 1;

  पर्यंत (!Bytes.empty()) {
    indent(FB.IndentLevel);

    जर (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*हे, Offset + ओळ_Index, HPS, OffsetWidth);
      *हे << ": ";
    }

    स्वतः ओळ = Bytes.take_front(FB.अंक_Per_ओळ);

    माप_प्रकार CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    साठी (माप_प्रकार I = 0; I < ओळ.size(); ++I, CharsPrinted += 2) {
      जर (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *हे << " ";
      }
      llvm::write_hex(*हे, ओळ[I], HPS, 2);
    }

    जर (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      प्रतिज्ञा(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *हे << "|";

      // Print the ASCII char values for each byte on this line
      साठी (uint8_t Byte : ओळ) {
        जर (isPrint(Byte))
          *हे << static_cast<अक्षर>(Byte);
        अन्यथा
          *हे << '.';
      }
      *हे << '|';
    }

    Bytes = Bytes.drop_front(ओळ.size());
    ओळ_Index += ओळ.size();
    जर (ओळ_Index < Size)
      *हे << '\n';
  }
  परत *हे;
}

ढाँचा <अक्षर C>
स्थिर raw_ostream &write_padding(raw_ostream &OS, अचिन्हित अंक_Chars) {
  स्थिर स्थिरांक अक्षर Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  जर (अंक_Chars < std::size(Chars))
    परत OS.लिख(Chars, अंक_Chars);

  पर्यंत (अंक_Chars) {
    अचिन्हित अंक_To_Write = std::min(अंक_Chars, (अचिन्हित)std::size(Chars) - 1);
    OS.लिख(Chars, अंक_To_Write);
    अंक_Chars -= अंक_To_Write;
  }
  परत OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(अचिन्हित अंक_Spaces) {
  परत write_padding<' '>(*हे, अंक_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(अचिन्हित अंक_Zeros) {
  परत write_padding<'\0'>(*हे, अंक_Zeros);
}

तार्किक raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  जर (!ColorEnabled)
    परत असत्य;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  जर (sys::Process::ColorNeedsFlush() && !is_displayed())
    परत असत्य;

  जर (sys::Process::ColorNeedsFlush())
    flush();

  परत सत्य;
}

raw_ostream &raw_ostream::changeColor(गणना Colors colors, तार्किक bold, तार्किक bg) {
  जर (!prepare_colors())
    परत *हे;

  स्थिरांक अक्षर *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<अक्षर>(colors), bold, bg);
  जर (colorcode)
    लिख(colorcode, म_माप(colorcode));
  परत *हे;
}

raw_ostream &raw_ostream::resetColor() {
  जर (!prepare_colors())
    परत *हे;

  जर (स्थिरांक अक्षर *colorcode = sys::Process::ResetColor())
    लिख(colorcode, म_माप(colorcode));
  परत *हे;
}

raw_ostream &raw_ostream::reverseColor() {
  जर (!prepare_colors())
    परत *हे;

  जर (स्थिरांक अक्षर *colorcode = sys::Process::OutputReverse())
    लिख(colorcode, म_माप(colorcode));
  परत *हे;
}

शून्य raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

स्थिर पूर्णांक getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  जर (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    परत STDOUT_FILENO;
  }

  पूर्णांक FD;
  जर (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  अन्यथा
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  जर (EC)
    परत -1;

  परत FD;
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
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), सत्य) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(पूर्णांक fd, तार्किक shouldClose, तार्किक unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  जर (FD < 0 ) {
    ShouldClose = असत्य;
    परत;
  }

  enable_colors(सत्य);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  जर (FD <= STDERR_FILENO)
    ShouldClose = असत्य;

#ifdef _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#endif

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, प्रस्तुत_से);
  sys::fs::file_status Status;
  std::error_code EC = status(FD, Status);
  IsRegularFile = Status.type() == sys::fs::file_type::regular_file;
#ifdef _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#अन्यथा
  SupportsSeeking = !EC && loc != (off_t)-1;
#endif
  जर (!SupportsSeeking)
    pos = 0;
  अन्यथा
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  जर (FD >= 0) {
    flush();
    जर (ShouldClose) {
      जर (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  जर (FD == 2) परत;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  जर (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#जर defined(_WIN32)
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
स्थिर तार्किक write_console_impl(पूर्णांक FD, StringRef Data) {
  SmallVector<ब_अक्षर_प्रकार, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  जर (स्वतः EC = sys::windows::UTF8ToUTF16(Data, WideText))
    परत असत्य;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  माप_प्रकार MaxWriteSize = WideText.size();
  जर (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  माप_प्रकार WCharsWritten = 0;
  करा {
    माप_प्रकार WCharsToWrite =
        std::min(MaxWriteSize, WideText.size() - WCharsWritten);
    DWORD ActuallyWritten;
    तार्किक Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    जर (!Success)
      परत असत्य;

    WCharsWritten += ActuallyWritten;
  } पर्यंत (WCharsWritten != WideText.size());
  परत सत्य;
}
#endif

शून्य raw_fd_ostream::write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
  जर (TiedStream)
    TiedStream->flush();

  प्रतिज्ञा(FD >= 0 && "File already closed.");
  pos += Size;

#जर defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  जर (IsWindowsConsole)
    जर (write_console_impl(FD, StringRef(Ptr, Size)))
      परत;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  माप_प्रकार MaxWriteSize = INT32_MAX;

#जर defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  करा {
    माप_प्रकार ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::लिख(FD, Ptr, ChunkSize);

    जर (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      जर (त्रुटि_सं == EINTR || त्रुटि_सं == EAGAIN
#ifdef EWOULDBLOCK
          || त्रुटि_सं == EWOULDBLOCK
#endif
          )
        सुरूठेवा;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      जर (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && त्रुटि_सं == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        त्रुटि_सं = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      थांबा;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } पर्यंत (Size > 0);
}

शून्य raw_fd_ostream::बंद() {
  प्रतिज्ञा(ShouldClose);
  ShouldClose = असत्य;
  flush();
  जर (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  प्रतिज्ञा(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ifdef _WIN32
  pos = ::_lseeki64(FD, off, शुरू_से);
#अन्यथा
  pos = ::lseek(FD, off, शुरू_से);
#endif
  जर (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  परत pos;
}

शून्य raw_fd_ostream::pwrite_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  लिख(Ptr, Size);
  seek(Pos);
}

माप_प्रकार raw_fd_ostream::preferred_buffer_size() स्थिरांक {
#जर defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  जर (IsWindowsConsole)
    परत 0;
  परत raw_ostream::preferred_buffer_size();
#किंवाजर defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  परत raw_ostream::preferred_buffer_size();
#अन्यथा
  प्रतिज्ञा(FD >= 0 && "File not yet open!");
  संरचना stat statbuf;
  जर (ख_स्थिति(FD, &statbuf) != 0)
    परत 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  जर (S_ISCHR(statbuf.st_mode) && is_displayed())
    परत 0;
  // Return the preferred block size.
  परत statbuf.st_blksize;
#endif
}

तार्किक raw_fd_ostream::is_displayed() स्थिरांक {
  परत sys::Process::FileDescriptorIsDisplayed(FD);
}

तार्किक raw_fd_ostream::has_colors() स्थिरांक {
  जर (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  परत *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  जर (!EC)
    परत sys::fs::FileLocker(FD);
  परत errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration स्थिरांक& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  जर (!EC)
    परत sys::fs::FileLocker(FD);
  परत errorCodeToError(EC);
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
  परत S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  स्थिर std::error_code EC = enableAutoConversion(STDERR_FILENO);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  // Set standard error to be unbuffered.
  स्थिर raw_fd_ostream S(STDERR_FILENO, असत्य, सत्य);
  परत S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  स्थिर raw_null_ostream S;
  परत S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     सत्य, असत्य, OStreamKind::OK_FDStream) {
  जर (EC)
    परत;

  जर (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(पूर्णांक fd, तार्किक shouldClose)
    : raw_fd_ostream(fd, shouldClose, असत्य, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::पठ(अक्षर *Ptr, माप_प्रकार Size) {
  प्रतिज्ञा(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::पठ(get_fd(), (शून्य *)Ptr, Size);
  जर (Ret >= 0)
    inc_pos(Ret);
  अन्यथा
    error_detected(errnoAsErrorCode());
  परत Ret;
}

तार्किक raw_fd_stream::classof(स्थिरांक raw_ostream *OS) {
  परत OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

शून्य raw_string_ostream::write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() स्थिरांक { परत OS.size(); }

शून्य raw_svector_ostream::write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
  OS.append(Ptr, Ptr + Size);
}

शून्य raw_svector_ostream::pwrite_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size,
                                      uint64_t Offset) {
  स_नकल(OS.data() + Offset, Ptr, Size);
}

तार्किक raw_svector_ostream::classof(स्थिरांक raw_ostream *OS) {
  परत OS->get_kind() == OStreamKind::OK_SVecStream;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#ifndef न_संशोधन
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#endif
}

शून्य raw_null_ostream::write_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size) {
}

uint64_t raw_null_ostream::current_pos() स्थिरांक {
  परत 0;
}

शून्य raw_null_ostream::pwrite_impl(स्थिरांक अक्षर *Ptr, माप_प्रकार Size,
                                   uint64_t Offset) {}

शून्य raw_pwrite_stream::anchor() {}

शून्य buffer_ostream::anchor() {}

शून्य buffer_unique_ostream::anchor() {}

स्थिर तार्किक isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  जर (OutputFileName.equals_insensitive("NUL"))
    परत सत्य;
#endif
  परत OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  जर (OutputFileName == "-")
    परत Write(outs());

  जर (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    परत Write(Out);
  }

  अचिन्हित Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  जर (!Temp)
    परत createFileError(OutputFileName, Temp.takeError());

#जर defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  जर (स्वतः EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    जर (EC != std::errc::no_such_file_or_directory)
      परत createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, असत्य);

  जर (Error E = Write(Out)) {
    जर (Error DiscardError = Temp->discard())
      परत joinErrors(std::move(E), std::move(DiscardError));
    परत E;
  }
  Out.flush();

  परत Temp->keep(OutputFileName);
}
