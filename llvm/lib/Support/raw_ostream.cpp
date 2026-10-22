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

#समावेश "llvm/Support/raw_ostream.h"
#समावेश "llvm/ADT/StringExtras.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/AutoConvert.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/Duration.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/FormatVariadic.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/MathExtras.h"
#समावेश "llvm/Support/NativeFormatting.h"
#समावेश "llvm/Support/Process.h"
#समावेश "llvm/Support/Program.h"
#समावेश <algorithm>
#समावेश <cerrno>
#समावेश <cstdio>
#समावेश <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#अगर defined(HAVE_UNISTD_H)
# include <unistd.h>
#पूर्ण_अगर

#अगर defined(__CYGWIN__)
#समावेश <io.h>
#पूर्ण_अगर

#अगर defined(_MSC_VER)
#समावेश <io.h>
#अगर_अघोषित STDIN_FILENO
# define STDIN_FILENO 0
#पूर्ण_अगर
#अगर_अघोषित STDOUT_FILENO
# define STDOUT_FILENO 1
#पूर्ण_अगर
#अगर_अघोषित STDERR_FILENO
# define STDERR_FILENO 2
#पूर्ण_अगर
#पूर्ण_अगर

#अगर_घोषित _WIN32
#समावेश "llvm/Support/ConvertUTF.h"
#समावेश "llvm/Support/Signals.h"
#समावेश "llvm/Support/Windows/WindowsSupport.h"
#पूर्ण_अगर

using namespace llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  निश्चित(OutBufCur == OutBufStart &&
         "raw_ostream destructor called with non-empty buffer!");

  अगर (बफर_Mode == बफर_Kind::Internal_बफर)
    मिटाओ [] OutBufStart;
}

माप_प्रकार raw_ostream::preferred_बफर_आकार() स्थिर {
#अगर_घोषित _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  वापस (16 * 1024);
#अन्यथा
  // BUFSIZ is intended to be a reasonable default.
  वापस बफ_मान;
#पूर्ण_अगर
}

व्योम raw_ostream::SetBuffered() {
  // Ask the subclass to determine an appropriate buffer size.
  अगर (माप_प्रकार आकार = preferred_बफर_आकार())
    Set_बफर_आकार(आकार);
  अन्यथा
    // It may return 0, meaning this stream should be unbuffered.
    SetUnbuffered();
}

