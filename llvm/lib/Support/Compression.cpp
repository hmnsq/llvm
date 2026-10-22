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

#شامل "llvm/Support/Compression.h"
#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/ADT/SmallVector.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل <limits>
#شامل <optional>
#اگر LLVM_ENABLE_ZLIB
#شامل <zlib.h>
#اگر_ختم
#اگر LLVM_ENABLE_ZSTD
#شامل <zstd.h>
#اگر_ختم
#اگر LLVM_ENABLE_LZMA
#شامل <lzma.h>
#اگر_ختم

استعمال نامخلا llvm;
استعمال نامخلا llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ساکن منطقی is_Zlib_سرخی(ArrayRef<uint8_t> داخلہ) {
  اگر (داخلہ.سائز_2() < 2)
    واپس جھوٹ;
  غیرعلامتی CMF = داخلہ[0];
  غیرعلامتی FLG = داخلہ[1];
  اگر ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    واپس جھوٹ;
  واپس (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ساکن منطقی isZstdMagic(ArrayRef<uint8_t> داخلہ) {
  ساکن constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  واپس داخلہ.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ساکن std::optional<Format> identifyFormat(ArrayRef<uint8_t> داخلہ) {
  اگر (isZstdMagic(داخلہ))
    واپس Format::Zstd;
  اگر (is_Zlib_سرخی(داخلہ))
    واپس Format::Zlib;
  واپس std::nullopt;
}

ثابت حرف *compression::getReasonIfUnsupported(compression::Format F) {
  انتخاب (F) {
  حالت compression::Format::Zlib:
    اگر (zlib::is_دستیاب())
      واپس خالی_اشاریہ;
    واپس "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  حالت compression::Format::Zstd:
    اگر (zstd::is_دستیاب())
      واپس خالی_اشاریہ;
    واپس "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ثابت حرف *compression::getReasonIfUnsupported(ArrayRef<uint8_t> داخلہ) {
  اگر (std::optional<Format> F = identifyFormat(داخلہ))
    واپس getReasonIfUnsupported(*F);
  واپس "unknown compression format";
}

خالی compression::compress(Params P, ArrayRef<uint8_t> داخلہ,
                           Small_سمتیہ_Impl<uint8_t> &Output) {
  انتخاب (P.format) {
  حالت compression::Format::Zlib:
    zlib::compress(داخلہ, Output, P.level);
    توڑو;
  حالت compression::Format::Zstd:
    zstd::compress(داخلہ, Output, P.level, P.zstdEnableLdm);
    توڑو;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> داخلہ,
                              uint8_t *Output, size_t Uncompressed_سائز) {
  انتخاب (formatFor(T)) {
  حالت compression::Format::Zlib:
    واپس zlib::decompress(داخلہ, Output, Uncompressed_سائز);
  حالت compression::Format::Zstd:
    واپس zstd::decompress(داخلہ, Output, Uncompressed_سائز);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> داخلہ,
                              Small_سمتیہ_Impl<uint8_t> &Output,
                              size_t Uncompressed_سائز) {
  انتخاب (F) {
  حالت compression::Format::Zlib:
    واپس zlib::decompress(داخلہ, Output, Uncompressed_سائز);
  حالت compression::Format::Zstd:
    واپس zstd::decompress(داخلہ, Output, Uncompressed_سائز);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> داخلہ,
                              Small_سمتیہ_Impl<uint8_t> &Output,
                              size_t Uncompressed_سائز) {
  واپس decompress(formatFor(T), داخلہ, Output, Uncompressed_سائز);
}

Error compression::decompress(ArrayRef<uint8_t> داخلہ,
                              Small_سمتیہ_Impl<uint8_t> &Output,
                              size_t Uncompressed_سائز) {
  std::optional<Format> F = identifyFormat(داخلہ);
  اگر (ثابت حرف *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    واپس createStringError(Reason);
  واپس decompress(*F, داخلہ, Output, Uncompressed_سائز);
}

#اگر LLVM_ENABLE_ZLIB

ساکن StringRef convert_Zlib_رمز_مقصد_String(صحیح رمز) {
  انتخاب (رمز) {
  حالت Z_MEM_ERROR:
    واپس "zlib error: Z_MEM_ERROR";
  حالت Z_BUF_ERROR:
    واپس "zlib error: Z_BUF_ERROR";
  حالت Z_دھارا_ERROR:
    واپس "zlib error: Z_STREAM_ERROR";
  حالت Z_ڈیٹا_ERROR:
    واپس "zlib error: Z_DATA_ERROR";
  حالت Z_OK:
  طےشدہ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

منطقی zlib::is_دستیاب() { واپس سچ; }

خالی zlib::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحیح Level) {
  غیرعلامتی طویل Compressed_سائز = ::compressBound(داخلہ.سائز_2());
  Compressed_بفر.resize_for_overwrite(Compressed_سائز);
  صحیح Res = ::compress2((Bytef *)Compressed_بفر.ڈیٹا(), &Compressed_سائز,
                        (ثابت Bytef *)داخلہ.ڈیٹا(), داخلہ.سائز_2(), Level);
  اگر (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  أكد(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_بفر.ڈیٹا(), Compressed_سائز);
  اگر (Compressed_سائز < Compressed_بفر.سائز_2())
    Compressed_بفر.truncate(Compressed_سائز);
}

Error zlib::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Output,
                       size_t &Uncompressed_سائز) {
  صحیح Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_سائز,
                         (ثابت Bytef *)داخلہ.ڈیٹا(), داخلہ.سائز_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_سائز);
  واپس Res ? make_error<StringError>(convert_Zlib_رمز_مقصد_String(Res),
                                       inconvertible_Error_رمز())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Output,
                       size_t Uncompressed_سائز) {
  Output.resize_for_overwrite(Uncompressed_سائز);
  Error E = zlib::decompress(داخلہ, Output.ڈیٹا(), Uncompressed_سائز);
  اگر (Uncompressed_سائز < Output.سائز_2())
    Output.truncate(Uncompressed_سائز);
  واپس E;
}

#ورنہ
منطقی zlib::is_دستیاب() { واپس جھوٹ; }
خالی zlib::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحیح Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Uncompressed_بفر,
                       size_t &Uncompressed_سائز) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Uncompressed_بفر,
                       size_t Uncompressed_سائز) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#اگر_ختم

#اگر LLVM_ENABLE_ZSTD

منطقی zstd::is_دستیاب() { واپس سچ; }

#شامل <zstd.h> // Ensure ZSTD library is included

خالی zstd::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحیح Level,
                    منطقی EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  اگر (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  اگر (ZSTD_isError(ZSTD_CCtx_سیٹ_Parameter(
          Cctx, ZSTD_c_enable_Long_فاصلہ_Matching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  اگر (ZSTD_isError(
          ZSTD_CCtx_سیٹ_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  غیرعلامتی طویل Compressed_بفر_سائز = ZSTD_compressBound(داخلہ.سائز_2());
  Compressed_بفر.resize_for_overwrite(Compressed_بفر_سائز);

  size_t ثابت Compressed_سائز =
      ZSTD_compress2(Cctx, Compressed_بفر.ڈیٹا(), Compressed_بفر_سائز,
                     داخلہ.ڈیٹا(), داخلہ.سائز_2());

  ZSTD_freeCCtx(Cctx);

  اگر (ZSTD_isError(Compressed_سائز))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_بفر.ڈیٹا(), Compressed_سائز);
  اگر (Compressed_سائز < Compressed_بفر.سائز_2())
    Compressed_بفر.truncate(Compressed_سائز);
}

Error zstd::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Output,
                       size_t &Uncompressed_سائز) {
  ثابت size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_سائز, (ثابت uint8_t *)داخلہ.ڈیٹا(), داخلہ.سائز_2());
  Uncompressed_سائز = Res;
  اگر (ZSTD_isError(Res))
    واپس make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_رمز());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_سائز);
  واپس Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Output,
                       size_t Uncompressed_سائز) {
  Output.resize_for_overwrite(Uncompressed_سائز);
  Error E = zstd::decompress(داخلہ, Output.ڈیٹا(), Uncompressed_سائز);
  اگر (Uncompressed_سائز < Output.سائز_2())
    Output.truncate(Uncompressed_سائز);
  واپس E;
}

#ورنہ
منطقی zstd::is_دستیاب() { واپس جھوٹ; }
خالی zstd::compress(ArrayRef<uint8_t> داخلہ,
                    Small_سمتیہ_Impl<uint8_t> &Compressed_بفر, صحیح Level,
                    منطقی EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> داخلہ, uint8_t *Output,
                       size_t &Uncompressed_سائز) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> داخلہ,
                       Small_سمتیہ_Impl<uint8_t> &Output,
                       size_t Uncompressed_سائز) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#اگر_ختم

#اگر LLVM_ENABLE_LZMA

منطقی xz::is_دستیاب() { واپس سچ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ساکن ثابت حرف *convert_LZMACode_مقصد_String(lzma_ret رمز) {
  انتخاب (رمز) {
  حالت LZMA_دھارا_انتہا:
    واپس "lzma error: LZMA_STREAM_END";
  حالت LZMA_NO_CHECK:
    واپس "lzma error: LZMA_NO_CHECK";
  حالت LZMA_UNSUPPORTED_CHECK:
    واپس "lzma error: LZMA_UNSUPPORTED_CHECK";
  حالت LZMA_GET_CHECK:
    واپس "lzma error: LZMA_GET_CHECK";
  حالت LZMA_MEM_ERROR:
    واپس "lzma error: LZMA_MEM_ERROR";
  حالت LZMA_MEMLIMIT_ERROR:
    واپس "lzma error: LZMA_MEMLIMIT_ERROR";
  حالت LZMA_FORMAT_ERROR:
    واپس "lzma error: LZMA_FORMAT_ERROR";
  حالت LZMA_OPTIONS_ERROR:
    واپس "lzma error: LZMA_OPTIONS_ERROR";
  حالت LZMA_ڈیٹا_ERROR:
    واپس "lzma error: LZMA_DATA_ERROR";
  حالت LZMA_BUF_ERROR:
    واپس "lzma error: LZMA_BUF_ERROR";
  حالت LZMA_PROG_ERROR:
    واپس "lzma error: LZMA_PROG_ERROR";
  طےشدہ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ساکن Expected<uint64_t> get_Uncompressed_سائز(ArrayRef<uint8_t> داخلہ) {
  اگر (داخلہ.سائز_2() < LZMA_دھارا_سرخی_سائز)
    واپس createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        داخلہ.سائز_2(), size_t(LZMA_دھارا_سرخی_سائز));

  // Decode the xz footer.
  lzma_دھارا_flags FooterFlags{};
  lzma_ret Ret = lzma_دھارا_footer_decode(
      &FooterFlags, داخلہ.take_عقب(LZMA_دھارا_سرخی_سائز).ڈیٹا());
  اگر (Ret != LZMA_OK)
    واپس createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_مقصد_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_سائز = FooterFlags.backward_سائز + 2 * LZMA_دھارا_سرخی_سائز;
  اگر (داخلہ.سائز_2() < Min_سائز)
    واپس createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        داخلہ.سائز_2(), Min_سائز);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_اشاریہ *اشاریہ_2 = خالی_اشاریہ;
  llvm::scope_exit Free_اشاریہ([&] { lzma_اشاریہ_انتہا(اشاریہ_2, خالی_اشاریہ); });
  uint64_t Mem_حد_بندی = UINT64_MAX;
  size_t اندر_Pos = 0;
  Ret = lzma_اشاریہ_بفر_decode(
      &اشاریہ_2, &Mem_حد_بندی, خالی_اشاریہ,
      داخلہ.take_عقب(LZMA_دھارا_سرخی_سائز + FooterFlags.backward_سائز)
          .ڈیٹا(),
      &اندر_Pos, داخلہ.سائز_2());
  اگر (Ret != LZMA_OK)
    واپس createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_مقصد_String(Ret));

  واپس lzma_اشاریہ_uncompressed_سائز(اشاریہ_2);
}

Error xz::decompress(ArrayRef<uint8_t> داخلہ,
                     Small_سمتیہ_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_سائز = get_Uncompressed_سائز(داخلہ);
  اگر (!Uncompressed_سائز)
    واپس Uncompressed_سائز.takeError();

  اگر (*Uncompressed_سائز > std::numeric_limits<size_t>::max())
    واپس createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_سائز);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_سائز));
  uint64_t Mem_حد_بندی = UINT64_MAX;
  size_t اندر_Pos = 0;
  size_t برآمد_Pos = 0;
  lzma_ret Ret = lzma_دھارا_بفر_decode(
      &Mem_حد_بندی, /*flags=*/0, خالی_اشاریہ, داخلہ.ڈیٹا(), &اندر_Pos, داخلہ.سائز_2(),
      Output.ڈیٹا(), &برآمد_Pos, Output.سائز_2());
  اگر (Ret != LZMA_OK) {
    Output.clear();
    واپس createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_مقصد_String(Ret));
  }

  واپس Error::success();
}

#ورنہ

منطقی xz::is_دستیاب() { واپس جھوٹ; }
Error xz::decompress(ArrayRef<uint8_t> داخلہ,
                     Small_سمتیہ_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#اگر_ختم
