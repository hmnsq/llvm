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
#جيڪڏهن_پورو
#إذا LLVM_ENABLE_ZSTD
#ضمن <zstd.h>
#جيڪڏهن_پورو
#إذا LLVM_ENABLE_LZMA
#ضمن <lzma.h>
#جيڪڏهن_پورو

استعمال نطاق llvm;
استعمال نطاق llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ساكن منطقي is_Zlib_سرو(ArrayRef<uint8_t> داخلا) {
  إذا (داخلا.سائيز_2() < 2)
    أرجع خطأ;
  بنانشان CMF = داخلا[0];
  بنانشان FLG = داخلا[1];
  إذا ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    أرجع خطأ;
  أرجع (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ساكن منطقي isZstdMagic(ArrayRef<uint8_t> داخلا) {
  ساكن constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  أرجع داخلا.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ساكن std::optional<Format> identifyFormat(ArrayRef<uint8_t> داخلا) {
  إذا (isZstdMagic(داخلا))
    أرجع Format::Zstd;
  إذا (is_Zlib_سرو(داخلا))
    أرجع Format::Zlib;
  أرجع std::nullopt;
}

ثابت حرف *compression::getReasonIfUnsupported(compression::Format F) {
  بدل (F) {
  حالة compression::Format::Zlib:
    إذا (zlib::is_موجود())
      أرجع خالي_اشارو;
    أرجع "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  حالة compression::Format::Zstd:
    إذا (zstd::is_موجود())
      أرجع خالي_اشارو;
    أرجع "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ثابت حرف *compression::getReasonIfUnsupported(ArrayRef<uint8_t> داخلا) {
  إذا (std::optional<Format> F = identifyFormat(داخلا))
    أرجع getReasonIfUnsupported(*F);
  أرجع "unknown compression format";
}

فراغ compression::compress(Params P, ArrayRef<uint8_t> داخلا,
                           Small_ويڪٽر_Impl<uint8_t> &Output) {
  بدل (P.format) {
  حالة compression::Format::Zlib:
    zlib::compress(داخلا, Output, P.level);
    اكسر;
  حالة compression::Format::Zstd:
    zstd::compress(داخلا, Output, P.level, P.zstdEnableLdm);
    اكسر;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> داخلا,
                              uint8_t *Output, size_t Uncompressed_سائيز) {
  بدل (formatFor(T)) {
  حالة compression::Format::Zlib:
    أرجع zlib::decompress(داخلا, Output, Uncompressed_سائيز);
  حالة compression::Format::Zstd:
    أرجع zstd::decompress(داخلا, Output, Uncompressed_سائيز);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> داخلا,
                              Small_ويڪٽر_Impl<uint8_t> &Output,
                              size_t Uncompressed_سائيز) {
  بدل (F) {
  حالة compression::Format::Zlib:
    أرجع zlib::decompress(داخلا, Output, Uncompressed_سائيز);
  حالة compression::Format::Zstd:
    أرجع zstd::decompress(داخلا, Output, Uncompressed_سائيز);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> داخلا,
                              Small_ويڪٽر_Impl<uint8_t> &Output,
                              size_t Uncompressed_سائيز) {
  أرجع decompress(formatFor(T), داخلا, Output, Uncompressed_سائيز);
}

Error compression::decompress(ArrayRef<uint8_t> داخلا,
                              Small_ويڪٽر_Impl<uint8_t> &Output,
                              size_t Uncompressed_سائيز) {
  std::optional<Format> F = identifyFormat(داخلا);
  إذا (ثابت حرف *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    أرجع createStringError(Reason);
  أرجع decompress(*F, داخلا, Output, Uncompressed_سائيز);
}

#إذا LLVM_ENABLE_ZLIB

ساكن StringRef convert_Zlib_ڪوڊ_منزل_String(صحيح ڪوڊ) {
  بدل (ڪوڊ) {
  حالة Z_MEM_ERROR:
    أرجع "zlib error: Z_MEM_ERROR";
  حالة Z_BUF_ERROR:
    أرجع "zlib error: Z_BUF_ERROR";
  حالة Z_وهڪرو_ERROR:
    أرجع "zlib error: Z_STREAM_ERROR";
  حالة Z_معلومات_ERROR:
    أرجع "zlib error: Z_DATA_ERROR";
  حالة Z_OK:
  افتراضي:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

منطقي zlib::is_موجود() { أرجع سچ; }

فراغ zlib::compress(ArrayRef<uint8_t> داخلا,
                    Small_ويڪٽر_Impl<uint8_t> &Compressed_بفر, صحيح Level) {
  بنانشان ڊگهو Compressed_سائيز = ::compressBound(داخلا.سائيز_2());
  Compressed_بفر.resize_for_overwrite(Compressed_سائيز);
  صحيح Res = ::compress2((Bytef *)Compressed_بفر.معلومات(), &Compressed_سائيز,
                        (ثابت Bytef *)داخلا.معلومات(), داخلا.سائيز_2(), Level);
  إذا (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  أكد(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_بفر.معلومات(), Compressed_سائيز);
  إذا (Compressed_سائيز < Compressed_بفر.سائيز_2())
    Compressed_بفر.truncate(Compressed_سائيز);
}

Error zlib::decompress(ArrayRef<uint8_t> داخلا, uint8_t *Output,
                       size_t &Uncompressed_سائيز) {
  صحيح Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_سائيز,
                         (ثابت Bytef *)داخلا.معلومات(), داخلا.سائيز_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_سائيز);
  أرجع Res ? make_error<StringError>(convert_Zlib_ڪوڊ_منزل_String(Res),
                                       inconvertible_Error_ڪوڊ())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> داخلا,
                       Small_ويڪٽر_Impl<uint8_t> &Output,
                       size_t Uncompressed_سائيز) {
  Output.resize_for_overwrite(Uncompressed_سائيز);
  Error E = zlib::decompress(داخلا, Output.معلومات(), Uncompressed_سائيز);
  إذا (Uncompressed_سائيز < Output.سائيز_2())
    Output.truncate(Uncompressed_سائيز);
  أرجع E;
}

#وإلا
منطقي zlib::is_موجود() { أرجع خطأ; }
فراغ zlib::compress(ArrayRef<uint8_t> داخلا,
                    Small_ويڪٽر_Impl<uint8_t> &Compressed_بفر, صحيح Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> داخلا, uint8_t *Uncompressed_بفر,
                       size_t &Uncompressed_سائيز) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> داخلا,
                       Small_ويڪٽر_Impl<uint8_t> &Uncompressed_بفر,
                       size_t Uncompressed_سائيز) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#جيڪڏهن_پورو

#إذا LLVM_ENABLE_ZSTD

منطقي zstd::is_موجود() { أرجع سچ; }

#ضمن <zstd.h> // Ensure ZSTD library is included

فراغ zstd::compress(ArrayRef<uint8_t> داخلا,
                    Small_ويڪٽر_Impl<uint8_t> &Compressed_بفر, صحيح Level,
                    منطقي EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  إذا (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  إذا (ZSTD_isError(ZSTD_CCtx_سيٽ_Parameter(
          Cctx, ZSTD_c_enable_Long_مفاصلو_Matching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  إذا (ZSTD_isError(
          ZSTD_CCtx_سيٽ_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  بنانشان ڊگهو Compressed_بفر_سائيز = ZSTD_compressBound(داخلا.سائيز_2());
  Compressed_بفر.resize_for_overwrite(Compressed_بفر_سائيز);

  size_t ثابت Compressed_سائيز =
      ZSTD_compress2(Cctx, Compressed_بفر.معلومات(), Compressed_بفر_سائيز,
                     داخلا.معلومات(), داخلا.سائيز_2());

  ZSTD_freeCCtx(Cctx);

  إذا (ZSTD_isError(Compressed_سائيز))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_بفر.معلومات(), Compressed_سائيز);
  إذا (Compressed_سائيز < Compressed_بفر.سائيز_2())
    Compressed_بفر.truncate(Compressed_سائيز);
}

Error zstd::decompress(ArrayRef<uint8_t> داخلا, uint8_t *Output,
                       size_t &Uncompressed_سائيز) {
  ثابت size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_سائيز, (ثابت uint8_t *)داخلا.معلومات(), داخلا.سائيز_2());
  Uncompressed_سائيز = Res;
  إذا (ZSTD_isError(Res))
    أرجع make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_ڪوڊ());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_سائيز);
  أرجع Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> داخلا,
                       Small_ويڪٽر_Impl<uint8_t> &Output,
                       size_t Uncompressed_سائيز) {
  Output.resize_for_overwrite(Uncompressed_سائيز);
  Error E = zstd::decompress(داخلا, Output.معلومات(), Uncompressed_سائيز);
  إذا (Uncompressed_سائيز < Output.سائيز_2())
    Output.truncate(Uncompressed_سائيز);
  أرجع E;
}

#وإلا
منطقي zstd::is_موجود() { أرجع خطأ; }
فراغ zstd::compress(ArrayRef<uint8_t> داخلا,
                    Small_ويڪٽر_Impl<uint8_t> &Compressed_بفر, صحيح Level,
                    منطقي EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> داخلا, uint8_t *Output,
                       size_t &Uncompressed_سائيز) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> داخلا,
                       Small_ويڪٽر_Impl<uint8_t> &Output,
                       size_t Uncompressed_سائيز) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#جيڪڏهن_پورو

#إذا LLVM_ENABLE_LZMA

منطقي xz::is_موجود() { أرجع سچ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ساكن ثابت حرف *convert_LZMACode_منزل_String(lzma_ret ڪوڊ) {
  بدل (ڪوڊ) {
  حالة LZMA_وهڪرو_پڇاڙي:
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
  حالة LZMA_معلومات_ERROR:
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
ساكن Expected<uint64_t> get_Uncompressed_سائيز(ArrayRef<uint8_t> داخلا) {
  إذا (داخلا.سائيز_2() < LZMA_وهڪرو_سرو_سائيز)
    أرجع createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        داخلا.سائيز_2(), size_t(LZMA_وهڪرو_سرو_سائيز));

  // Decode the xz footer.
  lzma_وهڪرو_flags FooterFlags{};
  lzma_ret Ret = lzma_وهڪرو_footer_decode(
      &FooterFlags, داخلا.take_پٺ(LZMA_وهڪرو_سرو_سائيز).معلومات());
  إذا (Ret != LZMA_OK)
    أرجع createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_منزل_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_سائيز = FooterFlags.backward_سائيز + 2 * LZMA_وهڪرو_سرو_سائيز;
  إذا (داخلا.سائيز_2() < Min_سائيز)
    أرجع createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        داخلا.سائيز_2(), Min_سائيز);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_اشاريو *اشاريو_2 = خالي_اشارو;
  llvm::scope_exit Free_اشاريو([&] { lzma_اشاريو_پڇاڙي(اشاريو_2, خالي_اشارو); });
  uint64_t Mem_حد_بندي = UINT64_MAX;
  size_t اندريون_Pos = 0;
  Ret = lzma_اشاريو_بفر_decode(
      &اشاريو_2, &Mem_حد_بندي, خالي_اشارو,
      داخلا.take_پٺ(LZMA_وهڪرو_سرو_سائيز + FooterFlags.backward_سائيز)
          .معلومات(),
      &اندريون_Pos, داخلا.سائيز_2());
  إذا (Ret != LZMA_OK)
    أرجع createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_منزل_String(Ret));

  أرجع lzma_اشاريو_uncompressed_سائيز(اشاريو_2);
}

Error xz::decompress(ArrayRef<uint8_t> داخلا,
                     Small_ويڪٽر_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_سائيز = get_Uncompressed_سائيز(داخلا);
  إذا (!Uncompressed_سائيز)
    أرجع Uncompressed_سائيز.takeError();

  إذا (*Uncompressed_سائيز > std::numeric_limits<size_t>::max())
    أرجع createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_سائيز);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_سائيز));
  uint64_t Mem_حد_بندي = UINT64_MAX;
  size_t اندريون_Pos = 0;
  size_t ٻاهريون_Pos = 0;
  lzma_ret Ret = lzma_وهڪرو_بفر_decode(
      &Mem_حد_بندي, /*flags=*/0, خالي_اشارو, داخلا.معلومات(), &اندريون_Pos, داخلا.سائيز_2(),
      Output.معلومات(), &ٻاهريون_Pos, Output.سائيز_2());
  إذا (Ret != LZMA_OK) {
    Output.clear();
    أرجع createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_منزل_String(Ret));
  }

  أرجع Error::success();
}

#وإلا

منطقي xz::is_موجود() { أرجع خطأ; }
Error xz::decompress(ArrayRef<uint8_t> داخلا,
                     Small_ويڪٽر_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#جيڪڏهن_پورو
