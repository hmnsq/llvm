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

#அந்தர்பாவ "llvm/Support/Compression.h"
#அந்தர்பாவ "llvm/ADT/ScopeExit.h"
#அந்தர்பாவ "llvm/ADT/SmallVector.h"
#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/Compiler.h"
#அந்தர்பாவ "llvm/Support/Error.h"
#அந்தர்பாவ "llvm/Support/ErrorHandling.h"
#அந்தர்பாவ <limits>
#அந்தர்பாவ <optional>
#எனில் LLVM_ENABLE_ZLIB
#அந்தர்பாவ <zlib.h>
#endif
#எனில் LLVM_ENABLE_ZSTD
#அந்தர்பாவ <zstd.h>
#endif
#எனில் LLVM_ENABLE_LZMA
#அந்தர்பாவ <lzma.h>
#endif

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
நிலையான தருக்க isZlibHeader(ArrayRef<uint8_t> உள்ளீடு) {
  எனில் (உள்ளீடு.size() < 2)
    திரும்பு பொய்;
  unsigned CMF = உள்ளீடு[0];
  unsigned FLG = உள்ளீடு[1];
  எனில் ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    திரும்பு பொய்;
  திரும்பு (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
நிலையான தருக்க isZstdMagic(ArrayRef<uint8_t> உள்ளீடு) {
  நிலையான constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  திரும்பு உள்ளீடு.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
நிலையான std::optional<Format> identifyFormat(ArrayRef<uint8_t> உள்ளீடு) {
  எனில் (isZstdMagic(உள்ளீடு))
    திரும்பு Format::Zstd;
  எனில் (isZlibHeader(உள்ளீடு))
    திரும்பு Format::Zlib;
  திரும்பு std::nullopt;
}

மாறிலி எழுத்து *compression::getReasonIfUnsupported(compression::Format F) {
  தேர்வு (F) {
  நிலை compression::Format::Zlib:
    எனில் (zlib::isAvailable())
      திரும்பு nullptr;
    திரும்பு "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  நிலை compression::Format::Zstd:
    எனில் (zstd::isAvailable())
      திரும்பு nullptr;
    திரும்பு "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

மாறிலி எழுத்து *compression::getReasonIfUnsupported(ArrayRef<uint8_t> உள்ளீடு) {
  எனில் (std::optional<Format> F = identifyFormat(உள்ளீடு))
    திரும்பு getReasonIfUnsupported(*F);
  திரும்பு "unknown compression format";
}

வெறுமை compression::compress(Params P, ArrayRef<uint8_t> உள்ளீடு,
                           SmallVectorImpl<uint8_t> &Output) {
  தேர்வு (P.format) {
  நிலை compression::Format::Zlib:
    zlib::compress(உள்ளீடு, Output, P.level);
    நிறுத்து;
  நிலை compression::Format::Zstd:
    zstd::compress(உள்ளீடு, Output, P.level, P.zstdEnableLdm);
    நிறுத்து;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> உள்ளீடு,
                              uint8_t *Output, size_t UncompressedSize) {
  தேர்வு (formatFor(T)) {
  நிலை compression::Format::Zlib:
    திரும்பு zlib::decompress(உள்ளீடு, Output, UncompressedSize);
  நிலை compression::Format::Zstd:
    திரும்பு zstd::decompress(உள்ளீடு, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> உள்ளீடு,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  தேர்வு (F) {
  நிலை compression::Format::Zlib:
    திரும்பு zlib::decompress(உள்ளீடு, Output, UncompressedSize);
  நிலை compression::Format::Zstd:
    திரும்பு zstd::decompress(உள்ளீடு, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> உள்ளீடு,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  திரும்பு decompress(formatFor(T), உள்ளீடு, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> உள்ளீடு,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(உள்ளீடு);
  எனில் (மாறிலி எழுத்து *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    திரும்பு createStringError(Reason);
  திரும்பு decompress(*F, உள்ளீடு, Output, UncompressedSize);
}

#எனில் LLVM_ENABLE_ZLIB

நிலையான StringRef convertZlibCodeToString(முழு Code) {
  தேர்வு (Code) {
  நிலை Z_MEM_ERROR:
    திரும்பு "zlib error: Z_MEM_ERROR";
  நிலை Z_BUF_ERROR:
    திரும்பு "zlib error: Z_BUF_ERROR";
  நிலை Z_STREAM_ERROR:
    திரும்பு "zlib error: Z_STREAM_ERROR";
  நிலை Z_DATA_ERROR:
    திரும்பு "zlib error: Z_DATA_ERROR";
  நிலை Z_OK:
  இயல்பு:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

தருக்க zlib::isAvailable() { திரும்பு உண்மை; }

வெறுமை zlib::compress(ArrayRef<uint8_t> உள்ளீடு,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, முழு Level) {
  unsigned long CompressedSize = ::compressBound(உள்ளீடு.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  முழு Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (மாறிலி Bytef *)உள்ளீடு.data(), உள்ளீடு.size(), Level);
  எனில் (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  ப்ரதிஜ்ஞா(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  எனில் (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> உள்ளீடு, uint8_t *Output,
                       size_t &UncompressedSize) {
  முழு Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (மாறிலி Bytef *)உள்ளீடு.data(), உள்ளீடு.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  திரும்பு Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> உள்ளீடு,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(உள்ளீடு, Output.data(), UncompressedSize);
  எனில் (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  திரும்பு E;
}

#இல்லையெனில்
தருக்க zlib::isAvailable() { திரும்பு பொய்; }
வெறுமை zlib::compress(ArrayRef<uint8_t> உள்ளீடு,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, முழு Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> உள்ளீடு, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> உள்ளீடு,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#எனில் LLVM_ENABLE_ZSTD

தருக்க zstd::isAvailable() { திரும்பு உண்மை; }

#அந்தர்பாவ <zstd.h> // Ensure ZSTD library is included

வெறுமை zstd::compress(ArrayRef<uint8_t> உள்ளீடு,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, முழு Level,
                    தருக்க EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  எனில் (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  எனில் (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  எனில் (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(உள்ளீடு.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t மாறிலி CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     உள்ளீடு.data(), உள்ளீடு.size());

  ZSTD_freeCCtx(Cctx);

  எனில் (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  எனில் (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> உள்ளீடு, uint8_t *Output,
                       size_t &UncompressedSize) {
  மாறிலி size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (மாறிலி uint8_t *)உள்ளீடு.data(), உள்ளீடு.size());
  UncompressedSize = Res;
  எனில் (ZSTD_isError(Res))
    திரும்பு make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  திரும்பு Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> உள்ளீடு,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(உள்ளீடு, Output.data(), UncompressedSize);
  எனில் (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  திரும்பு E;
}

#இல்லையெனில்
தருக்க zstd::isAvailable() { திரும்பு பொய்; }
வெறுமை zstd::compress(ArrayRef<uint8_t> உள்ளீடு,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, முழு Level,
                    தருக்க EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> உள்ளீடு, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> உள்ளீடு,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#எனில் LLVM_ENABLE_LZMA

தருக்க xz::isAvailable() { திரும்பு உண்மை; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
நிலையான மாறிலி எழுத்து *convertLZMACodeToString(lzma_ret Code) {
  தேர்வு (Code) {
  நிலை LZMA_STREAM_END:
    திரும்பு "lzma error: LZMA_STREAM_END";
  நிலை LZMA_NO_CHECK:
    திரும்பு "lzma error: LZMA_NO_CHECK";
  நிலை LZMA_UNSUPPORTED_CHECK:
    திரும்பு "lzma error: LZMA_UNSUPPORTED_CHECK";
  நிலை LZMA_GET_CHECK:
    திரும்பு "lzma error: LZMA_GET_CHECK";
  நிலை LZMA_MEM_ERROR:
    திரும்பு "lzma error: LZMA_MEM_ERROR";
  நிலை LZMA_MEMLIMIT_ERROR:
    திரும்பு "lzma error: LZMA_MEMLIMIT_ERROR";
  நிலை LZMA_FORMAT_ERROR:
    திரும்பு "lzma error: LZMA_FORMAT_ERROR";
  நிலை LZMA_OPTIONS_ERROR:
    திரும்பு "lzma error: LZMA_OPTIONS_ERROR";
  நிலை LZMA_DATA_ERROR:
    திரும்பு "lzma error: LZMA_DATA_ERROR";
  நிலை LZMA_BUF_ERROR:
    திரும்பு "lzma error: LZMA_BUF_ERROR";
  நிலை LZMA_PROG_ERROR:
    திரும்பு "lzma error: LZMA_PROG_ERROR";
  இயல்பு:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
நிலையான Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> உள்ளீடு) {
  எனில் (உள்ளீடு.size() < LZMA_STREAM_HEADER_SIZE)
    திரும்பு createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        உள்ளீடு.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, உள்ளீடு.take_back(LZMA_STREAM_HEADER_SIZE).data());
  எனில் (Ret != LZMA_OK)
    திரும்பு createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  எனில் (உள்ளீடு.size() < MinSize)
    திரும்பு createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        உள்ளீடு.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      உள்ளீடு.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, உள்ளீடு.size());
  எனில் (Ret != LZMA_OK)
    திரும்பு createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  திரும்பு lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> உள்ளீடு,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(உள்ளீடு);
  எனில் (!UncompressedSize)
    திரும்பு UncompressedSize.takeError();

  எனில் (*UncompressedSize > std::numeric_limits<size_t>::max())
    திரும்பு createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, உள்ளீடு.data(), &InPos, உள்ளீடு.size(),
      Output.data(), &OutPos, Output.size());
  எனில் (Ret != LZMA_OK) {
    Output.clear();
    திரும்பு createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  திரும்பு Error::success();
}

#இல்லையெனில்

தருக்க xz::isAvailable() { திரும்பு பொய்; }
Error xz::decompress(ArrayRef<uint8_t> உள்ளீடு,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
