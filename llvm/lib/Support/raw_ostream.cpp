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
#د_که_پای

#إذا defined(__CYGWIN__)
#ضمن <io.h>
#د_که_پای

#إذا defined(_MSC_VER)
#ضمن <io.h>
#که_نه_تعریف_شوی STDIN_FILENO
# define STDIN_FILENO 0
#د_که_پای
#که_نه_تعریف_شوی STDOUT_FILENO
# define STDOUT_FILENO 1
#د_که_پای
#که_نه_تعریف_شوی STDERR_FILENO
# define STDERR_FILENO 2
#د_که_پای
#د_که_پای

#که_تعریف_شوی _WIN32
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#د_که_پای

کارول نطاق llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  أكد(بهر_Buf_Cur == بهر_Buf_پیل &&
         "raw_ostream destructor called with non-empty buffer!");

  إذا (بفر_ډېرپېښ == بفر_ډول::دننی_بفر)
    ړنګول [] بهر_Buf_پیل;
}

size_t raw_ostream::preferred_بفر_اندازه() ثابت {
#که_تعریف_شوی _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  أرجع (16 * 1024);
#وإلا
  // BUFSIZ is intended to be a reasonable default.
  أرجع BUFSIZ;
#د_که_پای
}

فراغ raw_ostream::سېټ_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  إذا (size_t اندازه = preferred_بفر_اندازه())
    سېټ_بفر_اندازه(اندازه);
  وإلا
    // It may return 0, meaning this stream should be unbuffered.
    سېټ_Unbuffered();
}

