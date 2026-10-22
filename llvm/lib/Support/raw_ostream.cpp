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

#شامل "llvm/Support/raw_ostream.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/AutoConvert.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/Duration.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/MathExtras.h"
#شامل "llvm/Support/NativeFormatting.h"
#شامل "llvm/Support/Process.h"
#شامل "llvm/Support/Program.h"
#شامل <algorithm>
#شامل <cerrno>
#شامل <cstdio>
#شامل <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#اگر defined(HAVE_UNISTD_H)
# include <unistd.h>
#اگر_ختم

#اگر defined(__CYGWIN__)
#شامل <io.h>
#اگر_ختم

#اگر defined(_MSC_VER)
#شامل <io.h>
#اگر_غیر_تعریف_شدہ STDIN_FILENO
# define STDIN_FILENO 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ STDOUT_FILENO
# define STDOUT_FILENO 1
#اگر_ختم
#اگر_غیر_تعریف_شدہ STDERR_FILENO
# define STDERR_FILENO 2
#اگر_ختم
#اگر_ختم

#اگر_تعریف_شدہ _WIN32
#شامل "llvm/Support/ConvertUTF.h"
#شامل "llvm/Support/Signals.h"
#شامل "llvm/Support/Windows/WindowsSupport.h"
#اگر_ختم

استعمال نامخلا llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  أكد(برآمد_Buf_Cur == برآمد_Buf_آغاز &&
         "raw_ostream destructor called with non-empty buffer!");

  اگر (بفر_منوال == بفر_قسم::اندرونی_بفر)
    حذف [] برآمد_Buf_آغاز;
}

size_t raw_ostream::preferred_بفر_سائز() ثابت {
#اگر_تعریف_شدہ _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  واپس (16 * 1024);
#ورنہ
  // BUFSIZ is intended to be a reasonable default.
  واپس BUFSIZ;
#اگر_ختم
}

خالی raw_ostream::سیٹ_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  اگر (size_t سائز = preferred_بفر_سائز())
    سیٹ_بفر_سائز(سائز);
  ورنہ
    // It may return 0, meaning this stream should be unbuffered.
    سیٹ_Unbuffered();
}

