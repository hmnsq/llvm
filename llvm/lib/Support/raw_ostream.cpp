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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AutoConvert.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Duration.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FormatVariadic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MathExtras.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/NativeFormatting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Process.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Program.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <algorithm>
#ਅਨ੍ਤਰ੍ਭਾਵ <cerrno>
#ਅਨ੍ਤਰ੍ਭਾਵ <cstdio>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>

// <fcntl.h> may provide O_BINARY.
# include <fcntl.h>

#ਜੇ defined(HAVE_UNISTD_H)
# include <unistd.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(__CYGWIN__)
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅੰਤ

#ਜੇ defined(_MSC_VER)
#ਅਨ੍ਤਰ੍ਭਾਵ <io.h>
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDIN_FILENO
# define STDIN_FILENO 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDOUT_FILENO
# define STDOUT_FILENO 1
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDERR_FILENO
# define STDERR_FILENO 2
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ConvertUTF.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

raw_ostream::~raw_ostream() {
  // raw_ostream's subclasses should take care to flush the buffer
  // in their destructors.
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ_Buf_Cur == ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ &&
         "raw_ostream destructor called with non-empty buffer!");

  ਜੇ (ਬਫਰ_ਬਹੁਲਕ == ਬਫਰ_ਕਿਸਮ::ਅੰਦਰੂਨੀ_ਬਫਰ)
    ਮਿਟਾਓ [] ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ;
}

size_t raw_ostream::preferred_ਬਫਰ_ਮਾਪ() ਅਚਲ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // On Windows BUFSIZ is only 512 which results in more calls to write. This
  // overhead can cause significant performance degradation. Therefore use a
  // better default.
  ਵਾਪਸ (16 * 1024);
#ਨਹੀਂਤਾਂ
  // BUFSIZ is intended to be a reasonable default.
  ਵਾਪਸ BUFSIZ;
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ raw_ostream::ਸੈੱਟ_Buffered() {
  // Ask the subclass to determine an appropriate buffer size.
  ਜੇ (size_t ਮਾਪ = preferred_ਬਫਰ_ਮਾਪ())
    ਸੈੱਟ_ਬਫਰ_ਮਾਪ(ਮਾਪ);
  ਨਹੀਂਤਾਂ
    // It may return 0, meaning this stream should be unbuffered.
    ਸੈੱਟ_Unbuffered();
}

