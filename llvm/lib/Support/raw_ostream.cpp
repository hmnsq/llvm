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

#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Duration.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MathExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/NativeFormatting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Process.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ <algorithm>
#ಅನ್ತರ್ಭಾವ <cerrno>
#ಅನ್ತರ್ಭಾವ <cstdio>
#ಅನ್ತರ್ಭಾವ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ಆದರೆ defined(HAVE_UNISTD_H)
# include <unistd.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(__CYGWIN__)
#ಅನ್ತರ್ಭಾವ <io.h>
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(_MSC_VER)
#ಅನ್ತರ್ಭಾವ <io.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDIN_FILENO
# define STDIN_FILENO 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDOUT_FILENO
# define STDOUT_FILENO 1
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDERR_FILENO
# define STDERR_FILENO 2
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "llvm/Support/ConvertUTF.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು_Buf_Cur == ಹೊರಹೋಗು_Buf_ಆರಂಭ &&
         "raw_ostream destructor called with non-empty buffer!");

  ಆದರೆ (ಬಫರ್_ಬಹುಲಕ == ಬಫರ್_ಬಗೆ::ಆಂತರಿಕ_ಬಫರ್)
    ಅಳಿಸು [] ಹೊರಹೋಗು_Buf_ಆರಂಭ;
}

size_t raw_ostream::preferred_ಬಫರ್_ಗಾತ್ರ() ಸ್ಥಿರಾಂಕ {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ಹಿಂತಿರುಗಿ (16 * 1024);
#ಇಲ್ಲದಿದ್ದರೆ
  // BUFSIZ is intended to be a reasonable default.
  ಹಿಂತಿರುಗಿ BUFSIZ;
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ raw_ostream::ಗಣ_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ಆದರೆ (size_t ಗಾತ್ರ_2 = preferred_ಬಫರ್_ಗಾತ್ರ())
    ಗಣ_ಬಫರ್_ಗಾತ್ರ(ಗಾತ್ರ_2);
  ಇಲ್ಲದಿದ್ದರೆ
    // It may return 0, meaning this stream should be unbuffered.
    ಗಣ_Unbuffered();
}

