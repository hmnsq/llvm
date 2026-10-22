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
#پایان_اگر

#إذا defined(__CYGWIN__)
#ضمن <io.h>
#پایان_اگر

#إذا defined(_MSC_VER)
#ضمن <io.h>
#اگر_تعریف_نشده STDIN_FILENO
# define STDIN_FILENO 0
#پایان_اگر
#اگر_تعریف_نشده STDOUT_FILENO
# define STDOUT_FILENO 1
#پایان_اگر
#اگر_تعریف_نشده STDERR_FILENO
# define STDERR_FILENO 2
#پایان_اگر
#پایان_اگر

#اگر_تعریف_شده _WIN32
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#پایان_اگر

استفاده نطاق llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  أكد(بیرون_Buf_Cur == بیرون_Buf_شروع &&
         "raw_ostream destructor called with non-empty buffer!");

  إذا (میانگیر_نما == میانگیر_نوع::داخلی_میانگیر)
    حذف [] بیرون_Buf_شروع;
}

size_t raw_ostream::preferred_میانگیر_اندازه() ثابت {
#اگر_تعریف_شده _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  أرجع (16 * 1024);
#وإلا
  // BUFSIZ is intended to be a reasonable default.
  أرجع BUFSIZ;
#پایان_اگر
}

فراغ raw_ostream::مجموعه_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  إذا (size_t اندازه = preferred_میانگیر_اندازه())
    مجموعه_میانگیر_اندازه(اندازه);
  وإلا
    // It may return 0, meaning this stream should be unbuffered.
    مجموعه_Unbuffered();
}