ਖਾਲੀ raw_ostream::ਸੈੱਟ_ਬਫਰ_And_ਬਹੁਲਕ(ਅੱਖਰ *ਬਫਰ_ਸ਼ੁਰੂ, size_t ਮਾਪ,
                                   ਬਫਰ_ਕਿਸਮ ਬਹੁਲਕ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(((ਬਹੁਲਕ == ਬਫਰ_ਕਿਸਮ::Unbuffered && !ਬਫਰ_ਸ਼ੁਰੂ && ਮਾਪ == 0) ||
          (ਬਹੁਲਕ != ਬਫਰ_ਕਿਸਮ::Unbuffered && ਬਫਰ_ਸ਼ੁਰੂ && ਮਾਪ != 0)) &&
         "stream must be unbuffered or have at least one byte");
  // Make sure the current buffer is free of content (we can't flush here; the
  // child buffer management logic will be in write_impl).
  ਪ੍ਰਤਿਜ੍ਞਾ(Get_ਅੰਕ_ਬਾਈਟਾਂ_ਵਿੱਚ_ਬਫਰ() == 0 && "Current buffer is non-empty!");

  ਜੇ (ਬਫਰ_ਬਹੁਲਕ == ਬਫਰ_ਕਿਸਮ::ਅੰਦਰੂਨੀ_ਬਫਰ)
    ਮਿਟਾਓ [] ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ;
  ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ = ਬਫਰ_ਸ਼ੁਰੂ;
  ਬਾਹਰਲਾ_Buf_ਸਿਰਾ = ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ+ਮਾਪ;
  ਬਾਹਰਲਾ_Buf_Cur = ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ;
  ਬਫਰ_ਬਹੁਲਕ = ਬਹੁਲਕ;

  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ <= ਬਾਹਰਲਾ_Buf_ਸਿਰਾ && "Invalid size!");
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ N) {
  write_integer(*ਇਹ, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਲੰਮਾ N) {
  write_integer(*ਇਹ, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ N) {
  write_integer(*ਇਹ, static_cast<uint64_t>(N), 0, IntegerStyle::Integer);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਲੰਮਾ ਲੰਮਾ N) {
  write_integer(*ਇਹ, static_cast<int64_t>(N), 0, IntegerStyle::Integer);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::write_hex(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ N) {
  llvm::write_hex(*ਇਹ, N, HexPrintStyle::ਹੇਠਲਾ_2);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(Colors C) {
  ਜੇ (C == Colors::RESET)
    resetColor();
  ਨਹੀਂਤਾਂ
    changeColor(C);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::write_uuid(ਅਚਲ uuid_t UUID) {
  ਲਈ (ਪੂਰਨ Idx = 0; Idx < 16; ++Idx) {
    *ਇਹ << format("%02" PRIX32, UUID[Idx]);
    ਜੇ (Idx == 3 || Idx == 5 || Idx == 7 || Idx == 9)
      *ਇਹ << "-";
  }
  ਵਾਪਸ *ਇਹ;
}


raw_ostream &raw_ostream::write_escaped(StringRef Str,
                                        ਤਾਰਕਿਕ UseHexEscapes) {
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c : Str) {
    ਚੋਣ (c) {
    ਹਾਲਤ '\\':
      *ਇਹ << '\\' << '\\';
      ਰੋਕੋ;
    ਹਾਲਤ '\t':
      *ਇਹ << '\\' << 't';
      ਰੋਕੋ;
    ਹਾਲਤ '\n':
      *ਇਹ << '\\' << 'n';
      ਰੋਕੋ;
    ਹਾਲਤ '"':
      *ਇਹ << '\\' << '"';
      ਰੋਕੋ;
    ਮੂਲ:
      ਜੇ (isPrint(c)) {
        *ਇਹ << c;
        ਰੋਕੋ;
      }

      // Write out the escaped representation.
      ਜੇ (UseHexEscapes) {
        *ਇਹ << '\\' << 'x';
        *ਇਹ << hexdigit((c >> 4) & 0xF);
        *ਇਹ << hexdigit((c >> 0) & 0xF);
      } ਨਹੀਂਤਾਂ {
        // Always use a full 3-character octal escape.
        *ਇਹ << '\\';
        *ਇਹ << ਅੱਖਰ('0' + ((c >> 6) & 7));
        *ਇਹ << ਅੱਖਰ('0' + ((c >> 3) & 7));
        *ਇਹ << ਅੱਖਰ('0' + ((c >> 0) & 7));
      }
    }
  }

  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਅਚਲ ਖਾਲੀ *P) {
  llvm::write_hex(*ਇਹ, (uintptr_t)P, HexPrintStyle::ਅਗੇਤਰ_ਹੇਠਲਾ);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਦੁੱਗਣਾ N) {
  llvm::write_double(*ਇਹ, N, FloatStyle::Exponent);
  ਵਾਪਸ *ਇਹ;
}

ਖਾਲੀ raw_ostream::flush_nonempty() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਬਾਹਰਲਾ_Buf_Cur > ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ && "Invalid call to flush_nonempty.");
  size_t ਲੰਮਾਈ = ਬਾਹਰਲਾ_Buf_Cur - ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ;
  ਬਾਹਰਲਾ_Buf_Cur = ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ;
  write_impl(ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ, ਲੰਮਾਈ);
}

raw_ostream &raw_ostream::ਲਿਖ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ C) {
  // Group exceptional cases into a single branch.
  ਜੇ (LLVM_UNLIKELY(ਬਾਹਰਲਾ_Buf_Cur >= ਬਾਹਰਲਾ_Buf_ਸਿਰਾ)) {
    ਜੇ (LLVM_UNLIKELY(!ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ)) {
      ਜੇ (ਬਫਰ_ਬਹੁਲਕ == ਬਫਰ_ਕਿਸਮ::Unbuffered) {
        write_impl(reinterpret_cast<ਅੱਖਰ *>(&C), 1);
        ਵਾਪਸ *ਇਹ;
      }
      // Set up a buffer and start over.
      ਸੈੱਟ_Buffered();
      ਵਾਪਸ ਲਿਖ(C);
    }

    flush_nonempty();
  }

  *ਬਾਹਰਲਾ_Buf_Cur++ = C;
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਲਿਖ(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  // Group exceptional cases into a single branch.
  ਜੇ (LLVM_UNLIKELY(size_t(ਬਾਹਰਲਾ_Buf_ਸਿਰਾ - ਬਾਹਰਲਾ_Buf_Cur) < ਮਾਪ)) {
    ਜੇ (LLVM_UNLIKELY(!ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ)) {
      ਜੇ (ਬਫਰ_ਬਹੁਲਕ == ਬਫਰ_ਕਿਸਮ::Unbuffered) {
        write_impl(Ptr, ਮਾਪ);
        ਵਾਪਸ *ਇਹ;
      }
      // Set up a buffer and start over.
      ਸੈੱਟ_Buffered();
      ਵਾਪਸ ਲਿਖ(Ptr, ਮਾਪ);
    }

    size_t ਅੰਕ_ਬਾਈਟਾਂ = ਬਾਹਰਲਾ_Buf_ਸਿਰਾ - ਬਾਹਰਲਾ_Buf_Cur;

    // If the buffer is empty at this point we have a string that is larger
    // than the buffer. Directly write the chunk that is a multiple of the
    // preferred buffer size and put the remainder in the buffer.
    ਜੇ (LLVM_UNLIKELY(ਬਾਹਰਲਾ_Buf_Cur == ਬਾਹਰਲਾ_Buf_ਸ਼ੁਰੂ)) {
      ਪ੍ਰਤਿਜ੍ਞਾ(ਅੰਕ_ਬਾਈਟਾਂ != 0 && "undefined behavior");
      size_t ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write_2 = ਮਾਪ - (ਮਾਪ % ਅੰਕ_ਬਾਈਟਾਂ);
      write_impl(Ptr, ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write_2);
      size_t ਬਾਈਟਾਂ_ਬਚਿਆ_2 = ਮਾਪ - ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write_2;
      ਜੇ (ਬਾਈਟਾਂ_ਬਚਿਆ_2 > size_t(ਬਾਹਰਲਾ_Buf_ਸਿਰਾ - ਬਾਹਰਲਾ_Buf_Cur)) {
        // Too much left over to copy into our buffer.
        ਵਾਪਸ ਲਿਖ(Ptr + ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write_2, ਬਾਈਟਾਂ_ਬਚਿਆ_2);
      }
      copy_ਮੰਜ਼ਿਲ_ਬਫਰ(Ptr + ਬਾਈਟਾਂ_ਮੰਜ਼ਿਲ_Write_2, ਬਾਈਟਾਂ_ਬਚਿਆ_2);
      ਵਾਪਸ *ਇਹ;
    }

    // We don't have enough space in the buffer to fit the string in. Insert as
    // much as possible, flush and start over with the remainder.
    copy_ਮੰਜ਼ਿਲ_ਬਫਰ(Ptr, ਅੰਕ_ਬਾਈਟਾਂ);
    flush_nonempty();
    ਵਾਪਸ ਲਿਖ(Ptr + ਅੰਕ_ਬਾਈਟਾਂ, ਮਾਪ - ਅੰਕ_ਬਾਈਟਾਂ);
  }

  copy_ਮੰਜ਼ਿਲ_ਬਫਰ(Ptr, ਮਾਪ);

  ਵਾਪਸ *ਇਹ;
}

ਖਾਲੀ raw_ostream::copy_ਮੰਜ਼ਿਲ_ਬਫਰ(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(ਮਾਪ <= size_t(ਬਾਹਰਲਾ_Buf_ਸਿਰਾ - ਬਾਹਰਲਾ_Buf_Cur) && "Buffer overrun!");

  // Handle short strings specially, memcpy isn't very good at very short
  // strings.
  ਚੋਣ (ਮਾਪ) {
  ਹਾਲਤ 4: ਬਾਹਰਲਾ_Buf_Cur[3] = Ptr[3]; [[fallthrough]];
  ਹਾਲਤ 3: ਬਾਹਰਲਾ_Buf_Cur[2] = Ptr[2]; [[fallthrough]];
  ਹਾਲਤ 2: ਬਾਹਰਲਾ_Buf_Cur[1] = Ptr[1]; [[fallthrough]];
  ਹਾਲਤ 1: ਬਾਹਰਲਾ_Buf_Cur[0] = Ptr[0]; [[fallthrough]];
  ਹਾਲਤ 0: ਰੋਕੋ;
  ਮੂਲ:
    ਸਮ੍ਰਿਤੀ_ਨਕਲ(ਬਾਹਰਲਾ_Buf_Cur, Ptr, ਮਾਪ);
    ਰੋਕੋ;
  }

  ਬਾਹਰਲਾ_Buf_Cur += ਮਾਪ;
}

// Formatted output. Snprint returns the snprintf-style length the output
// needs, excluding the '\0'. A negative value is a genuine error, which a
// larger buffer can't fix, so give up and print nothing.
raw_ostream &
raw_ostream::ਸੰਚਾਲਕ<<(function_ref<ਪੂਰਨ(ਅੱਖਰ *, size_t)> Snprint) {
  // If we have more than a few bytes left in our output buffer, try
  // formatting directly onto its end.
  size_t ਮਾਪ = 128;
  size_t ਬਫਰ_ਬਾਈਟਾਂ_ਖੱਬਾ = ਬਾਹਰਲਾ_Buf_ਸਿਰਾ - ਬਾਹਰਲਾ_Buf_Cur;
  ਜੇ (ਬਫਰ_ਬਾਈਟਾਂ_ਖੱਬਾ > 3) {
    ਪੂਰਨ N = Snprint(ਬਾਹਰਲਾ_Buf_Cur, ਬਫਰ_ਬਾਈਟਾਂ_ਖੱਬਾ);
    ਜੇ (N < 0)
      ਵਾਪਸ *ਇਹ;
    // Common case is that we have plenty of space.
    ਜੇ (size_t(N) < ਬਫਰ_ਬਾਈਟਾਂ_ਖੱਬਾ) {
      ਬਾਹਰਲਾ_Buf_Cur += N;
      ਵਾਪਸ *ਇਹ;
    }
    // N excludes the '\0', so N + 1 bytes are exactly enough.
    ਮਾਪ = size_t(N) + 1;
  }

  // Otherwise format into a SmallVector resized to fit.
  Small_ਸਦਿਸ਼<ਅੱਖਰ, 128> V;
  ਲਈ (;;) {
    V.resize(ਮਾਪ);
    ਪੂਰਨ N = Snprint(V.ਅੰਕੜੇ(), ਮਾਪ);
    ਜੇ (N < 0)
      ਵਾਪਸ *ਇਹ;
    ਜੇ (size_t(N) < ਮਾਪ)
      ਵਾਪਸ ਲਿਖ(V.ਅੰਕੜੇ(), N);
    ਮਾਪ = size_t(N) + 1;
  }
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਅਚਲ formatv_object_ਅਧਾਰ &Obj) {
  Obj.format(*ਇਹ);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਅਚਲ FormattedString &FS) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੱਬਾ_Indent = 0;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਸੱਜਾ_Indent = 0;
  ਅਚਲ ssize_t Difference = FS.ਚੌੜਾਈ - FS.Str.ਮਾਪ_2();
  ਜੇ (Difference > 0) {
    ਚੋਣ (FS.Justify) {
    ਹਾਲਤ FormattedString::JustifyNone:
      ਰੋਕੋ;
    ਹਾਲਤ FormattedString::Justify_ਖੱਬਾ:
      ਸੱਜਾ_Indent = Difference;
      ਰੋਕੋ;
    ਹਾਲਤ FormattedString::Justify_ਸੱਜਾ:
      ਖੱਬਾ_Indent = Difference;
      ਰੋਕੋ;
    ਹਾਲਤ FormattedString::JustifyCenter:
      ਖੱਬਾ_Indent = Difference / 2;
      ਸੱਜਾ_Indent = Difference - ਖੱਬਾ_Indent;
      ਰੋਕੋ;
    }
  }
  indent(ਖੱਬਾ_Indent);
  (*ਇਹ) << FS.Str;
  indent(ਸੱਜਾ_Indent);
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਅਚਲ Formatted_ਸੰਖਿਆ &FN) {
  ਜੇ (FN.Hex) {
    HexPrintStyle Style;
    ਜੇ (FN.ਉੱਤਲਾ_2 && FN.Hex_ਅਗੇਤਰ)
      Style = HexPrintStyle::ਅਗੇਤਰ_ਉੱਤਲਾ;
    ਨਹੀਂਤਾਂ ਜੇ (FN.ਉੱਤਲਾ_2 && !FN.Hex_ਅਗੇਤਰ)
      Style = HexPrintStyle::ਉੱਤਲਾ_2;
    ਨਹੀਂਤਾਂ ਜੇ (!FN.ਉੱਤਲਾ_2 && FN.Hex_ਅਗੇਤਰ)
      Style = HexPrintStyle::ਅਗੇਤਰ_ਹੇਠਲਾ;
    ਨਹੀਂਤਾਂ
      Style = HexPrintStyle::ਹੇਠਲਾ_2;
    llvm::write_hex(*ਇਹ, FN.Hex_ਮੁੱਲ, Style, FN.ਚੌੜਾਈ);
  } ਨਹੀਂਤਾਂ {
    llvm::SmallString<16> ਬਫਰ;
    llvm::raw_svector_ostream ਧਾਰਾ(ਬਫਰ);
    llvm::write_integer(ਧਾਰਾ, FN.Dec_ਮੁੱਲ, 0, IntegerStyle::Integer);
    ਜੇ (ਬਫਰ.ਮਾਪ_2() < FN.ਚੌੜਾਈ)
      indent(FN.ਚੌੜਾਈ - ਬਫਰ.ਮਾਪ_2());
    (*ਇਹ) << ਬਫਰ;
  }
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਸੰਚਾਲਕ<<(ਅਚਲ Formatted_ਬਾਈਟਾਂ &FB) {
  ਜੇ (FB.ਬਾਈਟਾਂ_2.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ *ਇਹ;

  size_t ਸਤਰ_ਸੂਚਕਾਂਕ = 0;
  ਸਵੈਚਾਲਿਤ ਬਾਈਟਾਂ_2 = FB.ਬਾਈਟਾਂ_2;
  ਅਚਲ size_t ਮਾਪ = ਬਾਈਟਾਂ_2.ਮਾਪ_2();
  HexPrintStyle HPS = FB.ਉੱਤਲਾ_2 ? HexPrintStyle::ਉੱਤਲਾ_2 : HexPrintStyle::ਹੇਠਲਾ_2;
  uint64_t ਵਿੱਥ_ਚੌੜਾਈ = 0;
  ਜੇ (FB.ਪਹਿਲਾ_Byte_ਵਿੱਥ) {
    // Figure out how many nibbles are needed to print the largest offset
    // represented by this data set, so that we can align the offset field
    // to the right width.
    size_t ਸਤਰਾਂ = ਮਾਪ / FB.ਅੰਕ_ਪ੍ਰਤੀ_ਸਤਰ;
    uint64_t Max_ਵਿੱਥ = *FB.ਪਹਿਲਾ_Byte_ਵਿੱਥ + ਸਤਰਾਂ * FB.ਅੰਕ_ਪ੍ਰਤੀ_ਸਤਰ;
    ਬਿਨਾਨਿਸ਼ਾਨੀ Power = 0;
    ਜੇ (Max_ਵਿੱਥ > 0)
      Power = llvm::Log2_64_Ceil(Max_ਵਿੱਥ);
    ਵਿੱਥ_ਚੌੜਾਈ = std::max<uint64_t>(4, llvm::align_ਮੰਜ਼ਿਲ(Power, 4) / 4);
  }

  // The width of a block of data including all spaces for group separators.
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Byte_Groups =
      align_ਮੰਜ਼ਿਲ(FB.ਅੰਕ_ਪ੍ਰਤੀ_ਸਤਰ, FB.Byte_ਸਮੂਹ_ਮਾਪ) / FB.Byte_ਸਮੂਹ_ਮਾਪ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਖੰਡ_Char_ਚੌੜਾਈ = FB.ਅੰਕ_ਪ੍ਰਤੀ_ਸਤਰ * 2 + ਅੰਕ_Byte_Groups - 1;

  ਜਦੋਂ (!ਬਾਈਟਾਂ_2.ਖਾਲੀ_ਥਾਵਾਂ()) {
    indent(FB.IndentLevel);

    ਜੇ (FB.ਪਹਿਲਾ_Byte_ਵਿੱਥ) {
      uint64_t ਵਿੱਥ = *FB.ਪਹਿਲਾ_Byte_ਵਿੱਥ;
      llvm::write_hex(*ਇਹ, ਵਿੱਥ + ਸਤਰ_ਸੂਚਕਾਂਕ, HPS, ਵਿੱਥ_ਚੌੜਾਈ);
      *ਇਹ << ": ";
    }

    ਸਵੈਚਾਲਿਤ ਸਤਰ = ਬਾਈਟਾਂ_2.take_front(FB.ਅੰਕ_ਪ੍ਰਤੀ_ਸਤਰ);

    size_t CharsPrinted = 0;
    // Print the hex bytes for this line in groups
    ਲਈ (size_t I = 0; I < ਸਤਰ.ਮਾਪ_2(); ++I, CharsPrinted += 2) {
      ਜੇ (I && (I % FB.Byte_ਸਮੂਹ_ਮਾਪ) == 0) {
        ++CharsPrinted;
        *ਇਹ << " ";
      }
      llvm::write_hex(*ਇਹ, ਸਤਰ[I], HPS, 2);
    }

    ਜੇ (FB.ASCII) {
      // Print any spaces needed for any bytes that we didn't print on this
      // line so that the ASCII bytes are correctly aligned.
      ਪ੍ਰਤਿਜ੍ਞਾ(ਖੰਡ_Char_ਚੌੜਾਈ >= CharsPrinted);
      indent(ਖੰਡ_Char_ਚੌੜਾਈ - CharsPrinted + 2);
      *ਇਹ << "|";

      // Print the ASCII char values for each byte on this line
      ਲਈ (uint8_t Byte : ਸਤਰ) {
        ਜੇ (isPrint(Byte))
          *ਇਹ << static_cast<ਅੱਖਰ>(Byte);
        ਨਹੀਂਤਾਂ
          *ਇਹ << '.';
      }
      *ਇਹ << '|';
    }

    ਬਾਈਟਾਂ_2 = ਬਾਈਟਾਂ_2.drop_front(ਸਤਰ.ਮਾਪ_2());
    ਸਤਰ_ਸੂਚਕਾਂਕ += ਸਤਰ.ਮਾਪ_2();
    ਜੇ (ਸਤਰ_ਸੂਚਕਾਂਕ < ਮਾਪ)
      *ਇਹ << '\n';
  }
  ਵਾਪਸ *ਇਹ;
}

ਸਾਂਚਾ <ਅੱਖਰ C>
ਸਥਿਰ raw_ostream &write_padding(raw_ostream &OS, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Chars) {
  ਸਥਿਰ ਅਚਲ ਅੱਖਰ Chars[] = {C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C,
                               C, C, C, C, C, C, C, C, C, C, C, C, C, C, C, C};

  // Usually the indentation is small, handle it with a fastpath.
  ਜੇ (ਅੰਕ_Chars < std::ਮਾਪ_2(Chars))
    ਵਾਪਸ OS.ਲਿਖ(Chars, ਅੰਕ_Chars);

  ਜਦੋਂ (ਅੰਕ_Chars) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਮੰਜ਼ਿਲ_Write = std::min(ਅੰਕ_Chars, (ਬਿਨਾਨਿਸ਼ਾਨੀ)std::ਮਾਪ_2(Chars) - 1);
    OS.ਲਿਖ(Chars, ਅੰਕ_ਮੰਜ਼ਿਲ_Write);
    ਅੰਕ_Chars -= ਅੰਕ_ਮੰਜ਼ਿਲ_Write;
  }
  ਵਾਪਸ OS;
}

/// indent - Insert 'NumSpaces' spaces.
raw_ostream &raw_ostream::indent(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Spaces) {
  ਵਾਪਸ write_padding<' '>(*ਇਹ, ਅੰਕ_Spaces);
}

/// write_zeros - Insert 'NumZeros' nulls.
raw_ostream &raw_ostream::write_zeros(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_Zeros) {
  ਵਾਪਸ write_padding<'\0'>(*ਇਹ, ਅੰਕ_Zeros);
}

ਤਾਰਕਿਕ raw_ostream::prepare_colors() {
  // Colors were explicitly disabled.
  ਜੇ (!ColorEnabled)
    ਵਾਪਸ ਝੂਠ;

  // Colors require changing the terminal but this stream is not going to a
  // terminal.
  ਜੇ (sys::Process::ColorNeedsFlush() && !is_displayed())
    ਵਾਪਸ ਝੂਠ;

  ਜੇ (sys::Process::ColorNeedsFlush())
    flush();

  ਵਾਪਸ ਸੱਚ;
}

raw_ostream &raw_ostream::changeColor(ਗਿਣਤੀ Colors colors, ਤਾਰਕਿਕ bold, ਤਾਰਕਿਕ bg) {
  ਜੇ (!prepare_colors())
    ਵਾਪਸ *ਇਹ;

  ਅਚਲ ਅੱਖਰ *colorcode =
      (colors == SAVEDCOLOR)
          ? sys::Process::OutputBold(bg)
          : sys::Process::OutputColor(static_cast<ਅੱਖਰ>(colors), bold, bg);
  ਜੇ (colorcode)
    ਲਿਖ(colorcode, ਲੜੀ_ਲੰਬਾਈ(colorcode));
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::resetColor() {
  ਜੇ (!prepare_colors())
    ਵਾਪਸ *ਇਹ;

  ਜੇ (ਅਚਲ ਅੱਖਰ *colorcode = sys::Process::ResetColor())
    ਲਿਖ(colorcode, ਲੜੀ_ਲੰਬਾਈ(colorcode));
  ਵਾਪਸ *ਇਹ;
}

raw_ostream &raw_ostream::ਉਲਟ_Color() {
  ਜੇ (!prepare_colors())
    ਵਾਪਸ *ਇਹ;

  ਜੇ (ਅਚਲ ਅੱਖਰ *colorcode = sys::Process::Output_ਉਲਟ())
    ਲਿਖ(colorcode, ਲੜੀ_ਲੰਬਾਈ(colorcode));
  ਵਾਪਸ *ਇਹ;
}

ਖਾਲੀ raw_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  raw_fd_ostream
//===----------------------------------------------------------------------===//

ਸਥਿਰ ਪੂਰਨ getFD(StringRef Filename, std::error_ਕੋਡ &EC,
                 sys::fs::CreationDisposition Disp, sys::fs::FileAccess Access,
                 sys::fs::OpenFlags Flags) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਪ੍ਰਤਿਜ੍ਞਾ((Access & sys::fs::FA_Write) &&
         "Cannot make a raw_ostream from a read-only descriptor!");

  // Handle "-" as stdout. Note that when we do this, we consider ourself
  // the owner of stdout and may set the "binary" flag globally based on Flags.
  ਜੇ (Filename == "-") {
    EC = std::error_ਕੋਡ();
    // Change stdout's text/binary mode based on the Flags.
    sys::Change_Stdout_ਬਹੁਲਕ(Flags);
    ਵਾਪਸ STDOUT_FILENO;
  }

  ਪੂਰਨ FD;
  ਜੇ (Access & sys::fs::FA_Read)
    EC = sys::fs::openFileForReadWrite(Filename, FD, Disp, Flags);
  ਨਹੀਂਤਾਂ
    EC = sys::fs::openFileForWrite(Filename, FD, Disp, Flags);
  ਜੇ (EC)
    ਵਾਪਸ -1;

  ਵਾਪਸ FD;
}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ਕੋਡ &EC)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     sys::fs::ਦਾ_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ਕੋਡ &EC,
                               sys::fs::CreationDisposition Disp)
    : raw_fd_ostream(Filename, EC, Disp, sys::fs::FA_Write, sys::fs::ਦਾ_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ਕੋਡ &EC,
                               sys::fs::FileAccess Access)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, Access,
                     sys::fs::ਦਾ_None) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ਕੋਡ &EC,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(Filename, EC, sys::fs::CD_CreateAlways, sys::fs::FA_Write,
                     Flags) {}

