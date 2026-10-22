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
#यदि_अन्तः

#यदि defined(__CYGWIN__)
#अन्तर्भाव <io.h>
#यदि_अन्तः

#यदि defined(_MSC_VER)
#अन्तर्भाव <io.h>
#यदि_अपरिभाषितम् STDIN_FILENO
# define STDIN_FILENO 0
#यदि_अन्तः
#यदि_अपरिभाषितम् STDOUT_FILENO
# define STDOUT_FILENO 1
#यदि_अन्तः
#यदि_अपरिभाषितम् STDERR_FILENO
# define STDERR_FILENO 2
#यदि_अन्तः
#यदि_अन्तः

#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "llvm/Support/ConvertUTF.h"
#अन्तर्भाव "llvm/Support/Signals.h"
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#यदि_अन्तः

उपयुज्य नामाकाश llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  प्रतिज्ञा(बहिर्गमनम्_Buf_Cur == बहिर्गमनम्_Buf_Start &&
         "raw_ostream destructor called with non-empty buffer!");

  यदि (बफर_बहुलकम् == बफर_विधा::आन्तरिकम्_बफर)
    विलोपय [] बहिर्गमनम्_Buf_Start;
}

size_t raw_ostream::preferred_बफर_आकार() ध्रुव {
#यदि_परिभाषितम् _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  प्रत्यागम (16 * 1024);
#अन्यथा
  // BUFSIZ is intended to be a reasonable default.
  प्रत्यागम BUFSIZ;
#यदि_अन्तः
}

शून्य raw_ostream::समुच्चयः_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  यदि (size_t आकार = preferred_बफर_आकार())
    समुच्चयः_बफर_आकार(आकार);
  अन्यथा
    // It may return 0, meaning this stream should be unbuffered.
    समुच्चयः_Unbuffered();
}