فراغ raw_ostream::سېټ_بفر_And_ډېرپېښ(حرف *بفر_پیل, size_t اندازه,
                                   بفر_ډول ډېرپېښ) {
  أكد(((ډېرپېښ == بفر_ډول::Unbuffered && !بفر_پیل && اندازه == 0) ||
          (ډېرپېښ != بفر_ډول::Unbuffered && بفر_پیل && اندازه != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  أكد(Get_عدد_بایټونه_دننه_بفر() == 0 && "Current buffer is non-empty!");

  إذا (بفر_ډېرپېښ == بفر_ډول::دننی_بفر)
    ړنګول [] بهر_Buf_پیل;
  بهر_Buf_پیل = بفر_پیل;
  بهر_Buf_پای_ټکی = بهر_Buf_پیل+اندازه;
  بهر_Buf_Cur = بهر_Buf_پیل;
  بفر_ډېرپېښ = ډېرپېښ;

  أكد(بهر_Buf_پیل <= بهر_Buf_پای_ټکی && "Invalid size!");
}

raw_ostream &raw_ostream::چلوونکی<<(بینښه اوږد N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(اوږد N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(بینښه اوږد اوږد N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(اوږد اوږد N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::write_hex(بینښه اوږد اوږد N) {
  llvm::write_hex(*هذا, N, HexPrintStyle::ښکتنی_2);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(Colors C) {
  إذا (C == Colors::RESET)
    resetColor();
  وإلا
    changeColor(C);
  أرجع *هذا;
}

raw_ostream &raw_ostream::write_uuid(ثابت uuid_t UUID) {
  لكل (صحيح Idx = 0; Idx < 16; ++Idx) {
    *هذا << format("%02" PRIX32, UUID[Idx]);
    إذا (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *هذا << "-";
  }
  أرجع *هذا;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        منطقي UseHexEscapes) {
  لكل (بینښه حرف c : Str) {
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

  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(ثابت فراغ *P) {
  llvm::write_hex(*هذا, (uintptr_t)P, HexPrintStyle::مختاړی_ښکتنی);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(مزدوج N) {
  llvm::write_double(*هذا, N, FloatStyle::Exponent);
  أرجع *هذا;
}

فراغ raw_ostream::flush_nonempty() {
  أكد(بهر_Buf_Cur > بهر_Buf_پیل && "Invalid call to flush_nonempty.");
  size_t اوږدتیا = بهر_Buf_Cur - بهر_Buf_پیل;
  بهر_Buf_Cur = بهر_Buf_پیل;
  write_impl(بهر_Buf_پیل, اوږدتیا);
}

raw_ostream &raw_ostream::اكتب(بینښه حرف C) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(بهر_Buf_Cur >= بهر_Buf_پای_ټکی)) {
    إذا (LLVM_UNLIKELY(!بهر_Buf_پیل)) {
      إذا (بفر_ډېرپېښ == بفر_ډول::Unbuffered) {
        write_impl(reinterpret_cast<حرف *>(&C), 1);
        أرجع *هذا;
      }
      // Set up a buffer and start over.
      سېټ_Buffered();
      أرجع اكتب(C);
    }

    flush_nonempty();
  }

  *بهر_Buf_Cur++ = C;
  أرجع *هذا;
}

raw_ostream &raw_ostream::اكتب(ثابت حرف *Ptr, size_t اندازه) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(size_t(بهر_Buf_پای_ټکی - بهر_Buf_Cur) < اندازه)) {
    إذا (LLVM_UNLIKELY(!بهر_Buf_پیل)) {
      إذا (بفر_ډېرپېښ == بفر_ډول::Unbuffered) {
        write_impl(Ptr, اندازه);
        أرجع *هذا;
      }
      // Set up a buffer and start over.
      سېټ_Buffered();
      أرجع اكتب(Ptr, اندازه);
    }

    size_t عدد_بایټونه = بهر_Buf_پای_ټکی - بهر_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    إذا (LLVM_UNLIKELY(بهر_Buf_Cur == بهر_Buf_پیل)) {
      أكد(عدد_بایټونه != 0 && "undefined behavior");
      size_t بایټونه_منزل_Write_2 = اندازه - (اندازه % عدد_بایټونه);
      write_impl(Ptr, بایټونه_منزل_Write_2);
      size_t بایټونه_پاتې_شوی_2 = اندازه - بایټونه_منزل_Write_2;
      إذا (بایټونه_پاتې_شوی_2 > size_t(بهر_Buf_پای_ټکی - بهر_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        أرجع اكتب(Ptr + بایټونه_منزل_Write_2, بایټونه_پاتې_شوی_2);
      }
      copy_منزل_بفر(Ptr + بایټونه_منزل_Write_2, بایټونه_پاتې_شوی_2);
      أرجع *هذا;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_منزل_بفر(Ptr, عدد_بایټونه);
    flush_nonempty();
    أرجع اكتب(Ptr + عدد_بایټونه, اندازه - عدد_بایټونه);
  }

  copy_منزل_بفر(Ptr, اندازه);

  أرجع *هذا;
}

فراغ raw_ostream::copy_منزل_بفر(ثابت حرف *Ptr, size_t اندازه) {
  أكد(اندازه <= size_t(بهر_Buf_پای_ټکی - بهر_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  بدل (اندازه) {
  حالة 4: بهر_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  حالة 3: بهر_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  حالة 2: بهر_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  حالة 1: بهر_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  حالة 0: اكسر;
  افتراضي:
    حافظه_نقل(بهر_Buf_Cur, Ptr, اندازه);
    اكسر;
  }

  بهر_Buf_Cur += اندازه;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::چلوونکی<<(function_ref<صحيح(حرف *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t اندازه = 128;
  size_t بفر_بایټونه_کیڼ = بهر_Buf_پای_ټکی - بهر_Buf_Cur;
  إذا (بفر_بایټونه_کیڼ > 3) {
    صحيح N = Snprint(بهر_Buf_Cur, بفر_بایټونه_کیڼ);
    إذا (N < 0)
      أرجع *هذا;
    // Common case is that we have plenty of space.
    إذا (size_t(N) < بفر_بایټونه_کیڼ) {
      بهر_Buf_Cur += N;
      أرجع *هذا;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    اندازه = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_ویکټور<حرف, 128> V;
  لكل (;;) {
    V.resize(اندازه);
    صحيح N = Snprint(V.معلومات(), اندازه);
    إذا (N < 0)
      أرجع *هذا;
    إذا (size_t(N) < اندازه)
      أرجع اكتب(V.معلومات(), N);
    اندازه = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::چلوونکی<<(ثابت formatv_object_بنسټ &Obj) {
  Obj.format(*هذا);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(ثابت FormattedString &FS) {
  بینښه کیڼ_Indent = 0;
  بینښه ښی_Indent = 0;
  ثابت ssize_t Difference = FS.پلنوالی - FS.Str.اندازه_2();
  إذا (Difference > 0) {
    بدل (FS.Justify) {
    حالة FormattedString::JustifyNone:
      اكسر;
    حالة FormattedString::Justify_کیڼ:
      ښی_Indent = Difference;
      اكسر;
    حالة FormattedString::Justify_ښی:
      کیڼ_Indent = Difference;
      اكسر;
    حالة FormattedString::JustifyCenter:
      کیڼ_Indent = Difference / 2;
      ښی_Indent = Difference - کیڼ_Indent;
      اكسر;
    }
  }
  indent(کیڼ_Indent);
  (*هذا) << FS.Str;
  indent(ښی_Indent);
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(ثابت Formatted_شمېره &FN) {
  إذا (FN.Hex) {
    HexPrintStyle Style;
    إذا (FN.پاسنی_2 && FN.Hex_مختاړی)
      Style = HexPrintStyle::مختاړی_پاسنی;
    وإلا إذا (FN.پاسنی_2 && !FN.Hex_مختاړی)
      Style = HexPrintStyle::پاسنی_2;
    وإلا إذا (!FN.پاسنی_2 && FN.Hex_مختاړی)
      Style = HexPrintStyle::مختاړی_ښکتنی;
    وإلا
      Style = HexPrintStyle::ښکتنی_2;
    llvm::write_hex(*هذا, FN.Hex_ارزښت, Style, FN.پلنوالی);
  } وإلا {
    llvm::SmallString<16> بفر;
    llvm::raw_svector_ostream بهیر(بفر);
    llvm::write_integer(بهیر, FN.Dec_ارزښت, 0, IntegerStyle::Integer);
    إذا (بفر.اندازه_2() < FN.پلنوالی)
      indent(FN.پلنوالی - بفر.اندازه_2());
    (*هذا) << بفر;
  }
  أرجع *هذا;
}

raw_ostream &raw_ostream::چلوونکی<<(ثابت Formatted_بایټونه &FB) {
  إذا (FB.بایټونه_2.تشه())
    أرجع *هذا;

  size_t کرښه_شاخص = 0;
  خپلکاری بایټونه_2 = FB.بایټونه_2;
  ثابت size_t اندازه = بایټونه_2.اندازه_2();
  HexPrintStyle HPS = FB.پاسنی_2 ? HexPrintStyle::پاسنی_2 : HexPrintStyle::ښکتنی_2;
  uint64_t دننه_واټن_پلنوالی = 0;
  إذا (FB.لومړی_Byte_دننه_واټن) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t کرښې = اندازه / FB.عدد_فی_کرښه;
    uint64_t Max_دننه_واټن = *FB.لومړی_Byte_دننه_واټن + کرښې * FB.عدد_فی_کرښه;
    بینښه Power = 0;
    إذا (Max_دننه_واټن > 0)
      Power = llvm::Log2_64_Ceil(Max_دننه_واټن);
    دننه_واټن_پلنوالی = std::max<uint64_t>(4, llvm::align_منزل(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  بینښه عدد_Byte_Groups =
      align_منزل(FB.عدد_فی_کرښه, FB.Byte_ډله_اندازه) / FB.Byte_ډله_اندازه;
  بینښه بلاک_Char_پلنوالی = FB.عدد_فی_کرښه * 2 + عدد_Byte_Groups - 1;

  طالما (!بایټونه_2.تشه()) {
    indent(FB.IndentLevel);

    إذا (FB.لومړی_Byte_دننه_واټن) {
      uint64_t دننه_واټن = *FB.لومړی_Byte_دننه_واټن;
      llvm::write_hex(*هذا, دننه_واټن + کرښه_شاخص, HPS, دننه_واټن_پلنوالی);
      *هذا << ": ";
    }

    خپلکاری کرښه = بایټونه_2.take_front(FB.عدد_فی_کرښه);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    لكل (size_t I = 0; I < کرښه.اندازه_2(); ++I, CharsPrinted += 2) {
      إذا (I && (I % FB.Byte_ډله_اندازه) == 0) {
        ++CharsPrinted;
        *هذا << " ";
      }
      llvm::write_hex(*هذا, کرښه[I], HPS, 2);
    }

    إذا (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(بلاک_Char_پلنوالی >= CharsPrinted);
      indent(بلاک_Char_پلنوالی - CharsPrinted + 2);
      *هذا << "|";

      // Print the ASCII char values for each byte on this line
      لكل (uint8_t Byte : کرښه) {
        إذا (isPrint(Byte))
          *هذا << static_cast<حرف>(Byte);
        وإلا
          *هذا << '.';
      }
      *هذا << '|';
    }

    بایټونه_2 = بایټونه_2.drop_front(کرښه.اندازه_2());
    کرښه_شاخص += کرښه.اندازه_2();
    إذا (کرښه_شاخص < اندازه)
      *هذا << '\n';
  }
  أرجع *هذا;
}

کينډۍ <حرف C>
ساكن raw_ostream &write_padding(raw_ostream &OS, بینښه عدد_Chars) {
  ساكن ثابت حرف Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  إذا (عدد_Chars < std::اندازه_2(Chars))
    أرجع OS.اكتب(Chars, عدد_Chars);

  طالما (عدد_Chars) {
    بینښه عدد_منزل_Write = std::min(عدد_Chars, (بینښه)std::اندازه_2(Chars) - 1);
    OS.اكتب(Chars, عدد_منزل_Write);
    عدد_Chars -= عدد_منزل_Write;
  }
  أرجع OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(بینښه عدد_Spaces) {
  أرجع write_padding<' '>(*هذا, عدد_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(بینښه عدد_Zeros) {
  أرجع write_padding<'\0'>(*هذا, عدد_Zeros);
}

منطقي raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  إذا (!ColorEnabled)
    أرجع خطأ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  إذا (sys::Process::ColorNeedsFlush() && !is_displayed())
    أرجع خطأ;

  إذا (sys::Process::ColorNeedsFlush())
    flush();

  أرجع ریښتیا;
}

raw_ostream &raw_ostream::changeColor(تعداد Colors colors, منطقي bold, منطقي bg) {
  إذا (!prepare_colors())
    أرجع *هذا;

  ثابت حرف *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<حرف>(colors), bold, bg);
  إذا (colorcode)
    اكتب(colorcode, مزي_اوږدوالی(colorcode));
  أرجع *هذا;
}

raw_ostream &raw_ostream::resetColor() {
  إذا (!prepare_colors())
    أرجع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, مزي_اوږدوالی(colorcode));
  أرجع *هذا;
}

raw_ostream &raw_ostream::سرچپه_Color() {
  إذا (!prepare_colors())
    أرجع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::Output_سرچپه())
    اكتب(colorcode, مزي_اوږدوالی(colorcode));
  أرجع *هذا;
}

فراغ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ساكن صحيح getFD(StringRef Filename, std::error_کوډ &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  أكد((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  إذا (Filename == "-") {
    EC = std::error_کوډ();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_ډېرپېښ(Flags);
    أرجع STDOUT_FILENO;
  }

  صحيح FD;
  إذا (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  وإلا
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  إذا (EC)
    أرجع -1;

  أرجع FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کوډ &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::د_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کوډ &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::د_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کوډ &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::د_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کوډ &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کوډ &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), ریښتیا) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(صحيح fd, منطقي shouldClose, منطقي unbuffered,
                               OStream_ډول K)
    : raw_pwrite_بهیر(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  إذا (FD < 0 ) {
    ShouldClose = خطأ;
    أرجع;
  }

  enable_colors(ریښتیا);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  إذا (FD <= STDERR_FILENO)
    ShouldClose = خطأ;

#که_تعریف_شوی _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#د_که_پای

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_حالت حالت;
  std::error_کوډ EC = حالت_2(FD, حالت);
  IsRegularFile = حالت.type() == sys::fs::file_type::regular_file;
#که_تعریف_شوی _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#وإلا
  SupportsSeeking = !EC && loc != (off_t)-1;
#د_که_پای
  إذا (!SupportsSeeking)
    pos = 0;
  وإلا
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  إذا (FD >= 0) {
    flush();
    إذا (ShouldClose) {
      إذا (خپلکاری EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#که_تعریف_شوی __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  إذا (FD == 2) أرجع;
#د_که_پای

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  إذا (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().پیغام_2());
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
ساكن منطقي write_console_impl(صحيح FD, StringRef معلومات_2) {
  Small_ویکټور<wchar_t, 256> Wide_متن;

  // Fall back to ::write if it wasn't valid UTF-8.
  إذا (خپلکاری EC = sys::windows::UTF8_منزل_UTF16(معلومات_2, Wide_متن))
    أرجع خطأ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_اندازه = Wide_متن.اندازه_2();
  إذا (!RunningWindows8OrGreater())
    Max_Write_اندازه = 32767;

  size_t WCharsWritten = 0;
  افعل {
    size_t WChars_منزل_Write =
        std::min(Max_Write_اندازه, Wide_متن.اندازه_2() - WCharsWritten);
    DWORD ActuallyWritten;
    منطقي Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_متن[WCharsWritten],
                        WChars_منزل_Write, &ActuallyWritten,
                        /*Reserved=*/تش_اشاره);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    إذا (!Success)
      أرجع خطأ;

    WCharsWritten += ActuallyWritten;
  } طالما (WCharsWritten != Wide_متن.اندازه_2());
  أرجع ریښتیا;
}
#د_که_پای

فراغ raw_fd_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  إذا (Tied_بهیر)
    Tied_بهیر->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += اندازه;

#إذا defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  إذا (IsWindowsConsole)
    إذا (write_console_impl(FD, StringRef(Ptr, اندازه)))
      أرجع;
#د_که_پای

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_اندازه = INT32_MAX;

#إذا defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_اندازه = 1024 * 1024 * 1024;
#د_که_پای

  افعل {
    size_t Chunk_اندازه = std::min(اندازه, Max_Write_اندازه);
    ssize_t ret = ::اكتب(FD, Ptr, Chunk_اندازه);

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
#که_تعریف_شوی EWOULDBLOCK
          || errno == EWOULDBLOCK
#د_که_پای
          )
        استمر;

#که_تعریف_شوی _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_وروستی_Error = Get_وروستی_Error();
      إذا (Win_وروستی_Error == ERROR_BROKEN_PIPE ||
          (Win_وروستی_Error == ERROR_NO_معلومات && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#د_که_پای
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_کوډ());
      اكسر;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    اندازه -= ret;
  } طالما (اندازه > 0);
}

فراغ raw_fd_ostream::close() {
  أكد(ShouldClose);
  ShouldClose = خطأ;
  flush();
  إذا (خپلکاری EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#که_تعریف_شوی _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#وإلا
  pos = ::lseek(FD, off, SEEK_SET);
#د_که_پای
  إذا (pos == (uint64_t)-1)
    error_detected(errno_As_Error_کوډ());
  أرجع pos;
}

فراغ raw_fd_ostream::pwrite_impl(ثابت حرف *Ptr, size_t اندازه,
                                 uint64_t دننه_واټن) {
  uint64_t Pos = tell();
  seek(دننه_واټن);
  اكتب(Ptr, اندازه);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_بفر_اندازه() ثابت {
#إذا defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  إذا (IsWindowsConsole)
    أرجع 0;
  أرجع raw_ostream::preferred_بفر_اندازه();
#وإلاإذا defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  أرجع raw_ostream::preferred_بفر_اندازه();
#وإلا
  أكد(FD >= 0 && "File not yet open!");
  بنية stat statbuf;
  إذا (fstat(FD, &statbuf) != 0)
    أرجع 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  إذا (S_ISCHR(statbuf.st_ډېرپېښ) && is_displayed())
    أرجع 0;
  // Return the preferred block size.
  أرجع statbuf.st_blksize;
#د_که_پای
}

منطقي raw_fd_ostream::is_displayed() ثابت {
  أرجع sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقي raw_fd_ostream::has_colors() ثابت {
  إذا (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  أرجع *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::قلف() {
  std::error_کوډ EC = sys::fs::قلف_File(FD);
  إذا (!EC)
    أرجع sys::fs::FileLocker(FD);
  أرجع error_کوډ_منزل_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_قلف_For(Duration ثابت& Timeout) {
  std::error_کوډ EC = sys::fs::try_قلف_File(FD, Timeout.getDuration());
  إذا (!EC)
    أرجع sys::fs::FileLocker(FD);
  أرجع error_کوډ_منزل_Error(EC);
}

فراغ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_کوډ EC;

  // On z/OS we need to enable auto conversion
  ساكن std::error_کوډ EC1 = enableAutoConversion(STDOUT_FILENO);
  أكد(!EC1);
  (فراغ)EC1;

  ساكن raw_fd_ostream S("-", EC, sys::fs::د_None);
  أكد(!EC);
  أرجع S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ساكن std::error_کوډ EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (فراغ)EC;

  // Set standard error to be unbuffered.
  ساكن raw_fd_ostream S(STDERR_FILENO, خطأ, ریښتیا);
  أرجع S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ساكن raw_null_ostream S;
  أرجع S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_بهیر::raw_fd_بهیر(StringRef Filename, std::error_کوډ &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::د_None),
                     ریښتیا, خطأ, OStream_ډول::OK_FDStream) {
  إذا (EC)
    أرجع;

  إذا (!isRegularFile())
    EC = std::make_error_کوډ(std::errc::invalid_دلیل);
}

raw_fd_بهیر::raw_fd_بهیر(صحيح fd, منطقي shouldClose)
    : raw_fd_ostream(fd, shouldClose, خطأ, OStream_ډول::OK_FDStream) {}

ssize_t raw_fd_بهیر::اقرأ(حرف *Ptr, size_t اندازه) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (فراغ *)Ptr, اندازه);
  إذا (Ret >= 0)
    inc_pos(Ret);
  وإلا
    error_detected(errno_As_Error_کوډ());
  أرجع Ret;
}

منطقي raw_fd_بهیر::classof(ثابت raw_ostream *OS) {
  أرجع OS->get_ډول_2() == OStream_ډول::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

فراغ raw_string_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  OS.append(Ptr, اندازه);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ثابت { أرجع OS.اندازه_2(); }

فراغ raw_svector_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  OS.append(Ptr, Ptr + اندازه);
}

فراغ raw_svector_ostream::pwrite_impl(ثابت حرف *Ptr, size_t اندازه,
                                      uint64_t دننه_واټن) {
  حافظه_نقل(OS.معلومات() + دننه_واټن, Ptr, اندازه);
}

منطقي raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  أرجع OS->get_ډول_2() == OStream_ډول::OK_SVec_بهیر;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#که_نه_تعریف_شوی NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#د_که_پای
}

فراغ raw_null_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
}

uint64_t raw_null_ostream::current_pos() ثابت {
  أرجع 0;
}

فراغ raw_null_ostream::pwrite_impl(ثابت حرف *Ptr, size_t اندازه,
                                   uint64_t دننه_واټن) {}

فراغ raw_pwrite_بهیر::anchor() {}

فراغ بفر_ostream::anchor() {}

فراغ بفر_unique_ostream::anchor() {}

ساكن منطقي isNullDeviceName(StringRef OutputFileName) {
#که_تعریف_شوی _WIN32
  إذا (OutputFileName.equals_insensitive("NUL"))
    أرجع ریښتیا;
#د_که_پای
  أرجع OutputFileName == "/dev/null";
}

Error llvm::write_منزل_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  إذا (OutputFileName == "-")
    أرجع Write(outs());

  إذا (isNullDeviceName(OutputFileName)) {
    raw_null_ostream بهر;
    أرجع Write(بهر);
  }

  بینښه ډېرپېښ = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::لنډمهاله_File> لنډمهاله =
      sys::fs::لنډمهاله_File::create(OutputFileName + ".temp-stream-%%%%%%", ډېرپېښ);
  إذا (!لنډمهاله)
    أرجع createFileError(OutputFileName, لنډمهاله.takeError());

#إذا defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  إذا (خپلکاری EC = llvm::copy_File_نښان_ځانګړتیاوې(OutputFileName.str(), لنډمهاله->FD)) {
    إذا (EC != std::errc::no_such_file_or_directory)
      أرجع createFileError(OutputFileName, EC);
  }
#د_که_پای

  raw_fd_ostream بهر(لنډمهاله->FD, خطأ);

  إذا (Error E = Write(بهر)) {
    إذا (Error DiscardError = لنډمهاله->discard())
      أرجع joinErrors(std::move(E), std::move(DiscardError));
    أرجع E;
  }
  بهر.flush();

  أرجع لنډمهاله->keep(OutputFileName);
}
