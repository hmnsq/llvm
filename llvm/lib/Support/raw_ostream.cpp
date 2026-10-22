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
#جيڪڏهن_پورو

#إذا defined(__CYGWIN__)
#ضمن <io.h>
#جيڪڏهن_پورو

#إذا defined(_MSC_VER)
#ضمن <io.h>
#جيڪڏهن_وصف_نه_ٿيل STDIN_FILENO
# define STDIN_FILENO 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل STDOUT_FILENO
# define STDOUT_FILENO 1
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل STDERR_FILENO
# define STDERR_FILENO 2
#جيڪڏهن_پورو
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "llvm/Support/ConvertUTF.h"
#ضمن "llvm/Support/Signals.h"
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#جيڪڏهن_پورو

استعمال نطاق llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  أكد(ٻاهريون_Buf_Cur == ٻاهريون_Buf_شروعات &&
         "raw_ostream destructor called with non-empty buffer!");

  إذا (بفر_گهڻائي == بفر_قسم::اندروني_بفر)
    ميٽيو [] ٻاهريون_Buf_شروعات;
}

size_t raw_ostream::preferred_بفر_سائيز() ثابت {
#جيڪڏهن_وصف_ٿيل _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  أرجع (16 * 1024);
#وإلا
  // BUFSIZ is intended to be a reasonable default.
  أرجع BUFSIZ;
#جيڪڏهن_پورو
}

فراغ raw_ostream::سيٽ_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  إذا (size_t سائيز = preferred_بفر_سائيز())
    سيٽ_بفر_سائيز(سائيز);
  وإلا
    // It may return 0, meaning this stream should be unbuffered.
    سيٽ_Unbuffered();
}

