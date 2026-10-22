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

#অন্তর্ভাব "llvm/Support/Compression.h"
#অন্তর্ভাব "llvm/ADT/ScopeExit.h"
#অন্তর্ভাব "llvm/ADT/SmallVector.h"
#অন্তর্ভাব "llvm/ADT/StringRef.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/Compiler.h"
#অন্তর্ভাব "llvm/Support/Error.h"
#অন্তর্ভাব "llvm/Support/ErrorHandling.h"
#অন্তর্ভাব <limits>
#অন্তর্ভাব <optional>
#যদি LLVM_ENABLE_ZLIB
#অন্তর্ভাব <zlib.h>
#endif
#যদি LLVM_ENABLE_ZSTD
#অন্তর্ভাব <zstd.h>
#endif
#যদি LLVM_ENABLE_LZMA
#অন্তর্ভাব <lzma.h>
#endif

using নামাকাশ llvm;
using নামাকাশ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
স্থির যৌক্তিক isZlibHeader(ArrayRef<uint8_t> প্রবেশ) {
  যদি (প্রবেশ.আকার_3() < 2)
    ফেরত মিথ্যা;
  unsigned CMF = প্রবেশ[0];
  unsigned FLG = প্রবেশ[1];
  যদি ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ফেরত মিথ্যা;
  ফেরত (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
স্থির যৌক্তিক isZstdMagic(ArrayRef<uint8_t> প্রবেশ) {
  স্থির constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ফেরত প্রবেশ.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
স্থির std::optional<Format> identifyFormat(ArrayRef<uint8_t> প্রবেশ) {
  যদি (isZstdMagic(প্রবেশ))
    ফেরত Format::Zstd;
  যদি (isZlibHeader(প্রবেশ))
    ফেরত Format::Zlib;
  ফেরত std::nullopt;
}

ধ্রুবক অক্ষর *compression::getReasonIfUnsupported(compression::Format F) {
  নির্বাচন (F) {
  ক্ষেত্র compression::Format::Zlib:
    যদি (zlib::isAvailable())
      ফেরত nullptr;
    ফেরত "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  ক্ষেত্র compression::Format::Zstd:
    যদি (zstd::isAvailable())
      ফেরত nullptr;
    ফেরত "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ধ্রুবক অক্ষর *compression::getReasonIfUnsupported(ArrayRef<uint8_t> প্রবেশ) {
  যদি (std::optional<Format> F = identifyFormat(প্রবেশ))
    ফেরত getReasonIfUnsupported(*F);
  ফেরত "unknown compression format";
}

শূন্য compression::compress(Params P, ArrayRef<uint8_t> প্রবেশ,
                           SmallVectorImpl<uint8_t> &Output) {
  নির্বাচন (P.format) {
  ক্ষেত্র compression::Format::Zlib:
    zlib::compress(প্রবেশ, Output, P.level);
    বিরতি;
  ক্ষেত্র compression::Format::Zstd:
    zstd::compress(প্রবেশ, Output, P.level, P.zstdEnableLdm);
    বিরতি;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> প্রবেশ,
                              uint8_t *Output, size_t Uncompressed_আকার) {
  নির্বাচন (formatFor(T)) {
  ক্ষেত্র compression::Format::Zlib:
    ফেরত zlib::decompress(প্রবেশ, Output, Uncompressed_আকার);
  ক্ষেত্র compression::Format::Zstd:
    ফেরত zstd::decompress(প্রবেশ, Output, Uncompressed_আকার);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> প্রবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t Uncompressed_আকার) {
  নির্বাচন (F) {
  ক্ষেত্র compression::Format::Zlib:
    ফেরত zlib::decompress(প্রবেশ, Output, Uncompressed_আকার);
  ক্ষেত্র compression::Format::Zstd:
    ফেরত zstd::decompress(প্রবেশ, Output, Uncompressed_আকার);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> প্রবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t Uncompressed_আকার) {
  ফেরত decompress(formatFor(T), প্রবেশ, Output, Uncompressed_আকার);
}

Error compression::decompress(ArrayRef<uint8_t> প্রবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t Uncompressed_আকার) {
  std::optional<Format> F = identifyFormat(প্রবেশ);
  যদি (ধ্রুবক অক্ষর *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ফেরত createStringError(Reason);
  ফেরত decompress(*F, প্রবেশ, Output, Uncompressed_আকার);
}

#যদি LLVM_ENABLE_ZLIB

স্থির StringRef convertZlibCodeToString(পূর্ণসংখ্যা Code) {
  নির্বাচন (Code) {
  ক্ষেত্র Z_MEM_ERROR:
    ফেরত "zlib error: Z_MEM_ERROR";
  ক্ষেত্র Z_BUF_ERROR:
    ফেরত "zlib error: Z_BUF_ERROR";
  ক্ষেত্র Z_STREAM_ERROR:
    ফেরত "zlib error: Z_STREAM_ERROR";
  ক্ষেত্র Z_তথ্য_ERROR:
    ফেরত "zlib error: Z_DATA_ERROR";
  ক্ষেত্র Z_OK:
  পূর্বনির্ধারিত:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

যৌক্তিক zlib::isAvailable() { ফেরত সত্য; }

শূন্য zlib::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &Compressed_বাফার, পূর্ণসংখ্যা Level) {
  unsigned long Compressed_আকার = ::compressBound(প্রবেশ.আকার_3());
  Compressed_বাফার.resize_for_overwrite(Compressed_আকার);
  পূর্ণসংখ্যা Res = ::compress2((Bytef *)Compressed_বাফার.তথ্য(), &Compressed_আকার,
                        (ধ্রুবক Bytef *)প্রবেশ.তথ্য(), প্রবেশ.আকার_3(), Level);
  যদি (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  প্রতিজ্ঞা(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_বাফার.তথ্য(), Compressed_আকার);
  যদি (Compressed_আকার < Compressed_বাফার.আকার_3())
    Compressed_বাফার.truncate(Compressed_আকার);
}

Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Output,
                       size_t &Uncompressed_আকার) {
  পূর্ণসংখ্যা Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_আকার,
                         (ধ্রুবক Bytef *)প্রবেশ.তথ্য(), প্রবেশ.আকার_3());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_আকার);
  ফেরত Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t Uncompressed_আকার) {
  Output.resize_for_overwrite(Uncompressed_আকার);
  Error E = zlib::decompress(প্রবেশ, Output.তথ্য(), Uncompressed_আকার);
  যদি (Uncompressed_আকার < Output.আকার_3())
    Output.truncate(Uncompressed_আকার);
  ফেরত E;
}

#অন্যথা
যৌক্তিক zlib::isAvailable() { ফেরত মিথ্যা; }
শূন্য zlib::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &Compressed_বাফার, পূর্ণসংখ্যা Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Uncompressed_বাফার,
                       size_t &Uncompressed_আকার) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Uncompressed_বাফার,
                       size_t Uncompressed_আকার) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#যদি LLVM_ENABLE_ZSTD

যৌক্তিক zstd::isAvailable() { ফেরত সত্য; }

#অন্তর্ভাব <zstd.h> // Ensure ZSTD library is included

শূন্য zstd::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &Compressed_বাফার, পূর্ণসংখ্যা Level,
                    যৌক্তিক EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  যদি (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  যদি (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  যদি (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long Compressed_বাফার_আকার = ZSTD_compressBound(প্রবেশ.আকার_3());
  Compressed_বাফার.resize_for_overwrite(Compressed_বাফার_আকার);

  size_t ধ্রুবক Compressed_আকার =
      ZSTD_compress2(Cctx, Compressed_বাফার.তথ্য(), Compressed_বাফার_আকার,
                     প্রবেশ.তথ্য(), প্রবেশ.আকার_3());

  ZSTD_freeCCtx(Cctx);

  যদি (ZSTD_isError(Compressed_আকার))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_বাফার.তথ্য(), Compressed_আকার);
  যদি (Compressed_আকার < Compressed_বাফার.আকার_3())
    Compressed_বাফার.truncate(Compressed_আকার);
}

Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Output,
                       size_t &Uncompressed_আকার) {
  ধ্রুবক size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_আকার, (ধ্রুবক uint8_t *)প্রবেশ.তথ্য(), প্রবেশ.আকার_3());
  Uncompressed_আকার = Res;
  যদি (ZSTD_isError(Res))
    ফেরত make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_আকার);
  ফেরত Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t Uncompressed_আকার) {
  Output.resize_for_overwrite(Uncompressed_আকার);
  Error E = zstd::decompress(প্রবেশ, Output.তথ্য(), Uncompressed_আকার);
  যদি (Uncompressed_আকার < Output.আকার_3())
    Output.truncate(Uncompressed_আকার);
  ফেরত E;
}

#অন্যথা
যৌক্তিক zstd::isAvailable() { ফেরত মিথ্যা; }
শূন্য zstd::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &Compressed_বাফার, পূর্ণসংখ্যা Level,
                    যৌক্তিক EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Output,
                       size_t &Uncompressed_আকার) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t Uncompressed_আকার) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#যদি LLVM_ENABLE_LZMA

যৌক্তিক xz::isAvailable() { ফেরত সত্য; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
স্থির ধ্রুবক অক্ষর *convertLZMACodeToString(lzma_ret Code) {
  নির্বাচন (Code) {
  ক্ষেত্র LZMA_STREAM_END:
    ফেরত "lzma error: LZMA_STREAM_END";
  ক্ষেত্র LZMA_NO_CHECK:
    ফেরত "lzma error: LZMA_NO_CHECK";
  ক্ষেত্র LZMA_UNSUPPORTED_CHECK:
    ফেরত "lzma error: LZMA_UNSUPPORTED_CHECK";
  ক্ষেত্র LZMA_GET_CHECK:
    ফেরত "lzma error: LZMA_GET_CHECK";
  ক্ষেত্র LZMA_MEM_ERROR:
    ফেরত "lzma error: LZMA_MEM_ERROR";
  ক্ষেত্র LZMA_MEMLIMIT_ERROR:
    ফেরত "lzma error: LZMA_MEMLIMIT_ERROR";
  ক্ষেত্র LZMA_FORMAT_ERROR:
    ফেরত "lzma error: LZMA_FORMAT_ERROR";
  ক্ষেত্র LZMA_OPTIONS_ERROR:
    ফেরত "lzma error: LZMA_OPTIONS_ERROR";
  ক্ষেত্র LZMA_তথ্য_ERROR:
    ফেরত "lzma error: LZMA_DATA_ERROR";
  ক্ষেত্র LZMA_BUF_ERROR:
    ফেরত "lzma error: LZMA_BUF_ERROR";
  ক্ষেত্র LZMA_PROG_ERROR:
    ফেরত "lzma error: LZMA_PROG_ERROR";
  পূর্বনির্ধারিত:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
স্থির Expected<uint64_t> get_Uncompressed_আকার(ArrayRef<uint8_t> প্রবেশ) {
  যদি (প্রবেশ.আকার_3() < LZMA_STREAM_HEADER_আকার)
    ফেরত createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        প্রবেশ.আকার_3(), size_t(LZMA_STREAM_HEADER_আকার));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, প্রবেশ.take_back(LZMA_STREAM_HEADER_আকার).তথ্য());
  যদি (Ret != LZMA_OK)
    ফেরত createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_আকার = FooterFlags.backward_আকার + 2 * LZMA_STREAM_HEADER_আকার;
  যদি (প্রবেশ.আকার_3() < Min_আকার)
    ফেরত createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        প্রবেশ.আকার_3(), Min_আকার);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_সূচক *সূচক_2 = nullptr;
  llvm::scope_exit Free_সূচক([&] { lzma_সূচক_end(সূচক_2, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_সূচক_বাফার_decode(
      &সূচক_2, &MemLimit, nullptr,
      প্রবেশ.take_back(LZMA_STREAM_HEADER_আকার + FooterFlags.backward_আকার)
          .তথ্য(),
      &InPos, প্রবেশ.আকার_3());
  যদি (Ret != LZMA_OK)
    ফেরত createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  ফেরত lzma_সূচক_uncompressed_আকার(সূচক_2);
}

Error xz::decompress(ArrayRef<uint8_t> প্রবেশ,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_আকার = get_Uncompressed_আকার(প্রবেশ);
  যদি (!Uncompressed_আকার)
    ফেরত Uncompressed_আকার.takeError();

  যদি (*Uncompressed_আকার > std::numeric_limits<size_t>::max())
    ফেরত createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_আকার);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_আকার));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_বাফার_decode(
      &MemLimit, /*flags=*/0, nullptr, প্রবেশ.তথ্য(), &InPos, প্রবেশ.আকার_3(),
      Output.তথ্য(), &OutPos, Output.আকার_3());
  যদি (Ret != LZMA_OK) {
    Output.clear();
    ফেরত createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  ফেরত Error::success();
}

#অন্যথা

যৌক্তিক xz::isAvailable() { ফেরত মিথ্যা; }
Error xz::decompress(ArrayRef<uint8_t> প্রবেশ,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