ಶೂನ್ಯ raw_ostream::ಗಣ_ಬಫರ್_And_ಬಹುಲಕ(ಅಕ್ಷರ *ಬಫರ್_ಆರಂಭ, size_t ಗಾತ್ರ_2,
                                   ಬಫರ್_ಬಗೆ ಬಹುಲಕ) {
  ಪ್ರತಿಜ್ಞಾ(((ಬಹುಲಕ == ಬಫರ್_ಬಗೆ::Unbuffered && !ಬಫರ್_ಆರಂಭ && ಗಾತ್ರ_2 == 0) ||
          (ಬಹುಲಕ != ಬಫರ್_ಬಗೆ::Unbuffered && ಬಫರ್_ಆರಂಭ && ಗಾತ್ರ_2 != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  ಪ್ರತಿಜ್ಞಾ(Get_ಅಂಕೆ_ಬೈಟುಗಳು_ಒಳ_ಬಫರ್() == 0 && "Current buffer is non-empty!");

  ಆದರೆ (ಬಫರ್_ಬಹುಲಕ == ಬಫರ್_ಬಗೆ::ಆಂತರಿಕ_ಬಫರ್)
    ಅಳಿಸು [] ಹೊರಹೋಗು_Buf_ಆರಂಭ;
  ಹೊರಹೋಗು_Buf_ಆರಂಭ = ಬಫರ್_ಆರಂಭ;
  ಹೊರಹೋಗು_Buf_ಕೊನೆ = ಹೊರಹೋಗು_Buf_ಆರಂಭ+ಗಾತ್ರ_2;
  ಹೊರಹೋಗು_Buf_Cur = ಹೊರಹೋಗು_Buf_ಆರಂಭ;
  ಬಫರ್_ಬಹುಲಕ = ಬಹುಲಕ;

  ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು_Buf_ಆರಂಭ <= ಹೊರಹೋಗು_Buf_ಕೊನೆ && "Invalid size!");
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ N) {
  write_integer(*ಇದು, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ದೀರ್ಘ N) {
  write_integer(*ಇದು, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ N) {
  write_integer(*ಇದು, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ದೀರ್ಘ ದೀರ್ಘ N) {
  write_integer(*ಇದು, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::write_hex(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ N) {
  llvm::write_hex(*ಇದು, N, HexPrintStyle::ಅಧೋ_2);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(Colors C) {
  ಆದರೆ (C == Colors::RESET)
    resetColor();
  ಇಲ್ಲದಿದ್ದರೆ
    changeColor(C);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::write_uuid(ಸ್ಥಿರಾಂಕ uuid_t UUID) {
  ಗಾಗಿ (ಪೂರ್ಣಾಂಕ Idx = 0; Idx < 16; ++Idx) {
    *ಇದು << format("%02" PRIX32, UUID[Idx]);
    ಆದರೆ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *ಇದು << "-";
  }
  ಹಿಂತಿರುಗಿ *ಇದು;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        ತಾರ್ಕಿಕ UseHexEscapes) {
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c : Str) {
    ಆಯ್ಕೆ (c) {
    ಸ್ಥಿತಿ '\\':
      *ಇದು << '\\' << '\\';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\t':
      *ಇದು << '\\' << 't';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '\n':
      *ಇದು << '\\' << 'n';
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ '"':
      *ಇದು << '\\' << '"';
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಆದರೆ (isPrint(c)) {
        *ಇದು << c;
        ನಿಲ್ಲಿಸು;
      }

      // Write out the escaped representation.
      ಆದರೆ (UseHexEscapes) {
        *ಇದು << '\\' << 'x';
        *ಇದು << hexdigit((c >> 4) & 0xF);
        *ಇದು << hexdigit((c >> 0) & 0xF);
      } ಇಲ್ಲದಿದ್ದರೆ {
        // Always use a full 3-character octal escape.
        *ಇದು << '\\';
        *ಇದು << ಅಕ್ಷರ('0' + ((c >> 6) & 7));
        *ಇದು << ಅಕ್ಷರ('0' + ((c >> 3) & 7));
        *ಇದು << ಅಕ್ಷರ('0' + ((c >> 0) & 7));
      }
    }
  }

  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *P) {
  llvm::write_hex(*ಇದು, (uintptr_t)P, HexPrintStyle::ಪೂರ್ವಪ್ರತ್ಯಯ_ಅಧೋ);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ದ್ವಿಗುಣ N) {
  llvm::write_double(*ಇದು, N, FloatStyle::Exponent);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಶೂನ್ಯ raw_ostream::flush_nonempty() {
  ಪ್ರತಿಜ್ಞಾ(ಹೊರಹೋಗು_Buf_Cur > ಹೊರಹೋಗು_Buf_ಆರಂಭ && "Invalid call to flush_nonempty.");
  size_t ಉದ್ದಳತೆ = ಹೊರಹೋಗು_Buf_Cur - ಹೊರಹೋಗು_Buf_ಆರಂಭ;
  ಹೊರಹೋಗು_Buf_Cur = ಹೊರಹೋಗು_Buf_ಆರಂಭ;
  write_impl(ಹೊರಹೋಗು_Buf_ಆರಂಭ, ಉದ್ದಳತೆ);
}

raw_ostream &raw_ostream::ಲಿಖ(ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ C) {
  // Group exceptional cases into a single branch.
  ಆದರೆ (LLVM_UNLIKELY(ಹೊರಹೋಗು_Buf_Cur >= ಹೊರಹೋಗು_Buf_ಕೊನೆ)) {
    ಆದರೆ (LLVM_UNLIKELY(!ಹೊರಹೋಗು_Buf_ಆರಂಭ)) {
      ಆದರೆ (ಬಫರ್_ಬಹುಲಕ == ಬಫರ್_ಬಗೆ::Unbuffered) {
        write_impl(reinterpret_cast<ಅಕ್ಷರ *>(&C), 1);
        ಹಿಂತಿರುಗಿ *ಇದು;
      }
      // Set up a buffer and start over.
      ಗಣ_Buffered();
      ಹಿಂತಿರುಗಿ ಲಿಖ(C);
    }

    flush_nonempty();
  }

  *ಹೊರಹೋಗು_Buf_Cur++ = C;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಲಿಖ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  // Group exceptional cases into a single branch.
  ಆದರೆ (LLVM_UNLIKELY(size_t(ಹೊರಹೋಗು_Buf_ಕೊನೆ - ಹೊರಹೋಗು_Buf_Cur) < ಗಾತ್ರ_2)) {
    ಆದರೆ (LLVM_UNLIKELY(!ಹೊರಹೋಗು_Buf_ಆರಂಭ)) {
      ಆದರೆ (ಬಫರ್_ಬಹುಲಕ == ಬಫರ್_ಬಗೆ::Unbuffered) {
        write_impl(Ptr, ಗಾತ್ರ_2);
        ಹಿಂತಿರುಗಿ *ಇದು;
      }
      // Set up a buffer and start over.
      ಗಣ_Buffered();
      ಹಿಂತಿರುಗಿ ಲಿಖ(Ptr, ಗಾತ್ರ_2);
    }

    size_t ಅಂಕೆ_ಬೈಟುಗಳು = ಹೊರಹೋಗು_Buf_ಕೊನೆ - ಹೊರಹೋಗು_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ಆದರೆ (LLVM_UNLIKELY(ಹೊರಹೋಗು_Buf_Cur == ಹೊರಹೋಗು_Buf_ಆರಂಭ)) {
      ಪ್ರತಿಜ್ಞಾ(ಅಂಕೆ_ಬೈಟುಗಳು != 0 && "undefined behavior");
      size_t ಬೈಟುಗಳು_ಗಮ್ಯ_Write_2 = ಗಾತ್ರ_2 - (ಗಾತ್ರ_2 % ಅಂಕೆ_ಬೈಟುಗಳು);
      write_impl(Ptr, ಬೈಟುಗಳು_ಗಮ್ಯ_Write_2);
      size_t ಬೈಟುಗಳು_ಉಳಿದ_2 = ಗಾತ್ರ_2 - ಬೈಟುಗಳು_ಗಮ್ಯ_Write_2;
      ಆದರೆ (ಬೈಟುಗಳು_ಉಳಿದ_2 > size_t(ಹೊರಹೋಗು_Buf_ಕೊನೆ - ಹೊರಹೋಗು_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        ಹಿಂತಿರುಗಿ ಲಿಖ(Ptr + ಬೈಟುಗಳು_ಗಮ್ಯ_Write_2, ಬೈಟುಗಳು_ಉಳಿದ_2);
      }
      copy_ಗಮ್ಯ_ಬಫರ್(Ptr + ಬೈಟುಗಳು_ಗಮ್ಯ_Write_2, ಬೈಟುಗಳು_ಉಳಿದ_2);
      ಹಿಂತಿರುಗಿ *ಇದು;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_ಗಮ್ಯ_ಬಫರ್(Ptr, ಅಂಕೆ_ಬೈಟುಗಳು);
    flush_nonempty();
    ಹಿಂತಿರುಗಿ ಲಿಖ(Ptr + ಅಂಕೆ_ಬೈಟುಗಳು, ಗಾತ್ರ_2 - ಅಂಕೆ_ಬೈಟುಗಳು);
  }

  copy_ಗಮ್ಯ_ಬಫರ್(Ptr, ಗಾತ್ರ_2);

  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಶೂನ್ಯ raw_ostream::copy_ಗಮ್ಯ_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  ಪ್ರತಿಜ್ಞಾ(ಗಾತ್ರ_2 <= size_t(ಹೊರಹೋಗು_Buf_ಕೊನೆ - ಹೊರಹೋಗು_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  ಆಯ್ಕೆ (ಗಾತ್ರ_2) {
  ಸ್ಥಿತಿ 4: ಹೊರಹೋಗು_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  ಸ್ಥಿತಿ 3: ಹೊರಹೋಗು_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  ಸ್ಥಿತಿ 2: ಹೊರಹೋಗು_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  ಸ್ಥಿತಿ 1: ಹೊರಹೋಗು_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  ಸ್ಥಿತಿ 0: ನಿಲ್ಲಿಸು;
  ಪೂರ್ವನಿಯೋಜಿತ:
    ಸ್ಮರಣೆ_ನಕಲು(ಹೊರಹೋಗು_Buf_Cur, Ptr, ಗಾತ್ರ_2);
    ನಿಲ್ಲಿಸು;
  }

  ಹೊರಹೋಗು_Buf_Cur += ಗಾತ್ರ_2;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(function_ref<ಪೂರ್ಣಾಂಕ(ಅಕ್ಷರ *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t ಗಾತ್ರ_2 = 128;
  size_t ಬಫರ್_ಬೈಟುಗಳು_ಎಡ = ಹೊರಹೋಗು_Buf_ಕೊನೆ - ಹೊರಹೋಗು_Buf_Cur;
  ಆದರೆ (ಬಫರ್_ಬೈಟುಗಳು_ಎಡ > 3) {
    ಪೂರ್ಣಾಂಕ N = Snprint(ಹೊರಹೋಗು_Buf_Cur, ಬಫರ್_ಬೈಟುಗಳು_ಎಡ);
    ಆದರೆ (N < 0)
      ಹಿಂತಿರುಗಿ *ಇದು;
    // Common case is that we have plenty of space.
    ಆದರೆ (size_t(N) < ಬಫರ್_ಬೈಟುಗಳು_ಎಡ) {
      ಹೊರಹೋಗು_Buf_Cur += N;
      ಹಿಂತಿರುಗಿ *ಇದು;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    ಗಾತ್ರ_2 = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_ಸದಿಶ<ಅಕ್ಷರ, 128> V;
  ಗಾಗಿ (;;) {
    V.resize(ಗಾತ್ರ_2);
    ಪೂರ್ಣಾಂಕ N = Snprint(V.ದತ್ತಾಂಶ(), ಗಾತ್ರ_2);
    ಆದರೆ (N < 0)
      ಹಿಂತಿರುಗಿ *ಇದು;
    ಆದರೆ (size_t(N) < ಗಾತ್ರ_2)
      ಹಿಂತಿರುಗಿ ಲಿಖ(V.ದತ್ತಾಂಶ(), N);
    ಗಾತ್ರ_2 = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಸ್ಥಿರಾಂಕ formatv_object_ತಳ &Obj) {
  Obj.format(*ಇದು);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಸ್ಥಿರಾಂಕ FormattedString &FS) {
  ಚಿಹ್ನೆರಹಿತ ಎಡ_Indent = 0;
  ಚಿಹ್ನೆರಹಿತ ಬಲ_Indent = 0;
  ಸ್ಥಿರಾಂಕ ssize_t Difference = FS.ಅಗಲ - FS.Str.ಗಾತ್ರ_3();
  ಆದರೆ (Difference > 0) {
    ಆಯ್ಕೆ (FS.Justify) {
    ಸ್ಥಿತಿ FormattedString::JustifyNone:
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FormattedString::Justify_ಎಡ:
      ಬಲ_Indent = Difference;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FormattedString::Justify_ಬಲ:
      ಎಡ_Indent = Difference;
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ FormattedString::JustifyCenter:
      ಎಡ_Indent = Difference / 2;
      ಬಲ_Indent = Difference - ಎಡ_Indent;
      ನಿಲ್ಲಿಸು;
    }
  }
  indent(ಎಡ_Indent);
  (*ಇದು) << FS.Str;
  indent(ಬಲ_Indent);
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಸ್ಥಿರಾಂಕ Formatted_ಸಂಖ್ಯೆ &FN) {
  ಆದರೆ (FN.Hex) {
    HexPrintStyle Style;
    ಆದರೆ (FN.ಊರ್ಧ್ವ_2 && FN.Hex_ಪೂರ್ವಪ್ರತ್ಯಯ)
      Style = HexPrintStyle::ಪೂರ್ವಪ್ರತ್ಯಯ_ಊರ್ಧ್ವ;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (FN.ಊರ್ಧ್ವ_2 && !FN.Hex_ಪೂರ್ವಪ್ರತ್ಯಯ)
      Style = HexPrintStyle::ಊರ್ಧ್ವ_2;
    ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!FN.ಊರ್ಧ್ವ_2 && FN.Hex_ಪೂರ್ವಪ್ರತ್ಯಯ)
      Style = HexPrintStyle::ಪೂರ್ವಪ್ರತ್ಯಯ_ಅಧೋ;
    ಇಲ್ಲದಿದ್ದರೆ
      Style = HexPrintStyle::ಅಧೋ_2;
    llvm::write_hex(*ಇದು, FN.Hex_ಮೌಲ್ಯ, Style, FN.ಅಗಲ);
  } ಇಲ್ಲದಿದ್ದರೆ {
    llvm::SmallString<16> ಬಫರ್;
    llvm::raw_svector_ostream ಹರಿವು(ಬಫರ್);
    llvm::write_integer(ಹರಿವು, FN.Dec_ಮೌಲ್ಯ, 0, IntegerStyle::Integer);
    ಆದರೆ (ಬಫರ್.ಗಾತ್ರ_3() < FN.ಅಗಲ)
      indent(FN.ಅಗಲ - ಬಫರ್.ಗಾತ್ರ_3());
    (*ಇದು) << ಬಫರ್;
  }
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಕ್ರಿಯಾಕಾರಕ<<(ಸ್ಥಿರಾಂಕ Formatted_ಬೈಟುಗಳು &FB) {
  ಆದರೆ (FB.ಬೈಟುಗಳು_2.ಖಾಲಿಗಳು())
    ಹಿಂತಿರುಗಿ *ಇದು;

  size_t ಸಾಲು_ಸೂಚ್ಯಂಕ = 0;
  ಸ್ವಯಂಚಾಲಿತ ಬೈಟುಗಳು_2 = FB.ಬೈಟುಗಳು_2;
  ಸ್ಥಿರಾಂಕ size_t ಗಾತ್ರ_2 = ಬೈಟುಗಳು_2.ಗಾತ್ರ_3();
  HexPrintStyle HPS = FB.ಊರ್ಧ್ವ_2 ? HexPrintStyle::ಊರ್ಧ್ವ_2 : HexPrintStyle::ಅಧೋ_2;
  uint64_t ಸ್ಥಾನಾಂತರ_ಅಗಲ = 0;
  ಆದರೆ (FB.ಮೊದಲ_Byte_ಸ್ಥಾನಾಂತರ) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t ಸಾಲುಗಳು = ಗಾತ್ರ_2 / FB.ಅಂಕೆ_ಪ್ರತಿ_ಸಾಲು;
    uint64_t Max_ಸ್ಥಾನಾಂತರ = *FB.ಮೊದಲ_Byte_ಸ್ಥಾನಾಂತರ + ಸಾಲುಗಳು * FB.ಅಂಕೆ_ಪ್ರತಿ_ಸಾಲು;
    ಚಿಹ್ನೆರಹಿತ Power = 0;
    ಆದರೆ (Max_ಸ್ಥಾನಾಂತರ > 0)
      Power = llvm::Log2_64_Ceil(Max_ಸ್ಥಾನಾಂತರ);
    ಸ್ಥಾನಾಂತರ_ಅಗಲ = std::max<uint64_t>(4, llvm::align_ಗಮ್ಯ(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Byte_Groups =
      align_ಗಮ್ಯ(FB.ಅಂಕೆ_ಪ್ರತಿ_ಸಾಲು, FB.Byte_ಗುಂಪು_ಗಾತ್ರ) / FB.Byte_ಗುಂಪು_ಗಾತ್ರ;
  ಚಿಹ್ನೆರಹಿತ ಖಂಡ_Char_ಅಗಲ = FB.ಅಂಕೆ_ಪ್ರತಿ_ಸಾಲು * 2 + ಅಂಕೆ_Byte_Groups - 1;

  ಆಗ (!ಬೈಟುಗಳು_2.ಖಾಲಿಗಳು()) {
    indent(FB.IndentLevel);

    ಆದರೆ (FB.ಮೊದಲ_Byte_ಸ್ಥಾನಾಂತರ) {
      uint64_t ಸ್ಥಾನಾಂತರ = *FB.ಮೊದಲ_Byte_ಸ್ಥಾನಾಂತರ;
      llvm::write_hex(*ಇದು, ಸ್ಥಾನಾಂತರ + ಸಾಲು_ಸೂಚ್ಯಂಕ, HPS, ಸ್ಥಾನಾಂತರ_ಅಗಲ);
      *ಇದು << ": ";
    }

    ಸ್ವಯಂಚಾಲಿತ ಸಾಲು = ಬೈಟುಗಳು_2.take_front(FB.ಅಂಕೆ_ಪ್ರತಿ_ಸಾಲು);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    ಗಾಗಿ (size_t I = 0; I < ಸಾಲು.ಗಾತ್ರ_3(); ++I, CharsPrinted += 2) {
      ಆದರೆ (I && (I % FB.Byte_ಗುಂಪು_ಗಾತ್ರ) == 0) {
        ++CharsPrinted;
        *ಇದು << " ";
      }
      llvm::write_hex(*ಇದು, ಸಾಲು[I], HPS, 2);
    }

    ಆದರೆ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      ಪ್ರತಿಜ್ಞಾ(ಖಂಡ_Char_ಅಗಲ >= CharsPrinted);
      indent(ಖಂಡ_Char_ಅಗಲ - CharsPrinted + 2);
      *ಇದು << "|";

      // Print the ASCII char values for each byte on this line
      ಗಾಗಿ (uint8_t Byte : ಸಾಲು) {
        ಆದರೆ (isPrint(Byte))
          *ಇದು << static_cast<ಅಕ್ಷರ>(Byte);
        ಇಲ್ಲದಿದ್ದರೆ
          *ಇದು << '.';
      }
      *ಇದು << '|';
    }

    ಬೈಟುಗಳು_2 = ಬೈಟುಗಳು_2.drop_front(ಸಾಲು.ಗಾತ್ರ_3());
    ಸಾಲು_ಸೂಚ್ಯಂಕ += ಸಾಲು.ಗಾತ್ರ_3();
    ಆದರೆ (ಸಾಲು_ಸೂಚ್ಯಂಕ < ಗಾತ್ರ_2)
      *ಇದು << '\n';
  }
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಮಾದರಿ <ಅಕ್ಷರ C>
ಸ್ಥಿರ raw_ostream &write_padding(raw_ostream &OS, ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Chars) {
  ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ಆದರೆ (ಅಂಕೆ_Chars < std::ಗಾತ್ರ_3(Chars))
    ಹಿಂತಿರುಗಿ OS.ಲಿಖ(Chars, ಅಂಕೆ_Chars);

  ಆಗ (ಅಂಕೆ_Chars) {
    ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಗಮ್ಯ_Write = std::min(ಅಂಕೆ_Chars, (ಚಿಹ್ನೆರಹಿತ)std::ಗಾತ್ರ_3(Chars) - 1);
    OS.ಲಿಖ(Chars, ಅಂಕೆ_ಗಮ್ಯ_Write);
    ಅಂಕೆ_Chars -= ಅಂಕೆ_ಗಮ್ಯ_Write;
  }
  ಹಿಂತಿರುಗಿ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Spaces) {
  ಹಿಂತಿರುಗಿ write_padding<' '>(*ಇದು, ಅಂಕೆ_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_Zeros) {
  ಹಿಂತಿರುಗಿ write_padding<'\0'>(*ಇದು, ಅಂಕೆ_Zeros);
}

ತಾರ್ಕಿಕ raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ಆದರೆ (!ColorEnabled)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ಆದರೆ (sys::Process::ColorNeedsFlush() && !is_displayed())
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  ಆದರೆ (sys::Process::ColorNeedsFlush())
    flush();

  ಹಿಂತಿರುಗಿ ನಿಜ;
}

raw_ostream &raw_ostream::changeColor(ಎಣಿಕೆ Colors colors, ತಾರ್ಕಿಕ bold, ತಾರ್ಕಿಕ bg) {
  ಆದರೆ (!prepare_colors())
    ಹಿಂತಿರುಗಿ *ಇದು;

  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<ಅಕ್ಷರ>(colors), bold, bg);
  ಆದರೆ (colorcode)
    ಲಿಖ(colorcode, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(colorcode));
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::resetColor() {
  ಆದರೆ (!prepare_colors())
    ಹಿಂತಿರುಗಿ *ಇದು;

  ಆದರೆ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *colorcode = sys::Process::ResetColor())
    ಲಿಖ(colorcode, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(colorcode));
  ಹಿಂತಿರುಗಿ *ಇದು;
}

raw_ostream &raw_ostream::ಹಿಮ್ಮುಖ_Color() {
  ಆದರೆ (!prepare_colors())
    ಹಿಂತಿರುಗಿ *ಇದು;

  ಆದರೆ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *colorcode = sys::Process::Output_ಹಿಮ್ಮುಖ())
    ಲಿಖ(colorcode, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(colorcode));
  ಹಿಂತಿರುಗಿ *ಇದು;
}

ಶೂನ್ಯ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ getFD(StringRef Filename, std::error_ಸಂಕೇತ &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಪ್ರತಿಜ್ಞಾ((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ಆದರೆ (Filename == "-") {
    EC = std::error_ಸಂಕೇತ();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_ಬಹುಲಕ(Flags);
    ಹಿಂತಿರುಗಿ STDOUT_FILENO;
  }

  ಪೂರ್ಣಾಂಕ FD;
  ಆದರೆ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  ಇಲ್ಲದಿದ್ದರೆ
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ -1;

  ಹಿಂತಿರುಗಿ FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ಸಂಕೇತ &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::ದ_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ಸಂಕೇತ &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::ದ_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ಸಂಕೇತ &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::ದ_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ಸಂಕೇತ &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ಸಂಕೇತ &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), ನಿಜ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(ಪೂರ್ಣಾಂಕ fd, ತಾರ್ಕಿಕ shouldClose, ತಾರ್ಕಿಕ unbuffered,
                               OStream_ಬಗೆ K)
    : raw_pwrite_ಹರಿವು(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  ಸ್ವಯಂಚಾಲಿತ BypassSandbox = sys::sandbox::scopedDisable();

  ಆದರೆ (FD < 0 ) {
    ShouldClose = ಸುಳ್ಳು;
    ಹಿಂತಿರುಗಿ;
  }

  enable_colors(ನಿಜ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ಆದರೆ (FD <= STDERR_FILENO)
    ShouldClose = ಸುಳ್ಳು;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#ಆದರೆ_ಅಂತ್ಯ

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_ಸ್ಥಿತಿ ಸ್ಥಿತಿ_2;
  std::error_ಸಂಕೇತ EC = ಸ್ಥಿತಿ_3(FD, ಸ್ಥಿತಿ_2);
  IsRegularFile = ಸ್ಥಿತಿ_2.type() == sys::fs::file_type::regular_file;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#ಇಲ್ಲದಿದ್ದರೆ
  SupportsSeeking = !EC && loc != (off_t)-1;
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (!SupportsSeeking)
    pos = 0;
  ಇಲ್ಲದಿದ್ದರೆ
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ಆದರೆ (FD >= 0) {
    flush();
    ಆದರೆ (ShouldClose) {
      ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ಆದರೆ (FD == 2) ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ಆದರೆ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().ಸಂದೇಶ_2());
}

#ಆದರೆ defined(_WIN32)
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
ಸ್ಥಿರ ತಾರ್ಕಿಕ write_console_impl(ಪೂರ್ಣಾಂಕ FD, StringRef ದತ್ತಾಂಶ_2) {
  Small_ಸದಿಶ<wchar_t, 256> Wide_ಪಠ್ಯ;

  // Fall back to ::write if it wasn't valid UTF-8.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = sys::windows::UTF8_ಗಮ್ಯ_UTF16(ದತ್ತಾಂಶ_2, Wide_ಪಠ್ಯ))
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_ಗಾತ್ರ = Wide_ಪಠ್ಯ.ಗಾತ್ರ_3();
  ಆದರೆ (!RunningWindows8OrGreater())
    Max_Write_ಗಾತ್ರ = 32767;

  size_t WCharsWritten = 0;
  ಮಾಡು {
    size_t WChars_ಗಮ್ಯ_Write =
        std::min(Max_Write_ಗಾತ್ರ, Wide_ಪಠ್ಯ.ಗಾತ್ರ_3() - WCharsWritten);
    DWORD ActuallyWritten;
    ತಾರ್ಕಿಕ Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_ಪಠ್ಯ[WCharsWritten],
                        WChars_ಗಮ್ಯ_Write, &ActuallyWritten,
                        /*Reserved=*/ಶೂನ್ಯಸೂಚಕ);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ಆದರೆ (!Success)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    WCharsWritten += ActuallyWritten;
  } ಆಗ (WCharsWritten != Wide_ಪಠ್ಯ.ಗಾತ್ರ_3());
  ಹಿಂತಿರುಗಿ ನಿಜ;
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ raw_fd_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  ಆದರೆ (Tied_ಹರಿವು)
    Tied_ಹರಿವು->flush();

  ಪ್ರತಿಜ್ಞಾ(FD >= 0 && "File already closed.");
  pos += ಗಾತ್ರ_2;

#ಆದರೆ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ಆದರೆ (IsWindowsConsole)
    ಆದರೆ (write_console_impl(FD, StringRef(Ptr, ಗಾತ್ರ_2)))
      ಹಿಂತಿರುಗಿ;
#ಆದರೆ_ಅಂತ್ಯ

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_ಗಾತ್ರ = INT32_MAX;

#ಆದರೆ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_ಗಾತ್ರ = 1024 * 1024 * 1024;
#ಆದರೆ_ಅಂತ್ಯ

  ಮಾಡು {
    size_t Chunk_ಗಾತ್ರ = std::min(ಗಾತ್ರ_2, Max_Write_ಗಾತ್ರ);
    ssize_t ret = ::ಲಿಖ(FD, Ptr, Chunk_ಗಾತ್ರ);

    ಆದರೆ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ಆದರೆ (errno == EINTR || errno == EAGAIN
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ EWOULDBLOCK
          || errno == EWOULDBLOCK
#ಆದರೆ_ಅಂತ್ಯ
          )
        ಮುಂದುವರಿ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_ಕೊನೆಯ_Error = Get_ಕೊನೆಯ_Error();
      ಆದರೆ (Win_ಕೊನೆಯ_Error == ERROR_BROKEN_PIPE ||
          (Win_ಕೊನೆಯ_Error == ERROR_NO_ದತ್ತಾಂಶ && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#ಆದರೆ_ಅಂತ್ಯ
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_ಸಂಕೇತ());
      ನಿಲ್ಲಿಸು;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    ಗಾತ್ರ_2 -= ret;
  } ಆಗ (ಗಾತ್ರ_2 > 0);
}

ಶೂನ್ಯ raw_fd_ostream::close() {
  ಪ್ರತಿಜ್ಞಾ(ShouldClose);
  ShouldClose = ಸುಳ್ಳು;
  flush();
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  ಪ್ರತಿಜ್ಞಾ(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#ಇಲ್ಲದಿದ್ದರೆ
  pos = ::lseek(FD, off, SEEK_SET);
#ಆದರೆ_ಅಂತ್ಯ
  ಆದರೆ (pos == (uint64_t)-1)
    error_detected(errno_As_Error_ಸಂಕೇತ());
  ಹಿಂತಿರುಗಿ pos;
}

ಶೂನ್ಯ raw_fd_ostream::pwrite_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2,
                                 uint64_t ಸ್ಥಾನಾಂತರ) {
  uint64_t Pos = tell();
  seek(ಸ್ಥಾನಾಂತರ);
  ಲಿಖ(Ptr, ಗಾತ್ರ_2);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_ಬಫರ್_ಗಾತ್ರ() ಸ್ಥಿರಾಂಕ {
#ಆದರೆ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ಆದರೆ (IsWindowsConsole)
    ಹಿಂತಿರುಗಿ 0;
  ಹಿಂತಿರುಗಿ raw_ostream::preferred_ಬಫರ್_ಗಾತ್ರ();
#ಅಥವಾಆದರೆ defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ಹಿಂತಿರುಗಿ raw_ostream::preferred_ಬಫರ್_ಗಾತ್ರ();
#ಇಲ್ಲದಿದ್ದರೆ
  ಪ್ರತಿಜ್ಞಾ(FD >= 0 && "File not yet open!");
  ರಚನೆ stat statbuf;
  ಆದರೆ (fstat(FD, &statbuf) != 0)
    ಹಿಂತಿರುಗಿ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ಆದರೆ (S_ISCHR(statbuf.st_ಬಹುಲಕ) && is_displayed())
    ಹಿಂತಿರುಗಿ 0;
  // Return the preferred block size.
  ಹಿಂತಿರುಗಿ statbuf.st_blksize;
#ಆದರೆ_ಅಂತ್ಯ
}

ತಾರ್ಕಿಕ raw_fd_ostream::is_displayed() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ sys::Process::FileDescriptorIsDisplayed(FD);
}

ತಾರ್ಕಿಕ raw_fd_ostream::has_colors() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ಹಿಂತಿರುಗಿ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::ಬೀಗ_2() {
  std::error_ಸಂಕೇತ EC = sys::fs::ಬೀಗ_File(FD);
  ಆದರೆ (!EC)
    ಹಿಂತಿರುಗಿ sys::fs::FileLocker(FD);
  ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_ಬೀಗ_For(Duration ಸ್ಥಿರಾಂಕ& Timeout) {
  std::error_ಸಂಕೇತ EC = sys::fs::try_ಬೀಗ_File(FD, Timeout.getDuration());
  ಆದರೆ (!EC)
    ಹಿಂತಿರುಗಿ sys::fs::FileLocker(FD);
  ಹಿಂತಿರುಗಿ error_ಸಂಕೇತ_ಗಮ್ಯ_Error(EC);
}

ಶೂನ್ಯ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_ಸಂಕೇತ EC;

  // On z/OS we need to enable auto conversion
  ಸ್ಥಿರ std::error_ಸಂಕೇತ EC1 = enableAutoConversion(STDOUT_FILENO);
  ಪ್ರತಿಜ್ಞಾ(!EC1);
  (ಶೂನ್ಯ)EC1;

  ಸ್ಥಿರ raw_fd_ostream S("-", EC, sys::fs::ದ_None);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  ಹಿಂತಿರುಗಿ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ಸ್ಥಿರ std::error_ಸಂಕೇತ EC = enableAutoConversion(STDERR_FILENO);
  ಪ್ರತಿಜ್ಞಾ(!EC);
  (ಶೂನ್ಯ)EC;

  // Set standard error to be unbuffered.
  ಸ್ಥಿರ raw_fd_ostream S(STDERR_FILENO, ಸುಳ್ಳು, ನಿಜ);
  ಹಿಂತಿರುಗಿ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ಸ್ಥಿರ raw_null_ostream S;
  ಹಿಂತಿರುಗಿ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_ಹರಿವು::raw_fd_ಹರಿವು(StringRef Filename, std::error_ಸಂಕೇತ &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::ದ_None),
                     ನಿಜ, ಸುಳ್ಳು, OStream_ಬಗೆ::OK_FDStream) {
  ಆದರೆ (EC)
    ಹಿಂತಿರುಗಿ;

  ಆದರೆ (!isRegularFile())
    EC = std::make_error_ಸಂಕೇತ(std::errc::invalid_ವಾದ);
}

raw_fd_ಹರಿವು::raw_fd_ಹರಿವು(ಪೂರ್ಣಾಂಕ fd, ತಾರ್ಕಿಕ shouldClose)
    : raw_fd_ostream(fd, shouldClose, ಸುಳ್ಳು, OStream_ಬಗೆ::OK_FDStream) {}

ssize_t raw_fd_ಹರಿವು::ಪಠ(ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  ಪ್ರತಿಜ್ಞಾ(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::ಪಠ(get_fd(), (ಶೂನ್ಯ *)Ptr, ಗಾತ್ರ_2);
  ಆದರೆ (Ret >= 0)
    inc_pos(Ret);
  ಇಲ್ಲದಿದ್ದರೆ
    error_detected(errno_As_Error_ಸಂಕೇತ());
  ಹಿಂತಿರುಗಿ Ret;
}

ತಾರ್ಕಿಕ raw_fd_ಹರಿವು::classof(ಸ್ಥಿರಾಂಕ raw_ostream *OS) {
  ಹಿಂತಿರುಗಿ OS->get_ಬಗೆ_2() == OStream_ಬಗೆ::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

ಶೂನ್ಯ raw_string_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  OS.append(Ptr, ಗಾತ್ರ_2);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ OS.ಗಾತ್ರ_3(); }

ಶೂನ್ಯ raw_svector_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
  OS.append(Ptr, Ptr + ಗಾತ್ರ_2);
}

ಶೂನ್ಯ raw_svector_ostream::pwrite_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2,
                                      uint64_t ಸ್ಥಾನಾಂತರ) {
  ಸ್ಮರಣೆ_ನಕಲು(OS.ದತ್ತಾಂಶ() + ಸ್ಥಾನಾಂತರ, Ptr, ಗಾತ್ರ_2);
}

ತಾರ್ಕಿಕ raw_svector_ostream::classof(ಸ್ಥಿರಾಂಕ raw_ostream *OS) {
  ಹಿಂತಿರುಗಿ OS->get_ಬಗೆ_2() == OStream_ಬಗೆ::OK_SVec_ಹರಿವು;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ raw_null_ostream::write_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2) {
}

uint64_t raw_null_ostream::current_pos() ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ raw_null_ostream::pwrite_impl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Ptr, size_t ಗಾತ್ರ_2,
                                   uint64_t ಸ್ಥಾನಾಂತರ) {}

ಶೂನ್ಯ raw_pwrite_ಹರಿವು::anchor() {}

ಶೂನ್ಯ ಬಫರ್_ostream::anchor() {}

ಶೂನ್ಯ ಬಫರ್_unique_ostream::anchor() {}

ಸ್ಥಿರ ತಾರ್ಕಿಕ isNullDeviceName(StringRef OutputFileName) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  ಆದರೆ (OutputFileName.equals_insensitive("NUL"))
    ಹಿಂತಿರುಗಿ ನಿಜ;
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ OutputFileName == "/dev/null";
}

Error llvm::write_ಗಮ್ಯ_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ಆದರೆ (OutputFileName == "-")
    ಹಿಂತಿರುಗಿ Write(outs());

  ಆದರೆ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream ಹೊರಹೋಗು;
    ಹಿಂತಿರುಗಿ Write(ಹೊರಹೋಗು);
  }

  ಚಿಹ್ನೆರಹಿತ ಬಹುಲಕ = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::ತಾತ್ಕಾಲಿಕ_File> ತಾತ್ಕಾಲಿಕ =
      sys::fs::ತಾತ್ಕಾಲಿಕ_File::create(OutputFileName + ".temp-stream-%%%%%%", ಬಹುಲಕ);
  ಆದರೆ (!ತಾತ್ಕಾಲಿಕ)
    ಹಿಂತಿರುಗಿ createFileError(OutputFileName, ತಾತ್ಕಾಲಿಕ.takeError());

#ಆದರೆ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ಆದರೆ (ಸ್ವಯಂಚಾಲಿತ EC = llvm::copy_File_ಟ್ಯಾಗ್_ಗುಣಲಕ್ಷಣಗಳು(OutputFileName.str(), ತಾತ್ಕಾಲಿಕ->FD)) {
    ಆದರೆ (EC != std::errc::no_such_file_or_directory)
      ಹಿಂತಿರುಗಿ createFileError(OutputFileName, EC);
  }
#ಆದರೆ_ಅಂತ್ಯ

  raw_fd_ostream ಹೊರಹೋಗು(ತಾತ್ಕಾಲಿಕ->FD, ಸುಳ್ಳು);

  ಆದರೆ (Error E = Write(ಹೊರಹೋಗು)) {
    ಆದರೆ (Error DiscardError = ತಾತ್ಕಾಲಿಕ->discard())
      ಹಿಂತಿರುಗಿ joinErrors(std::move(E), std::move(DiscardError));
    ಹಿಂತಿರುಗಿ E;
  }
  ಹೊರಹೋಗು.flush();

  ಹಿಂತಿರುಗಿ ತಾತ್ಕಾಲಿಕ->keep(OutputFileName);
}