raw_fd_ostream::raw_fd_ostream(StringRef Filename, std::error_ਕੋਡ &EC,
                               sys::fs::CreationDisposition Disp,
                               sys::fs::FileAccess Access,
                               sys::fs::OpenFlags Flags)
    : raw_fd_ostream(getFD(Filename, EC, Disp, Access, Flags), ਸੱਚ) {}

/// FD is the file descriptor that this writes to.  If ShouldClose is true, this
/// closes the file when the stream is destroyed.
raw_fd_ostream::raw_fd_ostream(ਪੂਰਨ fd, ਤਾਰਕਿਕ shouldClose, ਤਾਰਕਿਕ unbuffered,
                               OStream_ਕਿਸਮ K)
    : raw_pwrite_ਧਾਰਾ(unbuffered, K), FD(fd), ShouldClose(shouldClose) {
  // FIXME(sandboxing): Remove this by adopting `llvm::vfs::OutputBackend`.
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  ਜੇ (FD < 0 ) {
    ShouldClose = ਝੂਠ;
    ਵਾਪਸ;
  }

  enable_colors(ਸੱਚ);

  // Do not attempt to close stdout or stderr. We used to try to maintain the
  // property that tools that support writing file to stdout should not also
  // write informational output to stdout, but in practice we were never able to
  // maintain this invariant. Many features have been added to LLVM and clang
  // (-fdump-record-layouts, optimization remarks, etc) that print to stdout, so
  // users must simply be aware that mixed output and remarks is a possibility.
  ਜੇ (FD <= STDERR_FILENO)
    ShouldClose = ਝੂਠ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // Check if this is a console device. This is not equivalent to isatty.
  IsWindowsConsole =
      ::GetFileType((HANDLE)::_get_osfhandle(fd)) == FILE_TYPE_CHAR;
#ਜੇ_ਅੰਤ

  // Get the starting position.
  off_t loc = ::lseek(FD, 0, SEEK_CUR);
  sys::fs::file_ਸਥਿਤੀ ਸਥਿਤੀ;
  std::error_ਕੋਡ EC = ਸਥਿਤੀ_2(FD, ਸਥਿਤੀ);
  IsRegularFile = ਸਥਿਤੀ.type() == sys::fs::file_type::regular_file;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  // MSVCRT's _lseek(SEEK_CUR) doesn't return -1 for pipes.
  SupportsSeeking = !EC && IsRegularFile;
#ਨਹੀਂਤਾਂ
  SupportsSeeking = !EC && loc != (off_t)-1;
#ਜੇ_ਅੰਤ
  ਜੇ (!SupportsSeeking)
    pos = 0;
  ਨਹੀਂਤਾਂ
    pos = static_cast<uint64_t>(loc);
}