خالی raw_ostream::سیٹ_بفر_And_منوال(حرف *بفر_آغاز, size_t سائز,
                                   بفر_قسم منوال) {
  أكد(((منوال == بفر_قسم::Unbuffered && !بفر_آغاز && سائز == 0) ||
          (منوال != بفر_قسم::Unbuffered && بفر_آغاز && سائز != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  أكد(Get_عدد_بائٹیں_اندر_بفر() == 0 && "Current buffer is non-empty!");

  اگر (بفر_منوال == بفر_قسم::اندرونی_بفر)
    حذف [] برآمد_Buf_آغاز;
  برآمد_Buf_آغاز = بفر_آغاز;
  برآمد_Buf_انتہا = برآمد_Buf_آغاز+سائز;
  برآمد_Buf_Cur = برآمد_Buf_آغاز;
  بفر_منوال = منوال;

  أكد(برآمد_Buf_آغاز <= برآمد_Buf_انتہا && "Invalid size!");
}

raw_ostream &raw_ostream::عامل<<(غیرعلامتی طویل N) {
  write_integer(*یہ, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(طویل N) {
  write_integer(*یہ, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(غیرعلامتی طویل طویل N) {
  write_integer(*یہ, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(طویل طویل N) {
  write_integer(*یہ, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  واپس *یہ;
}

raw_ostream &raw_ostream::write_hex(غیرعلامتی طویل طویل N) {
  llvm::write_hex(*یہ, N, HexPrintStyle::زیریں_2);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(Colors C) {
  اگر (C == Colors::RESET)
    resetColor();
  ورنہ
    changeColor(C);
  واپس *یہ;
}

raw_ostream &raw_ostream::write_uuid(ثابت uuid_t UUID) {
  برائے (صحیح Idx = 0; Idx < 16; ++Idx) {
    *یہ << format("%02" PRIX32, UUID[Idx]);
    اگر (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *یہ << "-";
  }
  واپس *یہ;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        منطقی UseHexEscapes) {
  برائے (غیرعلامتی حرف c : Str) {
    انتخاب (c) {
    حالت '\\':
      *یہ << '\\' << '\\';
      توڑو;
    حالت '\t':
      *یہ << '\\' << 't';
      توڑو;
    حالت '\n':
      *یہ << '\\' << 'n';
      توڑو;
    حالت '"':
      *یہ << '\\' << '"';
      توڑو;
    طےشدہ:
      اگر (isPrint(c)) {
        *یہ << c;
        توڑو;
      }

      // Write out the escaped representation.
      اگر (UseHexEscapes) {
        *یہ << '\\' << 'x';
        *یہ << hexdigit((c >> 4) & 0xF);
        *یہ << hexdigit((c >> 0) & 0xF);
      } ورنہ {
        // Always use a full 3-character octal escape.
        *یہ << '\\';
        *یہ << حرف('0' + ((c >> 6) & 7));
        *یہ << حرف('0' + ((c >> 3) & 7));
        *یہ << حرف('0' + ((c >> 0) & 7));
      }
    }
  }

  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(ثابت خالی *P) {
  llvm::write_hex(*یہ, (uintptr_t)P, HexPrintStyle::سابقہ_زیریں);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(دوہرا N) {
  llvm::write_double(*یہ, N, FloatStyle::Exponent);
  واپس *یہ;
}

خالی raw_ostream::flush_nonempty() {
  أكد(برآمد_Buf_Cur > برآمد_Buf_آغاز && "Invalid call to flush_nonempty.");
  size_t لمبائی = برآمد_Buf_Cur - برآمد_Buf_آغاز;
  برآمد_Buf_Cur = برآمد_Buf_آغاز;
  write_impl(برآمد_Buf_آغاز, لمبائی);
}

raw_ostream &raw_ostream::اكتب(غیرعلامتی حرف C) {
  // Group exceptional cases into a single branch.
  اگر (LLVM_UNLIKELY(برآمد_Buf_Cur >= برآمد_Buf_انتہا)) {
    اگر (LLVM_UNLIKELY(!برآمد_Buf_آغاز)) {
      اگر (بفر_منوال == بفر_قسم::Unbuffered) {
        write_impl(reinterpret_cast<حرف *>(&C), 1);
        واپس *یہ;
      }
      // Set up a buffer and start over.
      سیٹ_Buffered();
      واپس اكتب(C);
    }

    flush_nonempty();
  }

  *برآمد_Buf_Cur++ = C;
  واپس *یہ;
}

raw_ostream &raw_ostream::اكتب(ثابت حرف *Ptr, size_t سائز) {
  // Group exceptional cases into a single branch.
  اگر (LLVM_UNLIKELY(size_t(برآمد_Buf_انتہا - برآمد_Buf_Cur) < سائز)) {
    اگر (LLVM_UNLIKELY(!برآمد_Buf_آغاز)) {
      اگر (بفر_منوال == بفر_قسم::Unbuffered) {
        write_impl(Ptr, سائز);
        واپس *یہ;
      }
      // Set up a buffer and start over.
      سیٹ_Buffered();
      واپس اكتب(Ptr, سائز);
    }

    size_t عدد_بائٹیں = برآمد_Buf_انتہا - برآمد_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    اگر (LLVM_UNLIKELY(برآمد_Buf_Cur == برآمد_Buf_آغاز)) {
      أكد(عدد_بائٹیں != 0 && "undefined behavior");
      size_t بائٹیں_مقصد_Write_2 = سائز - (سائز % عدد_بائٹیں);
      write_impl(Ptr, بائٹیں_مقصد_Write_2);
      size_t بائٹیں_بقایا_2 = سائز - بائٹیں_مقصد_Write_2;
      اگر (بائٹیں_بقایا_2 > size_t(برآمد_Buf_انتہا - برآمد_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        واپس اكتب(Ptr + بائٹیں_مقصد_Write_2, بائٹیں_بقایا_2);
      }
      copy_مقصد_بفر(Ptr + بائٹیں_مقصد_Write_2, بائٹیں_بقایا_2);
      واپس *یہ;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_مقصد_بفر(Ptr, عدد_بائٹیں);
    flush_nonempty();
    واپس اكتب(Ptr + عدد_بائٹیں, سائز - عدد_بائٹیں);
  }

  copy_مقصد_بفر(Ptr, سائز);

  واپس *یہ;
}

خالی raw_ostream::copy_مقصد_بفر(ثابت حرف *Ptr, size_t سائز) {
  أكد(سائز <= size_t(برآمد_Buf_انتہا - برآمد_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  انتخاب (سائز) {
  حالت 4: برآمد_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  حالت 3: برآمد_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  حالت 2: برآمد_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  حالت 1: برآمد_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  حالت 0: توڑو;
  طےشدہ:
    حافظہ_نقل(برآمد_Buf_Cur, Ptr, سائز);
    توڑو;
  }

  برآمد_Buf_Cur += سائز;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::عامل<<(function_ref<صحیح(حرف *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t سائز = 128;
  size_t بفر_بائٹیں_بایاں = برآمد_Buf_انتہا - برآمد_Buf_Cur;
  اگر (بفر_بائٹیں_بایاں > 3) {
    صحیح N = Snprint(برآمد_Buf_Cur, بفر_بائٹیں_بایاں);
    اگر (N < 0)
      واپس *یہ;
    // Common case is that we have plenty of space.
    اگر (size_t(N) < بفر_بائٹیں_بایاں) {
      برآمد_Buf_Cur += N;
      واپس *یہ;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    سائز = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_سمتیہ<حرف, 128> V;
  برائے (;;) {
    V.resize(سائز);
    صحیح N = Snprint(V.ڈیٹا(), سائز);
    اگر (N < 0)
      واپس *یہ;
    اگر (size_t(N) < سائز)
      واپس اكتب(V.ڈیٹا(), N);
    سائز = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::عامل<<(ثابت formatv_object_بنیاد &Obj) {
  Obj.format(*یہ);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(ثابت FormattedString &FS) {
  غیرعلامتی بایاں_Indent = 0;
  غیرعلامتی دایاں_Indent = 0;
  ثابت ssize_t Difference = FS.چوڑائی - FS.Str.سائز_2();
  اگر (Difference > 0) {
    انتخاب (FS.Justify) {
    حالت FormattedString::JustifyNone:
      توڑو;
    حالت FormattedString::Justify_بایاں:
      دایاں_Indent = Difference;
      توڑو;
    حالت FormattedString::Justify_دایاں:
      بایاں_Indent = Difference;
      توڑو;
    حالت FormattedString::JustifyCenter:
      بایاں_Indent = Difference / 2;
      دایاں_Indent = Difference - بایاں_Indent;
      توڑو;
    }
  }
  indent(بایاں_Indent);
  (*یہ) << FS.Str;
  indent(دایاں_Indent);
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(ثابت Formatted_نمبر &FN) {
  اگر (FN.Hex) {
    HexPrintStyle Style;
    اگر (FN.بالائی_2 && FN.Hex_سابقہ)
      Style = HexPrintStyle::سابقہ_بالائی;
    ورنہ اگر (FN.بالائی_2 && !FN.Hex_سابقہ)
      Style = HexPrintStyle::بالائی_2;
    ورنہ اگر (!FN.بالائی_2 && FN.Hex_سابقہ)
      Style = HexPrintStyle::سابقہ_زیریں;
    ورنہ
      Style = HexPrintStyle::زیریں_2;
    llvm::write_hex(*یہ, FN.Hex_قدر, Style, FN.چوڑائی);
  } ورنہ {
    llvm::SmallString<16> بفر;
    llvm::raw_svector_ostream دھارا(بفر);
    llvm::write_integer(دھارا, FN.Dec_قدر, 0, IntegerStyle::Integer);
    اگر (بفر.سائز_2() < FN.چوڑائی)
      indent(FN.چوڑائی - بفر.سائز_2());
    (*یہ) << بفر;
  }
  واپس *یہ;
}

raw_ostream &raw_ostream::عامل<<(ثابت Formatted_بائٹیں &FB) {
  اگر (FB.بائٹیں_2.خالی_خانے())
    واپس *یہ;

  size_t سطر_اشاریہ = 0;
  خودکار بائٹیں_2 = FB.بائٹیں_2;
  ثابت size_t سائز = بائٹیں_2.سائز_2();
  HexPrintStyle HPS = FB.بالائی_2 ? HexPrintStyle::بالائی_2 : HexPrintStyle::زیریں_2;
  uint64_t انحراف_چوڑائی = 0;
  اگر (FB.پہلا_Byte_انحراف) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t سطریں = سائز / FB.عدد_فی_سطر;
    uint64_t Max_انحراف = *FB.پہلا_Byte_انحراف + سطریں * FB.عدد_فی_سطر;
    غیرعلامتی Power = 0;
    اگر (Max_انحراف > 0)
      Power = llvm::Log2_64_Ceil(Max_انحراف);
    انحراف_چوڑائی = std::max<uint64_t>(4, llvm::align_مقصد(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  غیرعلامتی عدد_Byte_Groups =
      align_مقصد(FB.عدد_فی_سطر, FB.Byte_گروہ_سائز) / FB.Byte_گروہ_سائز;
  غیرعلامتی بلاک_Char_چوڑائی = FB.عدد_فی_سطر * 2 + عدد_Byte_Groups - 1;

  جب (!بائٹیں_2.خالی_خانے()) {
    indent(FB.IndentLevel);

    اگر (FB.پہلا_Byte_انحراف) {
      uint64_t انحراف = *FB.پہلا_Byte_انحراف;
      llvm::write_hex(*یہ, انحراف + سطر_اشاریہ, HPS, انحراف_چوڑائی);
      *یہ << ": ";
    }

    خودکار سطر = بائٹیں_2.take_front(FB.عدد_فی_سطر);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    برائے (size_t I = 0; I < سطر.سائز_2(); ++I, CharsPrinted += 2) {
      اگر (I && (I % FB.Byte_گروہ_سائز) == 0) {
        ++CharsPrinted;
        *یہ << " ";
      }
      llvm::write_hex(*یہ, سطر[I], HPS, 2);
    }

    اگر (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(بلاک_Char_چوڑائی >= CharsPrinted);
      indent(بلاک_Char_چوڑائی - CharsPrinted + 2);
      *یہ << "|";

      // Print the ASCII char values for each byte on this line
      برائے (uint8_t Byte : سطر) {
        اگر (isPrint(Byte))
          *یہ << static_cast<حرف>(Byte);
        ورنہ
          *یہ << '.';
      }
      *یہ << '|';
    }

    بائٹیں_2 = بائٹیں_2.drop_front(سطر.سائز_2());
    سطر_اشاریہ += سطر.سائز_2();
    اگر (سطر_اشاریہ < سائز)
      *یہ << '\n';
  }
  واپس *یہ;
}

سانچہ <حرف C>
ساکن raw_ostream &write_padding(raw_ostream &OS, غیرعلامتی عدد_Chars) {
  ساکن ثابت حرف Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  اگر (عدد_Chars < std::سائز_2(Chars))
    واپس OS.اكتب(Chars, عدد_Chars);

  جب (عدد_Chars) {
    غیرعلامتی عدد_مقصد_Write = std::min(عدد_Chars, (غیرعلامتی)std::سائز_2(Chars) - 1);
    OS.اكتب(Chars, عدد_مقصد_Write);
    عدد_Chars -= عدد_مقصد_Write;
  }
  واپس OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(غیرعلامتی عدد_Spaces) {
  واپس write_padding<' '>(*یہ, عدد_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(غیرعلامتی عدد_Zeros) {
  واپس write_padding<'\0'>(*یہ, عدد_Zeros);
}

منطقی raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  اگر (!ColorEnabled)
    واپس جھوٹ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  اگر (sys::Process::ColorNeedsFlush() && !is_displayed())
    واپس جھوٹ;

  اگر (sys::Process::ColorNeedsFlush())
    flush();

  واپس سچ;
}

raw_ostream &raw_ostream::changeColor(شمار Colors colors, منطقی bold, منطقی bg) {
  اگر (!prepare_colors())
    واپس *یہ;

  ثابت حرف *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<حرف>(colors), bold, bg);
  اگر (colorcode)
    اكتب(colorcode, لڑی_لمبائی(colorcode));
  واپس *یہ;
}

raw_ostream &raw_ostream::resetColor() {
  اگر (!prepare_colors())
    واپس *یہ;

  اگر (ثابت حرف *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, لڑی_لمبائی(colorcode));
  واپس *یہ;
}

raw_ostream &raw_ostream::الٹ_Color() {
  اگر (!prepare_colors())
    واپس *یہ;

  اگر (ثابت حرف *colorcode = sys::Process::Output_الٹ())
    اكتب(colorcode, لڑی_لمبائی(colorcode));
  واپس *یہ;
}

خالی raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ساکن صحیح getFD(StringRef Filename, std::error_رمز &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  أكد((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  اگر (Filename == "-") {
    EC = std::error_رمز();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_منوال(Flags);
    واپس STDOUT_FILENO;
  }

  صحیح FD;
  اگر (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  ورنہ
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  اگر (EC)
    واپس -1;

  واپس FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_رمز &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::کا_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_رمز &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::کا_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_رمز &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::کا_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_رمز &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_رمز &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), سچ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(صحیح fd, منطقی shouldClose, منطقی unbuffered,
                               OStream_قسم K)
    : raw_pwrite_دھارا(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  اگر (FD < 0 ) {
    ShouldClose = جھوٹ;
    واپس;
  }

  enable_colors(سچ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  اگر (FD <= STDERR_FILENO)
    ShouldClose = جھوٹ;

#اگر_تعریف_شدہ _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#اگر_ختم

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_حالت حالت_2;
  std::error_رمز EC = حالت_3(FD, حالت_2);
  IsRegularFile = حالت_2.type() == sys::fs::file_type::regular_file;
#اگر_تعریف_شدہ _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#ورنہ
  SupportsSeeking = !EC && loc != (off_t)-1;
#اگر_ختم
  اگر (!SupportsSeeking)
    pos = 0;
  ورنہ
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  اگر (FD >= 0) {
    flush();
    اگر (ShouldClose) {
      اگر (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#اگر_تعریف_شدہ __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  اگر (FD == 2) واپس;
#اگر_ختم

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  اگر (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().پیغام_2());
}

#اگر defined(_WIN32)
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
ساکن منطقی write_console_impl(صحیح FD, StringRef ڈیٹا_2) {
  Small_سمتیہ<wchar_t, 256> Wide_متن;

  // Fall back to ::write if it wasn't valid UTF-8.
  اگر (خودکار EC = sys::windows::UTF8_مقصد_UTF16(ڈیٹا_2, Wide_متن))
    واپس جھوٹ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_سائز = Wide_متن.سائز_2();
  اگر (!RunningWindows8OrGreater())
    Max_Write_سائز = 32767;

  size_t WCharsWritten = 0;
  کرو {
    size_t WChars_مقصد_Write =
        std::min(Max_Write_سائز, Wide_متن.سائز_2() - WCharsWritten);
    DWORD ActuallyWritten;
    منطقی Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_متن[WCharsWritten],
                        WChars_مقصد_Write, &ActuallyWritten,
                        /*Reserved=*/خالی_اشاریہ);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    اگر (!Success)
      واپس جھوٹ;

    WCharsWritten += ActuallyWritten;
  } جب (WCharsWritten != Wide_متن.سائز_2());
  واپس سچ;
}
#اگر_ختم

خالی raw_fd_ostream::write_impl(ثابت حرف *Ptr, size_t سائز) {
  اگر (Tied_دھارا)
    Tied_دھارا->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += سائز;

#اگر defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  اگر (IsWindowsConsole)
    اگر (write_console_impl(FD, StringRef(Ptr, سائز)))
      واپس;
#اگر_ختم

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_سائز = INT32_MAX;

#اگر defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_سائز = 1024 * 1024 * 1024;
#اگر_ختم

  کرو {
    size_t Chunk_سائز = std::min(سائز, Max_Write_سائز);
    ssize_t ret = ::اكتب(FD, Ptr, Chunk_سائز);

    اگر (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      اگر (errno == EINTR || errno == EAGAIN
#اگر_تعریف_شدہ EWOULDBLOCK
          || errno == EWOULDBLOCK
#اگر_ختم
          )
        جاری;

#اگر_تعریف_شدہ _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_آخری_Error = Get_آخری_Error();
      اگر (Win_آخری_Error == ERROR_BROKEN_PIPE ||
          (Win_آخری_Error == ERROR_NO_ڈیٹا && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#اگر_ختم
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_رمز());
      توڑو;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    سائز -= ret;
  } جب (سائز > 0);
}

خالی raw_fd_ostream::close() {
  أكد(ShouldClose);
  ShouldClose = جھوٹ;
  flush();
  اگر (خودکار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#اگر_تعریف_شدہ _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#ورنہ
  pos = ::lseek(FD, off, SEEK_SET);
#اگر_ختم
  اگر (pos == (uint64_t)-1)
    error_detected(errno_As_Error_رمز());
  واپس pos;
}

خالی raw_fd_ostream::pwrite_impl(ثابت حرف *Ptr, size_t سائز,
                                 uint64_t انحراف) {
  uint64_t Pos = tell();
  seek(انحراف);
  اكتب(Ptr, سائز);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_بفر_سائز() ثابت {
#اگر defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  اگر (IsWindowsConsole)
    واپس 0;
  واپس raw_ostream::preferred_بفر_سائز();
#وگرنہ_اگر defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  واپس raw_ostream::preferred_بفر_سائز();
#ورنہ
  أكد(FD >= 0 && "File not yet open!");
  ساخت stat statbuf;
  اگر (fstat(FD, &statbuf) != 0)
    واپس 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  اگر (S_ISCHR(statbuf.st_منوال) && is_displayed())
    واپس 0;
  // Return the preferred block size.
  واپس statbuf.st_blksize;
#اگر_ختم
}

منطقی raw_fd_ostream::is_displayed() ثابت {
  واپس sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقی raw_fd_ostream::has_colors() ثابت {
  اگر (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  واپس *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::تالا_2() {
  std::error_رمز EC = sys::fs::تالا_File(FD);
  اگر (!EC)
    واپس sys::fs::FileLocker(FD);
  واپس error_رمز_مقصد_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_تالا_For(Duration ثابت& Timeout) {
  std::error_رمز EC = sys::fs::try_تالا_File(FD, Timeout.getDuration());
  اگر (!EC)
    واپس sys::fs::FileLocker(FD);
  واپس error_رمز_مقصد_Error(EC);
}

خالی raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_رمز EC;

  // On z/OS we need to enable auto conversion
  ساکن std::error_رمز EC1 = enableAutoConversion(STDOUT_FILENO);
  أكد(!EC1);
  (خالی)EC1;

  ساکن raw_fd_ostream S("-", EC, sys::fs::کا_None);
  أكد(!EC);
  واپس S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ساکن std::error_رمز EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (خالی)EC;

  // Set standard error to be unbuffered.
  ساکن raw_fd_ostream S(STDERR_FILENO, جھوٹ, سچ);
  واپس S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ساکن raw_null_ostream S;
  واپس S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_دھارا::raw_fd_دھارا(StringRef Filename, std::error_رمز &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::کا_None),
                     سچ, جھوٹ, OStream_قسم::OK_FDStream) {
  اگر (EC)
    واپس;

  اگر (!isRegularFile())
    EC = std::make_error_رمز(std::errc::invalid_دلیل);
}

raw_fd_دھارا::raw_fd_دھارا(صحیح fd, منطقی shouldClose)
    : raw_fd_ostream(fd, shouldClose, جھوٹ, OStream_قسم::OK_FDStream) {}

ssize_t raw_fd_دھارا::اقرأ(حرف *Ptr, size_t سائز) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (خالی *)Ptr, سائز);
  اگر (Ret >= 0)
    inc_pos(Ret);
  ورنہ
    error_detected(errno_As_Error_رمز());
  واپس Ret;
}

منطقی raw_fd_دھارا::classof(ثابت raw_ostream *OS) {
  واپس OS->get_قسم_2() == OStream_قسم::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

خالی raw_string_ostream::write_impl(ثابت حرف *Ptr, size_t سائز) {
  OS.append(Ptr, سائز);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ثابت { واپس OS.سائز_2(); }

خالی raw_svector_ostream::write_impl(ثابت حرف *Ptr, size_t سائز) {
  OS.append(Ptr, Ptr + سائز);
}

خالی raw_svector_ostream::pwrite_impl(ثابت حرف *Ptr, size_t سائز,
                                      uint64_t انحراف) {
  حافظہ_نقل(OS.ڈیٹا() + انحراف, Ptr, سائز);
}

منطقی raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  واپس OS->get_قسم_2() == OStream_قسم::OK_SVec_دھارا;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#اگر_غیر_تعریف_شدہ NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#اگر_ختم
}

خالی raw_null_ostream::write_impl(ثابت حرف *Ptr, size_t سائز) {
}

uint64_t raw_null_ostream::current_pos() ثابت {
  واپس 0;
}

خالی raw_null_ostream::pwrite_impl(ثابت حرف *Ptr, size_t سائز,
                                   uint64_t انحراف) {}

خالی raw_pwrite_دھارا::anchor() {}

خالی بفر_ostream::anchor() {}

خالی بفر_unique_ostream::anchor() {}

ساکن منطقی isNullDeviceName(StringRef OutputFileName) {
#اگر_تعریف_شدہ _WIN32
  اگر (OutputFileName.equals_insensitive("NUL"))
    واپس سچ;
#اگر_ختم
  واپس OutputFileName == "/dev/null";
}

Error llvm::write_مقصد_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  اگر (OutputFileName == "-")
    واپس Write(outs());

  اگر (isNullDeviceName(OutputFileName)) {
    raw_null_ostream برآمد;
    واپس Write(برآمد);
  }

  غیرعلامتی منوال = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::عارضی_File> عارضی =
      sys::fs::عارضی_File::create(OutputFileName + ".temp-stream-%%%%%%", منوال);
  اگر (!عارضی)
    واپس createFileError(OutputFileName, عارضی.takeError());

#اگر defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  اگر (خودکار EC = llvm::copy_File_لیبل_اوصاف(OutputFileName.str(), عارضی->FD)) {
    اگر (EC != std::errc::no_such_file_or_directory)
      واپس createFileError(OutputFileName, EC);
  }
#اگر_ختم

  raw_fd_ostream برآمد(عارضی->FD, جھوٹ);

  اگر (Error E = Write(برآمد)) {
    اگر (Error DiscardError = عارضی->discard())
      واپس joinErrors(std::move(E), std::move(DiscardError));
    واپس E;
  }
  برآمد.flush();

  واپس عارضی->keep(OutputFileName);
}