فراغ raw_ostream::سيٽ_بفر_And_گهڻائي(حرف *بفر_شروعات, size_t سائيز,
                                   بفر_قسم گهڻائي) {
  أكد(((گهڻائي == بفر_قسم::Unbuffered && !بفر_شروعات && سائيز == 0) ||
          (گهڻائي != بفر_قسم::Unbuffered && بفر_شروعات && سائيز != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  أكد(Get_انگ_بائيٽون_اندريون_بفر() == 0 && "Current buffer is non-empty!");

  إذا (بفر_گهڻائي == بفر_قسم::اندروني_بفر)
    ميٽيو [] ٻاهريون_Buf_شروعات;
  ٻاهريون_Buf_شروعات = بفر_شروعات;
  ٻاهريون_Buf_پڇاڙي = ٻاهريون_Buf_شروعات+سائيز;
  ٻاهريون_Buf_Cur = ٻاهريون_Buf_شروعات;
  بفر_گهڻائي = گهڻائي;

  أكد(ٻاهريون_Buf_شروعات <= ٻاهريون_Buf_پڇاڙي && "Invalid size!");
}

raw_ostream &raw_ostream::عامل<<(بنانشان ڊگهو N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(ڊگهو N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(بنانشان ڊگهو ڊگهو N) {
  write_integer(*هذا, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(ڊگهو ڊگهو N) {
  write_integer(*هذا, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  أرجع *هذا;
}

raw_ostream &raw_ostream::write_hex(بنانشان ڊگهو ڊگهو N) {
  llvm::write_hex(*هذا, N, HexPrintStyle::هيٺاهون_2);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(Colors C) {
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
  لكل (بنانشان حرف c : Str) {
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

raw_ostream &raw_ostream::عامل<<(ثابت فراغ *P) {
  llvm::write_hex(*هذا, (uintptr_t)P, HexPrintStyle::اڳياڙي_هيٺاهون);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(مزدوج N) {
  llvm::write_double(*هذا, N, FloatStyle::Exponent);
  أرجع *هذا;
}

فراغ raw_ostream::flush_nonempty() {
  أكد(ٻاهريون_Buf_Cur > ٻاهريون_Buf_شروعات && "Invalid call to flush_nonempty.");
  size_t ڊيگهائي = ٻاهريون_Buf_Cur - ٻاهريون_Buf_شروعات;
  ٻاهريون_Buf_Cur = ٻاهريون_Buf_شروعات;
  write_impl(ٻاهريون_Buf_شروعات, ڊيگهائي);
}

raw_ostream &raw_ostream::اكتب(بنانشان حرف C) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(ٻاهريون_Buf_Cur >= ٻاهريون_Buf_پڇاڙي)) {
    إذا (LLVM_UNLIKELY(!ٻاهريون_Buf_شروعات)) {
      إذا (بفر_گهڻائي == بفر_قسم::Unbuffered) {
        write_impl(reinterpret_cast<حرف *>(&C), 1);
        أرجع *هذا;
      }
      // Set up a buffer and start over.
      سيٽ_Buffered();
      أرجع اكتب(C);
    }

    flush_nonempty();
  }

  *ٻاهريون_Buf_Cur++ = C;
  أرجع *هذا;
}

raw_ostream &raw_ostream::اكتب(ثابت حرف *Ptr, size_t سائيز) {
  // Group exceptional cases into a single branch.
  إذا (LLVM_UNLIKELY(size_t(ٻاهريون_Buf_پڇاڙي - ٻاهريون_Buf_Cur) < سائيز)) {
    إذا (LLVM_UNLIKELY(!ٻاهريون_Buf_شروعات)) {
      إذا (بفر_گهڻائي == بفر_قسم::Unbuffered) {
        write_impl(Ptr, سائيز);
        أرجع *هذا;
      }
      // Set up a buffer and start over.
      سيٽ_Buffered();
      أرجع اكتب(Ptr, سائيز);
    }

    size_t انگ_بائيٽون = ٻاهريون_Buf_پڇاڙي - ٻاهريون_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    إذا (LLVM_UNLIKELY(ٻاهريون_Buf_Cur == ٻاهريون_Buf_شروعات)) {
      أكد(انگ_بائيٽون != 0 && "undefined behavior");
      size_t بائيٽون_منزل_Write_2 = سائيز - (سائيز % انگ_بائيٽون);
      write_impl(Ptr, بائيٽون_منزل_Write_2);
      size_t بائيٽون_بچيل_2 = سائيز - بائيٽون_منزل_Write_2;
      إذا (بائيٽون_بچيل_2 > size_t(ٻاهريون_Buf_پڇاڙي - ٻاهريون_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        أرجع اكتب(Ptr + بائيٽون_منزل_Write_2, بائيٽون_بچيل_2);
      }
      copy_منزل_بفر(Ptr + بائيٽون_منزل_Write_2, بائيٽون_بچيل_2);
      أرجع *هذا;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_منزل_بفر(Ptr, انگ_بائيٽون);
    flush_nonempty();
    أرجع اكتب(Ptr + انگ_بائيٽون, سائيز - انگ_بائيٽون);
  }

  copy_منزل_بفر(Ptr, سائيز);

  أرجع *هذا;
}

فراغ raw_ostream::copy_منزل_بفر(ثابت حرف *Ptr, size_t سائيز) {
  أكد(سائيز <= size_t(ٻاهريون_Buf_پڇاڙي - ٻاهريون_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  بدل (سائيز) {
  حالة 4: ٻاهريون_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  حالة 3: ٻاهريون_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  حالة 2: ٻاهريون_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  حالة 1: ٻاهريون_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  حالة 0: اكسر;
  افتراضي:
    يادگيري_نقل(ٻاهريون_Buf_Cur, Ptr, سائيز);
    اكسر;
  }

  ٻاهريون_Buf_Cur += سائيز;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::عامل<<(function_ref<صحيح(حرف *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t سائيز = 128;
  size_t بفر_بائيٽون_کاٻو = ٻاهريون_Buf_پڇاڙي - ٻاهريون_Buf_Cur;
  إذا (بفر_بائيٽون_کاٻو > 3) {
    صحيح N = Snprint(ٻاهريون_Buf_Cur, بفر_بائيٽون_کاٻو);
    إذا (N < 0)
      أرجع *هذا;
    // Common case is that we have plenty of space.
    إذا (size_t(N) < بفر_بائيٽون_کاٻو) {
      ٻاهريون_Buf_Cur += N;
      أرجع *هذا;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    سائيز = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_ويڪٽر<حرف, 128> V;
  لكل (;;) {
    V.resize(سائيز);
    صحيح N = Snprint(V.معلومات(), سائيز);
    إذا (N < 0)
      أرجع *هذا;
    إذا (size_t(N) < سائيز)
      أرجع اكتب(V.معلومات(), N);
    سائيز = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::عامل<<(ثابت formatv_object_بنياد &Obj) {
  Obj.format(*هذا);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(ثابت FormattedString &FS) {
  بنانشان کاٻو_Indent = 0;
  بنانشان ساڄو_Indent = 0;
  ثابت ssize_t Difference = FS.ويڪر - FS.Str.سائيز_2();
  إذا (Difference > 0) {
    بدل (FS.Justify) {
    حالة FormattedString::JustifyNone:
      اكسر;
    حالة FormattedString::Justify_کاٻو:
      ساڄو_Indent = Difference;
      اكسر;
    حالة FormattedString::Justify_ساڄو:
      کاٻو_Indent = Difference;
      اكسر;
    حالة FormattedString::JustifyCenter:
      کاٻو_Indent = Difference / 2;
      ساڄو_Indent = Difference - کاٻو_Indent;
      اكسر;
    }
  }
  indent(کاٻو_Indent);
  (*هذا) << FS.Str;
  indent(ساڄو_Indent);
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(ثابت Formatted_عدد &FN) {
  إذا (FN.Hex) {
    HexPrintStyle Style;
    إذا (FN.مٿانهون_2 && FN.Hex_اڳياڙي)
      Style = HexPrintStyle::اڳياڙي_مٿانهون;
    وإلا إذا (FN.مٿانهون_2 && !FN.Hex_اڳياڙي)
      Style = HexPrintStyle::مٿانهون_2;
    وإلا إذا (!FN.مٿانهون_2 && FN.Hex_اڳياڙي)
      Style = HexPrintStyle::اڳياڙي_هيٺاهون;
    وإلا
      Style = HexPrintStyle::هيٺاهون_2;
    llvm::write_hex(*هذا, FN.Hex_قدر, Style, FN.ويڪر);
  } وإلا {
    llvm::SmallString<16> بفر;
    llvm::raw_svector_ostream وهڪرو(بفر);
    llvm::write_integer(وهڪرو, FN.Dec_قدر, 0, IntegerStyle::Integer);
    إذا (بفر.سائيز_2() < FN.ويڪر)
      indent(FN.ويڪر - بفر.سائيز_2());
    (*هذا) << بفر;
  }
  أرجع *هذا;
}

raw_ostream &raw_ostream::عامل<<(ثابت Formatted_بائيٽون &FB) {
  إذا (FB.بائيٽون_2.خالي_جايون())
    أرجع *هذا;

  size_t سٽ_اشاريو = 0;
  خودڪار بائيٽون_2 = FB.بائيٽون_2;
  ثابت size_t سائيز = بائيٽون_2.سائيز_2();
  HexPrintStyle HPS = FB.مٿانهون_2 ? HexPrintStyle::مٿانهون_2 : HexPrintStyle::هيٺاهون_2;
  uint64_t وٿي_ويڪر = 0;
  إذا (FB.پهريون_Byte_وٿي) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t سٽون = سائيز / FB.انگ_في_سٽ;
    uint64_t Max_وٿي = *FB.پهريون_Byte_وٿي + سٽون * FB.انگ_في_سٽ;
    بنانشان Power = 0;
    إذا (Max_وٿي > 0)
      Power = llvm::Log2_64_Ceil(Max_وٿي);
    وٿي_ويڪر = std::max<uint64_t>(4, llvm::align_منزل(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  بنانشان انگ_Byte_Groups =
      align_منزل(FB.انگ_في_سٽ, FB.Byte_گروهه_سائيز) / FB.Byte_گروهه_سائيز;
  بنانشان بلاڪ_Char_ويڪر = FB.انگ_في_سٽ * 2 + انگ_Byte_Groups - 1;

  طالما (!بائيٽون_2.خالي_جايون()) {
    indent(FB.IndentLevel);

    إذا (FB.پهريون_Byte_وٿي) {
      uint64_t وٿي = *FB.پهريون_Byte_وٿي;
      llvm::write_hex(*هذا, وٿي + سٽ_اشاريو, HPS, وٿي_ويڪر);
      *هذا << ": ";
    }

    خودڪار سٽ = بائيٽون_2.take_front(FB.انگ_في_سٽ);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    لكل (size_t I = 0; I < سٽ.سائيز_2(); ++I, CharsPrinted += 2) {
      إذا (I && (I % FB.Byte_گروهه_سائيز) == 0) {
        ++CharsPrinted;
        *هذا << " ";
      }
      llvm::write_hex(*هذا, سٽ[I], HPS, 2);
    }

    إذا (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      أكد(بلاڪ_Char_ويڪر >= CharsPrinted);
      indent(بلاڪ_Char_ويڪر - CharsPrinted + 2);
      *هذا << "|";

      // Print the ASCII char values for each byte on this line
      لكل (uint8_t Byte : سٽ) {
        إذا (isPrint(Byte))
          *هذا << static_cast<حرف>(Byte);
        وإلا
          *هذا << '.';
      }
      *هذا << '|';
    }

    بائيٽون_2 = بائيٽون_2.drop_front(سٽ.سائيز_2());
    سٽ_اشاريو += سٽ.سائيز_2();
    إذا (سٽ_اشاريو < سائيز)
      *هذا << '\n';
  }
  أرجع *هذا;
}

سانچو <حرف C>
ساكن raw_ostream &write_padding(raw_ostream &OS, بنانشان انگ_Chars) {
  ساكن ثابت حرف Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  إذا (انگ_Chars < std::سائيز_2(Chars))
    أرجع OS.اكتب(Chars, انگ_Chars);

  طالما (انگ_Chars) {
    بنانشان انگ_منزل_Write = std::min(انگ_Chars, (بنانشان)std::سائيز_2(Chars) - 1);
    OS.اكتب(Chars, انگ_منزل_Write);
    انگ_Chars -= انگ_منزل_Write;
  }
  أرجع OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(بنانشان انگ_Spaces) {
  أرجع write_padding<' '>(*هذا, انگ_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(بنانشان انگ_Zeros) {
  أرجع write_padding<'\0'>(*هذا, انگ_Zeros);
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

  أرجع سچ;
}

raw_ostream &raw_ostream::changeColor(تعداد Colors colors, منطقي bold, منطقي bg) {
  إذا (!prepare_colors())
    أرجع *هذا;

  ثابت حرف *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<حرف>(colors), bold, bg);
  إذا (colorcode)
    اكتب(colorcode, لڙي_ڊيگهه(colorcode));
  أرجع *هذا;
}

raw_ostream &raw_ostream::resetColor() {
  إذا (!prepare_colors())
    أرجع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::ResetColor())
    اكتب(colorcode, لڙي_ڊيگهه(colorcode));
  أرجع *هذا;
}

raw_ostream &raw_ostream::ابتو_Color() {
  إذا (!prepare_colors())
    أرجع *هذا;

  إذا (ثابت حرف *colorcode = sys::Process::Output_ابتو())
    اكتب(colorcode, لڙي_ڊيگهه(colorcode));
  أرجع *هذا;
}

فراغ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ساكن صحيح getFD(StringRef Filename, std::error_ڪوڊ &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  أكد((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  إذا (Filename == "-") {
    EC = std::error_ڪوڊ();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_گهڻائي(Flags);
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

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ڪوڊ &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::جو_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ڪوڊ &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::جو_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ڪوڊ &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::جو_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ڪوڊ &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ڪوڊ &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), سچ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(صحيح fd, منطقي shouldClose, منطقي unbuffered,
                               OStream_قسم K)
    : raw_pwrite_وهڪرو(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  إذا (FD < 0 ) {
    ShouldClose = خطأ;
    أرجع;
  }

  enable_colors(سچ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  إذا (FD <= STDERR_FILENO)
    ShouldClose = خطأ;

#جيڪڏهن_وصف_ٿيل _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#جيڪڏهن_پورو

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_حالت حالت;
  std::error_ڪوڊ EC = حالت_2(FD, حالت);
  IsRegularFile = حالت.type() == sys::fs::file_type::regular_file;
#جيڪڏهن_وصف_ٿيل _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#وإلا
  SupportsSeeking = !EC && loc != (off_t)-1;
#جيڪڏهن_پورو
  إذا (!SupportsSeeking)
    pos = 0;
  وإلا
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  إذا (FD >= 0) {
    flush();
    إذا (ShouldClose) {
      إذا (خودڪار EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#جيڪڏهن_وصف_ٿيل __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  إذا (FD == 2) أرجع;
#جيڪڏهن_پورو

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  إذا (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().پيغام());
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
  Small_ويڪٽر<wchar_t, 256> Wide_متن;

  // Fall back to ::write if it wasn't valid UTF-8.
  إذا (خودڪار EC = sys::windows::UTF8_منزل_UTF16(معلومات_2, Wide_متن))
    أرجع خطأ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_سائيز = Wide_متن.سائيز_2();
  إذا (!RunningWindows8OrGreater())
    Max_Write_سائيز = 32767;

  size_t WCharsWritten = 0;
  افعل {
    size_t WChars_منزل_Write =
        std::min(Max_Write_سائيز, Wide_متن.سائيز_2() - WCharsWritten);
    DWORD ActuallyWritten;
    منطقي Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_متن[WCharsWritten],
                        WChars_منزل_Write, &ActuallyWritten,
                        /*Reserved=*/خالي_اشارو);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    إذا (!Success)
      أرجع خطأ;

    WCharsWritten += ActuallyWritten;
  } طالما (WCharsWritten != Wide_متن.سائيز_2());
  أرجع سچ;
}
#جيڪڏهن_پورو

فراغ raw_fd_ostream::write_impl(ثابت حرف *Ptr, size_t سائيز) {
  إذا (Tied_وهڪرو)
    Tied_وهڪرو->flush();

  أكد(FD >= 0 && "File already closed.");
  pos += سائيز;

#إذا defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  إذا (IsWindowsConsole)
    إذا (write_console_impl(FD, StringRef(Ptr, سائيز)))
      أرجع;
#جيڪڏهن_پورو

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_سائيز = INT32_MAX;

#إذا defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_سائيز = 1024 * 1024 * 1024;
#جيڪڏهن_پورو

  افعل {
    size_t Chunk_سائيز = std::min(سائيز, Max_Write_سائيز);
    ssize_t ret = ::اكتب(FD, Ptr, Chunk_سائيز);

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
#جيڪڏهن_وصف_ٿيل EWOULDBLOCK
          || errno == EWOULDBLOCK
#جيڪڏهن_پورو
          )
        استمر;

#جيڪڏهن_وصف_ٿيل _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_آخري_Error = Get_آخري_Error();
      إذا (Win_آخري_Error == ERROR_BROKEN_PIPE ||
          (Win_آخري_Error == ERROR_NO_معلومات && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#جيڪڏهن_پورو
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_ڪوڊ());
      اكسر;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    سائيز -= ret;
  } طالما (سائيز > 0);
}

فراغ raw_fd_ostream::close() {
  أكد(ShouldClose);
  ShouldClose = خطأ;
  flush();
  إذا (خودڪار EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  أكد(SupportsSeeking && "Stream does not support seeking!");
  flush();
#جيڪڏهن_وصف_ٿيل _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#وإلا
  pos = ::lseek(FD, off, SEEK_SET);
#جيڪڏهن_پورو
  إذا (pos == (uint64_t)-1)
    error_detected(errno_As_Error_ڪوڊ());
  أرجع pos;
}

فراغ raw_fd_ostream::pwrite_impl(ثابت حرف *Ptr, size_t سائيز,
                                 uint64_t وٿي) {
  uint64_t Pos = tell();
  seek(وٿي);
  اكتب(Ptr, سائيز);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_بفر_سائيز() ثابت {
#إذا defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  إذا (IsWindowsConsole)
    أرجع 0;
  أرجع raw_ostream::preferred_بفر_سائيز();
#وإلاإذا defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  أرجع raw_ostream::preferred_بفر_سائيز();
#وإلا
  أكد(FD >= 0 && "File not yet open!");
  بنية stat statbuf;
  إذا (fstat(FD, &statbuf) != 0)
    أرجع 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  إذا (S_ISCHR(statbuf.st_گهڻائي) && is_displayed())
    أرجع 0;
  // Return the preferred block size.
  أرجع statbuf.st_blksize;
#جيڪڏهن_پورو
}

منطقي raw_fd_ostream::is_displayed() ثابت {
  أرجع sys::Process::FileDescriptorIsDisplayed(FD);
}

منطقي raw_fd_ostream::has_colors() ثابت {
  إذا (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  أرجع *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::تالو() {
  std::error_ڪوڊ EC = sys::fs::تالو_File(FD);
  إذا (!EC)
    أرجع sys::fs::FileLocker(FD);
  أرجع error_ڪوڊ_منزل_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_تالو_For(Duration ثابت& Timeout) {
  std::error_ڪوڊ EC = sys::fs::try_تالو_File(FD, Timeout.getDuration());
  إذا (!EC)
    أرجع sys::fs::FileLocker(FD);
  أرجع error_ڪوڊ_منزل_Error(EC);
}

فراغ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_ڪوڊ EC;

  // On z/OS we need to enable auto conversion
  ساكن std::error_ڪوڊ EC1 = enableAutoConversion(STDOUT_FILENO);
  أكد(!EC1);
  (فراغ)EC1;

  ساكن raw_fd_ostream S("-", EC, sys::fs::جو_None);
  أكد(!EC);
  أرجع S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ساكن std::error_ڪوڊ EC = enableAutoConversion(STDERR_FILENO);
  أكد(!EC);
  (فراغ)EC;

  // Set standard error to be unbuffered.
  ساكن raw_fd_ostream S(STDERR_FILENO, خطأ, سچ);
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

raw_fd_وهڪرو::raw_fd_وهڪرو(StringRef Filename, std::error_ڪوڊ &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::جو_None),
                     سچ, خطأ, OStream_قسم::OK_FDStream) {
  إذا (EC)
    أرجع;

  إذا (!isRegularFile())
    EC = std::make_error_ڪوڊ(std::errc::invalid_دليل);
}

raw_fd_وهڪرو::raw_fd_وهڪرو(صحيح fd, منطقي shouldClose)
    : raw_fd_ostream(fd, shouldClose, خطأ, OStream_قسم::OK_FDStream) {}

ssize_t raw_fd_وهڪرو::اقرأ(حرف *Ptr, size_t سائيز) {
  أكد(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::اقرأ(get_fd(), (فراغ *)Ptr, سائيز);
  إذا (Ret >= 0)
    inc_pos(Ret);
  وإلا
    error_detected(errno_As_Error_ڪوڊ());
  أرجع Ret;
}

منطقي raw_fd_وهڪرو::classof(ثابت raw_ostream *OS) {
  أرجع OS->get_قسم_2() == OStream_قسم::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

فراغ raw_string_ostream::write_impl(ثابت حرف *Ptr, size_t سائيز) {
  OS.append(Ptr, سائيز);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ثابت { أرجع OS.سائيز_2(); }

فراغ raw_svector_ostream::write_impl(ثابت حرف *Ptr, size_t سائيز) {
  OS.append(Ptr, Ptr + سائيز);
}

فراغ raw_svector_ostream::pwrite_impl(ثابت حرف *Ptr, size_t سائيز,
                                      uint64_t وٿي) {
  يادگيري_نقل(OS.معلومات() + وٿي, Ptr, سائيز);
}

منطقي raw_svector_ostream::classof(ثابت raw_ostream *OS) {
  أرجع OS->get_قسم_2() == OStream_قسم::OK_SVec_وهڪرو;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#جيڪڏهن_پورو
}

فراغ raw_null_ostream::write_impl(ثابت حرف *Ptr, size_t سائيز) {
}

uint64_t raw_null_ostream::current_pos() ثابت {
  أرجع 0;
}

فراغ raw_null_ostream::pwrite_impl(ثابت حرف *Ptr, size_t سائيز,
                                   uint64_t وٿي) {}

فراغ raw_pwrite_وهڪرو::anchor() {}

فراغ بفر_ostream::anchor() {}

فراغ بفر_unique_ostream::anchor() {}

ساكن منطقي isNullDeviceName(StringRef OutputFileName) {
#جيڪڏهن_وصف_ٿيل _WIN32
  إذا (OutputFileName.equals_insensitive("NUL"))
    أرجع سچ;
#جيڪڏهن_پورو
  أرجع OutputFileName == "/dev/null";
}

Error llvm::write_منزل_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  إذا (OutputFileName == "-")
    أرجع Write(outs());

  إذا (isNullDeviceName(OutputFileName)) {
    raw_null_ostream ٻاهريون;
    أرجع Write(ٻاهريون);
  }

  بنانشان گهڻائي = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::عارضي_File> عارضي =
      sys::fs::عارضي_File::create(OutputFileName + ".temp-stream-%%%%%%", گهڻائي);
  إذا (!عارضي)
    أرجع createFileError(OutputFileName, عارضي.takeError());

#إذا defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  إذا (خودڪار EC = llvm::copy_File_ٽيگ_خاصيتون(OutputFileName.str(), عارضي->FD)) {
    إذا (EC != std::errc::no_such_file_or_directory)
      أرجع createFileError(OutputFileName, EC);
  }
#جيڪڏهن_پورو

  raw_fd_ostream ٻاهريون(عارضي->FD, خطأ);

  إذا (Error E = Write(ٻاهريون)) {
    إذا (Error DiscardError = عارضي->discard())
      أرجع joinErrors(std::move(E), std::move(DiscardError));
    أرجع E;
  }
  ٻاهريون.flush();

  أرجع عارضي->keep(OutputFileName);
}