raw_fd_ostream::~raw_fd_ostream() {
  ਜੇ (FD >= 0) {
    flush();
    ਜੇ (ShouldClose) {
      ਜੇ (ਸਵੈਚਾਲਿਤ EC = sys::Process::SafelyCloseFileDescriptor(FD))
        error_detected(EC);
    }
  }

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MINGW32__
  // On mingw, global dtors should not call exit().
  // report_fatal_error() invokes exit(). We know report_fatal_error()
  // might not write messages to stderr when any errors were detected
  // on FD == 2.
  ਜੇ (FD == 2) ਵਾਪਸ;
#ਜੇ_ਅੰਤ

  // If there are any pending errors, report them now. Clients wishing
  // to avoid report_fatal_error calls should check for errors with
  // has_error() and clear the error flag with clear_error() before
  // destructing raw_ostream objects which may have errors.
  ਜੇ (has_error())
    reportFatalUsageError(Twine("IO failure on output stream: ") +
                          error().ਸੁਨੇਹਾ_2());
}

#ਜੇ defined(_WIN32)
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
ਸਥਿਰ ਤਾਰਕਿਕ write_console_impl(ਪੂਰਨ FD, StringRef ਅੰਕੜੇ_2) {
  Small_ਸਦਿਸ਼<wchar_t, 256> Wide_ਪਾਠ;

  // Fall back to ::write if it wasn't valid UTF-8.
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = sys::windows::UTF8_ਮੰਜ਼ਿਲ_UTF16(ਅੰਕੜੇ_2, Wide_ਪਾਠ))
    ਵਾਪਸ ਝੂਠ;

  // On Windows 7 and earlier, WriteConsoleW has a low maximum amount of data
  // that can be written to the console at a time.
  size_t Max_Write_ਮਾਪ = Wide_ਪਾਠ.ਮਾਪ_2();
  ਜੇ (!RunningWindows8OrGreater())
    Max_Write_ਮਾਪ = 32767;

  size_t WCharsWritten = 0;
  ਕਰੋ {
    size_t WChars_ਮੰਜ਼ਿਲ_Write =
        std::min(Max_Write_ਮਾਪ, Wide_ਪਾਠ.ਮਾਪ_2() - WCharsWritten);
    DWORD ActuallyWritten;
    ਤਾਰਕਿਕ Success =
        ::WriteConsoleW((HANDLE)::_get_osfhandle(FD), &Wide_ਪਾਠ[WCharsWritten],
                        WChars_ਮੰਜ਼ਿਲ_Write, &ActuallyWritten,
                        /*Reserved=*/ਖਾਲੀ_ਸੂਚਕ);

    // The most likely reason for WriteConsoleW to fail is that FD no longer
    // points to a console. Fall back to ::write. If this isn't the first loop
    // iteration, something is truly wrong.
    ਜੇ (!Success)
      ਵਾਪਸ ਝੂਠ;

    WCharsWritten += ActuallyWritten;
  } ਜਦੋਂ (WCharsWritten != Wide_ਪਾਠ.ਮਾਪ_2());
  ਵਾਪਸ ਸੱਚ;
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ raw_fd_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  ਜੇ (Tied_ਧਾਰਾ)
    Tied_ਧਾਰਾ->flush();

  ਪ੍ਰਤਿਜ੍ਞਾ(FD >= 0 && "File already closed.");
  pos += ਮਾਪ;