शून्य raw_ostream::समुच्चयः_बफर_And_बहुलकम्(वर्ण *बफर_Start, size_t आकार,
                                   बफर_विधा बहुलकम्) {
  प्रतिज्ञा(((बहुलकम् == बफर_विधा::Unbuffered && !बफर_Start && आकार == 0) ||
          (बहुलकम् != बफर_विधा::Unbuffered && बफर_Start && आकार != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  प्रतिज्ञा(Get_अङ्कः_अष्टकानि_अन्तर्_बफर() == 0 && "Current buffer is non-empty!");

  यदि (बफर_बहुलकम् == बफर_विधा::आन्तरिकम्_बफर)
    विलोपय [] बहिर्गमनम्_Buf_Start;
  बहिर्गमनम्_Buf_Start = बफर_Start;
  बहिर्गमनम्_Buf_अन्तः = बहिर्गमनम्_Buf_Start+आकार;
  बहिर्गमनम्_Buf_Cur = बहिर्गमनम्_Buf_Start;
  बफर_बहुलकम् = बहुलकम्;

  प्रतिज्ञा(बहिर्गमनम्_Buf_Start <= बहिर्गमनम्_Buf_अन्तः && "Invalid size!");
}

raw_ostream &raw_ostream::संक्रियकः<<(अचिह्नित दीर्घ N) {
  write_integer(*एतत्, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(दीर्घ N) {
  write_integer(*एतत्, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(अचिह्नित दीर्घ दीर्घ N) {
  write_integer(*एतत्, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(दीर्घ दीर्घ N) {
  write_integer(*एतत्, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::write_hex(अचिह्नित दीर्घ दीर्घ N) {
  llvm::write_hex(*एतत्, N, HexPrintStyle::अधस्तनम्_2);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(Colors C) {
  यदि (C == Colors::RESET)
    resetColor();
  अन्यथा
    changeColor(C);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::write_uuid(ध्रुव uuid_t UUID) {
  प्रति (पूर्णाङ्क Idx = 0; Idx < 16; ++Idx) {
    *एतत् << format("%02" PRIX32, UUID[Idx]);
    यदि (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *एतत् << "-";
  }
  प्रत्यागम *एतत्;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        तार्किक UseHexEscapes) {
  प्रति (अचिह्नित वर्ण c : Str) {
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
      यदि (isPrint(c)) {
        *एतत् << c;
        भङ्ग;
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

  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(ध्रुव शून्य *P) {
  llvm::write_hex(*एतत्, (uintptr_t)P, HexPrintStyle::उपसर्गः_अधस्तनम्);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(द्विगुण N) {
  llvm::write_double(*एतत्, N, FloatStyle::Exponent);
  प्रत्यागम *एतत्;
}

शून्य raw_ostream::flush_nonempty() {
  प्रतिज्ञा(बहिर्गमनम्_Buf_Cur > बहिर्गमनम्_Buf_Start && "Invalid call to flush_nonempty.");
  size_t दीर्घता = बहिर्गमनम्_Buf_Cur - बहिर्गमनम्_Buf_Start;
  बहिर्गमनम्_Buf_Cur = बहिर्गमनम्_Buf_Start;
  write_impl(बहिर्गमनम्_Buf_Start, दीर्घता);
}

raw_ostream &raw_ostream::लिख(अचिह्नित वर्ण C) {
  // Group exceptional cases into a single branch.
  यदि (LLVM_UNLIKELY(बहिर्गमनम्_Buf_Cur >= बहिर्गमनम्_Buf_अन्तः)) {
    यदि (LLVM_UNLIKELY(!बहिर्गमनम्_Buf_Start)) {
      यदि (बफर_बहुलकम् == बफर_विधा::Unbuffered) {
        write_impl(reinterpret_cast<वर्ण *>(&C), 1);
        प्रत्यागम *एतत्;
      }
      // Set up a buffer and start over.
      समुच्चयः_Buffered();
      प्रत्यागम लिख(C);
    }

    flush_nonempty();
  }

  *बहिर्गमनम्_Buf_Cur++ = C;
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::लिख(ध्रुव वर्ण *Ptr, size_t आकार) {
  // Group exceptional cases into a single branch.
  यदि (LLVM_UNLIKELY(size_t(बहिर्गमनम्_Buf_अन्तः - बहिर्गमनम्_Buf_Cur) < आकार)) {
    यदि (LLVM_UNLIKELY(!बहिर्गमनम्_Buf_Start)) {
      यदि (बफर_बहुलकम् == बफर_विधा::Unbuffered) {
        write_impl(Ptr, आकार);
        प्रत्यागम *एतत्;
      }
      // Set up a buffer and start over.
      समुच्चयः_Buffered();
      प्रत्यागम लिख(Ptr, आकार);
    }

    size_t अङ्कः_अष्टकानि = बहिर्गमनम्_Buf_अन्तः - बहिर्गमनम्_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    यदि (LLVM_UNLIKELY(बहिर्गमनम्_Buf_Cur == बहिर्गमनम्_Buf_Start)) {
      प्रतिज्ञा(अङ्कः_अष्टकानि != 0 && "undefined behavior");
      size_t अष्टकानि_गन्तव्यम्_Write_2 = आकार - (आकार % अङ्कः_अष्टकानि);
      write_impl(Ptr, अष्टकानि_गन्तव्यम्_Write_2);
      size_t अष्टकानि_Remaining_2 = आकार - अष्टकानि_गन्तव्यम्_Write_2;
      यदि (अष्टकानि_Remaining_2 > size_t(बहिर्गमनम्_Buf_अन्तः - बहिर्गमनम्_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        प्रत्यागम लिख(Ptr + अष्टकानि_गन्तव्यम्_Write_2, अष्टकानि_Remaining_2);
      }
      copy_गन्तव्यम्_बफर(Ptr + अष्टकानि_गन्तव्यम्_Write_2, अष्टकानि_Remaining_2);
      प्रत्यागम *एतत्;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_गन्तव्यम्_बफर(Ptr, अङ्कः_अष्टकानि);
    flush_nonempty();
    प्रत्यागम लिख(Ptr + अङ्कः_अष्टकानि, आकार - अङ्कः_अष्टकानि);
  }

  copy_गन्तव्यम्_बफर(Ptr, आकार);

  प्रत्यागम *एतत्;
}

शून्य raw_ostream::copy_गन्तव्यम्_बफर(ध्रुव वर्ण *Ptr, size_t आकार) {
  प्रतिज्ञा(आकार <= size_t(बहिर्गमनम्_Buf_अन्तः - बहिर्गमनम्_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  विकल्प (आकार) {
  पक्ष 4: बहिर्गमनम्_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  पक्ष 3: बहिर्गमनम्_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  पक्ष 2: बहिर्गमनम्_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  पक्ष 1: बहिर्गमनम्_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  पक्ष 0: भङ्ग;
  स्वभाव:
    स्मृतिं_प्रतिलिख(बहिर्गमनम्_Buf_Cur, Ptr, आकार);
    भङ्ग;
  }

  बहिर्गमनम्_Buf_Cur += आकार;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::संक्रियकः<<(function_ref<पूर्णाङ्क(वर्ण *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t आकार = 128;
  size_t बफर_अष्टकानि_वामः = बहिर्गमनम्_Buf_अन्तः - बहिर्गमनम्_Buf_Cur;
  यदि (बफर_अष्टकानि_वामः > 3) {
    पूर्णाङ्क N = Snprint(बहिर्गमनम्_Buf_Cur, बफर_अष्टकानि_वामः);
    यदि (N < 0)
      प्रत्यागम *एतत्;
    // Common case is that we have plenty of space.
    यदि (size_t(N) < बफर_अष्टकानि_वामः) {
      बहिर्गमनम्_Buf_Cur += N;
      प्रत्यागम *एतत्;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    आकार = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_सदिशम्<वर्ण, 128> V;
  प्रति (;;) {
    V.resize(आकार);
    पूर्णाङ्क N = Snprint(V.दत्त(), आकार);
    यदि (N < 0)
      प्रत्यागम *एतत्;
    यदि (size_t(N) < आकार)
      प्रत्यागम लिख(V.दत्त(), N);
    आकार = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::संक्रियकः<<(ध्रुव formatv_object_आधारः &Obj) {
  Obj.format(*एतत्);
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(ध्रुव FormattedString &FS) {
  अचिह्नित वामः_Indent = 0;
  अचिह्नित दक्षिणः_Indent = 0;
  ध्रुव ssize_t Difference = FS.विस्तारः - FS.Str.आकार_2();
  यदि (Difference > 0) {
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

raw_ostream &raw_ostream::संक्रियकः<<(ध्रुव Formatted_सङ्ख्या &FN) {
  यदि (FN.Hex) {
    HexPrintStyle Style;
    यदि (FN.ऊर्ध्वस्थम्_2 && FN.Hex_उपसर्गः)
      Style = HexPrintStyle::उपसर्गः_ऊर्ध्वस्थम्;
    अन्यथा यदि (FN.ऊर्ध्वस्थम्_2 && !FN.Hex_उपसर्गः)
      Style = HexPrintStyle::ऊर्ध्वस्थम्_2;
    अन्यथा यदि (!FN.ऊर्ध्वस्थम्_2 && FN.Hex_उपसर्गः)
      Style = HexPrintStyle::उपसर्गः_अधस्तनम्;
    अन्यथा
      Style = HexPrintStyle::अधस्तनम्_2;
    llvm::write_hex(*एतत्, FN.Hex_मानम्, Style, FN.विस्तारः);
  } अन्यथा {
    llvm::SmallString<16> बफर;
    llvm::raw_svector_ostream प्रवाहः(बफर);
    llvm::write_integer(प्रवाहः, FN.Dec_मानम्, 0, IntegerStyle::Integer);
    यदि (बफर.आकार_2() < FN.विस्तारः)
      indent(FN.विस्तारः - बफर.आकार_2());
    (*एतत्) << बफर;
  }
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::संक्रियकः<<(ध्रुव Formatted_अष्टकानि &FB) {
  यदि (FB.अष्टकानि_2.रिक्तानि())
    प्रत्यागम *एतत्;

  size_t पङ्क्तिः_सूचकांक = 0;
  स्वचालित अष्टकानि_2 = FB.अष्टकानि_2;
  ध्रुव size_t आकार = अष्टकानि_2.आकार_2();
  HexPrintStyle HPS = FB.ऊर्ध्वस्थम्_2 ? HexPrintStyle::ऊर्ध्वस्थम्_2 : HexPrintStyle::अधस्तनम्_2;
  uint64_t विस्थापनम्_विस्तारः = 0;
  यदि (FB.First_Byte_विस्थापनम्) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t पङ्क्तयः = आकार / FB.अङ्कः_प्रत्येकम्_पङ्क्तिः;
    uint64_t Max_विस्थापनम् = *FB.First_Byte_विस्थापनम् + पङ्क्तयः * FB.अङ्कः_प्रत्येकम्_पङ्क्तिः;
    अचिह्नित Power = 0;
    यदि (Max_विस्थापनम् > 0)
      Power = llvm::Log2_64_Ceil(Max_विस्थापनम्);
    विस्थापनम्_विस्तारः = std::max<uint64_t>(4, llvm::align_गन्तव्यम्(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  अचिह्नित अङ्कः_Byte_Groups =
      align_गन्तव्यम्(FB.अङ्कः_प्रत्येकम्_पङ्क्तिः, FB.Byte_गणः_आकार) / FB.Byte_गणः_आकार;
  अचिह्नित खण्डम्_Char_विस्तारः = FB.अङ्कः_प्रत्येकम्_पङ्क्तिः * 2 + अङ्कः_Byte_Groups - 1;

  यावत् (!अष्टकानि_2.रिक्तानि()) {
    indent(FB.IndentLevel);

    यदि (FB.First_Byte_विस्थापनम्) {
      uint64_t विस्थापनम् = *FB.First_Byte_विस्थापनम्;
      llvm::write_hex(*एतत्, विस्थापनम् + पङ्क्तिः_सूचकांक, HPS, विस्थापनम्_विस्तारः);
      *एतत् << ": ";
    }

    स्वचालित पङ्क्तिः = अष्टकानि_2.take_front(FB.अङ्कः_प्रत्येकम्_पङ्क्तिः);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    प्रति (size_t I = 0; I < पङ्क्तिः.आकार_2(); ++I, CharsPrinted += 2) {
      यदि (I && (I % FB.Byte_गणः_आकार) == 0) {
        ++CharsPrinted;
        *एतत् << " ";
      }
      llvm::write_hex(*एतत्, पङ्क्तिः[I], HPS, 2);
    }

    यदि (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      प्रतिज्ञा(खण्डम्_Char_विस्तारः >= CharsPrinted);
      indent(खण्डम्_Char_विस्तारः - CharsPrinted + 2);
      *एतत् << "|";

      // Print the ASCII char values for each byte on this line
      प्रति (uint8_t Byte : पङ्क्तिः) {
        यदि (isPrint(Byte))
          *एतत् << static_cast<वर्ण>(Byte);
        अन्यथा
          *एतत् << '.';
      }
      *एतत् << '|';
    }

    अष्टकानि_2 = अष्टकानि_2.drop_front(पङ्क्तिः.आकार_2());
    पङ्क्तिः_सूचकांक += पङ्क्तिः.आकार_2();
    यदि (पङ्क्तिः_सूचकांक < आकार)
      *एतत् << '\n';
  }
  प्रत्यागम *एतत्;
}

प्रतिरूपम् <वर्ण C>
स्थिर raw_ostream &write_padding(raw_ostream &OS, अचिह्नित अङ्कः_Chars) {
  स्थिर ध्रुव वर्ण Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  यदि (अङ्कः_Chars < std::आकार_2(Chars))
    प्रत्यागम OS.लिख(Chars, अङ्कः_Chars);

  यावत् (अङ्कः_Chars) {
    अचिह्नित अङ्कः_गन्तव्यम्_Write = std::min(अङ्कः_Chars, (अचिह्नित)std::आकार_2(Chars) - 1);
    OS.लिख(Chars, अङ्कः_गन्तव्यम्_Write);
    अङ्कः_Chars -= अङ्कः_गन्तव्यम्_Write;
  }
  प्रत्यागम OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(अचिह्नित अङ्कः_Spaces) {
  प्रत्यागम write_padding<' '>(*एतत्, अङ्कः_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(अचिह्नित अङ्कः_Zeros) {
  प्रत्यागम write_padding<'\0'>(*एतत्, अङ्कः_Zeros);
}

तार्किक raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  यदि (!ColorEnabled)
    प्रत्यागम असत्य;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  यदि (sys::Process::ColorNeedsFlush() && !is_displayed())
    प्रत्यागम असत्य;

  यदि (sys::Process::ColorNeedsFlush())
    flush();

  प्रत्यागम सत्य;
}

raw_ostream &raw_ostream::changeColor(गणना Colors colors, तार्किक bold, तार्किक bg) {
  यदि (!prepare_colors())
    प्रत्यागम *एतत्;

  ध्रुव वर्ण *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<वर्ण>(colors), bold, bg);
  यदि (colorcode)
    लिख(colorcode, वाक्यदैर्घ्यम्(colorcode));
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::resetColor() {
  यदि (!prepare_colors())
    प्रत्यागम *एतत्;

  यदि (ध्रुव वर्ण *colorcode = sys::Process::ResetColor())
    लिख(colorcode, वाक्यदैर्घ्यम्(colorcode));
  प्रत्यागम *एतत्;
}

raw_ostream &raw_ostream::reverseColor() {
  यदि (!prepare_colors())
    प्रत्यागम *एतत्;

  यदि (ध्रुव वर्ण *colorcode = sys::Process::OutputReverse())
    लिख(colorcode, वाक्यदैर्घ्यम्(colorcode));
  प्रत्यागम *एतत्;
}

शून्य raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

स्थिर पूर्णाङ्क getFD(StringRef Filename, std::error_संहिता &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  प्रतिज्ञा((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  यदि (Filename == "-") {
    EC = std::error_संहिता();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_बहुलकम्(Flags);
    प्रत्यागम STDOUT_FILENO;
  }

  पूर्णाङ्क FD;
  यदि (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  अन्यथा
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  यदि (EC)
    प्रत्यागम -1;

  प्रत्यागम FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_संहिता &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::सम्बन्धि_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_संहिता &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::सम्बन्धि_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_संहिता &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::सम्बन्धि_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_संहिता &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_संहिता &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), सत्य) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(पूर्णाङ्क fd, तार्किक shouldClose, तार्किक unbuffered,
                               OStream_विधा K)
    : raw_pwrite_प्रवाहः(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  यदि (FD < 0 ) {
    ShouldClose = असत्य;
    प्रत्यागम;
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

#यदि_परिभाषितम् _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#यदि_अन्तः

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_स्थितिः स्थितिः;
  std::error_संहिता EC = स्थितिः_2(FD, स्थितिः);
  IsRegularFile = स्थितिः.type() == sys::fs::file_type::regular_file;
#यदि_परिभाषितम् _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#अन्यथा
  SupportsSeeking = !EC && loc != (off_t)-1;
#यदि_अन्तः
  यदि (!SupportsSeeking)
    pos = 0;
  अन्यथा
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  यदि (FD >= 0) {
    flush();
    यदि (ShouldClose) {
      यदि (स्वचालित EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#यदि_परिभाषितम् __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  यदि (FD == 2) प्रत्यागम;
#यदि_अन्तः

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  यदि (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().सन्देशः_2());
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
स्थिर तार्किक write_console_impl(पूर्णाङ्क FD, StringRef दत्त_2) {
  Small_सदिशम्<wchar_t, 256> WideText;

  // Fall back to ::write if it wasn't valid UTF-8.
  यदि (स्वचालित EC = sys::windows::UTF8_गन्तव्यम्_UTF16(दत्त_2, WideText))
    प्रत्यागम असत्य;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_आकार = WideText.आकार_2();
  यदि (!RunningWindows8OrGreater())
    Max_Write_आकार = 32767;

  size_t WCharsWritten = 0;
  कुरु {
    size_t WChars_गन्तव्यम्_Write =
        std::min(Max_Write_आकार, WideText.आकार_2() - WCharsWritten);
    DWORD ActuallyWritten;
    तार्किक Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &WideText[WCharsWritten],
                        WChars_गन्तव्यम्_Write, &ActuallyWritten,
                        /*Reserved=*/शून्यसूचकः);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    यदि (!Success)
      प्रत्यागम असत्य;

    WCharsWritten += ActuallyWritten;
  } यावत् (WCharsWritten != WideText.आकार_2());
  प्रत्यागम सत्य;
}
#यदि_अन्तः

शून्य raw_fd_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t आकार) {
  यदि (Tied_प्रवाहः)
    Tied_प्रवाहः->flush();

  प्रतिज्ञा(FD >= 0 && "File already closed.");
  pos += आकार;

#यदि defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  यदि (IsWindowsConsole)
    यदि (write_console_impl(FD, StringRef(Ptr, आकार)))
      प्रत्यागम;
#यदि_अन्तः

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_आकार = INT32_MAX;

#यदि defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_आकार = 1024 * 1024 * 1024;
#यदि_अन्तः

  कुरु {
    size_t Chunk_आकार = std::min(आकार, Max_Write_आकार);
    ssize_t ret = ::लिख(FD, Ptr, Chunk_आकार);

    यदि (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      यदि (errno == EINTR || errno == EAGAIN
#यदि_परिभाषितम् EWOULDBLOCK
          || errno == EWOULDBLOCK
#यदि_अन्तः
          )
        अनुवर्त;

#यदि_परिभाषितम् _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_चरमः_Error = Get_चरमः_Error();
      यदि (Win_चरमः_Error == ERROR_BROKEN_PIPE ||
          (Win_चरमः_Error == ERROR_NO_दत्त && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#यदि_अन्तः
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_संहिता());
      भङ्ग;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    आकार -= ret;
  } यावत् (आकार > 0);
}

शून्य raw_fd_ostream::close() {
  प्रतिज्ञा(ShouldClose);
  ShouldClose = असत्य;
  flush();
  यदि (स्वचालित EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  प्रतिज्ञा(SupportsSeeking && "Stream does not support seeking!");
  flush();
#यदि_परिभाषितम् _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#अन्यथा
  pos = ::lseek(FD, off, SEEK_SET);
#यदि_अन्तः
  यदि (pos == (uint64_t)-1)
    error_detected(errno_As_Error_संहिता());
  प्रत्यागम pos;
}

शून्य raw_fd_ostream::pwrite_impl(ध्रुव वर्ण *Ptr, size_t आकार,
                                 uint64_t विस्थापनम्) {
  uint64_t Pos = tell();
  seek(विस्थापनम्);
  लिख(Ptr, आकार);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_बफर_आकार() ध्रुव {
#यदि defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  यदि (IsWindowsConsole)
    प्रत्यागम 0;
  प्रत्यागम raw_ostream::preferred_बफर_आकार();
#नोचेद्यदि defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  प्रत्यागम raw_ostream::preferred_बफर_आकार();
#अन्यथा
  प्रतिज्ञा(FD >= 0 && "File not yet open!");
  संरचना stat statbuf;
  यदि (fstat(FD, &statbuf) != 0)
    प्रत्यागम 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  यदि (S_ISCHR(statbuf.st_बहुलकम्) && is_displayed())
    प्रत्यागम 0;
  // Return the preferred block size.
  प्रत्यागम statbuf.st_blksize;
#यदि_अन्तः
}

तार्किक raw_fd_ostream::is_displayed() ध्रुव {
  प्रत्यागम sys::Process::FileDescriptorIsDisplayed(FD);
}

तार्किक raw_fd_ostream::has_colors() ध्रुव {
  यदि (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  प्रत्यागम *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::तालकम्() {
  std::error_संहिता EC = sys::fs::तालकम्_File(FD);
  यदि (!EC)
    प्रत्यागम sys::fs::FileLocker(FD);
  प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_तालकम्_For(Duration ध्रुव& Timeout) {
  std::error_संहिता EC = sys::fs::try_तालकम्_File(FD, Timeout.getDuration());
  यदि (!EC)
    प्रत्यागम sys::fs::FileLocker(FD);
  प्रत्यागम error_संहिता_गन्तव्यम्_Error(EC);
}

शून्य raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_संहिता EC;

  // On z/OS we need to enable auto conversion
  स्थिर std::error_संहिता EC1 = enableAutoConversion(STDOUT_FILENO);
  प्रतिज्ञा(!EC1);
  (शून्य)EC1;

  स्थिर raw_fd_ostream S("-", EC, sys::fs::सम्बन्धि_None);
  प्रतिज्ञा(!EC);
  प्रत्यागम S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  स्थिर std::error_संहिता EC = enableAutoConversion(STDERR_FILENO);
  प्रतिज्ञा(!EC);
  (शून्य)EC;

  // Set standard error to be unbuffered.
  स्थिर raw_fd_ostream S(STDERR_FILENO, असत्य, सत्य);
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

raw_fd_प्रवाहः::raw_fd_प्रवाहः(StringRef Filename, std::error_संहिता &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::सम्बन्धि_None),
                     सत्य, असत्य, OStream_विधा::OK_FDStream) {
  यदि (EC)
    प्रत्यागम;

  यदि (!isRegularFile())
    EC = std::make_error_संहिता(std::errc::invalid_तर्कः);
}

raw_fd_प्रवाहः::raw_fd_प्रवाहः(पूर्णाङ्क fd, तार्किक shouldClose)
    : raw_fd_ostream(fd, shouldClose, असत्य, OStream_विधा::OK_FDStream) {}

ssize_t raw_fd_प्रवाहः::पठ(वर्ण *Ptr, size_t आकार) {
  प्रतिज्ञा(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::पठ(get_fd(), (शून्य *)Ptr, आकार);
  यदि (Ret >= 0)
    inc_pos(Ret);
  अन्यथा
    error_detected(errno_As_Error_संहिता());
  प्रत्यागम Ret;
}

तार्किक raw_fd_प्रवाहः::classof(ध्रुव raw_ostream *OS) {
  प्रत्यागम OS->get_विधा_2() == OStream_विधा::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

शून्य raw_string_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t आकार) {
  OS.append(Ptr, आकार);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ध्रुव { प्रत्यागम OS.आकार_2(); }

शून्य raw_svector_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t आकार) {
  OS.append(Ptr, Ptr + आकार);
}

शून्य raw_svector_ostream::pwrite_impl(ध्रुव वर्ण *Ptr, size_t आकार,
                                      uint64_t विस्थापनम्) {
  स्मृतिं_प्रतिलिख(OS.दत्त() + विस्थापनम्, Ptr, आकार);
}

तार्किक raw_svector_ostream::classof(ध्रुव raw_ostream *OS) {
  प्रत्यागम OS->get_विधा_2() == OStream_विधा::OK_SVec_प्रवाहः;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#यदि_अपरिभाषितम् NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#यदि_अन्तः
}

शून्य raw_null_ostream::write_impl(ध्रुव वर्ण *Ptr, size_t आकार) {
}

uint64_t raw_null_ostream::current_pos() ध्रुव {
  प्रत्यागम 0;
}

शून्य raw_null_ostream::pwrite_impl(ध्रुव वर्ण *Ptr, size_t आकार,
                                   uint64_t विस्थापनम्) {}

शून्य raw_pwrite_प्रवाहः::anchor() {}

शून्य बफर_ostream::anchor() {}

शून्य बफर_unique_ostream::anchor() {}

स्थिर तार्किक isNullDeviceName(StringRef OutputFileName) {
#यदि_परिभाषितम् _WIN32
  यदि (OutputFileName.equals_insensitive("NUL"))
    प्रत्यागम सत्य;
#यदि_अन्तः
  प्रत्यागम OutputFileName == "/dev/null";
}

Error llvm::write_गन्तव्यम्_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  यदि (OutputFileName == "-")
    प्रत्यागम Write(outs());

  यदि (isNullDeviceName(OutputFileName)) {
    raw_null_ostream बहिर्गमनम्;
    प्रत्यागम Write(बहिर्गमनम्);
  }

  अचिह्नित बहुलकम् = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::अस्थायी_File> अस्थायी =
      sys::fs::अस्थायी_File::create(OutputFileName + ".temp-stream-%%%%%%", बहुलकम्);
  यदि (!अस्थायी)
    प्रत्यागम createFileError(OutputFileName, अस्थायी.takeError());

#यदि defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  यदि (स्वचालित EC = llvm::copy_File_चिह्नकम्_गुणाः(OutputFileName.str(), अस्थायी->FD)) {
    यदि (EC != std::errc::no_such_file_or_directory)
      प्रत्यागम createFileError(OutputFileName, EC);
  }
#यदि_अन्तः

  raw_fd_ostream बहिर्गमनम्(अस्थायी->FD, असत्य);

  यदि (Error E = Write(बहिर्गमनम्)) {
    यदि (Error DiscardError = अस्थायी->discard())
      प्रत्यागम joinErrors(std::move(E), std::move(DiscardError));
    प्रत्यागम E;
  }
  बहिर्गमनम्.flush();

  प्रत्यागम अस्थायी->keep(OutputFileName);
}
