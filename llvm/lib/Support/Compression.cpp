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
#endif
#اگر LLVM_ENABLE_ZSTD
#شامل <zstd.h>
#endif
#اگر LLVM_ENABLE_LZMA
#شامل <lzma.h>
#endif

using فضاینام llvm;
using فضاینام llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ایستا منطقی isZlibHeader(ArrayRef<uint8_t> درون_داد) {
  اگر (درون_داد.size() < 2)
    بازگشت نادرست;
  unsigned CMF = درون_داد[0];
  unsigned FLG = درون_داد[1];
  اگر ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    بازگشت نادرست;
  بازگشت (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ایستا منطقی isZstdMagic(ArrayRef<uint8_t> درون_داد) {
  ایستا constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  بازگشت درون_داد.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ایستا std::optional<Format> identifyFormat(ArrayRef<uint8_t> درون_داد) {
  اگر (isZstdMagic(درون_داد))
    بازگشت Format::Zstd;
  اگر (isZlibHeader(درون_داد))
    بازگشت Format::Zlib;
  بازگشت std::nullopt;
}

ثابت نویسه *compression::getReasonIfUnsupported(compression::Format F) {
  انتخاب (F) {
  حالت compression::Format::Zlib:
    اگر (zlib::isAvailable())
      بازگشت nullptr;
    بازگشت "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  حالت compression::Format::Zstd:
    اگر (zstd::isAvailable())
      بازگشت nullptr;
    بازگشت "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ثابت نویسه *compression::getReasonIfUnsupported(ArrayRef<uint8_t> درون_داد) {
  اگر (std::optional<Format> F = identifyFormat(درون_داد))
    بازگشت getReasonIfUnsupported(*F);
  بازگشت "unknown compression format";
}

تهی compression::compress(Params P, ArrayRef<uint8_t> درون_داد,
                           SmallVectorImpl<uint8_t> &Output) {
  انتخاب (P.format) {
  حالت compression::Format::Zlib:
    zlib::compress(درون_داد, Output, P.level);
    بشکن;
  حالت compression::Format::Zstd:
    zstd::compress(درون_داد, Output, P.level, P.zstdEnableLdm);
    بشکن;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> درون_داد,
                              uint8_t *Output, size_t UncompressedSize) {
  انتخاب (formatFor(T)) {
  حالت compression::Format::Zlib:
    بازگشت zlib::decompress(درون_داد, Output, UncompressedSize);
  حالت compression::Format::Zstd:
    بازگشت zstd::decompress(درون_داد, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> درون_داد,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  انتخاب (F) {
  حالت compression::Format::Zlib:
    بازگشت zlib::decompress(درون_داد, Output, UncompressedSize);
  حالت compression::Format::Zstd:
    بازگشت zstd::decompress(درون_داد, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> درون_داد,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  بازگشت decompress(formatFor(T), درون_داد, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> درون_داد,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(درون_داد);
  اگر (ثابت نویسه *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    بازگشت createStringError(Reason);
  بازگشت decompress(*F, درون_داد, Output, UncompressedSize);
}

#اگر LLVM_ENABLE_ZLIB

ایستا StringRef convertZlibCodeToString(صحیح Code) {
  انتخاب (Code) {
  حالت Z_MEM_ERROR:
    بازگشت "zlib error: Z_MEM_ERROR";
  حالت Z_BUF_ERROR:
    بازگشت "zlib error: Z_BUF_ERROR";
  حالت Z_STREAM_ERROR:
    بازگشت "zlib error: Z_STREAM_ERROR";
  حالت Z_DATA_ERROR:
    بازگشت "zlib error: Z_DATA_ERROR";
  حالت Z_OK:
  پیشفرض:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

منطقی zlib::isAvailable() { بازگشت درست; }

تهی zlib::compress(ArrayRef<uint8_t> درون_داد,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحیح Level) {
  unsigned long CompressedSize = ::compressBound(درون_داد.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  صحیح Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ثابت Bytef *)درون_داد.data(), درون_داد.size(), Level);
  اگر (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  أكد(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  اگر (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> درون_داد, uint8_t *Output,
                       size_t &UncompressedSize) {
  صحیح Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ثابت Bytef *)درون_داد.data(), درون_داد.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  بازگشت Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> درون_داد,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(درون_داد, Output.data(), UncompressedSize);
  اگر (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  بازگشت E;
}

#وگرنه
منطقی zlib::isAvailable() { بازگشت نادرست; }
تهی zlib::compress(ArrayRef<uint8_t> درون_داد,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحیح Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> درون_داد, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> درون_داد,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#اگر LLVM_ENABLE_ZSTD

منطقی zstd::isAvailable() { بازگشت درست; }

#شامل <zstd.h> // Ensure ZSTD library is included

تهی zstd::compress(ArrayRef<uint8_t> درون_داد,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحیح Level,
                    منطقی EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  اگر (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  اگر (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  اگر (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(درون_داد.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ثابت CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     درون_داد.data(), درون_داد.size());

  ZSTD_freeCCtx(Cctx);

  اگر (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  اگر (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> درون_داد, uint8_t *Output,
                       size_t &UncompressedSize) {
  ثابت size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ثابت uint8_t *)درون_داد.data(), درون_داد.size());
  UncompressedSize = Res;
  اگر (ZSTD_isError(Res))
    بازگشت make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  بازگشت Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> درون_داد,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(درون_داد, Output.data(), UncompressedSize);
  اگر (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  بازگشت E;
}

#وگرنه
منطقی zstd::isAvailable() { بازگشت نادرست; }
تهی zstd::compress(ArrayRef<uint8_t> درون_داد,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحیح Level,
                    منطقی EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> درون_داد, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> درون_داد,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#اگر LLVM_ENABLE_LZMA

منطقی xz::isAvailable() { بازگشت درست; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ایستا ثابت نویسه *convertLZMACodeToString(lzma_ret Code) {
  انتخاب (Code) {
  حالت LZMA_STREAM_END:
    بازگشت "lzma error: LZMA_STREAM_END";
  حالت LZMA_NO_CHECK:
    بازگشت "lzma error: LZMA_NO_CHECK";
  حالت LZMA_UNSUPPORTED_CHECK:
    بازگشت "lzma error: LZMA_UNSUPPORTED_CHECK";
  حالت LZMA_GET_CHECK:
    بازگشت "lzma error: LZMA_GET_CHECK";
  حالت LZMA_MEM_ERROR:
    بازگشت "lzma error: LZMA_MEM_ERROR";
  حالت LZMA_MEMLIMIT_ERROR:
    بازگشت "lzma error: LZMA_MEMLIMIT_ERROR";
  حالت LZMA_FORMAT_ERROR:
    بازگشت "lzma error: LZMA_FORMAT_ERROR";
  حالت LZMA_OPTIONS_ERROR:
    بازگشت "lzma error: LZMA_OPTIONS_ERROR";
  حالت LZMA_DATA_ERROR:
    بازگشت "lzma error: LZMA_DATA_ERROR";
  حالت LZMA_BUF_ERROR:
    بازگشت "lzma error: LZMA_BUF_ERROR";
  حالت LZMA_PROG_ERROR:
    بازگشت "lzma error: LZMA_PROG_ERROR";
  پیشفرض:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ایستا Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> درون_داد) {
  اگر (درون_داد.size() < LZMA_STREAM_HEADER_SIZE)
    بازگشت createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        درون_داد.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, درون_داد.take_back(LZMA_STREAM_HEADER_SIZE).data());
  اگر (Ret != LZMA_OK)
    بازگشت createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  اگر (درون_داد.size() < MinSize)
    بازگشت createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        درون_داد.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      درون_داد.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, درون_داد.size());
  اگر (Ret != LZMA_OK)
    بازگشت createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  بازگشت lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> درون_داد,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(درون_داد);
  اگر (!UncompressedSize)
    بازگشت UncompressedSize.takeError();

  اگر (*UncompressedSize > std::numeric_limits<size_t>::max())
    بازگشت createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, درون_داد.data(), &InPos, درون_داد.size(),
      Output.data(), &OutPos, Output.size());
  اگر (Ret != LZMA_OK) {
    Output.clear();
    بازگشت createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  بازگشت Error::success();
}

#وگرنه

منطقی xz::isAvailable() { بازگشت نادرست; }
Error xz::decompress(ArrayRef<uint8_t> درون_داد,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