#ਜੇ defined(_WIN32)
  // If this is a Windows console device, try re-encoding from UTF-8 to UTF-16
  // and using WriteConsoleW. If that fails, fall back to plain write().
  ਜੇ (IsWindowsConsole)
    ਜੇ (write_console_impl(FD, StringRef(Ptr, ਮਾਪ)))
      ਵਾਪਸ;
#ਜੇ_ਅੰਤ

  // The maximum write size is limited to INT32_MAX. A write
  // greater than SSIZE_MAX is implementation-defined in POSIX,
  // and Windows _write requires 32 bit input.
  size_t Max_Write_ਮਾਪ = INT32_MAX;

#ਜੇ defined(__linux__)
  // It is observed that Linux returns EINVAL for a very large write (>2G).
  // Make it a reasonably small value.
  Max_Write_ਮਾਪ = 1024 * 1024 * 1024;
#ਜੇ_ਅੰਤ

  ਕਰੋ {
    size_t Chunk_ਮਾਪ = std::min(ਮਾਪ, Max_Write_ਮਾਪ);
    ssize_t ret = ::ਲਿਖ(FD, Ptr, Chunk_ਮਾਪ);

    ਜੇ (ret < 0) {
      // If it's a recoverable error, swallow it and retry the write.
      //
      // Ideally we wouldn't ever see EAGAIN or EWOULDBLOCK here, since
      // raw_ostream isn't designed to do non-blocking I/O. However, some
      // programs, such as old versions of bjam, have mistakenly used
      // O_NONBLOCK. For compatibility, emulate blocking semantics by
      // spinning until the write succeeds. If you don't want spinning,
      // don't use O_NONBLOCK file descriptors with raw_ostream.
      ਜੇ (errno == EINTR || errno == EAGAIN
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ EWOULDBLOCK
          || errno == EWOULDBLOCK
#ਜੇ_ਅੰਤ
          )
        ਜਾਰੀ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
      // Windows equivalents of SIGPIPE/EPIPE.
      DWORD Win_ਆਖਰੀ_Error = Get_ਆਖਰੀ_Error();
      ਜੇ (Win_ਆਖਰੀ_Error == ERROR_BROKEN_PIPE ||
          (Win_ਆਖਰੀ_Error == ERROR_NO_ਅੰਕੜੇ && errno == EINVAL)) {
        llvm::sys::CallOneShotPipeSignalHandler();
        errno = EPIPE;
      }
#ਜੇ_ਅੰਤ
      // Otherwise it's a non-recoverable error. Note it and quit.
      error_detected(errno_As_Error_ਕੋਡ());
      ਰੋਕੋ;
    }

    // The write may have written some or all of the data. Update the
    // size and buffer pointer to reflect the remainder that needs
    // to be written. If there are no bytes left, we're done.
    Ptr += ret;
    ਮਾਪ -= ret;
  } ਜਦੋਂ (ਮਾਪ > 0);
}