فراغ raw_ostream::مجموعه_میانگیر_And_نما(حرف *میانگیر_شروع, size_t اندازه,
                                   میانگیر_نوع نما) {
  أكد(((نما == میانگیر_نوع::Unbuffered && !میانگیر_شروع && اندازه == 0) ||
          (نما != میانگیر_نوع::Unbuffered && میانگیر_شروع && اندازه != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  أكد(Get_رقم_بایتها_درون_میانگیر() == 0 && "Current buffer is non-empty!");

  إذا (میانگیر_نما == میانگیر_نوع::داخلی_میانگیر)
    حذف [] بیرون_Buf_شروع;
  بیرون_Buf_شروع = میانگیر_شروع;
  بیرون_Buf_انتها = بیرون_Buf_شروع+اندازه;
  بیرون_Buf_Cur = بیرون_Buf_شروع;
  میانگیر_نما = نما;

  أكد(بیرون_Buf_شروع <= بیرون_Buf_انتها && "Invalid size!");
}

raw_ostream &raw_ostream::عملگر<<(بیعلامت دراز N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(دراز N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(بیعلامت دراز دراز N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(دراز دراز N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::write_hex(بیعلامت دراز دراز N) {
  llvm::write_hex(*هذا, N, HexPrintStyle::پایینی_2);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(Colors C) {
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
  لكل (بیعلامت حرف c : Str) {
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

raw_ostream &raw_ostream::عملگر<<(ثابت فراغ *P) {
  llvm::write_hex(*هذا, (uintptr_t)P, HexPrintStyle::پیشوند_پایینی);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(مزدوج N) {
  llvm::write_double(*هذا, N, FloatStyle::Exponent);
  أرجع *هذا;
}

فراغ raw_ostream::flush_nonempty() {
  أكد(بیرون_Buf_Cur > بیرون_Buf_شروع && "Invalid call to flush_nonempty.");
  size_t درازی = بیرون_Buf_Cur - بیرون_Buf_شروع;
  بیرون_Buf_Cur = بیرون_Buf_شروع;
  write_impl(بیرون_Buf_شروع, درازی);
}

raw_ostream &raw_ostream::اكتب(بیعلامت حرف C) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(بیرون_Buf_Cur >= بیرون_Buf_انتها)) {
    إذا (LLVM_UNLIKELY(!بیرون_Buf_شروع)) {
      إذا (میانگیر_نما == میانگیر_نوع::Unbuffered) {
        write_impl(reinterpret_cast<حرف *>(&C), 1);
        أرجع *هذا;
      }
      // Set up a buffer and start over.
      مجموعه_Buffered();
      أرجع اكتب(C);
    }

    flush_nonempty();
  }

  *بیرون_Buf_Cur++ = C;
  أرجع *هذا;
}

raw_ostream &raw_ostream::اكتب(ثابت حرف *Ptr, size_t اندازه) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(size_t(بیرون_Buf_انتها - بیرون_Buf_Cur) < اندازه)) {
    إذا (LLVM_UNLIKELY(!بیرون_Buf_شروع)) {
      إذا (میانگیر_نما == میانگیر_نوع::Unbuffered) {
        write_impl(Ptr, اندازه);
        أرجع *هذا;
      }
      // Set up a buffer and start over.
      مجموعه_Buffered();
      أرجع اكتب(Ptr, اندازه);
    }

    size_t رقم_بایتها = بیرون_Buf_انتها - بیرون_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    إذا (LLVM_UNLIKELY(بیرون_Buf_Cur == بیرون_Buf_شروع)) {
      أكد(رقم_بایتها != 0 && "undefined behavior");
      size_t بایتها_مقصد_Write_2 = اندازه - (اندازه % رقم_بایتها);
      write_impl(Ptr, بایتها_مقصد_Write_2);
      size_t بایتها_مانده_2 = اندازه - بایتها_مقصد_Write_2;
      إذا (بایتها_مانده_2 > size_t(بیرون_Buf_انتها - بیرون_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        أرجع اكتب(Ptr + بایتها_مقصد_Write_2, بایتها_مانده_2);
      }
      copy_مقصد_میانگیر(Ptr + بایتها_مقصد_Write_2, بایتها_مانده_2);
      أرجع *هذا;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_مقصد_میانگیر(Ptr, رقم_بایتها);
    flush_nonempty();
    أرجع اكتب(Ptr + رقم_بایتها, اندازه - رقم_بایتها);
  }

  copy_مقصد_میانگیر(Ptr, اندازه);

  أرجع *هذا;
}

فراغ raw_ostream::copy_مقصد_میانگیر(ثابت حرف *Ptr, size_t اندازه) {
  أكد(اندازه <= size_t(بیرون_Buf_انتها - بیرون_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  بدل (اندازه) {
  حالة 4: بیرون_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  حالة 3: بیرون_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  حالة 2: بیرون_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  حالة 1: بیرون_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  حالة 0: اكسر;
  افتراضي:
    کپی_حافظه(بیرون_Buf_Cur, Ptr, اندازه);
    اكسر;
  }

  بیرون_Buf_Cur += اندازه;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::عملگر<<(function_ref<صحيح(حرف *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t اندازه = 128;
  size_t میانگیر_بایتها_چپ = بیرون_Buf_انتها - بیرون_Buf_Cur;
  إذا (میانگیر_بایتها_چپ > 3) {
    صحيح N = Snprint(بیرون_Buf_Cur, میانگیر_بایتها_چپ);
    إذا (N < 0)
      أرجع *هذا;
    // Common case is that we have plenty of space.
    إذا (size_t(N) < میانگیر_بایتها_چپ) {
      بیرون_Buf_Cur += N;
      أرجع *هذا;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    اندازه = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_وکتور<حرف, 128> V;
  لكل (;;) {
    V.resize(اندازه);
    صحيح N = Snprint(V.داده(), اندازه);
    إذا (N < 0)
      أرجع *هذا;
    إذا (size_t(N) < اندازه)
      أرجع اكتب(V.داده(), N);
    اندازه = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::عملگر<<(ثابت formatv_object_پایه &Obj) {
  Obj.format(*هذا);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(ثابت FormattedString &FS) {
  بیعلامت چپ_Indent = 0;
  بیعلامت راست_Indent = 0;
  ثابت ssize_t Difference = FS.عرض - FS.Str.اندازه_2();
  إذا (Difference > 0) {
    بدل (FS.Justify) {
    حالة FormattedString::JustifyNone:
      اكسر;
    حالة FormattedString::Justify_چپ:
      راست_Indent = Difference;
      اكسر;
    حالة FormattedString::Justify_راست:
      چپ_Indent = Difference;
      اكسر;
    حالة FormattedString::JustifyCenter:
      چپ_Indent = Difference / 2;
      راست_Indent = Difference - چپ_Indent;
      اكسر;
    }
  }
  indent(چپ_Indent);
  (*هذا) << FS.Str;
  indent(راست_Indent);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(ثابت Formatted_عدد &FN) {
  إذا (FN.Hex) {
    HexPrintStyle Style;
    إذا (FN.فوقانی_2 && FN.Hex_پیشوند)
      Style = HexPrintStyle::پیشوند_فوقانی;
    وإلا إذا (FN.فوقانی_2 && !FN.Hex_پیشوند)
      Style = HexPrintStyle::فوقانی_2;
    وإلا إذا (!FN.فوقانی_2 && FN.Hex_پیشوند)
      Style = HexPrintStyle::پیشوند_پایینی;
    وإلا
      Style = HexPrintStyle::پایینی_2;
    llvm::write_hex(*هذا, FN.Hex_مقدار, Style, FN.عرض);
  } وإلا {
    llvm::SmallString<16> میانگیر;
    llvm::raw_svector_ostream جریان(میانگیر);
    llvm::write_integer(جریان, FN.Dec_مقدار, 0, IntegerStyle::Integer);
    إذا (میانگیر.اندازه_2() < FN.عرض)
      indent(FN.عرض - میانگیر.اندازه_2());
    (*هذا) << میانگیر;
  }
  أرجع *هذا;
}

raw_ostream &raw_ostream::عملگر<<(ثابت Formatted_بایتها &FB) {
  إذا (FB.بایتها_2.خالی())
    أرجع *هذا;

  size_t سطر_شاخص = 0;
  خودکار بایتها_2 = FB.بایتها_2;
  ثابت size_t اندازه = بایتها_2.اندازه_2();
  HexPrintStyle HPS = FB.فوقانی_2 ? HexPrintStyle::فوقانی_2 : HexPrintStyle::پایینی_2;
  uint64_t جابه_جایی_عرض = 0;
  إذا (FB.اولی_Byte_جابه_جایی) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t سطرها = اندازه / FB.رقم_در_هر_سطر;
    uint64_t Max_جابه_جایی = *FB.اولی_Byte_جابه_جایی + سطرها * FB.رقم_در_هر_سطر;
    بیعلامت Power = 0;
    إذا (Max_جابه_جایی > 0)
      Power = llvm::Log2_64_Ceil(Max_جابه_جایی);
    جابه_جایی_عرض = std::max<uint64_t>(4, llvm::align_مقصد(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  بیعلامت رقم_Byte_Groups =
      align_مقصد(FB.رقم_در_هر_سطر, FB.Byte_گروه_اندازه) / FB.Byte_گروه_اندازه;
  بیعلامت بلوک_Char_عرض = FB.رقم_در_هر_سطر * 2 + رقم_Byte_Groups - 1;

  طالما (!بایتها_2.خالی()) {
    indent(FB.IndentLevel);

    إذا (FB.اولی_Byte_جابه_جایی) {
      uint64_t جابه_جایی = *FB.اولی_Byte_جابه_جایی;
      llvm::write_hex(*هذا, جابه_جایی + سطر_شاخص, HPS, جابه_جایی_عرض);
      *هذا << ": ";
    }

    خودکار سطر = بایتها_2.take_front(FB.رقم_در_هر_سطر);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    لكل (size_t I = 0; I < سطر.اندازه_2(); ++I, CharsPrinted += 2) {
      إذا (I && (I % FB.Byte_گروه_اندازه) == 0) {
        ++CharsPrinted;
        *هذا << " ";
      }
      llvm::write_hex(*هذا, سطر[I], HPS, 2);
    }

    إذا (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(بلوک_Char_عرض >= CharsPrinted);
      indent(بلوک_Char_عرض - CharsPrinted + 2);
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

    بایتها_2 = بایتها_2.drop_front(سطر.اندازه_2());
    سطر_شاخص += سطر.اندازه_2();
    إذا (سطر_شاخص < اندازه)
      *هذا << '\n';
  }
  أرجع *هذا;
}

قالب <حرف C>
ساكن raw_ostream &write_padding(raw_ostream &OS, بیعلامت رقم_Chars) {
  ساكن ثابت حرف Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  إذا (رقم_Chars < std::اندازه_2(Chars))
    أرجع OS.اكتب(Chars, رقم_Chars);

  طالما (رقم_Chars) {
    بیعلامت رقم_مقصد_Write = std::min(رقم_Chars, (بیعلامت)std::اندازه_2(Chars) - 1);
    OS.اكتب(Chars, رقم_مقصد_Write);
    رقم_Chars -= رقم_مقصد_Write;
  }
  أرجع OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(بیعلامت رقم_Spaces) {
  أرجع write_padding<' '>(*هذا, رقم_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(بیعلامت رقم_Zeros) {
  أرجع write_padding<'\0'>(*هذا, رقم_Zeros);
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

  أرجع درست;
}

raw_ostream &raw_ostream::changeColor(تعداد Colors colors, منطقي bold, منطقي bg) {
  إذا (!prepare_colors())
    أرجع *هذا;

  ثابت حرف *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<حرف>(colors), bold, bg);
  إذا (colorcode)
    اكتب(colorcode, طول_رشته(colorcode));
  أرجع *هذا;
}

raw_ostream &raw_ostream::resetColor() {
  إذا (!prepare_colors())
    أرجع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, طول_رشته(colorcode));
  أرجع *هذا;
}

raw_ostream &raw_ostream::معکوس_Color() {
  إذا (!prepare_colors())
    أرجع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::Output_معکوس())
    اكتب(colorcode, طول_رشته(colorcode));
  أرجع *هذا;
}

فراغ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ساكن صحيح getFD(StringRef Filename, std::error_کد &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  إذا (Filename == "-") {
    EC = std::error_کد();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_نما(Flags);
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

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کد &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::از_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کد &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::از_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کد &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::از_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کد &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_کد &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), درست) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(صحيح fd, منطقي shouldClose, منطقي unbuffered,
                               OStream_نوع K)
    : raw_pwrite_جریان(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (FD < 0 ) {
    ShouldClose = خطأ;
    أرجع;
  }

  enable_colors(درست);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  إذا (FD <= STDERR_FILENO)
    ShouldClose = خطأ;

#اگر_تعریف_شده _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#پایان_اگر

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_وضعیت وضعیت;
  std::error_کد EC = وضعیت_2(FD, وضعیت);
  IsRegularFile = وضعیت.type() == sys::fs::file_type::regular_file;
#اگر_تعریف_شده _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#وإلا
  SupportsSeeking = !EC && loc != (off_t)-1;
#پایان_اگر
  إذا (!SupportsSeeking)
    pos = 0;
  وإلا
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  إذا (FD >= 0) {
    flush();
    إذا (ShouldClose) {
      إذا (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#اگر_تعریف_شده __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  إذا (FD == 2) أرجع;
#پایان_اگر

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  إذا (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().پیام_2());
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
ساكن منطقي write_console_impl(صحيح FD, StringRef داده_2) {
  Small_وکتور<wchar_t, 256> Wide_متن;

  // Fall back to ::write if it wasn't valid UTF-8.
  إذا (خودکار EC = sys::windows::UTF8_مقصد_UTF16(داده_2, Wide_متن))
    أرجع خطأ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_اندازه = Wide_متن.اندازه_2();
  إذا (!RunningWindows8OrGreater())
    Max_Write_اندازه = 32767;

  size_t WCharsWritten = 0;
  افعل {
    size_t WChars_مقصد_Write =
        std::min(Max_Write_اندازه, Wide_متن.اندازه_2() - WCharsWritten);
    DWORD ActuallyWritten;
    منطقي Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_متن[WCharsWritten],
                        WChars_مقصد_Write, &ActuallyWritten,
                        /*Reserved=*/اشاره_تهی);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    إذا (!Success)
      أرجع خطأ;

    WCharsWritten += ActuallyWritten;
  } طالما (WCharsWritten != Wide_متن.اندازه_2());
  أرجع درست;
}
#پایان_اگر

فراغ raw_fd_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
  إذا (Tied_جریان)
    Tied_جریان->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += اندازه;

#إذا defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  إذا (IsWindowsConsole)
    إذا (write_console_impl(FD, StringRef(Ptr, اندازه)))
      أرجع;
#پایان_اگر

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_اندازه = INT32_MAX;

#إذا defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_اندازه = 1024 * 1024 * 1024;
#پایان_اگر

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
#اگر_تعریف_شده EWOULDBLOCK
          || errno == EWOULDBLOCK
#پایان_اگر
          )
        استمر;

#اگر_تعریف_شده _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_آخرین_Error = Get_آخرین_Error();
      إذا (Win_آخرین_Error == ERROR_BROKEN_PIPE ||
          (Win_آخرین_Error == ERROR_NO_داده && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#پایان_اگر
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_کد());
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
  إذا (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#اگر_تعریف_شده _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#وإلا
  pos = ::lseek(FD, off, SEEK_SET);
#پایان_اگر
  إذا (pos == (uint64_t)-1)
    error_detected(errno_As_Error_کد());
  أرجع pos;
}

فراغ raw_fd_ostream::pwrite_impl(ثابت حرف *Ptr, size_t اندازه,
                                 uint64_t جابه_جایی) {
  uint64_t Pos = tell();
  seek(جابه_جایی);
  اكتب(Ptr, اندازه);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_میانگیر_اندازه() ثابت {
#إذا defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  إذا (IsWindowsConsole)
    أرجع 0;
  أرجع raw_ostream::preferred_میانگیر_اندازه();
#وإلاإذا defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  أرجع raw_ostream::preferred_میانگیر_اندازه();
#وإلا
  أكد(FD >= 0 && "File not yet open!");
  بنية stat statbuf;
  إذا (fstat(FD, &statbuf) != 0)
    أرجع 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  إذا (S_ISCHR(statbuf.st_نما) && is_displayed())
    أرجع 0;
  // Return the preferred block size.
  أرجع statbuf.st_blksize;
#پایان_اگر
}

منطقي raw_fd_ostream::is_displayed() ثابت {
  أرجع sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقي raw_fd_ostream::has_colors() ثابت {
  إذا (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  أرجع *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::قفل_2() {
  std::error_کد EC = sys::fs::قفل_File(FD);
  إذا (!EC)
    أرجع sys::fs::FileLocker(FD);
  أرجع error_کد_مقصد_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_قفل_For(Duration ثابت& Timeout) {
  std::error_کد EC = sys::fs::try_قفل_File(FD, Timeout.getDuration());
  إذا (!EC)
    أرجع sys::fs::FileLocker(FD);
  أرجع error_کد_مقصد_Error(EC);
}

فراغ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_کد EC;

  // On z/OS we need to enable auto conversion
  ساكن std::error_کد EC1 = enableAutoConversion(STDOUT_FILENO);
  أكد(!EC1);
  (فراغ)EC1;

  ساكن raw_fd_ostream S("-", EC, sys::fs::از_None);
  أكد(!EC);
  أرجع S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ساكن std::error_کد EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (فراغ)EC;

  // Set standard error to be unbuffered.
  ساكن raw_fd_ostream S(STDERR_FILENO, خطأ, درست);
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

raw_fd_جریان::raw_fd_جریان(StringRef Filename, std::error_کد &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::از_None),
                     درست, خطأ, OStream_نوع::OK_FDStream) {
  إذا (EC)
    أرجع;

  إذا (!isRegularFile())
    EC = std::make_error_کد(std::errc::invalid_آرگومان);
}

raw_fd_جریان::raw_fd_جریان(صحيح fd, منطقي shouldClose)
    : raw_fd_ostream(fd, shouldClose, خطأ, OStream_نوع::OK_FDStream) {}

ssize_t raw_fd_جریان::اقرأ(حرف *Ptr, size_t اندازه) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (فراغ *)Ptr, اندازه);
  إذا (Ret >= 0)
    inc_pos(Ret);
  وإلا
    error_detected(errno_As_Error_کد());
  أرجع Ret;
}

منطقي raw_fd_جریان::classof(ثابت raw_ostream *OS) {
  أرجع OS->get_نوع_2() == OStream_نوع::OK_FDStream;
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
                                      uint64_t جابه_جایی) {
  کپی_حافظه(OS.داده() + جابه_جایی, Ptr, اندازه);
}

منطقي raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  أرجع OS->get_نوع_2() == OStream_نوع::OK_SVec_جریان;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#اگر_تعریف_نشده NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#پایان_اگر
}

فراغ raw_null_ostream::write_impl(ثابت حرف *Ptr, size_t اندازه) {
}

uint64_t raw_null_ostream::current_pos() ثابت {
  أرجع 0;
}

فراغ raw_null_ostream::pwrite_impl(ثابت حرف *Ptr, size_t اندازه,
                                   uint64_t جابه_جایی) {}

فراغ raw_pwrite_جریان::anchor() {}

فراغ میانگیر_ostream::anchor() {}

فراغ میانگیر_unique_ostream::anchor() {}

ساكن منطقي isNullDeviceName(StringRef OutputFileName) {
#اگر_تعریف_شده _WIN32
  إذا (OutputFileName.equals_insensitive("NUL"))
    أرجع درست;
#پایان_اگر
  أرجع OutputFileName == "/dev/null";
}

Error llvm::write_مقصد_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  إذا (OutputFileName == "-")
    أرجع Write(outs());

  إذا (isNullDeviceName(OutputFileName)) {
    raw_null_ostream بیرون;
    أرجع Write(بیرون);
  }

  بیعلامت نما = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::موقت_File> موقت =
      sys::fs::موقت_File::create(OutputFileName + ".temp-stream-%%%%%%", نما);
  إذا (!موقت)
    أرجع createFileError(OutputFileName, موقت.takeError());

#إذا defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  إذا (خودکار EC = llvm::copy_File_برچسب_صفات(OutputFileName.str(), موقت->FD)) {
    إذا (EC != std::errc::no_such_file_or_directory)
      أرجع createFileError(OutputFileName, EC);
  }
#پایان_اگر

  raw_fd_ostream بیرون(موقت->FD, خطأ);

  إذا (Error E = Write(بیرون)) {
    إذا (Error DiscardError = موقت->discard())
      أرجع joinErrors(std::move(E), std::move(DiscardError));
    أرجع E;
  }
  بیرون.flush();

  أرجع موقت->keep(OutputFileName);
}
