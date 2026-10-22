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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Compression.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/ScopeExit.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallVector.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Compiler.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ <limits>
#ಅನ್ತರ್ಭಾವ <optional>
#ಆದರೆ LLVM_ENABLE_ZLIB
#ಅನ್ತರ್ಭಾವ <zlib.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ LLVM_ENABLE_ZSTD
#ಅನ್ತರ್ಭಾವ <zstd.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ LLVM_ENABLE_LZMA
#ಅನ್ತರ್ಭಾವ <lzma.h>
#ಆದರೆ_ಅಂತ್ಯ

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ಸ್ಥಿರ ತಾರ್ಕಿಕ is_Zlib_ಶೀರ್ಷಿಕೆ(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() < 2)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಚಿಹ್ನೆರಹಿತ CMF = ಒಳಹರಿವು[0];
  ಚಿಹ್ನೆರಹಿತ FLG = ಒಳಹರಿವು[1];
  ಆದರೆ ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
  ಹಿಂತಿರುಗಿ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isZstdMagic(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಸ್ಥಿರ constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ಹಿಂತಿರುಗಿ ಒಳಹರಿವು.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ಸ್ಥಿರ std::optional<Format> identifyFormat(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಆದರೆ (isZstdMagic(ಒಳಹರಿವು))
    ಹಿಂತಿರುಗಿ Format::Zstd;
  ಆದರೆ (is_Zlib_ಶೀರ್ಷಿಕೆ(ಒಳಹರಿವು))
    ಹಿಂತಿರುಗಿ Format::Zlib;
  ಹಿಂತಿರುಗಿ std::nullopt;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *compression::getReasonIfUnsupported(compression::Format F) {
  ಆಯ್ಕೆ (F) {
  ಸ್ಥಿತಿ compression::Format::Zlib:
    ಆದರೆ (zlib::is_ಲಭ್ಯ())
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    ಹಿಂತಿರುಗಿ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  ಸ್ಥಿತಿ compression::Format::Zstd:
    ಆದರೆ (zstd::is_ಲಭ್ಯ())
      ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
    ಹಿಂತಿರುಗಿ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *compression::getReasonIfUnsupported(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಆದರೆ (std::optional<Format> F = identifyFormat(ಒಳಹರಿವು))
    ಹಿಂತಿರುಗಿ getReasonIfUnsupported(*F);
  ಹಿಂತಿರುಗಿ "unknown compression format";
}

ಶೂನ್ಯ compression::compress(Params P, ArrayRef<uint8_t> ಒಳಹರಿವು,
                           Small_ಸದಿಶ_Impl<uint8_t> &Output) {
  ಆಯ್ಕೆ (P.format) {
  ಸ್ಥಿತಿ compression::Format::Zlib:
    zlib::compress(ಒಳಹರಿವು, Output, P.level);
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ compression::Format::Zstd:
    zstd::compress(ಒಳಹರಿವು, Output, P.level, P.zstdEnableLdm);
    ನಿಲ್ಲಿಸು;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ಒಳಹರಿವು,
                              uint8_t *Output, size_t Uncompressed_ಗಾತ್ರ) {
  ಆಯ್ಕೆ (formatFor(T)) {
  ಸ್ಥಿತಿ compression::Format::Zlib:
    ಹಿಂತಿರುಗಿ zlib::decompress(ಒಳಹರಿವು, Output, Uncompressed_ಗಾತ್ರ);
  ಸ್ಥಿತಿ compression::Format::Zstd:
    ಹಿಂತಿರುಗಿ zstd::decompress(ಒಳಹರಿವು, Output, Uncompressed_ಗಾತ್ರ);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> ಒಳಹರಿವು,
                              Small_ಸದಿಶ_Impl<uint8_t> &Output,
                              size_t Uncompressed_ಗಾತ್ರ) {
  ಆಯ್ಕೆ (F) {
  ಸ್ಥಿತಿ compression::Format::Zlib:
    ಹಿಂತಿರುಗಿ zlib::decompress(ಒಳಹರಿವು, Output, Uncompressed_ಗಾತ್ರ);
  ಸ್ಥಿತಿ compression::Format::Zstd:
    ಹಿಂತಿರುಗಿ zstd::decompress(ಒಳಹರಿವು, Output, Uncompressed_ಗಾತ್ರ);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ಒಳಹರಿವು,
                              Small_ಸದಿಶ_Impl<uint8_t> &Output,
                              size_t Uncompressed_ಗಾತ್ರ) {
  ಹಿಂತಿರುಗಿ decompress(formatFor(T), ಒಳಹರಿವು, Output, Uncompressed_ಗಾತ್ರ);
}

Error compression::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                              Small_ಸದಿಶ_Impl<uint8_t> &Output,
                              size_t Uncompressed_ಗಾತ್ರ) {
  std::optional<Format> F = identifyFormat(ಒಳಹರಿವು);
  ಆದರೆ (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ಹಿಂತಿರುಗಿ createStringError(Reason);
  ಹಿಂತಿರುಗಿ decompress(*F, ಒಳಹರಿವು, Output, Uncompressed_ಗಾತ್ರ);
}

#ಆದರೆ LLVM_ENABLE_ZLIB

ಸ್ಥಿರ StringRef convert_Zlib_ಸಂಕೇತ_ಗಮ್ಯ_String(ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2) {
  ಆಯ್ಕೆ (ಸಂಕೇತ_2) {
  ಸ್ಥಿತಿ Z_MEM_ERROR:
    ಹಿಂತಿರುಗಿ "zlib error: Z_MEM_ERROR";
  ಸ್ಥಿತಿ Z_BUF_ERROR:
    ಹಿಂತಿರುಗಿ "zlib error: Z_BUF_ERROR";
  ಸ್ಥಿತಿ Z_ಹರಿವು_ERROR:
    ಹಿಂತಿರುಗಿ "zlib error: Z_STREAM_ERROR";
  ಸ್ಥಿತಿ Z_ದತ್ತಾಂಶ_ERROR:
    ಹಿಂತಿರುಗಿ "zlib error: Z_DATA_ERROR";
  ಸ್ಥಿತಿ Z_OK:
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

ತಾರ್ಕಿಕ zlib::is_ಲಭ್ಯ() { ಹಿಂತಿರುಗಿ ನಿಜ; }

ಶೂನ್ಯ zlib::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    Small_ಸದಿಶ_Impl<uint8_t> &Compressed_ಬಫರ್, ಪೂರ್ಣಾಂಕ Level) {
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ Compressed_ಗಾತ್ರ = ::compressBound(ಒಳಹರಿವು.ಗಾತ್ರ_3());
  Compressed_ಬಫರ್.resize_for_overwrite(Compressed_ಗಾತ್ರ);
  ಪೂರ್ಣಾಂಕ Res = ::compress2((Bytef *)Compressed_ಬಫರ್.ದತ್ತಾಂಶ(), &Compressed_ಗಾತ್ರ,
                        (ಸ್ಥಿರಾಂಕ Bytef *)ಒಳಹರಿವು.ದತ್ತಾಂಶ(), ಒಳಹರಿವು.ಗಾತ್ರ_3(), Level);
  ಆದರೆ (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  ಪ್ರತಿಜ್ಞಾ(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_ಬಫರ್.ದತ್ತಾಂಶ(), Compressed_ಗಾತ್ರ);
  ಆದರೆ (Compressed_ಗಾತ್ರ < Compressed_ಬಫರ್.ಗಾತ್ರ_3())
    Compressed_ಬಫರ್.truncate(Compressed_ಗಾತ್ರ);
}

Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Output,
                       size_t &Uncompressed_ಗಾತ್ರ) {
  ಪೂರ್ಣಾಂಕ Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_ಗಾತ್ರ,
                         (ಸ್ಥಿರಾಂಕ Bytef *)ಒಳಹರಿವು.ದತ್ತಾಂಶ(), ಒಳಹರಿವು.ಗಾತ್ರ_3());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_ಗಾತ್ರ);
  ಹಿಂತಿರುಗಿ Res ? make_error<StringError>(convert_Zlib_ಸಂಕೇತ_ಗಮ್ಯ_String(Res),
                                       inconvertible_Error_ಸಂಕೇತ())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       Small_ಸದಿಶ_Impl<uint8_t> &Output,
                       size_t Uncompressed_ಗಾತ್ರ) {
  Output.resize_for_overwrite(Uncompressed_ಗಾತ್ರ);
  Error E = zlib::decompress(ಒಳಹರಿವು, Output.ದತ್ತಾಂಶ(), Uncompressed_ಗಾತ್ರ);
  ಆದರೆ (Uncompressed_ಗಾತ್ರ < Output.ಗಾತ್ರ_3())
    Output.truncate(Uncompressed_ಗಾತ್ರ);
  ಹಿಂತಿರುಗಿ E;
}

#ಇಲ್ಲದಿದ್ದರೆ
ತಾರ್ಕಿಕ zlib::is_ಲಭ್ಯ() { ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; }
ಶೂನ್ಯ zlib::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    Small_ಸದಿಶ_Impl<uint8_t> &Compressed_ಬಫರ್, ಪೂರ್ಣಾಂಕ Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Uncompressed_ಬಫರ್,
                       size_t &Uncompressed_ಗಾತ್ರ) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       Small_ಸದಿಶ_Impl<uint8_t> &Uncompressed_ಬಫರ್,
                       size_t Uncompressed_ಗಾತ್ರ) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ LLVM_ENABLE_ZSTD

ತಾರ್ಕಿಕ zstd::is_ಲಭ್ಯ() { ಹಿಂತಿರುಗಿ ನಿಜ; }

#ಅನ್ತರ್ಭಾವ <zstd.h> // Ensure ZSTD library is included

ಶೂನ್ಯ zstd::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    Small_ಸದಿಶ_Impl<uint8_t> &Compressed_ಬಫರ್, ಪೂರ್ಣಾಂಕ Level,
                    ತಾರ್ಕಿಕ EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  ಆದರೆ (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  ಆದರೆ (ZSTD_isError(ZSTD_CCtx_ಗಣ_Parameter(
          Cctx, ZSTD_c_enable_Long_ದೂರ_Matching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  ಆದರೆ (ZSTD_isError(
          ZSTD_CCtx_ಗಣ_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ Compressed_ಬಫರ್_ಗಾತ್ರ = ZSTD_compressBound(ಒಳಹರಿವು.ಗಾತ್ರ_3());
  Compressed_ಬಫರ್.resize_for_overwrite(Compressed_ಬಫರ್_ಗಾತ್ರ);

  size_t ಸ್ಥಿರಾಂಕ Compressed_ಗಾತ್ರ =
      ZSTD_compress2(Cctx, Compressed_ಬಫರ್.ದತ್ತಾಂಶ(), Compressed_ಬಫರ್_ಗಾತ್ರ,
                     ಒಳಹರಿವು.ದತ್ತಾಂಶ(), ಒಳಹರಿವು.ಗಾತ್ರ_3());

  ZSTD_freeCCtx(Cctx);

  ಆದರೆ (ZSTD_isError(Compressed_ಗಾತ್ರ))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_ಬಫರ್.ದತ್ತಾಂಶ(), Compressed_ಗಾತ್ರ);
  ಆದರೆ (Compressed_ಗಾತ್ರ < Compressed_ಬಫರ್.ಗಾತ್ರ_3())
    Compressed_ಬಫರ್.truncate(Compressed_ಗಾತ್ರ);
}

Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Output,
                       size_t &Uncompressed_ಗಾತ್ರ) {
  ಸ್ಥಿರಾಂಕ size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_ಗಾತ್ರ, (ಸ್ಥಿರಾಂಕ uint8_t *)ಒಳಹರಿವು.ದತ್ತಾಂಶ(), ಒಳಹರಿವು.ಗಾತ್ರ_3());
  Uncompressed_ಗಾತ್ರ = Res;
  ಆದರೆ (ZSTD_isError(Res))
    ಹಿಂತಿರುಗಿ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_ಸಂಕೇತ());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_ಗಾತ್ರ);
  ಹಿಂತಿರುಗಿ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       Small_ಸದಿಶ_Impl<uint8_t> &Output,
                       size_t Uncompressed_ಗಾತ್ರ) {
  Output.resize_for_overwrite(Uncompressed_ಗಾತ್ರ);
  Error E = zstd::decompress(ಒಳಹರಿವು, Output.ದತ್ತಾಂಶ(), Uncompressed_ಗಾತ್ರ);
  ಆದರೆ (Uncompressed_ಗಾತ್ರ < Output.ಗಾತ್ರ_3())
    Output.truncate(Uncompressed_ಗಾತ್ರ);
  ಹಿಂತಿರುಗಿ E;
}

#ಇಲ್ಲದಿದ್ದರೆ
ತಾರ್ಕಿಕ zstd::is_ಲಭ್ಯ() { ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; }
ಶೂನ್ಯ zstd::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    Small_ಸದಿಶ_Impl<uint8_t> &Compressed_ಬಫರ್, ಪೂರ್ಣಾಂಕ Level,
                    ತಾರ್ಕಿಕ EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Output,
                       size_t &Uncompressed_ಗಾತ್ರ) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       Small_ಸದಿಶ_Impl<uint8_t> &Output,
                       size_t Uncompressed_ಗಾತ್ರ) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ LLVM_ENABLE_LZMA

ತಾರ್ಕಿಕ xz::is_ಲಭ್ಯ() { ಹಿಂತಿರುಗಿ ನಿಜ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *convert_LZMACode_ಗಮ್ಯ_String(lzma_ret ಸಂಕೇತ_2) {
  ಆಯ್ಕೆ (ಸಂಕೇತ_2) {
  ಸ್ಥಿತಿ LZMA_ಹರಿವು_ಕೊನೆ:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_STREAM_END";
  ಸ್ಥಿತಿ LZMA_NO_CHECK:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_NO_CHECK";
  ಸ್ಥಿತಿ LZMA_UNSUPPORTED_CHECK:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_UNSUPPORTED_CHECK";
  ಸ್ಥಿತಿ LZMA_GET_CHECK:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_GET_CHECK";
  ಸ್ಥಿತಿ LZMA_MEM_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_MEM_ERROR";
  ಸ್ಥಿತಿ LZMA_MEMLIMIT_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_MEMLIMIT_ERROR";
  ಸ್ಥಿತಿ LZMA_FORMAT_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_FORMAT_ERROR";
  ಸ್ಥಿತಿ LZMA_OPTIONS_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_OPTIONS_ERROR";
  ಸ್ಥಿತಿ LZMA_ದತ್ತಾಂಶ_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_DATA_ERROR";
  ಸ್ಥಿತಿ LZMA_BUF_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_BUF_ERROR";
  ಸ್ಥಿತಿ LZMA_PROG_ERROR:
    ಹಿಂತಿರುಗಿ "lzma error: LZMA_PROG_ERROR";
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ಸ್ಥಿರ Expected<uint64_t> get_Uncompressed_ಗಾತ್ರ(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() < LZMA_ಹರಿವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        ಒಳಹರಿವು.ಗಾತ್ರ_3(), size_t(LZMA_ಹರಿವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ));

  // Decode the xz footer.
  lzma_ಹರಿವು_flags FooterFlags{};
  lzma_ret Ret = lzma_ಹರಿವು_footer_decode(
      &FooterFlags, ಒಳಹರಿವು.take_ಹಿಂಭಾಗ(LZMA_ಹರಿವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ).ದತ್ತಾಂಶ());
  ಆದರೆ (Ret != LZMA_OK)
    ಹಿಂತಿರುಗಿ createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_ಗಮ್ಯ_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_ಗಾತ್ರ = FooterFlags.backward_ಗಾತ್ರ + 2 * LZMA_ಹರಿವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ;
  ಆದರೆ (ಒಳಹರಿವು.ಗಾತ್ರ_3() < Min_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        ಒಳಹರಿವು.ಗಾತ್ರ_3(), Min_ಗಾತ್ರ);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_ಸೂಚ್ಯಂಕ *ಸೂಚ್ಯಂಕ_2 = ಶೂನ್ಯಸೂಚಕ;
  llvm::scope_exit Free_ಸೂಚ್ಯಂಕ([&] { lzma_ಸೂಚ್ಯಂಕ_ಕೊನೆ(ಸೂಚ್ಯಂಕ_2, ಶೂನ್ಯಸೂಚಕ); });
  uint64_t Mem_ಎಲ್ಲೆ = UINT64_MAX;
  size_t ಒಳ_Pos = 0;
  Ret = lzma_ಸೂಚ್ಯಂಕ_ಬಫರ್_decode(
      &ಸೂಚ್ಯಂಕ_2, &Mem_ಎಲ್ಲೆ, ಶೂನ್ಯಸೂಚಕ,
      ಒಳಹರಿವು.take_ಹಿಂಭಾಗ(LZMA_ಹರಿವು_ಶೀರ್ಷಿಕೆ_ಗಾತ್ರ + FooterFlags.backward_ಗಾತ್ರ)
          .ದತ್ತಾಂಶ(),
      &ಒಳ_Pos, ಒಳಹರಿವು.ಗಾತ್ರ_3());
  ಆದರೆ (Ret != LZMA_OK)
    ಹಿಂತಿರುಗಿ createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_ಗಮ್ಯ_String(Ret));

  ಹಿಂತಿರುಗಿ lzma_ಸೂಚ್ಯಂಕ_uncompressed_ಗಾತ್ರ(ಸೂಚ್ಯಂಕ_2);
}

Error xz::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                     Small_ಸದಿಶ_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_ಗಾತ್ರ = get_Uncompressed_ಗಾತ್ರ(ಒಳಹರಿವು);
  ಆದರೆ (!Uncompressed_ಗಾತ್ರ)
    ಹಿಂತಿರುಗಿ Uncompressed_ಗಾತ್ರ.takeError();

  ಆದರೆ (*Uncompressed_ಗಾತ್ರ > std::numeric_limits<size_t>::max())
    ಹಿಂತಿರುಗಿ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_ಗಾತ್ರ);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_ಗಾತ್ರ));
  uint64_t Mem_ಎಲ್ಲೆ = UINT64_MAX;
  size_t ಒಳ_Pos = 0;
  size_t ಹೊರಹೋಗು_Pos = 0;
  lzma_ret Ret = lzma_ಹರಿವು_ಬಫರ್_decode(
      &Mem_ಎಲ್ಲೆ, /*flags=*/0, ಶೂನ್ಯಸೂಚಕ, ಒಳಹರಿವು.ದತ್ತಾಂಶ(), &ಒಳ_Pos, ಒಳಹರಿವು.ಗಾತ್ರ_3(),
      Output.ದತ್ತಾಂಶ(), &ಹೊರಹೋಗು_Pos, Output.ಗಾತ್ರ_3());
  ಆದರೆ (Ret != LZMA_OK) {
    Output.clear();
    ಹಿಂತಿರುಗಿ createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_ಗಮ್ಯ_String(Ret));
  }

  ಹಿಂತಿರುಗಿ Error::success();
}

#ಇಲ್ಲದಿದ್ದರೆ

ತಾರ್ಕಿಕ xz::is_ಲಭ್ಯ() { ಹಿಂತಿರುಗಿ ಸುಳ್ಳು; }
Error xz::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                     Small_ಸದಿಶ_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#ಆದರೆ_ಅಂತ್ಯ