ਖਾਲੀ raw_fd_ostream::close() {
  ਪ੍ਰਤਿਜ੍ਞਾ(ShouldClose);
  ShouldClose = ਝੂਠ;
  flush();
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = sys::Process::SafelyCloseFileDescriptor(FD))
    error_detected(EC);
  FD = -1;
}

uint64_t raw_fd_ostream::seek(uint64_t off) {
  ਪ੍ਰਤਿਜ੍ਞਾ(SupportsSeeking && "Stream does not support seeking!");
  flush();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  pos = ::_lseeki64(FD, off, SEEK_SET);
#ਨਹੀਂਤਾਂ
  pos = ::lseek(FD, off, SEEK_SET);
#ਜੇ_ਅੰਤ
  ਜੇ (pos == (uint64_t)-1)
    error_detected(errno_As_Error_ਕੋਡ());
  ਵਾਪਸ pos;
}

ਖਾਲੀ raw_fd_ostream::pwrite_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ,
                                 uint64_t ਵਿੱਥ) {
  uint64_t Pos = tell();
  seek(ਵਿੱਥ);
  ਲਿਖ(Ptr, ਮਾਪ);
  seek(Pos);
}

size_t raw_fd_ostream::preferred_ਬਫਰ_ਮਾਪ() ਅਚਲ {
#ਜੇ defined(_WIN32)
  // Disable buffering for console devices. Console output is re-encoded from
  // UTF-8 to UTF-16 on Windows, and buffering it would require us to split the
  // buffer on a valid UTF-8 codepoint boundary. Terminal buffering is disabled
  // below on most other OSs, so do the same thing on Windows and avoid that
  // complexity.
  ਜੇ (IsWindowsConsole)
    ਵਾਪਸ 0;
  ਵਾਪਸ raw_ostream::preferred_ਬਫਰ_ਮਾਪ();
#ਜਾਂਜੇ defined(__MVS__)
  // The buffer size on z/OS is defined with macro BUFSIZ, which can be
  // retrieved by invoking function raw_ostream::preferred_buffer_size().
  ਵਾਪਸ raw_ostream::preferred_ਬਫਰ_ਮਾਪ();
#ਨਹੀਂਤਾਂ
  ਪ੍ਰਤਿਜ੍ਞਾ(FD >= 0 && "File not yet open!");
  ਬਣਤਰ stat statbuf;
  ਜੇ (fstat(FD, &statbuf) != 0)
    ਵਾਪਸ 0;

  // If this is a terminal, don't use buffering. Line buffering
  // would be a more traditional thing to do, but it's not worth
  // the complexity.
  ਜੇ (S_ISCHR(statbuf.st_ਬਹੁਲਕ) && is_displayed())
    ਵਾਪਸ 0;
  // Return the preferred block size.
  ਵਾਪਸ statbuf.st_blksize;
#ਜੇ_ਅੰਤ
}

