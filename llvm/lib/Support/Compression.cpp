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

#অন্তর্ভাৱ "llvm/Support/Compression.h"
#অন্তর্ভাৱ "llvm/ADT/ScopeExit.h"
#অন্তর্ভাৱ "llvm/ADT/SmallVector.h"
#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Support/Compiler.h"
#অন্তর্ভাৱ "llvm/Support/Error.h"
#অন্তর্ভাৱ "llvm/Support/ErrorHandling.h"
#অন্তর্ভাৱ <limits>
#অন্তর্ভাৱ <optional>
#যদি LLVM_ENABLE_ZLIB
#অন্তর্ভাৱ <zlib.h>
#endif
#যদি LLVM_ENABLE_ZSTD
#অন্তর্ভাৱ <zstd.h>
#endif
#যদি LLVM_ENABLE_LZMA
#অন্তর্ভাৱ <lzma.h>
#endif

using নামাকাশ llvm;
using নামাকাশ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
স্থির তার্কিক isZlibHeader(ArrayRef<uint8_t> প্ৰবেশ) {
  যদি (প্ৰবেশ.size() < 2)
    ঘূৰাই মিছা;
  unsigned CMF = প্ৰবেশ[0];
  unsigned FLG = প্ৰবেশ[1];
  যদি ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ঘূৰাই মিছা;
  ঘূৰাই (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
স্থির তার্কিক isZstdMagic(ArrayRef<uint8_t> প্ৰবেশ) {
  স্থির constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ঘূৰাই প্ৰবেশ.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
স্থির std::optional<Format> identifyFormat(ArrayRef<uint8_t> প্ৰবেশ) {
  যদি (isZstdMagic(প্ৰবেশ))
    ঘূৰাই Format::Zstd;
  যদি (isZlibHeader(প্ৰবেশ))
    ঘূৰাই Format::Zlib;
  ঘূৰাই std::nullopt;
}

ধ্রুৱ ৱর্ণ *compression::getReasonIfUnsupported(compression::Format F) {
  ৱিকল্প (F) {
  পক্ষ compression::Format::Zlib:
    যদি (zlib::isAvailable())
      ঘূৰাই nullptr;
    ঘূৰাই "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  পক্ষ compression::Format::Zstd:
    যদি (zstd::isAvailable())
      ঘূৰাই nullptr;
    ঘূৰাই "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ধ্রুৱ ৱর্ণ *compression::getReasonIfUnsupported(ArrayRef<uint8_t> প্ৰবেশ) {
  যদি (std::optional<Format> F = identifyFormat(প্ৰবেশ))
    ঘূৰাই getReasonIfUnsupported(*F);
  ঘূৰাই "unknown compression format";
}

শূন্য compression::compress(Params P, ArrayRef<uint8_t> প্ৰবেশ,
                           SmallVectorImpl<uint8_t> &Output) {
  ৱিকল্প (P.format) {
  পক্ষ compression::Format::Zlib:
    zlib::compress(প্ৰবেশ, Output, P.level);
    ভঙ্গ;
  পক্ষ compression::Format::Zstd:
    zstd::compress(প্ৰবেশ, Output, P.level, P.zstdEnableLdm);
    ভঙ্গ;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> প্ৰবেশ,
                              uint8_t *Output, size_t UncompressedSize) {
  ৱিকল্প (formatFor(T)) {
  পক্ষ compression::Format::Zlib:
    ঘূৰাই zlib::decompress(প্ৰবেশ, Output, UncompressedSize);
  পক্ষ compression::Format::Zstd:
    ঘূৰাই zstd::decompress(প্ৰবেশ, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> প্ৰবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ৱিকল্প (F) {
  পক্ষ compression::Format::Zlib:
    ঘূৰাই zlib::decompress(প্ৰবেশ, Output, UncompressedSize);
  পক্ষ compression::Format::Zstd:
    ঘূৰাই zstd::decompress(প্ৰবেশ, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> প্ৰবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ঘূৰাই decompress(formatFor(T), প্ৰবেশ, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(প্ৰবেশ);
  যদি (ধ্রুৱ ৱর্ণ *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ঘূৰাই createStringError(Reason);
  ঘূৰাই decompress(*F, প্ৰবেশ, Output, UncompressedSize);
}

#যদি LLVM_ENABLE_ZLIB

স্থির StringRef convertZlibCodeToString(পূর্ণাঙ্ক Code) {
  ৱিকল্প (Code) {
  পক্ষ Z_MEM_ERROR:
    ঘূৰাই "zlib error: Z_MEM_ERROR";
  পক্ষ Z_BUF_ERROR:
    ঘূৰাই "zlib error: Z_BUF_ERROR";
  পক্ষ Z_STREAM_ERROR:
    ঘূৰাই "zlib error: Z_STREAM_ERROR";
  পক্ষ Z_DATA_ERROR:
    ঘূৰাই "zlib error: Z_DATA_ERROR";
  পক্ষ Z_OK:
  স্ৱভাৱ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

তার্কিক zlib::isAvailable() { ঘূৰাই সঁচা; }

শূন্য zlib::compress(ArrayRef<uint8_t> প্ৰবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level) {
  unsigned long CompressedSize = ::compressBound(প্ৰবেশ.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  পূর্ণাঙ্ক Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ধ্রুৱ Bytef *)প্ৰবেশ.data(), প্ৰবেশ.size(), Level);
  যদি (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  প্রতিজ্ঞা(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  যদি (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> প্ৰবেশ, uint8_t *Output,
                       size_t &UncompressedSize) {
  পূর্ণাঙ্ক Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ধ্রুৱ Bytef *)প্ৰবেশ.data(), প্ৰবেশ.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ঘূৰাই Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(প্ৰবেশ, Output.data(), UncompressedSize);
  যদি (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ঘূৰাই E;
}

#অন্যথা
তার্কিক zlib::isAvailable() { ঘূৰাই মিছা; }
শূন্য zlib::compress(ArrayRef<uint8_t> প্ৰবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> প্ৰবেশ, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#যদি LLVM_ENABLE_ZSTD

তার্কিক zstd::isAvailable() { ঘূৰাই সঁচা; }

#অন্তর্ভাৱ <zstd.h> // Ensure ZSTD library is included

শূন্য zstd::compress(ArrayRef<uint8_t> প্ৰবেশ,
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

  unsigned long CompressedBufferSize = ZSTD_compressBound(প্ৰবেশ.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ধ্রুৱ CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     প্ৰবেশ.data(), প্ৰবেশ.size());

  ZSTD_freeCCtx(Cctx);

  যদি (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  যদি (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> প্ৰবেশ, uint8_t *Output,
                       size_t &UncompressedSize) {
  ধ্রুৱ size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ধ্রুৱ uint8_t *)প্ৰবেশ.data(), প্ৰবেশ.size());
  UncompressedSize = Res;
  যদি (ZSTD_isError(Res))
    ঘূৰাই make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ঘূৰাই Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(প্ৰবেশ, Output.data(), UncompressedSize);
  যদি (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ঘূৰাই E;
}

#অন্যথা
তার্কিক zstd::isAvailable() { ঘূৰাই মিছা; }
শূন্য zstd::compress(ArrayRef<uint8_t> প্ৰবেশ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, পূর্ণাঙ্ক Level,
                    তার্কিক EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> প্ৰবেশ, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#যদি LLVM_ENABLE_LZMA

তার্কিক xz::isAvailable() { ঘূৰাই সঁচা; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
স্থির ধ্রুৱ ৱর্ণ *convertLZMACodeToString(lzma_ret Code) {
  ৱিকল্প (Code) {
  পক্ষ LZMA_STREAM_END:
    ঘূৰাই "lzma error: LZMA_STREAM_END";
  পক্ষ LZMA_NO_CHECK:
    ঘূৰাই "lzma error: LZMA_NO_CHECK";
  পক্ষ LZMA_UNSUPPORTED_CHECK:
    ঘূৰাই "lzma error: LZMA_UNSUPPORTED_CHECK";
  পক্ষ LZMA_GET_CHECK:
    ঘূৰাই "lzma error: LZMA_GET_CHECK";
  পক্ষ LZMA_MEM_ERROR:
    ঘূৰাই "lzma error: LZMA_MEM_ERROR";
  পক্ষ LZMA_MEMLIMIT_ERROR:
    ঘূৰাই "lzma error: LZMA_MEMLIMIT_ERROR";
  পক্ষ LZMA_FORMAT_ERROR:
    ঘূৰাই "lzma error: LZMA_FORMAT_ERROR";
  পক্ষ LZMA_OPTIONS_ERROR:
    ঘূৰাই "lzma error: LZMA_OPTIONS_ERROR";
  পক্ষ LZMA_DATA_ERROR:
    ঘূৰাই "lzma error: LZMA_DATA_ERROR";
  পক্ষ LZMA_BUF_ERROR:
    ঘূৰাই "lzma error: LZMA_BUF_ERROR";
  পক্ষ LZMA_PROG_ERROR:
    ঘূৰাই "lzma error: LZMA_PROG_ERROR";
  স্ৱভাৱ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
স্থির Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> প্ৰবেশ) {
  যদি (প্ৰবেশ.size() < LZMA_STREAM_HEADER_SIZE)
    ঘূৰাই createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        প্ৰবেশ.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, প্ৰবেশ.take_back(LZMA_STREAM_HEADER_SIZE).data());
  যদি (Ret != LZMA_OK)
    ঘূৰাই createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  যদি (প্ৰবেশ.size() < MinSize)
    ঘূৰাই createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        প্ৰবেশ.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      প্ৰবেশ.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, প্ৰবেশ.size());
  যদি (Ret != LZMA_OK)
    ঘূৰাই createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  ঘূৰাই lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(প্ৰবেশ);
  যদি (!UncompressedSize)
    ঘূৰাই UncompressedSize.takeError();

  যদি (*UncompressedSize > std::numeric_limits<size_t>::max())
    ঘূৰাই createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, প্ৰবেশ.data(), &InPos, প্ৰবেশ.size(),
      Output.data(), &OutPos, Output.size());
  যদি (Ret != LZMA_OK) {
    Output.clear();
    ঘূৰাই createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  ঘূৰাই Error::success();
}

#অন্যথা

তার্কিক xz::isAvailable() { ঘূৰাই মিছা; }
Error xz::decompress(ArrayRef<uint8_t> প্ৰবেশ,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
