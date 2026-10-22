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

#यदि defined(HAVE_UNISTD_H)
# include <unistd.h>
#endif

#यदि defined(__CYGWIN__)
#अन्तर्भाव <io.h>
#endif

#यदि defined(_MSC_VER)
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

  यदि (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
}

माप_प्रकार raw_ostream::preferred_buffer_size() अचर {
#ifdef _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  फर्काऊ (16 * 1024);
#अन्यथा
  // BUFSIZ is intended to be a reasonable default.
  फर्काऊ बफ_मान;
#endif
}

शून्य raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  यदि (माप_प्रकार Size = preferred_buffer_size())
    SetBufferSize(Size);
  अन्यथा
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

शून्य raw_ostream::SetBufferAndMode(वर्ण *BufferStart, माप_प्रकार Size,
                                   BufferKind Mode) {
  प्रतिज्ञा(((Mode == BufferKind::Unbuffered && !BufferStart && Size == 0) ||
          (Mode != BufferKind::Unbuffered && BufferStart && Size != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  प्रतिज्ञा(Get_अङ्क_Bytes_In_Buffer() == 0 && "Current buffer is non-empty!");

  यदि (BufferMode == BufferKind::InternalBuffer)
    delete [] OutBufStart;
  OutBufStart = BufferStart;
  OutBufEnd = OutBufStart+Size;
  OutBufCur = OutBufStart;
  BufferMode = Mode;

  प्रतिज्ञा(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::चालक<<(अचिन्हित दीर्घ N) {
  write_integer(*एतत्, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(दीर्घ N) {
  write_integer(*एतत्, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(अचिन्हित दीर्घ दीर्घ N) {
  write_integer(*एतत्, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(दीर्घ दीर्घ N) {
  write_integer(*एतत्, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::write_hex(अचिन्हित दीर्घ दीर्घ N) {
  llvm::write_hex(*एतत्, N, HexPrintStyle::Lower);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(Colors C) {
  यदि (C == Colors::RESET)
    resetColor();
  अन्यथा
    changeColor(C);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::write_uuid(अचर uuid_t UUID) {
  लागि (पूर्णाङ्क Idx = 0; Idx < 16; ++Idx) {
    *एतत् << format("%02" PRIX32, UUID[Idx]);
    यदि (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *एतत् << "-";
  }
  फर्काऊ *एतत्;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        तार्किक UseHexEscapes) {
  लागि (अचिन्हित वर्ण c : Str) {
    विकल्प (c) {
    पक्ष '\\':
      *एतत् << '\\' << '\\';
      रोक;
    पक्ष '\t':
      *एतत् << '\\' << 't';
      रोक;
    पक्ष '\n':
      *एतत् << '\\' << 'n';
      रोक;
    पक्ष '"':
      *एतत् << '\\' << '"';
      रोक;
    स्वभाव:
      यदि (isPrint(c)) {
        *एतत् << c;
        रोक;
      }

      // Write out the escaped representation.
      यदि (UseHexEscapes) {
        *एतत् << '\\' << 'x';
        *एतत् << hexdigit((c >> 4) & 0xF);
        *एतत् << hexdigit((c >> 0) & 0xF);
      } अन्यथा {
        // Always use a full 3-character octal escape.
        *एतत् << '\\';
        *एतत् << वर्ण('0' + ((c >> 6) & 7));
        *एतत् << वर्ण('0' + ((c >> 3) & 7));
        *एतत् << वर्ण('0' + ((c >> 0) & 7));
      }
    }
  }

  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(अचर शून्य *P) {
  llvm::write_hex(*एतत्, (uintptr_t)P, HexPrintStyle::PrefixLower);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(द्विगुण N) {
  llvm::write_double(*एतत्, N, FloatStyle::Exponent);
  फर्काऊ *एतत्;
}

शून्य raw_ostream::flush_nonempty() {
  प्रतिज्ञा(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  माप_प्रकार Length = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, Length);
}

raw_ostream &raw_ostream::लिख(अचिन्हित वर्ण C) {
  // Group exceptional cases into a single branch.
  यदि (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    यदि (LLVM_UNLIKELY(!OutBufStart)) {
      यदि (BufferMode == BufferKind::Unbuffered) {
        write_impl(reinterpret_cast<वर्ण *>(&C), 1);
        फर्काऊ *एतत्;
      }
      // Set up a buffer and start over.
      SetBuffered();
      फर्काऊ लिख(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::लिख(अचर वर्ण *Ptr, माप_प्रकार Size) {
  // Group exceptional cases into a single branch.
  यदि (LLVM_UNLIKELY(माप_प्रकार(OutBufEnd - OutBufCur) < Size)) {
    यदि (LLVM_UNLIKELY(!OutBufStart)) {
      यदि (BufferMode == BufferKind::Unbuffered) {
        write_impl(Ptr, Size);
        फर्काऊ *एतत्;
      }
      // Set up a buffer and start over.
      SetBuffered();
      फर्काऊ लिख(Ptr, Size);
    }

    माप_प्रकार अङ्क_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    यदि (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      प्रतिज्ञा(अङ्क_Bytes != 0 && "undefined behavior");
      माप_प्रकार BytesToWrite = Size - (Size % अङ्क_Bytes);
      write_impl(Ptr, BytesToWrite);
      माप_प्रकार BytesRemaining = Size - BytesToWrite;
      यदि (BytesRemaining > माप_प्रकार(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        फर्काऊ लिख(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_buffer(Ptr + BytesToWrite, BytesRemaining);
      फर्काऊ *एतत्;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_buffer(Ptr, अङ्क_Bytes);
    flush_nonempty();
    फर्काऊ लिख(Ptr + अङ्क_Bytes, Size - अङ्क_Bytes);
  }

  copy_to_buffer(Ptr, Size);

  फर्काऊ *एतत्;
}

शून्य raw_ostream::copy_to_buffer(अचर वर्ण *Ptr, माप_प्रकार Size) {
  प्रतिज्ञा(Size <= माप_प्रकार(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  विकल्प (Size) {
  पक्ष 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  पक्ष 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  पक्ष 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  पक्ष 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  पक्ष 0: रोक;
  स्वभाव:
    स_नकल(OutBufCur, Ptr, Size);
    रोक;
  }

  OutBufCur += Size;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::चालक<<(function_ref<पूर्णाङ्क(वर्ण *, माप_प्रकार)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  माप_प्रकार Size = 128;
  माप_प्रकार Buffer_Bytes_बायाँ = OutBufEnd - OutBufCur;
  यदि (Buffer_Bytes_बायाँ > 3) {
    पूर्णाङ्क N = Snprint(OutBufCur, Buffer_Bytes_बायाँ);
    यदि (N < 0)
      फर्काऊ *एतत्;
    // Common case is that we have plenty of space.
    यदि (माप_प्रकार(N) < Buffer_Bytes_बायाँ) {
      OutBufCur += N;
      फर्काऊ *एतत्;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    Size = माप_प्रकार(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<वर्ण, 128> V;
  लागि (;;) {
    V.resize(Size);
    पूर्णाङ्क N = Snprint(V.data(), Size);
    यदि (N < 0)
      फर्काऊ *एतत्;
    यदि (माप_प्रकार(N) < Size)
      फर्काऊ लिख(V.data(), N);
    Size = माप_प्रकार(N) + 1;
  }
}

raw_ostream &raw_ostream::चालक<<(अचर formatv_object_base &Obj) {
  Obj.format(*एतत्);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(अचर FormattedString &FS) {
  अचिन्हित बायाँ_Indent = 0;
  अचिन्हित दायाँ_Indent = 0;
  अचर ssize_t Difference = FS.Width - FS.Str.size();
  यदि (Difference > 0) {
    विकल्प (FS.Justify) {
    पक्ष FormattedString::JustifyNone:
      रोक;
    पक्ष FormattedString::Justify_बायाँ:
      दायाँ_Indent = Difference;
      रोक;
    पक्ष FormattedString::Justify_दायाँ:
      बायाँ_Indent = Difference;
      रोक;
    पक्ष FormattedString::JustifyCenter:
      बायाँ_Indent = Difference / 2;
      दायाँ_Indent = Difference - बायाँ_Indent;
      रोक;
    }
  }
  indent(बायाँ_Indent);
  (*एतत्) << FS.Str;
  indent(दायाँ_Indent);
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(अचर Formatted_सङ्ख्या &FN) {
  यदि (FN.Hex) {
    HexPrintStyle Style;
    यदि (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    अन्यथा यदि (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    अन्यथा यदि (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    अन्यथा
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*एतत्, FN.Hex_मान, Style, FN.Width);
  } अन्यथा {
    llvm::SmallString<16> Buffer;
    llvm::raw_svector_ostream Stream(Buffer);
    llvm::write_integer(Stream, FN.Dec_मान, 0, IntegerStyle::Integer);
    यदि (Buffer.size() < FN.Width)
      indent(FN.Width - Buffer.size());
    (*एतत्) << Buffer;
  }
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::चालक<<(अचर FormattedBytes &FB) {
  यदि (FB.Bytes.empty())
    फर्काऊ *एतत्;

  माप_प्रकार पङ्क्ति_Index = 0;
  स्वतः Bytes = FB.Bytes;
  अचर माप_प्रकार Size = Bytes.size();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  यदि (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    माप_प्रकार पङ्क्तिहरू = Size / FB.अङ्क_Per_पङ्क्ति;
    uint64_t MaxOffset = *FB.FirstByteOffset + पङ्क्तिहरू * FB.अङ्क_Per_पङ्क्ति;
    अचिन्हित Power = 0;
    यदि (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  अचिन्हित अङ्क_Byte_Groups =
      alignTo(FB.अङ्क_Per_पङ्क्ति, FB.ByteGroupSize) / FB.ByteGroupSize;
  अचिन्हित BlockCharWidth = FB.अङ्क_Per_पङ्क्ति * 2 + अङ्क_Byte_Groups - 1;

  जबसम्म (!Bytes.empty()) {
    indent(FB.IndentLevel);

    यदि (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*एतत्, Offset + पङ्क्ति_Index, HPS, OffsetWidth);
      *एतत् << ": ";
    }

    स्वतः पङ्क्ति = Bytes.take_front(FB.अङ्क_Per_पङ्क्ति);

    माप_प्रकार CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    लागि (माप_प्रकार I = 0; I < पङ्क्ति.size(); ++I, CharsPrinted += 2) {
      यदि (I && (I % FB.ByteGroupSize) == 0) {
        ++CharsPrinted;
        *एतत् << " ";
      }
      llvm::write_hex(*एतत्, पङ्क्ति[I], HPS, 2);
    }

    यदि (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      प्रतिज्ञा(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *एतत् << "|";

      // Print the ASCII char values for each byte on this line
      लागि (uint8_t Byte : पङ्क्ति) {
        यदि (isPrint(Byte))
          *एतत् << static_cast<वर्ण>(Byte);
        अन्यथा
          *एतत् << '.';
      }
      *एतत् << '|';
    }

    Bytes = Bytes.drop_front(पङ्क्ति.size());
    पङ्क्ति_Index += पङ्क्ति.size();
    यदि (पङ्क्ति_Index < Size)
      *एतत् << '\n';
  }
  फर्काऊ *एतत्;
}

ढाँचा <वर्ण C>
स्थिर raw_ostream &write_padding(raw_ostream &OS, अचिन्हित अङ्क_Chars) {
  स्थिर अचर वर्ण Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  यदि (अङ्क_Chars < std::size(Chars))
    फर्काऊ OS.लिख(Chars, अङ्क_Chars);

  जबसम्म (अङ्क_Chars) {
    अचिन्हित अङ्क_To_Write = std::min(अङ्क_Chars, (अचिन्हित)std::size(Chars) - 1);
    OS.लिख(Chars, अङ्क_To_Write);
    अङ्क_Chars -= अङ्क_To_Write;
  }
  फर्काऊ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(अचिन्हित अङ्क_Spaces) {
  फर्काऊ write_padding<' '>(*एतत्, अङ्क_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(अचिन्हित अङ्क_Zeros) {
  फर्काऊ write_padding<'\0'>(*एतत्, अङ्क_Zeros);
}

तार्किक raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  यदि (!ColorEnabled)
    फर्काऊ असत्य;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  यदि (sys::Process::ColorNeedsFlush() && !is_displayed())
    फर्काऊ असत्य;

  यदि (sys::Process::ColorNeedsFlush())
    flush();

  फर्काऊ सत्य;
}

raw_ostream &raw_ostream::changeColor(गणना Colors colors, तार्किक bold, तार्किक bg) {
  यदि (!prepare_colors())
    फर्काऊ *एतत्;

  अचर वर्ण *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<वर्ण>(colors), bold, bg);
  यदि (colorcode)
    लिख(colorcode, म_माप(colorcode));
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::resetColor() {
  यदि (!prepare_colors())
    फर्काऊ *एतत्;

  यदि (अचर वर्ण *colorcode = sys::Process::ResetColor())
    लिख(colorcode, म_माप(colorcode));
  फर्काऊ *एतत्;
}

raw_ostream &raw_ostream::reverseColor() {
  यदि (!prepare_colors())
    फर्काऊ *एतत्;

  यदि (अचर वर्ण *colorcode = sys::Process::OutputReverse())
    लिख(colorcode, म_माप(colorcode));
  फर्काऊ *एतत्;
}

शून्य raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

स्थिर पूर्णाङ्क getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  यदि (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    फर्काऊ STDOUT_FILENO;
  }

  पूर्णाङ्क FD;
  यदि (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  अन्यथा
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  यदि (EC)
    फर्काऊ -1;

  फर्काऊ FD;
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
raw_fd_ostream::raw_fd_ostream(पूर्णाङ्क fd, तार्किक shouldClose, तार्किक unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  यदि (FD < 0 ) {
    ShouldClose = असत्य;
    फर्काऊ;
  }

  enable_colors(सत्य);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  यदि (FD <= STDERR_FILENO)
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
  यदि (!SupportsSeeking)
    pos = 0;
  अन्यथा
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  यदि (FD >= 0) {
    flush();
    यदि (ShouldClose) {
      यदि (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ifdef __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  यदि (FD == 2) फर्काऊ;
#endif

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  यदि (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#यदि defined(_WIN32)
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
  SmallVector<ब_अक्षर_प्रकार, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  यदि (स्वतः EC = sys::windows::UTF8ToUTF16(Data, WideText))
    फर्काऊ असत्य;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  माप_प्रकार MaxWriteSize = WideText.size();
  यदि (!RunningWindows8OrGreater())
    MaxWriteSize = 32767;

  माप_प्रकार WCharsWritten = 0;
  गर {
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
    यदि (!Success)
      फर्काऊ असत्य;

    WCharsWritten += ActuallyWritten;
  } जबसम्म (WCharsWritten != WideText.size());
  फर्काऊ सत्य;
}
#endif

शून्य raw_fd_ostream::write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) {
  यदि (TiedStream)
    TiedStream->flush();

  प्रतिज्ञा(FD >= 0 && "File already closed.");
  pos += Size;

#यदि defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  यदि (IsWindowsConsole)
    यदि (write_console_impl(FD, StringRef(Ptr, Size)))
      फर्काऊ;
#endif

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  माप_प्रकार MaxWriteSize = INT32_MAX;

#यदि defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  MaxWriteSize = 1024 * 1024 * 1024;
#endif

  गर {
    माप_प्रकार ChunkSize = std::min(Size, MaxWriteSize);
    ssize_t ret = ::लिख(FD, Ptr, ChunkSize);

    यदि (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      यदि (त्रुटि_सं == EINTR || त्रुटि_सं == EAGAIN
#ifdef EWOULDBLOCK
          || त्रुटि_सं == EWOULDBLOCK
#endif
          )
        जारी;

#ifdef _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      यदि (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_DATA && त्रुटि_सं == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        त्रुटि_सं = EPIPE;
      }
#endif
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      रोक;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    Size -= ret;
  } जबसम्म (Size > 0);
}

शून्य raw_fd_ostream::बंद() {
  प्रतिज्ञा(ShouldClose);
  ShouldClose = असत्य;
  flush();
  यदि (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
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
  यदि (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  फर्काऊ pos;
}

शून्य raw_fd_ostream::pwrite_impl(अचर वर्ण *Ptr, माप_प्रकार Size,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  लिख(Ptr, Size);
  seek(Pos);
}

माप_प्रकार raw_fd_ostream::preferred_buffer_size() अचर {
#यदि defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  यदि (IsWindowsConsole)
    फर्काऊ 0;
  फर्काऊ raw_ostream::preferred_buffer_size();
#अथवायदि defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  फर्काऊ raw_ostream::preferred_buffer_size();
#अन्यथा
  प्रतिज्ञा(FD >= 0 && "File not yet open!");
  संरचना stat statbuf;
  यदि (ख_स्थिति(FD, &statbuf) != 0)
    फर्काऊ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  यदि (S_ISCHR(statbuf.st_mode) && is_displayed())
    फर्काऊ 0;
  // Return the preferred block size.
  फर्काऊ statbuf.st_blksize;
#endif
}

तार्किक raw_fd_ostream::is_displayed() अचर {
  फर्काऊ sys::Process::FileDescriptorIsDisplayed(FD);
}

तार्किक raw_fd_ostream::has_colors() अचर {
  यदि (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  फर्काऊ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  यदि (!EC)
    फर्काऊ sys::fs::FileLocker(FD);
  फर्काऊ errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration अचर& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  यदि (!EC)
    फर्काऊ sys::fs::FileLocker(FD);
  फर्काऊ errorCodeToError(EC);
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
  फर्काऊ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  स्थिर std::error_code EC = enableAutoConversion(STDERR_FILENO);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  // Set standard error to be unbuffered.
  स्थिर raw_fd_ostream S(STDERR_FILENO, असत्य, सत्य);
  फर्काऊ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  स्थिर raw_null_ostream S;
  फर्काऊ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     सत्य, असत्य, OStreamKind::OK_FDStream) {
  यदि (EC)
    फर्काऊ;

  यदि (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(पूर्णाङ्क fd, तार्किक shouldClose)
    : raw_fd_ostream(fd, shouldClose, असत्य, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::पठ(वर्ण *Ptr, माप_प्रकार Size) {
  प्रतिज्ञा(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::पठ(get_fd(), (शून्य *)Ptr, Size);
  यदि (Ret >= 0)
    inc_pos(Ret);
  अन्यथा
    error_detected(errnoAsErrorCode());
  फर्काऊ Ret;
}

तार्किक raw_fd_stream::classof(अचर raw_ostream *OS) {
  फर्काऊ OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

शून्य raw_string_ostream::write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) {
  OS.append(Ptr, Size);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() अचर { फर्काऊ OS.size(); }

शून्य raw_svector_ostream::write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) {
  OS.append(Ptr, Ptr + Size);
}

शून्य raw_svector_ostream::pwrite_impl(अचर वर्ण *Ptr, माप_प्रकार Size,
                                      uint64_t Offset) {
  स_नकल(OS.data() + Offset, Ptr, Size);
}

तार्किक raw_svector_ostream::classof(अचर raw_ostream *OS) {
  फर्काऊ OS->get_kind() == OStreamKind::OK_SVecStream;
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

शून्य raw_null_ostream::write_impl(अचर वर्ण *Ptr, माप_प्रकार Size) {
}

uint64_t raw_null_ostream::current_pos() अचर {
  फर्काऊ 0;
}

शून्य raw_null_ostream::pwrite_impl(अचर वर्ण *Ptr, माप_प्रकार Size,
                                   uint64_t Offset) {}

शून्य raw_pwrite_stream::anchor() {}

शून्य buffer_ostream::anchor() {}

शून्य buffer_unique_ostream::anchor() {}

स्थिर तार्किक isNullDeviceName(StringRef OutputFileName) {
#ifdef _WIN32
  यदि (OutputFileName.equals_insensitive("NUL"))
    फर्काऊ सत्य;
#endif
  फर्काऊ OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  यदि (OutputFileName == "-")
    फर्काऊ Write(outs());

  यदि (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    फर्काऊ Write(Out);
  }

  अचिन्हित Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::TempFile> Temp =
      sys::fs::TempFile::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  यदि (!Temp)
    फर्काऊ createFileError(OutputFileName, Temp.takeError());

#यदि defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  यदि (स्वतः EC = llvm::copyFileTagAttributes(OutputFileName.str(), Temp->FD)) {
    यदि (EC != std::errc::no_such_file_or_directory)
      फर्काऊ createFileError(OutputFileName, EC);
  }
#endif

  raw_fd_ostream Out(Temp->FD, असत्य);

  यदि (Error E = Write(Out)) {
    यदि (Error DiscardError = Temp->discard())
      फर्काऊ joinErrors(std::move(E), std::move(DiscardError));
    फर्काऊ E;
  }
  Out.flush();

  फर्काऊ Temp->keep(OutputFileName);
}