ਤਾਰਕਿਕ raw_fd_ostream::is_displayed() ਅਚਲ {
  ਵਾਪਸ sys::Process::FileDescriptorIsDisplayed(FD);
}

ਤਾਰਕਿਕ raw_fd_ostream::has_colors() ਅਚਲ {
  ਜੇ (!HasColors)
    HasColors = sys::Process::FileDescriptorHasColors(FD);
  ਵਾਪਸ *HasColors;
}

Expected<sys::fs::FileLocker> raw_fd_ostream::ਤਾਲਾ() {
  std::error_ਕੋਡ EC = sys::fs::ਤਾਲਾ_File(FD);
  ਜੇ (!EC)
    ਵਾਪਸ sys::fs::FileLocker(FD);
  ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
}

Expected<sys::fs::FileLocker>
raw_fd_ostream::try_ਤਾਲਾ_For(Duration ਅਚਲ& Timeout) {
  std::error_ਕੋਡ EC = sys::fs::try_ਤਾਲਾ_File(FD, Timeout.getDuration());
  ਜੇ (!EC)
    ਵਾਪਸ sys::fs::FileLocker(FD);
  ਵਾਪਸ error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(EC);
}

ਖਾਲੀ raw_fd_ostream::anchor() {}

//===----------------------------------------------------------------------===//
//  outs(), errs(), nulls()
//===----------------------------------------------------------------------===//

raw_fd_ostream &llvm::outs() {
  // Set buffer settings to model stdout behavior.
  std::error_ਕੋਡ EC;

  // On z/OS we need to enable auto conversion
  ਸਥਿਰ std::error_ਕੋਡ EC1 = enableAutoConversion(STDOUT_FILENO);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC1);
  (ਖਾਲੀ)EC1;

  ਸਥਿਰ raw_fd_ostream S("-", EC, sys::fs::ਦਾ_None);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC);
  ਵਾਪਸ S;
}

raw_fd_ostream &llvm::errs() {
  // On z/OS we need to enable auto conversion
  ਸਥਿਰ std::error_ਕੋਡ EC = enableAutoConversion(STDERR_FILENO);
  ਪ੍ਰਤਿਜ੍ਞਾ(!EC);
  (ਖਾਲੀ)EC;

  // Set standard error to be unbuffered.
  ਸਥਿਰ raw_fd_ostream S(STDERR_FILENO, ਝੂਠ, ਸੱਚ);
  ਵਾਪਸ S;
}

/// nulls() - This returns a reference to a raw_ostream which discards output.
raw_ostream &llvm::nulls() {
  ਸਥਿਰ raw_null_ostream S;
  ਵਾਪਸ S;
}

//===----------------------------------------------------------------------===//
// File Streams
//===----------------------------------------------------------------------===//