व्योम raw_ostream::Set_बफर_And_Mode(अक्षर *बफर_Start, माप_प्रकार आकार,
                                   बफर_Kind Mode) {
  निश्चित(((Mode == बफर_Kind::Unbuffered && !बफर_Start && आकार == 0) ||
          (Mode != बफर_Kind::Unbuffered && बफर_Start && आकार != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  निश्चित(Get_अंक_Bytes_In_बफर() == 0 && "Current buffer is non-empty!");

  अगर (बफर_Mode == बफर_Kind::Internal_बफर)
    मिटाओ [] OutBufStart;
  OutBufStart = बफर_Start;
  OutBufEnd = OutBufStart+आकार;
  OutBufCur = OutBufStart;
  बफर_Mode = Mode;

  निश्चित(OutBufStart <= OutBufEnd && "Invalid size!");
}

raw_ostream &raw_ostream::चालक<<(अचिन्हित दीर्घ N) {
  write_integer(*यह, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(दीर्घ N) {
  write_integer(*यह, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(अचिन्हित दीर्घ दीर्घ N) {
  write_integer(*यह, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(दीर्घ दीर्घ N) {
  write_integer(*यह, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  वापस *यह;
}

raw_ostream &raw_ostream::write_hex(अचिन्हित दीर्घ दीर्घ N) {
  llvm::write_hex(*यह, N, HexPrintStyle::Lower);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(Colors C) {
  अगर (C == Colors::RESET)
    resetColor();
  अन्यथा
    changeColor(C);
  वापस *यह;
}

raw_ostream &raw_ostream::write_uuid(स्थिर uuid_t UUID) {
  क्रम (पूर्णांक Idx = 0; Idx < 16; ++Idx) {
    *यह << format("%02" PRIX32, UUID[Idx]);
    अगर (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *यह << "-";
  }
  वापस *यह;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        bool UseHexEscapes) {
  क्रम (अचिन्हित अक्षर c : Str) {
    चयन (c) {
    हाल '\\':
      *यह << '\\' << '\\';
      अवरोध;
    हाल '\t':
      *यह << '\\' << 't';
      अवरोध;
    हाल '\n':
      *यह << '\\' << 'n';
      अवरोध;
    हाल '"':
      *यह << '\\' << '"';
      अवरोध;
    शेष:
      अगर (isPrint(c)) {
        *यह << c;
        अवरोध;
      }

      // Write out the escaped representation.
      अगर (UseHexEscapes) {
        *यह << '\\' << 'x';
        *यह << hexdigit((c >> 4) & 0xF);
        *यह << hexdigit((c >> 0) & 0xF);
      } अन्यथा {
        // Always use a full 3-character octal escape.
        *यह << '\\';
        *यह << अक्षर('0' + ((c >> 6) & 7));
        *यह << अक्षर('0' + ((c >> 3) & 7));
        *यह << अक्षर('0' + ((c >> 0) & 7));
      }
    }
  }

  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(स्थिर व्योम *P) {
  llvm::write_hex(*यह, (uintptr_t)P, HexPrintStyle::PrefixLower);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(द्विगुन N) {
  llvm::write_double(*यह, N, FloatStyle::Exponent);
  वापस *यह;
}

व्योम raw_ostream::flush_nonempty() {
  निश्चित(OutBufCur > OutBufStart && "Invalid call to flush_nonempty.");
  माप_प्रकार लंबाई = OutBufCur - OutBufStart;
  OutBufCur = OutBufStart;
  write_impl(OutBufStart, लंबाई);
}

raw_ostream &raw_ostream::ग_लिखो(अचिन्हित अक्षर C) {
  // Group exceptional cases into a single branch.
  अगर (LLVM_UNLIKELY(OutBufCur >= OutBufEnd)) {
    अगर (LLVM_UNLIKELY(!OutBufStart)) {
      अगर (बफर_Mode == बफर_Kind::Unbuffered) {
        write_impl(reinterpret_cast<अक्षर *>(&C), 1);
        वापस *यह;
      }
      // Set up a buffer and start over.
      SetBuffered();
      वापस ग_लिखो(C);
    }

    flush_nonempty();
  }

  *OutBufCur++ = C;
  वापस *यह;
}

raw_ostream &raw_ostream::ग_लिखो(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  // Group exceptional cases into a single branch.
  अगर (LLVM_UNLIKELY(माप_प्रकार(OutBufEnd - OutBufCur) < आकार)) {
    अगर (LLVM_UNLIKELY(!OutBufStart)) {
      अगर (बफर_Mode == बफर_Kind::Unbuffered) {
        write_impl(Ptr, आकार);
        वापस *यह;
      }
      // Set up a buffer and start over.
      SetBuffered();
      वापस ग_लिखो(Ptr, आकार);
    }

    माप_प्रकार अंक_Bytes = OutBufEnd - OutBufCur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    अगर (LLVM_UNLIKELY(OutBufCur == OutBufStart)) {
      निश्चित(अंक_Bytes != 0 && "undefined behavior");
      माप_प्रकार BytesToWrite = आकार - (आकार % अंक_Bytes);
      write_impl(Ptr, BytesToWrite);
      माप_प्रकार BytesRemaining = आकार - BytesToWrite;
      अगर (BytesRemaining > माप_प्रकार(OutBufEnd - OutBufCur)) {
        // Too much left over to copy into our buffer.
        वापस ग_लिखो(Ptr + BytesToWrite, BytesRemaining);
      }
      copy_to_बफर(Ptr + BytesToWrite, BytesRemaining);
      वापस *यह;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_to_बफर(Ptr, अंक_Bytes);
    flush_nonempty();
    वापस ग_लिखो(Ptr + अंक_Bytes, आकार - अंक_Bytes);
  }

  copy_to_बफर(Ptr, आकार);

  वापस *यह;
}

व्योम raw_ostream::copy_to_बफर(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  निश्चित(आकार <= माप_प्रकार(OutBufEnd - OutBufCur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  चयन (आकार) {
  हाल 4: OutBufCur[3] = Ptr[3]; [[fallthrough]];
  हाल 3: OutBufCur[2] = Ptr[2]; [[fallthrough]];
  हाल 2: OutBufCur[1] = Ptr[1]; [[fallthrough]];
  हाल 1: OutBufCur[0] = Ptr[0]; [[fallthrough]];
  हाल 0: अवरोध;
  शेष:
    स_नकल(OutBufCur, Ptr, आकार);
    अवरोध;
  }

  OutBufCur += आकार;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::चालक<<(function_ref<पूर्णांक(अक्षर *, माप_प्रकार)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  माप_प्रकार आकार = 128;
  माप_प्रकार बफर_Bytes_बायाँ = OutBufEnd - OutBufCur;
  अगर (बफर_Bytes_बायाँ > 3) {
    पूर्णांक N = Snprint(OutBufCur, बफर_Bytes_बायाँ);
    अगर (N < 0)
      वापस *यह;
    // Common case is that we have plenty of space.
    अगर (माप_प्रकार(N) < बफर_Bytes_बायाँ) {
      OutBufCur += N;
      वापस *यह;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    आकार = माप_प्रकार(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  SmallVector<अक्षर, 128> V;
  क्रम (;;) {
    V.resize(आकार);
    पूर्णांक N = Snprint(V.तथ्य(), आकार);
    अगर (N < 0)
      वापस *यह;
    अगर (माप_प्रकार(N) < आकार)
      वापस ग_लिखो(V.तथ्य(), N);
    आकार = माप_प्रकार(N) + 1;
  }
}

raw_ostream &raw_ostream::चालक<<(स्थिर formatv_object_base &Obj) {
  Obj.format(*यह);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(स्थिर FormattedString &FS) {
  अचिन्हित बायाँ_Indent = 0;
  अचिन्हित दायाँ_Indent = 0;
  स्थिर ssize_t Difference = FS.Width - FS.Str.आकार_2();
  अगर (Difference > 0) {
    चयन (FS.Justify) {
    हाल FormattedString::JustifyNone:
      अवरोध;
    हाल FormattedString::Justify_बायाँ:
      दायाँ_Indent = Difference;
      अवरोध;
    हाल FormattedString::Justify_दायाँ:
      बायाँ_Indent = Difference;
      अवरोध;
    हाल FormattedString::JustifyCenter:
      बायाँ_Indent = Difference / 2;
      दायाँ_Indent = Difference - बायाँ_Indent;
      अवरोध;
    }
  }
  indent(बायाँ_Indent);
  (*यह) << FS.Str;
  indent(दायाँ_Indent);
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(स्थिर Formatted_संख्या &FN) {
  अगर (FN.Hex) {
    HexPrintStyle Style;
    अगर (FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixUpper;
    अन्यथा अगर (FN.Upper && !FN.HexPrefix)
      Style = HexPrintStyle::Upper;
    अन्यथा अगर (!FN.Upper && FN.HexPrefix)
      Style = HexPrintStyle::PrefixLower;
    अन्यथा
      Style = HexPrintStyle::Lower;
    llvm::write_hex(*यह, FN.Hex_मूल्य, Style, FN.Width);
  } अन्यथा {
    llvm::SmallString<16> बफर;
    llvm::raw_svector_ostream Stream(बफर);
    llvm::write_integer(Stream, FN.Dec_मूल्य, 0, IntegerStyle::Integer);
    अगर (बफर.आकार_2() < FN.Width)
      indent(FN.Width - बफर.आकार_2());
    (*यह) << बफर;
  }
  वापस *यह;
}

raw_ostream &raw_ostream::चालक<<(स्थिर FormattedBytes &FB) {
  अगर (FB.Bytes.empty())
    वापस *यह;

  माप_प्रकार पंक्ति_सूचकांक = 0;
  स्वतः Bytes = FB.Bytes;
  स्थिर माप_प्रकार आकार = Bytes.आकार_2();
  HexPrintStyle HPS = FB.Upper ? HexPrintStyle::Upper : HexPrintStyle::Lower;
  uint64_t OffsetWidth = 0;
  अगर (FB.FirstByteOffset) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    माप_प्रकार पंक्तियाँ = आकार / FB.अंक_Per_पंक्ति;
    uint64_t MaxOffset = *FB.FirstByteOffset + पंक्तियाँ * FB.अंक_Per_पंक्ति;
    अचिन्हित Power = 0;
    अगर (MaxOffset > 0)
      Power = llvm::Log2_64_Ceil(MaxOffset);
    OffsetWidth = std::max<uint64_t>(4, llvm::alignTo(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  अचिन्हित अंक_Byte_Groups =
      alignTo(FB.अंक_Per_पंक्ति, FB.Byte_Group_आकार) / FB.Byte_Group_आकार;
  अचिन्हित BlockCharWidth = FB.अंक_Per_पंक्ति * 2 + अंक_Byte_Groups - 1;

  जबतक (!Bytes.empty()) {
    indent(FB.IndentLevel);

    अगर (FB.FirstByteOffset) {
      uint64_t Offset = *FB.FirstByteOffset;
      llvm::write_hex(*यह, Offset + पंक्ति_सूचकांक, HPS, OffsetWidth);
      *यह << ": ";
    }

    स्वतः पंक्ति_2 = Bytes.take_front(FB.अंक_Per_पंक्ति);

    माप_प्रकार CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    क्रम (माप_प्रकार I = 0; I < पंक्ति_2.आकार_2(); ++I, CharsPrinted += 2) {
      अगर (I && (I % FB.Byte_Group_आकार) == 0) {
        ++CharsPrinted;
        *यह << " ";
      }
      llvm::write_hex(*यह, पंक्ति_2[I], HPS, 2);
    }

    अगर (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      निश्चित(BlockCharWidth >= CharsPrinted);
      indent(BlockCharWidth - CharsPrinted + 2);
      *यह << "|";

      // Print the ASCII char values for each byte on this line
      क्रम (uint8_t Byte : पंक्ति_2) {
        अगर (isPrint(Byte))
          *यह << static_cast<अक्षर>(Byte);
        अन्यथा
          *यह << '.';
      }
      *यह << '|';
    }

    Bytes = Bytes.drop_front(पंक्ति_2.आकार_2());
    पंक्ति_सूचकांक += पंक्ति_2.आकार_2();
    अगर (पंक्ति_सूचकांक < आकार)
      *यह << '\n';
  }
  वापस *यह;
}

ढाँचा <अक्षर C>
अटल raw_ostream &write_padding(raw_ostream &OS, अचिन्हित अंक_Chars) {
  अटल स्थिर अक्षर Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  अगर (अंक_Chars < std::आकार_2(Chars))
    वापस OS.ग_लिखो(Chars, अंक_Chars);

  जबतक (अंक_Chars) {
    अचिन्हित अंक_To_Write = std::min(अंक_Chars, (अचिन्हित)std::आकार_2(Chars) - 1);
    OS.ग_लिखो(Chars, अंक_To_Write);
    अंक_Chars -= अंक_To_Write;
  }
  वापस OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(अचिन्हित अंक_Spaces) {
  वापस write_padding<' '>(*यह, अंक_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(अचिन्हित अंक_Zeros) {
  वापस write_padding<'\0'>(*यह, अंक_Zeros);
}

bool raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  अगर (!ColorEnabled)
    वापस false;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  अगर (sys::Process::ColorNeedsFlush() && !is_displayed())
    वापस false;

  अगर (sys::Process::ColorNeedsFlush())
    सफाई();

  वापस true;
}

raw_ostream &raw_ostream::changeColor(क्रमागत Colors colors, bool bold, bool bg) {
  अगर (!prepare_colors())
    वापस *यह;

  स्थिर अक्षर *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<अक्षर>(colors), bold, bg);
  अगर (colorcode)
    ग_लिखो(colorcode, म_माप(colorcode));
  वापस *यह;
}

raw_ostream &raw_ostream::resetColor() {
  अगर (!prepare_colors())
    वापस *यह;

  अगर (स्थिर अक्षर *colorcode = sys::Process::ResetColor())
    ग_लिखो(colorcode, म_माप(colorcode));
  वापस *यह;
}

raw_ostream &raw_ostream::reverseColor() {
  अगर (!prepare_colors())
    वापस *यह;

  अगर (स्थिर अक्षर *colorcode = sys::Process::OutputReverse())
    ग_लिखो(colorcode, म_माप(colorcode));
  वापस *यह;
}

व्योम raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

अटल पूर्णांक getFD(StringRef Filename, std::error_code &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  निश्चित((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  अगर (Filename == "-") {
    EC = std::error_code();
    // Change stdout's text/binary mode based on the Flags.
    sys::ChangeStdoutMode(Flags);
    वापस STDOUT_FILENO;
  }

  पूर्णांक FD;
  अगर (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  अन्यथा
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  अगर (EC)
    वापस -1;

  वापस FD;
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
raw_fd_ostream::raw_fd_ostream(पूर्णांक fd, bool shouldClose, bool unbuffered,
                               OStreamKind K)
    : raw_pwrite_stream(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  अगर (FD < 0 ) {
    ShouldClose = false;
    वापस;
  }

  enable_colors(true);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  अगर (FD <= STDERR_FILENO)
    ShouldClose = false;

#अगर_घोषित _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#पूर्ण_अगर

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, प्रस्तुत_से);
  sys::fs::file_status Status;
  std::error_code EC = status(FD, Status);
  IsRegularFile = Status.type() == sys::fs::file_type::regular_file;
#अगर_घोषित _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#अन्यथा
  SupportsSeeking = !EC && loc != (off_t)-1;
#पूर्ण_अगर
  अगर (!SupportsSeeking)
    pos = 0;
  अन्यथा
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  अगर (FD >= 0) {
    सफाई();
    अगर (ShouldClose) {
      अगर (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#अगर_घोषित __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  अगर (FD == 2) वापस;
#पूर्ण_अगर

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  अगर (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().message());
}

#अगर defined(_WIN32)
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
अटल bool write_console_impl(पूर्णांक FD, StringRef तथ्य_2) {
  SmallVector<ब_अक्षर_प्रकार, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  अगर (स्वतः EC = sys::windows::UTF8ToUTF16(तथ्य_2, WideText))
    वापस false;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  माप_प्रकार Max_Write_आकार = WideText.आकार_2();
  अगर (!RunningWindows8OrGreater())
    Max_Write_आकार = 32767;

  माप_प्रकार WCharsWritten = 0;
  करो {
    माप_प्रकार WCharsToWrite =
        std::min(Max_Write_आकार, WideText.आकार_2() - WCharsWritten);
    DWORD ActuallyWritten;
    bool Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WCharsToWrite, &ActuallyWritten,
                        /*Reserved=*/nullptr);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    अगर (!Success)
      वापस false;

    WCharsWritten += ActuallyWritten;
  } जबतक (WCharsWritten != WideText.आकार_2());
  वापस true;
}
#पूर्ण_अगर

व्योम raw_fd_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  अगर (TiedStream)
    TiedStream->सफाई();

  निश्चित(FD >= 0 && "File already closed.");
  pos += आकार;

#अगर defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  अगर (IsWindowsConsole)
    अगर (write_console_impl(FD, StringRef(Ptr, आकार)))
      वापस;
#पूर्ण_अगर

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  माप_प्रकार Max_Write_आकार = INT32_MAX;

#अगर defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_आकार = 1024 * 1024 * 1024;
#पूर्ण_अगर

  करो {
    माप_प्रकार Chunk_आकार = std::min(आकार, Max_Write_आकार);
    ssize_t ret = ::ग_लिखो(FD, Ptr, Chunk_आकार);

    अगर (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      अगर (त्रुटि_सं == EINTR || त्रुटि_सं == EAGAIN
#अगर_घोषित EWOULDBLOCK
          || त्रुटि_सं == EWOULDBLOCK
#पूर्ण_अगर
          )
        जारी;

#अगर_घोषित _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD WinLastError = GetLastError();
      अगर (WinLastError == ERROR_BROKEN_PIPE ||
          (WinLastError == ERROR_NO_तथ्य && त्रुटि_सं == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        त्रुटि_सं = EPIPE;
      }
#पूर्ण_अगर
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errnoAsErrorCode());
      अवरोध;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    आकार -= ret;
  } जबतक (आकार > 0);
}

व्योम raw_fd_ostream::बंद() {
  निश्चित(ShouldClose);
  ShouldClose = false;
  सफाई();
  अगर (स्वतः EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  निश्चित(SupportsSeeking && "Stream does not support seeking!");
  सफाई();
#अगर_घोषित _WIN32
  pos = ::_lseeki64(FD, off, शुरू_से);
#अन्यथा
  pos = ::lseek(FD, off, शुरू_से);
#पूर्ण_अगर
  अगर (pos == (uint64_t)-1)
    error_detected(errnoAsErrorCode());
  वापस pos;
}

व्योम raw_fd_ostream::pwrite_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार,
                                 uint64_t Offset) {
  uint64_t Pos = tell();
  seek(Offset);
  ग_लिखो(Ptr, आकार);
  seek(Pos);
}

माप_प्रकार raw_fd_ostream::preferred_बफर_आकार() स्थिर {
#अगर defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  अगर (IsWindowsConsole)
    वापस 0;
  वापस raw_ostream::preferred_बफर_आकार();
#या_अगर defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  वापस raw_ostream::preferred_बफर_आकार();
#अन्यथा
  निश्चित(FD >= 0 && "File not yet open!");
  काष्ठा stat statbuf;
  अगर (ख_स्थिति(FD, &statbuf) != 0)
    वापस 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  अगर (S_ISCHR(statbuf.st_mode) && is_displayed())
    वापस 0;
  // Return the preferred block size.
  वापस statbuf.st_blksize;
#पूर्ण_अगर
}

bool raw_fd_ostream::is_displayed() स्थिर {
  वापस sys::Process::FileDescriptorIsDisplayed(FD);
}

bool raw_fd_ostream::has_colors() स्थिर {
  अगर (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  वापस *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::lock() {
  std::error_code EC = sys::fs::lockFile(FD);
  अगर (!EC)
    वापस sys::fs::FileLocker(FD);
  वापस errorCodeToError(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::tryLockFor(Duration स्थिर& Timeout) {
  std::error_code EC = sys::fs::tryLockFile(FD, Timeout.getDuration());
  अगर (!EC)
    वापस sys::fs::FileLocker(FD);
  वापस errorCodeToError(EC);
}

व्योम raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::म_दो() {
  // Set buffer settings to model stdout behavior.
  std::error_code EC;

  // On z/OS we need to enable auto conversion
  अटल std::error_code EC1 = enableAutoConversion(STDOUT_FILENO);
  निश्चित(!EC1);
  (व्योम)EC1;

  अटल raw_fd_ostream S("-", EC, sys::fs::OF_None);
  निश्चित(!EC);
  वापस S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  अटल std::error_code EC = enableAutoConversion(STDERR_FILENO);
  निश्चित(!EC);
  (व्योम)EC;

  // Set standard error to be unbuffered.
  अटल raw_fd_ostream S(STDERR_FILENO, false, true);
  वापस S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  अटल raw_null_ostream S;
  वापस S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_stream::raw_fd_stream(StringRef Filename, std::error_code &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::OF_None),
                     true, false, OStreamKind::OK_FDStream) {
  अगर (EC)
    वापस;

  अगर (!isRegularFile())
    EC = std::make_error_code(std::errc::invalid_argument);
}

raw_fd_stream::raw_fd_stream(पूर्णांक fd, bool shouldClose)
    : raw_fd_ostream(fd, shouldClose, false, OStreamKind::OK_FDStream) {}

ssize_t raw_fd_stream::पढ़ो(अक्षर *Ptr, माप_प्रकार आकार) {
  निश्चित(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::पढ़ो(get_fd(), (व्योम *)Ptr, आकार);
  अगर (Ret >= 0)
    inc_pos(Ret);
  अन्यथा
    error_detected(errnoAsErrorCode());
  वापस Ret;
}

bool raw_fd_stream::classof(स्थिर raw_ostream *OS) {
  वापस OS->get_kind() == OStreamKind::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

व्योम raw_string_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  OS.append(Ptr, आकार);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() स्थिर { वापस OS.आकार_2(); }

व्योम raw_svector_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
  OS.append(Ptr, Ptr + आकार);
}

व्योम raw_svector_ostream::pwrite_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार,
                                      uint64_t Offset) {
  स_नकल(OS.तथ्य() + Offset, Ptr, आकार);
}

bool raw_svector_ostream::classof(स्थिर raw_ostream *OS) {
  वापस OS->get_kind() == OStreamKind::OK_SVecStream;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#अगर_अघोषित न_संशोधन
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  सफाई();
#पूर्ण_अगर
}

व्योम raw_null_ostream::write_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार) {
}

uint64_t raw_null_ostream::current_pos() स्थिर {
  वापस 0;
}

व्योम raw_null_ostream::pwrite_impl(स्थिर अक्षर *Ptr, माप_प्रकार आकार,
                                   uint64_t Offset) {}

व्योम raw_pwrite_stream::anchor() {}

व्योम बफर_ostream::anchor() {}

व्योम बफर_unique_ostream::anchor() {}

अटल bool isNullDeviceName(StringRef OutputFileName) {
#अगर_घोषित _WIN32
  अगर (OutputFileName.equals_insensitive("NUL"))
    वापस true;
#पूर्ण_अगर
  वापस OutputFileName == "/dev/null";
}

Error llvm::writeToOutput(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  अगर (OutputFileName == "-")
    वापस Write(म_दो());

  अगर (isNullDeviceName(OutputFileName)) {
    raw_null_ostream Out;
    वापस Write(Out);
  }

  अचिन्हित Mode = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::अस्थायी_File> अस्थायी =
      sys::fs::अस्थायी_File::create(OutputFileName + ".temp-stream-%%%%%%", Mode);
  अगर (!अस्थायी)
    वापस createFileError(OutputFileName, अस्थायी.takeError());

#अगर defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  अगर (स्वतः EC = llvm::copyFileTagAttributes(OutputFileName.str(), अस्थायी->FD)) {
    अगर (EC != std::errc::no_such_file_or_directory)
      वापस createFileError(OutputFileName, EC);
  }
#पूर्ण_अगर

  raw_fd_ostream Out(अस्थायी->FD, false);

  अगर (Error E = Write(Out)) {
    अगर (Error DiscardError = अस्थायी->discard())
      वापस joinErrors(std::move(E), std::move(DiscardError));
    वापस E;
  }
  Out.सफाई();

  वापस अस्थायी->keep(OutputFileName);
}
