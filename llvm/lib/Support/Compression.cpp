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
স্থির তার্কিক isZlibHeader(ArrayRef<uint8_t> প্রবেশ) {
  যদি (প্রবেশ.size() < 2)
    প্রত্যাগম অসত্য;
  unsigned CMF = প্রবেশ[0];
  unsigned FLG = প্রবেশ[1];
  যদি ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    প্রত্যাগম অসত্য;
  প্রত্যাগম (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
স্থির তার্কিক isZstdMagic(ArrayRef<uint8_t> প্রবেশ) {
  স্থির constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  প্রত্যাগম প্রবেশ.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
স্থির std::optional<Format> identifyFormat(ArrayRef<uint8_t> প্রবেশ) {
  যদি (isZstdMagic(প্রবেশ))
    প্রত্যাগম Format::Zstd;
  যদি (isZlibHeader(প্রবেশ))
    প্রত্যাগম Format::Zlib;
  প্রত্যাগম std::nullopt;
}

ধ্রুব বর্ণ *compression::getReasonIfUnsupported(compression::Format F) {
  বিকল্প (F) {
  পক্ষ compression::Format::Zlib:
    যদি (zlib::isAvailable())
      প্রত্যাগম nullptr;
    প্রত্যাগম "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  পক্ষ compression::Format::Zstd:
    যদি (zstd::isAvailable())
      প্রত্যাগম nullptr;
    প্রত্যাগম "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ধ্রুব বর্ণ *compression::getReasonIfUnsupported(ArrayRef<uint8_t> প্রবেশ) {
  যদি (std::optional<Format> F = identifyFormat(প্রবেশ))
    প্রত্যাগম getReasonIfUnsupported(*F);
  প্রত্যাগম "unknown compression format";
}

শূন্য compression::compress(Params P, ArrayRef<uint8_t> প্রবেশ,
                           SmallVectorImpl<uint8_t> &Output) {
  বিকল্প (P.format) {
  পক্ষ compression::Format::Zlib:
    zlib::compress(প্রবেশ, Output, P.level);
    ভঙ্গ;
  পক্ষ compression::Format::Zstd:
    zstd::compress(প্রবেশ, Output, P.level, P.zstdEnableLdm);
    ভঙ্গ;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> প্রবেশ,
                              uint8_t *Output, size_t UncompressedSize) {
  বিকল্প (formatFor(T)) {
  পক্ষ compression::Format::Zlib:
    প্রত্যাগম zlib::decompress(প্রবেশ, Output, UncompressedSize);
  পক্ষ compression::Format::Zstd:
    প্রত্যাগম zstd::decompress(প্রবেশ, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> প্রবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  বিকল্প (F) {
  পক্ষ compression::Format::Zlib:
    প্রত্যাগম zlib::decompress(প্রবেশ, Output, UncompressedSize);
  পক্ষ compression::Format::Zstd:
    প্রত্যাগম zstd::decompress(প্রবেশ, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> প্রবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  প্রত্যাগম decompress(formatFor(T), প্রবেশ, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> প্রবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(প্রবেশ);
  যদি (ধ্রুব বর্ণ *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    প্রত্যাগম createStringError(Reason);
  প্রত্যাগম decompress(*F, প্রবেশ, Output, UncompressedSize);
}

#যদি LLVM_ENABLE_ZLIB

স্থির StringRef convertZlibCodeToString(পূর্ণাঙ্ক Code) {
  বিকল্প (Code) {
  পক্ষ Z_MEM_ERROR:
    প্রত্যাগম "zlib error: Z_MEM_ERROR";
  পক্ষ Z_BUF_ERROR:
    প্রত্যাগম "zlib error: Z_BUF_ERROR";
  পক্ষ Z_STREAM_ERROR:
    প্রত্যাগম "zlib error: Z_STREAM_ERROR";
  পক্ষ Z_DATA_ERROR:
    প্রত্যাগম "zlib error: Z_DATA_ERROR";
  পক্ষ Z_OK:
  স্বভাব:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

তার্কিক zlib::isAvailable() { প্রত্যাগম সত্য; }

শূন্য zlib::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level) {
  unsigned long CompressedSize = ::compressBound(প্রবেশ.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  পূর্ণাঙ্ক Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ধ্রুব Bytef *)প্রবেশ.data(), প্রবেশ.size(), Level);
  যদি (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  প্রতিজ্ঞা(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  যদি (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Output,
                       size_t &UncompressedSize) {
  পূর্ণাঙ্ক Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ধ্রুব Bytef *)প্রবেশ.data(), প্রবেশ.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  প্রত্যাগম Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(প্রবেশ, Output.data(), UncompressedSize);
  যদি (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  প্রত্যাগম E;
}

#অন্যথা
তার্কিক zlib::isAvailable() { প্রত্যাগম অসত্য; }
শূন্য zlib::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#যদি LLVM_ENABLE_ZSTD

তার্কিক zstd::isAvailable() { প্রত্যাগম সত্য; }

#অন্তর্ভাব <zstd.h> // Ensure ZSTD library is included

শূন্য zstd::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level,
                    তার্কিক EnableLdm) {
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

  unsigned long CompressedBufferSize = ZSTD_compressBound(প্রবেশ.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ধ্রুব CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     প্রবেশ.data(), প্রবেশ.size());

  ZSTD_freeCCtx(Cctx);

  যদি (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  যদি (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Output,
                       size_t &UncompressedSize) {
  ধ্রুব size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ধ্রুব uint8_t *)প্রবেশ.data(), প্রবেশ.size());
  UncompressedSize = Res;
  যদি (ZSTD_isError(Res))
    প্রত্যাগম make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  প্রত্যাগম Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(প্রবেশ, Output.data(), UncompressedSize);
  যদি (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  প্রত্যাগম E;
}

#অন্যথা
তার্কিক zstd::isAvailable() { প্রত্যাগম অসত্য; }
শূন্য zstd::compress(ArrayRef<uint8_t> প্রবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level,
                    তার্কিক EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> প্রবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#যদি LLVM_ENABLE_LZMA

তার্কিক xz::isAvailable() { প্রত্যাগম সত্য; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
স্থির ধ্রুব বর্ণ *convertLZMACodeToString(lzma_ret Code) {
  বিকল্প (Code) {
  পক্ষ LZMA_STREAM_END:
    প্রত্যাগম "lzma error: LZMA_STREAM_END";
  পক্ষ LZMA_NO_CHECK:
    প্রত্যাগম "lzma error: LZMA_NO_CHECK";
  পক্ষ LZMA_UNSUPPORTED_CHECK:
    প্রত্যাগম "lzma error: LZMA_UNSUPPORTED_CHECK";
  পক্ষ LZMA_GET_CHECK:
    প্রত্যাগম "lzma error: LZMA_GET_CHECK";
  পক্ষ LZMA_MEM_ERROR:
    প্রত্যাগম "lzma error: LZMA_MEM_ERROR";
  পক্ষ LZMA_MEMLIMIT_ERROR:
    প্রত্যাগম "lzma error: LZMA_MEMLIMIT_ERROR";
  পক্ষ LZMA_FORMAT_ERROR:
    প্রত্যাগম "lzma error: LZMA_FORMAT_ERROR";
  পক্ষ LZMA_OPTIONS_ERROR:
    প্রত্যাগম "lzma error: LZMA_OPTIONS_ERROR";
  পক্ষ LZMA_DATA_ERROR:
    প্রত্যাগম "lzma error: LZMA_DATA_ERROR";
  পক্ষ LZMA_BUF_ERROR:
    প্রত্যাগম "lzma error: LZMA_BUF_ERROR";
  পক্ষ LZMA_PROG_ERROR:
    প্রত্যাগম "lzma error: LZMA_PROG_ERROR";
  স্বভাব:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
স্থির Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> প্রবেশ) {
  যদি (প্রবেশ.size() < LZMA_STREAM_HEADER_SIZE)
    প্রত্যাগম createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        প্রবেশ.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, প্রবেশ.take_back(LZMA_STREAM_HEADER_SIZE).data());
  যদি (Ret != LZMA_OK)
    প্রত্যাগম createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  যদি (প্রবেশ.size() < MinSize)
    প্রত্যাগম createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        প্রবেশ.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      প্রবেশ.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, প্রবেশ.size());
  যদি (Ret != LZMA_OK)
    প্রত্যাগম createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  প্রত্যাগম lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> প্রবেশ,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(প্রবেশ);
  যদি (!UncompressedSize)
    প্রত্যাগম UncompressedSize.takeError();

  যদি (*UncompressedSize > std::numeric_limits<size_t>::max())
    প্রত্যাগম createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, প্রবেশ.data(), &InPos, প্রবেশ.size(),
      Output.data(), &OutPos, Output.size());
  যদি (Ret != LZMA_OK) {
    Output.clear();
    প্রত্যাগম createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  প্রত্যাগম Error::success();
}

#অন্যথা

তার্কিক xz::isAvailable() { প্রত্যাগম অসত্য; }
Error xz::decompress(ArrayRef<uint8_t> প্রবেশ,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