raw_fd_ਧਾਰਾ::raw_fd_ਧਾਰਾ(StringRef Filename, std::error_ਕੋਡ &EC)
    : raw_fd_ostream(getFD(Filename, EC, sys::fs::CD_CreateAlways,
                           sys::fs::FA_Write | sys::fs::FA_Read,
                           sys::fs::ਦਾ_None),
                     ਸੱਚ, ਝੂਠ, OStream_ਕਿਸਮ::OK_FDStream) {
  ਜੇ (EC)
    ਵਾਪਸ;

  ਜੇ (!isRegularFile())
    EC = std::make_error_ਕੋਡ(std::errc::invalid_ਦਲੀਲ);
}

raw_fd_ਧਾਰਾ::raw_fd_ਧਾਰਾ(ਪੂਰਨ fd, ਤਾਰਕਿਕ shouldClose)
    : raw_fd_ostream(fd, shouldClose, ਝੂਠ, OStream_ਕਿਸਮ::OK_FDStream) {}

ssize_t raw_fd_ਧਾਰਾ::ਪਠ(ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(get_fd() >= 0 && "File already closed.");
  ssize_t Ret = ::ਪਠ(get_fd(), (ਖਾਲੀ *)Ptr, ਮਾਪ);
  ਜੇ (Ret >= 0)
    inc_pos(Ret);
  ਨਹੀਂਤਾਂ
    error_detected(errno_As_Error_ਕੋਡ());
  ਵਾਪਸ Ret;
}

ਤਾਰਕਿਕ raw_fd_ਧਾਰਾ::classof(ਅਚਲ raw_ostream *OS) {
  ਵਾਪਸ OS->get_ਕਿਸਮ_2() == OStream_ਕਿਸਮ::OK_FDStream;
}

//===----------------------------------------------------------------------===//
//  raw_string_ostream
//===----------------------------------------------------------------------===//

ਖਾਲੀ raw_string_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  OS.append(Ptr, ਮਾਪ);
}

//===----------------------------------------------------------------------===//
//  raw_svector_ostream
//===----------------------------------------------------------------------===//

uint64_t raw_svector_ostream::current_pos() ਅਚਲ { ਵਾਪਸ OS.ਮਾਪ_2(); }

ਖਾਲੀ raw_svector_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
  OS.append(Ptr, Ptr + ਮਾਪ);
}

ਖਾਲੀ raw_svector_ostream::pwrite_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ,
                                      uint64_t ਵਿੱਥ) {
  ਸਮ੍ਰਿਤੀ_ਨਕਲ(OS.ਅੰਕੜੇ() + ਵਿੱਥ, Ptr, ਮਾਪ);
}

ਤਾਰਕਿਕ raw_svector_ostream::classof(ਅਚਲ raw_ostream *OS) {
  ਵਾਪਸ OS->get_ਕਿਸਮ_2() == OStream_ਕਿਸਮ::OK_SVec_ਧਾਰਾ;
}

//===----------------------------------------------------------------------===//
//  raw_null_ostream
//===----------------------------------------------------------------------===//

raw_null_ostream::~raw_null_ostream() {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  // ~raw_ostream asserts that the buffer is empty. This isn't necessary
  // with raw_null_ostream, but it's better to have raw_null_ostream follow
  // the rules than to change the rules just for raw_null_ostream.
  flush();
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ raw_null_ostream::write_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ) {
}

uint64_t raw_null_ostream::current_pos() ਅਚਲ {
  ਵਾਪਸ 0;
}

ਖਾਲੀ raw_null_ostream::pwrite_impl(ਅਚਲ ਅੱਖਰ *Ptr, size_t ਮਾਪ,
                                   uint64_t ਵਿੱਥ) {}

ਖਾਲੀ raw_pwrite_ਧਾਰਾ::anchor() {}

ਖਾਲੀ ਬਫਰ_ostream::anchor() {}

ਖਾਲੀ ਬਫਰ_unique_ostream::anchor() {}

ਸਥਿਰ ਤਾਰਕਿਕ isNullDeviceName(StringRef OutputFileName) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  ਜੇ (OutputFileName.equals_insensitive("NUL"))
    ਵਾਪਸ ਸੱਚ;
#ਜੇ_ਅੰਤ
  ਵਾਪਸ OutputFileName == "/dev/null";
}

Error llvm::write_ਮੰਜ਼ਿਲ_Output(StringRef OutputFileName,
                          std::function<Error(raw_ostream &)> Write) {
  ਜੇ (OutputFileName == "-")
    ਵਾਪਸ Write(outs());

  ਜੇ (isNullDeviceName(OutputFileName)) {
    raw_null_ostream ਬਾਹਰਲਾ;
    ਵਾਪਸ Write(ਬਾਹਰਲਾ);
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਬਹੁਲਕ = sys::fs::all_read | sys::fs::all_write;
  Expected<sys::fs::ਆਰਜ਼ੀ_File> ਆਰਜ਼ੀ =
      sys::fs::ਆਰਜ਼ੀ_File::create(OutputFileName + ".temp-stream-%%%%%%", ਬਹੁਲਕ);
  ਜੇ (!ਆਰਜ਼ੀ)
    ਵਾਪਸ createFileError(OutputFileName, ਆਰਜ਼ੀ.takeError());

#ਜੇ defined(__MVS__)
  // The file tag needs to be set before any output can be written to the file.
  // Do this before creating the raw_fd_ostream to ensure the correct
  // sequence.
  ਜੇ (ਸਵੈਚਾਲਿਤ EC = llvm::copy_File_ਟੈਗ_ਗੁਣ(OutputFileName.str(), ਆਰਜ਼ੀ->FD)) {
    ਜੇ (EC != std::errc::no_such_file_or_directory)
      ਵਾਪਸ createFileError(OutputFileName, EC);
  }
#ਜੇ_ਅੰਤ

  raw_fd_ostream ਬਾਹਰਲਾ(ਆਰਜ਼ੀ->FD, ਝੂਠ);

  ਜੇ (Error E = Write(ਬਾਹਰਲਾ)) {
    ਜੇ (Error DiscardError = ਆਰਜ਼ੀ->discard())
      ਵਾਪਸ joinErrors(std::move(E), std::move(DiscardError));
    ਵਾਪਸ E;
  }
  ਬਾਹਰਲਾ.flush();

  ਵਾਪਸ ਆਰਜ਼ੀ->keep(OutputFileName);
}
