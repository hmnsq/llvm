//===--- Compression.cpp - Compression implementation ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file implements compression functions.
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compression.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ScopeExit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallVector.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Compiler.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <limits>
#ਅਨ੍ਤਰ੍ਭਾਵ <optional>
#ਜੇ LLVM_ENABLE_ZLIB
#ਅਨ੍ਤਰ੍ਭਾਵ <zlib.h>
#ਜੇ_ਅੰਤ
#ਜੇ LLVM_ENABLE_ZSTD
#ਅਨ੍ਤਰ੍ਭਾਵ <zstd.h>
#ਜੇ_ਅੰਤ
#ਜੇ LLVM_ENABLE_LZMA
#ਅਨ੍ਤਰ੍ਭਾਵ <lzma.h>
#ਜੇ_ਅੰਤ

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ਸਥਿਰ ਤਾਰਕਿਕ is_Zlib_ਸਿਰਲੇਖ(ArrayRef<uint8_t> ਨਿਵੇਸ਼) {
  ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() < 2)
    ਵਾਪਸ ਝੂਠ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ CMF = ਨਿਵੇਸ਼[0];
  ਬਿਨਾਨਿਸ਼ਾਨੀ FLG = ਨਿਵੇਸ਼[1];
  ਜੇ ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ਵਾਪਸ ਝੂਠ;
  ਵਾਪਸ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ਸਥਿਰ ਤਾਰਕਿਕ isZstdMagic(ArrayRef<uint8_t> ਨਿਵੇਸ਼) {
  ਸਥਿਰ constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ਵਾਪਸ ਨਿਵੇਸ਼.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ਸਥਿਰ std::optional<Format> identifyFormat(ArrayRef<uint8_t> ਨਿਵੇਸ਼) {
  ਜੇ (isZstdMagic(ਨਿਵੇਸ਼))
    ਵਾਪਸ Format::Zstd;
  ਜੇ (is_Zlib_ਸਿਰਲੇਖ(ਨਿਵੇਸ਼))
    ਵਾਪਸ Format::Zlib;
  ਵਾਪਸ std::nullopt;
}

ਅਚਲ ਅੱਖਰ *compression::getReasonIfUnsupported(compression::Format F) {
  ਚੋਣ (F) {
  ਹਾਲਤ compression::Format::Zlib:
    ਜੇ (zlib::is_ਉਪਲਬਧ())
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    ਵਾਪਸ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  ਹਾਲਤ compression::Format::Zstd:
    ਜੇ (zstd::is_ਉਪਲਬਧ())
      ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
    ਵਾਪਸ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ਅਚਲ ਅੱਖਰ *compression::getReasonIfUnsupported(ArrayRef<uint8_t> ਨਿਵੇਸ਼) {
  ਜੇ (std::optional<Format> F = identifyFormat(ਨਿਵੇਸ਼))
    ਵਾਪਸ getReasonIfUnsupported(*F);
  ਵਾਪਸ "unknown compression format";
}

ਖਾਲੀ compression::compress(Params P, ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                           Small_ਸਦਿਸ਼_Impl<uint8_t> &Output) {
  ਚੋਣ (P.format) {
  ਹਾਲਤ compression::Format::Zlib:
    zlib::compress(ਨਿਵੇਸ਼, Output, P.level);
    ਰੋਕੋ;
  ਹਾਲਤ compression::Format::Zstd:
    zstd::compress(ਨਿਵੇਸ਼, Output, P.level, P.zstdEnableLdm);
    ਰੋਕੋ;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                              uint8_t *Output, size_t Uncompressed_ਮਾਪ) {
  ਚੋਣ (formatFor(T)) {
  ਹਾਲਤ compression::Format::Zlib:
    ਵਾਪਸ zlib::decompress(ਨਿਵੇਸ਼, Output, Uncompressed_ਮਾਪ);
  ਹਾਲਤ compression::Format::Zstd:
    ਵਾਪਸ zstd::decompress(ਨਿਵੇਸ਼, Output, Uncompressed_ਮਾਪ);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                              Small_ਸਦਿਸ਼_Impl<uint8_t> &Output,
                              size_t Uncompressed_ਮਾਪ) {
  ਚੋਣ (F) {
  ਹਾਲਤ compression::Format::Zlib:
    ਵਾਪਸ zlib::decompress(ਨਿਵੇਸ਼, Output, Uncompressed_ਮਾਪ);
  ਹਾਲਤ compression::Format::Zstd:
    ਵਾਪਸ zstd::decompress(ਨਿਵੇਸ਼, Output, Uncompressed_ਮਾਪ);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                              Small_ਸਦਿਸ਼_Impl<uint8_t> &Output,
                              size_t Uncompressed_ਮਾਪ) {
  ਵਾਪਸ decompress(formatFor(T), ਨਿਵੇਸ਼, Output, Uncompressed_ਮਾਪ);
}

Error compression::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                              Small_ਸਦਿਸ਼_Impl<uint8_t> &Output,
                              size_t Uncompressed_ਮਾਪ) {
  std::optional<Format> F = identifyFormat(ਨਿਵੇਸ਼);
  ਜੇ (ਅਚਲ ਅੱਖਰ *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ਵਾਪਸ createStringError(Reason);
  ਵਾਪਸ decompress(*F, ਨਿਵੇਸ਼, Output, Uncompressed_ਮਾਪ);
}

#ਜੇ LLVM_ENABLE_ZLIB

ਸਥਿਰ StringRef convert_Zlib_ਕੋਡ_ਮੰਜ਼ਿਲ_String(ਪੂਰਨ ਕੋਡ) {
  ਚੋਣ (ਕੋਡ) {
  ਹਾਲਤ Z_MEM_ERROR:
    ਵਾਪਸ "zlib error: Z_MEM_ERROR";
  ਹਾਲਤ Z_BUF_ERROR:
    ਵਾਪਸ "zlib error: Z_BUF_ERROR";
  ਹਾਲਤ Z_ਧਾਰਾ_ERROR:
    ਵਾਪਸ "zlib error: Z_STREAM_ERROR";
  ਹਾਲਤ Z_ਅੰਕੜੇ_ERROR:
    ਵਾਪਸ "zlib error: Z_DATA_ERROR";
  ਹਾਲਤ Z_OK:
  ਮੂਲ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

ਤਾਰਕਿਕ zlib::is_ਉਪਲਬਧ() { ਵਾਪਸ ਸੱਚ; }

ਖਾਲੀ zlib::compress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                    Small_ਸਦਿਸ਼_Impl<uint8_t> &Compressed_ਬਫਰ, ਪੂਰਨ Level) {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ Compressed_ਮਾਪ = ::compressBound(ਨਿਵੇਸ਼.ਮਾਪ_2());
  Compressed_ਬਫਰ.resize_for_overwrite(Compressed_ਮਾਪ);
  ਪੂਰਨ Res = ::compress2((Bytef *)Compressed_ਬਫਰ.ਅੰਕੜੇ(), &Compressed_ਮਾਪ,
                        (ਅਚਲ Bytef *)ਨਿਵੇਸ਼.ਅੰਕੜੇ(), ਨਿਵੇਸ਼.ਮਾਪ_2(), Level);
  ਜੇ (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  ਪ੍ਰਤਿਜ੍ਞਾ(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_ਬਫਰ.ਅੰਕੜੇ(), Compressed_ਮਾਪ);
  ਜੇ (Compressed_ਮਾਪ < Compressed_ਬਫਰ.ਮਾਪ_2())
    Compressed_ਬਫਰ.truncate(Compressed_ਮਾਪ);
}

Error zlib::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼, uint8_t *Output,
                       size_t &Uncompressed_ਮਾਪ) {
  ਪੂਰਨ Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_ਮਾਪ,
                         (ਅਚਲ Bytef *)ਨਿਵੇਸ਼.ਅੰਕੜੇ(), ਨਿਵੇਸ਼.ਮਾਪ_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_ਮਾਪ);
  ਵਾਪਸ Res ? make_error<StringError>(convert_Zlib_ਕੋਡ_ਮੰਜ਼ਿਲ_String(Res),
                                       inconvertible_Error_ਕੋਡ())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                       Small_ਸਦਿਸ਼_Impl<uint8_t> &Output,
                       size_t Uncompressed_ਮਾਪ) {
  Output.resize_for_overwrite(Uncompressed_ਮਾਪ);
  Error E = zlib::decompress(ਨਿਵੇਸ਼, Output.ਅੰਕੜੇ(), Uncompressed_ਮਾਪ);
  ਜੇ (Uncompressed_ਮਾਪ < Output.ਮਾਪ_2())
    Output.truncate(Uncompressed_ਮਾਪ);
  ਵਾਪਸ E;
}

#ਨਹੀਂਤਾਂ
ਤਾਰਕਿਕ zlib::is_ਉਪਲਬਧ() { ਵਾਪਸ ਝੂਠ; }
ਖਾਲੀ zlib::compress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                    Small_ਸਦਿਸ਼_Impl<uint8_t> &Compressed_ਬਫਰ, ਪੂਰਨ Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼, uint8_t *Uncompressed_ਬਫਰ,
                       size_t &Uncompressed_ਮਾਪ) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                       Small_ਸਦਿਸ਼_Impl<uint8_t> &Uncompressed_ਬਫਰ,
                       size_t Uncompressed_ਮਾਪ) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#ਜੇ_ਅੰਤ

#ਜੇ LLVM_ENABLE_ZSTD

ਤਾਰਕਿਕ zstd::is_ਉਪਲਬਧ() { ਵਾਪਸ ਸੱਚ; }

#ਅਨ੍ਤਰ੍ਭਾਵ <zstd.h> // Ensure ZSTD library is included

ਖਾਲੀ zstd::compress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                    Small_ਸਦਿਸ਼_Impl<uint8_t> &Compressed_ਬਫਰ, ਪੂਰਨ Level,
                    ਤਾਰਕਿਕ EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  ਜੇ (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  ਜੇ (ZSTD_isError(ZSTD_CCtx_ਸੈੱਟ_Parameter(
          Cctx, ZSTD_c_enable_Long_ਦੂਰੀ_Matching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  ਜੇ (ZSTD_isError(
          ZSTD_CCtx_ਸੈੱਟ_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ Compressed_ਬਫਰ_ਮਾਪ = ZSTD_compressBound(ਨਿਵੇਸ਼.ਮਾਪ_2());
  Compressed_ਬਫਰ.resize_for_overwrite(Compressed_ਬਫਰ_ਮਾਪ);

  size_t ਅਚਲ Compressed_ਮਾਪ =
      ZSTD_compress2(Cctx, Compressed_ਬਫਰ.ਅੰਕੜੇ(), Compressed_ਬਫਰ_ਮਾਪ,
                     ਨਿਵੇਸ਼.ਅੰਕੜੇ(), ਨਿਵੇਸ਼.ਮਾਪ_2());

  ZSTD_freeCCtx(Cctx);

  ਜੇ (ZSTD_isError(Compressed_ਮਾਪ))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_ਬਫਰ.ਅੰਕੜੇ(), Compressed_ਮਾਪ);
  ਜੇ (Compressed_ਮਾਪ < Compressed_ਬਫਰ.ਮਾਪ_2())
    Compressed_ਬਫਰ.truncate(Compressed_ਮਾਪ);
}

Error zstd::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼, uint8_t *Output,
                       size_t &Uncompressed_ਮਾਪ) {
  ਅਚਲ size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_ਮਾਪ, (ਅਚਲ uint8_t *)ਨਿਵੇਸ਼.ਅੰਕੜੇ(), ਨਿਵੇਸ਼.ਮਾਪ_2());
  Uncompressed_ਮਾਪ = Res;
  ਜੇ (ZSTD_isError(Res))
    ਵਾਪਸ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_ਕੋਡ());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_ਮਾਪ);
  ਵਾਪਸ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                       Small_ਸਦਿਸ਼_Impl<uint8_t> &Output,
                       size_t Uncompressed_ਮਾਪ) {
  Output.resize_for_overwrite(Uncompressed_ਮਾਪ);
  Error E = zstd::decompress(ਨਿਵੇਸ਼, Output.ਅੰਕੜੇ(), Uncompressed_ਮਾਪ);
  ਜੇ (Uncompressed_ਮਾਪ < Output.ਮਾਪ_2())
    Output.truncate(Uncompressed_ਮਾਪ);
  ਵਾਪਸ E;
}

#ਨਹੀਂਤਾਂ
ਤਾਰਕਿਕ zstd::is_ਉਪਲਬਧ() { ਵਾਪਸ ਝੂਠ; }
ਖਾਲੀ zstd::compress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                    Small_ਸਦਿਸ਼_Impl<uint8_t> &Compressed_ਬਫਰ, ਪੂਰਨ Level,
                    ਤਾਰਕਿਕ EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼, uint8_t *Output,
                       size_t &Uncompressed_ਮਾਪ) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                       Small_ਸਦਿਸ਼_Impl<uint8_t> &Output,
                       size_t Uncompressed_ਮਾਪ) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#ਜੇ_ਅੰਤ

#ਜੇ LLVM_ENABLE_LZMA

ਤਾਰਕਿਕ xz::is_ਉਪਲਬਧ() { ਵਾਪਸ ਸੱਚ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *convert_LZMACode_ਮੰਜ਼ਿਲ_String(lzma_ret ਕੋਡ) {
  ਚੋਣ (ਕੋਡ) {
  ਹਾਲਤ LZMA_ਧਾਰਾ_ਸਿਰਾ:
    ਵਾਪਸ "lzma error: LZMA_STREAM_END";
  ਹਾਲਤ LZMA_NO_CHECK:
    ਵਾਪਸ "lzma error: LZMA_NO_CHECK";
  ਹਾਲਤ LZMA_UNSUPPORTED_CHECK:
    ਵਾਪਸ "lzma error: LZMA_UNSUPPORTED_CHECK";
  ਹਾਲਤ LZMA_GET_CHECK:
    ਵਾਪਸ "lzma error: LZMA_GET_CHECK";
  ਹਾਲਤ LZMA_MEM_ERROR:
    ਵਾਪਸ "lzma error: LZMA_MEM_ERROR";
  ਹਾਲਤ LZMA_MEMLIMIT_ERROR:
    ਵਾਪਸ "lzma error: LZMA_MEMLIMIT_ERROR";
  ਹਾਲਤ LZMA_FORMAT_ERROR:
    ਵਾਪਸ "lzma error: LZMA_FORMAT_ERROR";
  ਹਾਲਤ LZMA_OPTIONS_ERROR:
    ਵਾਪਸ "lzma error: LZMA_OPTIONS_ERROR";
  ਹਾਲਤ LZMA_ਅੰਕੜੇ_ERROR:
    ਵਾਪਸ "lzma error: LZMA_DATA_ERROR";
  ਹਾਲਤ LZMA_BUF_ERROR:
    ਵਾਪਸ "lzma error: LZMA_BUF_ERROR";
  ਹਾਲਤ LZMA_PROG_ERROR:
    ਵਾਪਸ "lzma error: LZMA_PROG_ERROR";
  ਮੂਲ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ਸਥਿਰ Expected<uint64_t> get_Uncompressed_ਮਾਪ(ArrayRef<uint8_t> ਨਿਵੇਸ਼) {
  ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() < LZMA_ਧਾਰਾ_ਸਿਰਲੇਖ_ਮਾਪ)
    ਵਾਪਸ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        ਨਿਵੇਸ਼.ਮਾਪ_2(), size_t(LZMA_ਧਾਰਾ_ਸਿਰਲੇਖ_ਮਾਪ));

  // Decode the xz footer.
  lzma_ਧਾਰਾ_flags FooterFlags{};
  lzma_ret Ret = lzma_ਧਾਰਾ_footer_decode(
      &FooterFlags, ਨਿਵੇਸ਼.take_ਪਿੱਠ(LZMA_ਧਾਰਾ_ਸਿਰਲੇਖ_ਮਾਪ).ਅੰਕੜੇ());
  ਜੇ (Ret != LZMA_OK)
    ਵਾਪਸ createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_ਮੰਜ਼ਿਲ_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_ਮਾਪ = FooterFlags.backward_ਮਾਪ + 2 * LZMA_ਧਾਰਾ_ਸਿਰਲੇਖ_ਮਾਪ;
  ਜੇ (ਨਿਵੇਸ਼.ਮਾਪ_2() < Min_ਮਾਪ)
    ਵਾਪਸ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        ਨਿਵੇਸ਼.ਮਾਪ_2(), Min_ਮਾਪ);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_ਸੂਚਕਾਂਕ *ਸੂਚਕਾਂਕ_2 = ਖਾਲੀ_ਸੂਚਕ;
  llvm::scope_exit Free_ਸੂਚਕਾਂਕ([&] { lzma_ਸੂਚਕਾਂਕ_ਸਿਰਾ(ਸੂਚਕਾਂਕ_2, ਖਾਲੀ_ਸੂਚਕ); });
  uint64_t Mem_ਹੱਦਬੰਦੀ = UINT64_MAX;
  size_t ਵਿੱਚ_Pos = 0;
  Ret = lzma_ਸੂਚਕਾਂਕ_ਬਫਰ_decode(
      &ਸੂਚਕਾਂਕ_2, &Mem_ਹੱਦਬੰਦੀ, ਖਾਲੀ_ਸੂਚਕ,
      ਨਿਵੇਸ਼.take_ਪਿੱਠ(LZMA_ਧਾਰਾ_ਸਿਰਲੇਖ_ਮਾਪ + FooterFlags.backward_ਮਾਪ)
          .ਅੰਕੜੇ(),
      &ਵਿੱਚ_Pos, ਨਿਵੇਸ਼.ਮਾਪ_2());
  ਜੇ (Ret != LZMA_OK)
    ਵਾਪਸ createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_ਮੰਜ਼ਿਲ_String(Ret));

  ਵਾਪਸ lzma_ਸੂਚਕਾਂਕ_uncompressed_ਮਾਪ(ਸੂਚਕਾਂਕ_2);
}

Error xz::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                     Small_ਸਦਿਸ਼_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_ਮਾਪ = get_Uncompressed_ਮਾਪ(ਨਿਵੇਸ਼);
  ਜੇ (!Uncompressed_ਮਾਪ)
    ਵਾਪਸ Uncompressed_ਮਾਪ.takeError();

  ਜੇ (*Uncompressed_ਮਾਪ > std::numeric_limits<size_t>::max())
    ਵਾਪਸ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_ਮਾਪ);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_ਮਾਪ));
  uint64_t Mem_ਹੱਦਬੰਦੀ = UINT64_MAX;
  size_t ਵਿੱਚ_Pos = 0;
  size_t ਬਾਹਰਲਾ_Pos = 0;
  lzma_ret Ret = lzma_ਧਾਰਾ_ਬਫਰ_decode(
      &Mem_ਹੱਦਬੰਦੀ, /*flags=*/0, ਖਾਲੀ_ਸੂਚਕ, ਨਿਵੇਸ਼.ਅੰਕੜੇ(), &ਵਿੱਚ_Pos, ਨਿਵੇਸ਼.ਮਾਪ_2(),
      Output.ਅੰਕੜੇ(), &ਬਾਹਰਲਾ_Pos, Output.ਮਾਪ_2());
  ਜੇ (Ret != LZMA_OK) {
    Output.clear();
    ਵਾਪਸ createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_ਮੰਜ਼ਿਲ_String(Ret));
  }

  ਵਾਪਸ Error::success();
}

#ਨਹੀਂਤਾਂ

ਤਾਰਕਿਕ xz::is_ਉਪਲਬਧ() { ਵਾਪਸ ਝੂਠ; }
Error xz::decompress(ArrayRef<uint8_t> ਨਿਵੇਸ਼,
                     Small_ਸਦਿਸ਼_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#ਜੇ_ਅੰਤ
