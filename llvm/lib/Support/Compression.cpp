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

#ضمن "llvm/Support/Compression.h"
#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن <limits>
#ضمن <optional>
#إذا LLVM_ENABLE_ZLIB
#ضمن <zlib.h>
#جے_انت
#إذا LLVM_ENABLE_ZSTD
#ضمن <zstd.h>
#جے_انت
#إذا LLVM_ENABLE_LZMA
#ضمن <lzma.h>
#جے_انت

ورتوں نطاق llvm;
ورتوں نطاق llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ساكن منطقي is_Zlib_سرخی(ArrayRef<uint8_t> داخلہ) {
  إذا (داخلہ.ناپ_2() < 2)
    أرجع خطأ;
  بنانشانی CMF = داخلہ[0];
  بنانشانی FLG = داخلہ[1];
  إذا ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    أرجع خطأ;
  أرجع (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ساكن منطقي isZstdMagic(ArrayRef<uint8_t> داخلہ) {
  ساكن constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  أرجع داخلہ.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ساكن std::optional<Format> identifyFormat(ArrayRef<uint8_t> داخلہ) {
  إذا (isZstdMagic(داخلہ))
    أرجع Format::Zstd;
  إذا (is_Zlib_سرخی(داخلہ))
    أرجع Format::Zlib;
  أرجع std::nullopt;
}

ثابت حرف *compression::getReasonIfUnsupported(compression::Format F) {
  بدل (F) {
  حالة compression::Format::Zlib:
    إذا (zlib::is_دستیاب())
      أرجع خالی_سوچک;
    أرجع "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  حالة compression::Format::Zstd:
    إذا (zstd::is_دستیاب())
      أرجع خالی_سوچک;
    أرجع "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ثابت حرف *compression::getReasonIfUnsupported(ArrayRef<uint8_t> داخلہ) {
  إذا (std::optional<Format> F = identifyFormat(داخلہ))
    أرجع getReasonIfUnsupported(*F);
  أرجع "unknown compression format";
}

فراغ compression::compress(Params P, ArrayRef<uint8_t> داخلہ,
                           Small_سمتیہ_Impl<uint8_t> &Output) {
  بدل (P.format) {
  حالة compression::Format::Zlib:
    zlib::compress(داخلہ, Output, P.level);
    اكسر;
  حالة compression::Format::Zstd:
    zstd::compress(داخلہ, Output, P.level, P.zstdEnableLdm);
    اكسر;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> داخلہ,
                              uint8_t *Output, size_t Uncompressed_ناپ) {
  بدل (formatFor(T)) {
  حالة compression::Format::Zlib:
    أرجع zlib::decompress(داخلہ, Output, Uncompressed_ناپ);
  حالة compression::Format::Zstd:
    أرجع zstd::decompress(داخلہ, Output, Uncompressed_ناپ);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> داخلہ,
                              Small_سمتیہ_Impl<uint8_t> &Output,
                              size_t Uncompressed_ناپ) {
  بدل (F) {
  حالة compression::Format::Zlib:
    أرجع zlib::decompress(داخلہ, Output, Uncompressed_ناپ);
  حالة compression::Format::Zstd:
    أرجع zstd::decompress(داخلہ, Output, Uncompressed_ناپ);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> داخلہ,
                              Small_سمتیہ_Impl<uint8_t> &Output,
                              size_t Uncompressed_ناپ) {
  أرجع decompress(formatFor(T), داخلہ, Output, Uncompressed_ناپ);
}

Error compression::decompress(ArrayRef<uint8_t> داخلہ,
                              Small_سمتیہ_Impl<uint8_t> &Output,
                              size_t Uncompressed_ناپ) {
  std::optional<Format> F = identifyFormat(داخلہ);
  إذا (ثابت حرف *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    أرجع createStringError(Reason);
  أرجع decompress(*F, داخلہ, Output, Uncompressed_ناپ);
}

#إذا LLVM_ENABLE_ZLIB

ساكن StringRef convert_Zlib_کوڈ_منزل_String(صحيح کوڈ) {
  بدل (کوڈ) {
  حالة Z_MEM_ERROR:
    أرجع "zlib error: Z_MEM_ERROR";
  حالة Z_BUF_ERROR:
    أرجع "zlib error: Z_BUF_ERROR";
  حالة Z_دھارا_ERROR:
    أرجع "zlib error: Z_STREAM_ERROR";
  حالة Z_انکڑے_ERROR:
    أرجع "zlib error: Z_DATA_ERROR";
  حالة Z_OK:
  افتراضي:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

منطقي zlib::is_دستیاب() { أرجع سچ; }

فراغ zlib::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحيح Level) {
  بنانشانی لما Compressed_ناپ = ::compressBound(داخلہ.ناپ_2());
  Compressed_بفر.resize_for_overwrite(Compressed_ناپ);
  صحيح Res = ::compress2((Bytef *)Compressed_بفر.انکڑے(), &Compressed_ناپ,
                        (ثابت Bytef *)داخلہ.انکڑے(), داخلہ.ناپ_2(), Level);
  إذا (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  أكد(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_بفر.انکڑے(), Compressed_ناپ);
  إذا (Compressed_ناپ < Compressed_بفر.ناپ_2())
    Compressed_بفر.truncate(Compressed_ناپ);
}

Error zlib::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Output,
                       size_t &Uncompressed_ناپ) {
  صحيح Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_ناپ,
                         (ثابت Bytef *)داخلہ.انکڑے(), داخلہ.ناپ_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_ناپ);
  أرجع Res ? make_error<StringError>(convert_Zlib_کوڈ_منزل_String(Res),
                                       inconvertible_Error_کوڈ())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Output,
                       size_t Uncompressed_ناپ) {
  Output.resize_for_overwrite(Uncompressed_ناپ);
  Error E = zlib::decompress(داخلہ, Output.انکڑے(), Uncompressed_ناپ);
  إذا (Uncompressed_ناپ < Output.ناپ_2())
    Output.truncate(Uncompressed_ناپ);
  أرجع E;
}

#وإلا
منطقي zlib::is_دستیاب() { أرجع خطأ; }
فراغ zlib::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحيح Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Uncompressed_بفر,
                       size_t &Uncompressed_ناپ) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Uncompressed_بفر,
                       size_t Uncompressed_ناپ) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#جے_انت

#إذا LLVM_ENABLE_ZSTD

منطقي zstd::is_دستیاب() { أرجع سچ; }

#ضمن <zstd.h> // Ensure ZSTD library is included

فراغ zstd::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحيح Level,
                    منطقي EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  إذا (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  إذا (ZSTD_isError(ZSTD_CCtx_سیٹ_Parameter(
          Cctx, ZSTD_c_enable_Long_فاصلہ_Matching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  إذا (ZSTD_isError(
          ZSTD_CCtx_سیٹ_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  بنانشانی لما Compressed_بفر_ناپ = ZSTD_compressBound(داخلہ.ناپ_2());
  Compressed_بفر.resize_for_overwrite(Compressed_بفر_ناپ);

  size_t ثابت Compressed_ناپ =
      ZSTD_compress2(Cctx, Compressed_بفر.انکڑے(), Compressed_بفر_ناپ,
                     داخلہ.انکڑے(), داخلہ.ناپ_2());

  ZSTD_freeCCtx(Cctx);

  إذا (ZSTD_isError(Compressed_ناپ))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_بفر.انکڑے(), Compressed_ناپ);
  إذا (Compressed_ناپ < Compressed_بفر.ناپ_2())
    Compressed_بفر.truncate(Compressed_ناپ);
}

Error zstd::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Output,
                       size_t &Uncompressed_ناپ) {
  ثابت size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_ناپ, (ثابت uint8_t *)داخلہ.انکڑے(), داخلہ.ناپ_2());
  Uncompressed_ناپ = Res;
  إذا (ZSTD_isError(Res))
    أرجع make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_کوڈ());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_ناپ);
  أرجع Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Output,
                       size_t Uncompressed_ناپ) {
  Output.resize_for_overwrite(Uncompressed_ناپ);
  Error E = zstd::decompress(داخلہ, Output.انکڑے(), Uncompressed_ناپ);
  إذا (Uncompressed_ناپ < Output.ناپ_2())
    Output.truncate(Uncompressed_ناپ);
  أرجع E;
}

#وإلا
منطقي zstd::is_دستیاب() { أرجع خطأ; }
فراغ zstd::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحيح Level,
                    منطقي EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Output,
                       size_t &Uncompressed_ناپ) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Output,
                       size_t Uncompressed_ناپ) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#جے_انت

#إذا LLVM_ENABLE_LZMA

منطقي xz::is_دستیاب() { أرجع سچ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ساكن ثابت حرف *convert_LZMACode_منزل_String(lzma_ret کوڈ) {
  بدل (کوڈ) {
  حالة LZMA_دھارا_سرا:
    أرجع "lzma error: LZMA_STREAM_END";
  حالة LZMA_NO_CHECK:
    أرجع "lzma error: LZMA_NO_CHECK";
  حالة LZMA_UNSUPPORTED_CHECK:
    أرجع "lzma error: LZMA_UNSUPPORTED_CHECK";
  حالة LZMA_GET_CHECK:
    أرجع "lzma error: LZMA_GET_CHECK";
  حالة LZMA_MEM_ERROR:
    أرجع "lzma error: LZMA_MEM_ERROR";
  حالة LZMA_MEMLIMIT_ERROR:
    أرجع "lzma error: LZMA_MEMLIMIT_ERROR";
  حالة LZMA_FORMAT_ERROR:
    أرجع "lzma error: LZMA_FORMAT_ERROR";
  حالة LZMA_OPTIONS_ERROR:
    أرجع "lzma error: LZMA_OPTIONS_ERROR";
  حالة LZMA_انکڑے_ERROR:
    أرجع "lzma error: LZMA_DATA_ERROR";
  حالة LZMA_BUF_ERROR:
    أرجع "lzma error: LZMA_BUF_ERROR";
  حالة LZMA_PROG_ERROR:
    أرجع "lzma error: LZMA_PROG_ERROR";
  افتراضي:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ساكن Expected<uint64_t> get_Uncompressed_ناپ(ArrayRef<uint8_t> داخلہ) {
  إذا (داخلہ.ناپ_2() < LZMA_دھارا_سرخی_ناپ)
    أرجع createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        داخلہ.ناپ_2(), size_t(LZMA_دھارا_سرخی_ناپ));

  // Decode the xz footer.
  lzma_دھارا_flags FooterFlags{};
  lzma_ret Ret = lzma_دھارا_footer_decode(
      &FooterFlags, داخلہ.take_پٹھ(LZMA_دھارا_سرخی_ناپ).انکڑے());
  إذا (Ret != LZMA_OK)
    أرجع createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_منزل_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_ناپ = FooterFlags.backward_ناپ + 2 * LZMA_دھارا_سرخی_ناپ;
  إذا (داخلہ.ناپ_2() < Min_ناپ)
    أرجع createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        داخلہ.ناپ_2(), Min_ناپ);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_اشاریہ *اشاریہ_2 = خالی_سوچک;
  llvm::scope_exit Free_اشاریہ([&] { lzma_اشاریہ_سرا(اشاریہ_2, خالی_سوچک); });
  uint64_t Mem_حد_بندی = UINT64_MAX;
  size_t وچ_Pos = 0;
  Ret = lzma_اشاریہ_بفر_decode(
      &اشاریہ_2, &Mem_حد_بندی, خالی_سوچک,
      داخلہ.take_پٹھ(LZMA_دھارا_سرخی_ناپ + FooterFlags.backward_ناپ)
          .انکڑے(),
      &وچ_Pos, داخلہ.ناپ_2());
  إذا (Ret != LZMA_OK)
    أرجع createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_منزل_String(Ret));

  أرجع lzma_اشاریہ_uncompressed_ناپ(اشاریہ_2);
}

Error xz::decompress(ArrayRef<uint8_t> داخلہ,
                     Small_سمتیہ_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_ناپ = get_Uncompressed_ناپ(داخلہ);
  إذا (!Uncompressed_ناپ)
    أرجع Uncompressed_ناپ.takeError();

  إذا (*Uncompressed_ناپ > std::numeric_limits<size_t>::max())
    أرجع createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_ناپ);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_ناپ));
  uint64_t Mem_حد_بندی = UINT64_MAX;
  size_t وچ_Pos = 0;
  size_t باہرلا_Pos = 0;
  lzma_ret Ret = lzma_دھارا_بفر_decode(
      &Mem_حد_بندی, /*flags=*/0, خالی_سوچک, داخلہ.انکڑے(), &وچ_Pos, داخلہ.ناپ_2(),
      Output.انکڑے(), &باہرلا_Pos, Output.ناپ_2());
  إذا (Ret != LZMA_OK) {
    Output.clear();
    أرجع createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_منزل_String(Ret));
  }

  أرجع Error::success();
}

#وإلا

منطقي xz::is_دستیاب() { أرجع خطأ; }
Error xz::decompress(ArrayRef<uint8_t> داخلہ,
                     Small_سمتیہ_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#جے_انت
