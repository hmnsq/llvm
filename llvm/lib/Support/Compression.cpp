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

#અન્તર્ભાવ "llvm/Support/Compression.h"
#અન્તર્ભાવ "llvm/ADT/ScopeExit.h"
#અન્તર્ભાવ "llvm/ADT/SmallVector.h"
#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/Compiler.h"
#અન્તર્ભાવ "llvm/Support/Error.h"
#અન્તર્ભાવ "llvm/Support/ErrorHandling.h"
#અન્તર્ભાવ <limits>
#અન્તર્ભાવ <optional>
#જો LLVM_ENABLE_ZLIB
#અન્તર્ભાવ <zlib.h>
#endif
#જો LLVM_ENABLE_ZSTD
#અન્તર્ભાવ <zstd.h>
#endif
#જો LLVM_ENABLE_LZMA
#અન્તર્ભાવ <lzma.h>
#endif

using નામાકાશ llvm;
using નામાકાશ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
સ્થિર તાર્કિક isZlibHeader(ArrayRef<uint8_t> આગત) {
  જો (આગત.size() < 2)
    પાછા ખોટું;
  unsigned CMF = આગત[0];
  unsigned FLG = આગત[1];
  જો ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    પાછા ખોટું;
  પાછા (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
સ્થિર તાર્કિક isZstdMagic(ArrayRef<uint8_t> આગત) {
  સ્થિર constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  પાછા આગત.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
સ્થિર std::optional<Format> identifyFormat(ArrayRef<uint8_t> આગત) {
  જો (isZstdMagic(આગત))
    પાછા Format::Zstd;
  જો (isZlibHeader(આગત))
    પાછા Format::Zlib;
  પાછા std::nullopt;
}

અચલ અક્ષર *compression::getReasonIfUnsupported(compression::Format F) {
  પસંદગી (F) {
  સ્થિતિ compression::Format::Zlib:
    જો (zlib::isAvailable())
      પાછા nullptr;
    પાછા "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  સ્થિતિ compression::Format::Zstd:
    જો (zstd::isAvailable())
      પાછા nullptr;
    પાછા "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

અચલ અક્ષર *compression::getReasonIfUnsupported(ArrayRef<uint8_t> આગત) {
  જો (std::optional<Format> F = identifyFormat(આગત))
    પાછા getReasonIfUnsupported(*F);
  પાછા "unknown compression format";
}

શૂન્ય compression::compress(Params P, ArrayRef<uint8_t> આગત,
                           SmallVectorImpl<uint8_t> &Output) {
  પસંદગી (P.format) {
  સ્થિતિ compression::Format::Zlib:
    zlib::compress(આગત, Output, P.level);
    તોડો;
  સ્થિતિ compression::Format::Zstd:
    zstd::compress(આગત, Output, P.level, P.zstdEnableLdm);
    તોડો;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> આગત,
                              uint8_t *Output, size_t UncompressedSize) {
  પસંદગી (formatFor(T)) {
  સ્થિતિ compression::Format::Zlib:
    પાછા zlib::decompress(આગત, Output, UncompressedSize);
  સ્થિતિ compression::Format::Zstd:
    પાછા zstd::decompress(આગત, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> આગત,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  પસંદગી (F) {
  સ્થિતિ compression::Format::Zlib:
    પાછા zlib::decompress(આગત, Output, UncompressedSize);
  સ્થિતિ compression::Format::Zstd:
    પાછા zstd::decompress(આગત, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> આગત,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  પાછા decompress(formatFor(T), આગત, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> આગત,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(આગત);
  જો (અચલ અક્ષર *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    પાછા createStringError(Reason);
  પાછા decompress(*F, આગત, Output, UncompressedSize);
}

#જો LLVM_ENABLE_ZLIB

સ્થિર StringRef convertZlibCodeToString(પૂર્ણાંક Code) {
  પસંદગી (Code) {
  સ્થિતિ Z_MEM_ERROR:
    પાછા "zlib error: Z_MEM_ERROR";
  સ્થિતિ Z_BUF_ERROR:
    પાછા "zlib error: Z_BUF_ERROR";
  સ્થિતિ Z_STREAM_ERROR:
    પાછા "zlib error: Z_STREAM_ERROR";
  સ્થિતિ Z_DATA_ERROR:
    પાછા "zlib error: Z_DATA_ERROR";
  સ્થિતિ Z_OK:
  મૂળભૂત:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

તાર્કિક zlib::isAvailable() { પાછા સાચું; }

શૂન્ય zlib::compress(ArrayRef<uint8_t> આગત,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, પૂર્ણાંક Level) {
  unsigned long CompressedSize = ::compressBound(આગત.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  પૂર્ણાંક Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (અચલ Bytef *)આગત.data(), આગત.size(), Level);
  જો (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  પ્રતિજ્ઞા(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  જો (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> આગત, uint8_t *Output,
                       size_t &UncompressedSize) {
  પૂર્ણાંક Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (અચલ Bytef *)આગત.data(), આગત.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  પાછા Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> આગત,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(આગત, Output.data(), UncompressedSize);
  જો (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  પાછા E;
}

#નહીંતર
તાર્કિક zlib::isAvailable() { પાછા ખોટું; }
શૂન્ય zlib::compress(ArrayRef<uint8_t> આગત,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, પૂર્ણાંક Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> આગત, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> આગત,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#જો LLVM_ENABLE_ZSTD

તાર્કિક zstd::isAvailable() { પાછા સાચું; }

#અન્તર્ભાવ <zstd.h> // Ensure ZSTD library is included

શૂન્ય zstd::compress(ArrayRef<uint8_t> આગત,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, પૂર્ણાંક Level,
                    તાર્કિક EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  જો (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  જો (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  જો (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(આગત.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t અચલ CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     આગત.data(), આગત.size());

  ZSTD_freeCCtx(Cctx);

  જો (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  જો (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> આગત, uint8_t *Output,
                       size_t &UncompressedSize) {
  અચલ size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (અચલ uint8_t *)આગત.data(), આગત.size());
  UncompressedSize = Res;
  જો (ZSTD_isError(Res))
    પાછા make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  પાછા Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> આગત,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(આગત, Output.data(), UncompressedSize);
  જો (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  પાછા E;
}

#નહીંતર
તાર્કિક zstd::isAvailable() { પાછા ખોટું; }
શૂન્ય zstd::compress(ArrayRef<uint8_t> આગત,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, પૂર્ણાંક Level,
                    તાર્કિક EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> આગત, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> આગત,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#જો LLVM_ENABLE_LZMA

તાર્કિક xz::isAvailable() { પાછા સાચું; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
સ્થિર અચલ અક્ષર *convertLZMACodeToString(lzma_ret Code) {
  પસંદગી (Code) {
  સ્થિતિ LZMA_STREAM_END:
    પાછા "lzma error: LZMA_STREAM_END";
  સ્થિતિ LZMA_NO_CHECK:
    પાછા "lzma error: LZMA_NO_CHECK";
  સ્થિતિ LZMA_UNSUPPORTED_CHECK:
    પાછા "lzma error: LZMA_UNSUPPORTED_CHECK";
  સ્થિતિ LZMA_GET_CHECK:
    પાછા "lzma error: LZMA_GET_CHECK";
  સ્થિતિ LZMA_MEM_ERROR:
    પાછા "lzma error: LZMA_MEM_ERROR";
  સ્થિતિ LZMA_MEMLIMIT_ERROR:
    પાછા "lzma error: LZMA_MEMLIMIT_ERROR";
  સ્થિતિ LZMA_FORMAT_ERROR:
    પાછા "lzma error: LZMA_FORMAT_ERROR";
  સ્થિતિ LZMA_OPTIONS_ERROR:
    પાછા "lzma error: LZMA_OPTIONS_ERROR";
  સ્થિતિ LZMA_DATA_ERROR:
    પાછા "lzma error: LZMA_DATA_ERROR";
  સ્થિતિ LZMA_BUF_ERROR:
    પાછા "lzma error: LZMA_BUF_ERROR";
  સ્થિતિ LZMA_PROG_ERROR:
    પાછા "lzma error: LZMA_PROG_ERROR";
  મૂળભૂત:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
સ્થિર Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> આગત) {
  જો (આગત.size() < LZMA_STREAM_HEADER_SIZE)
    પાછા createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        આગત.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, આગત.take_back(LZMA_STREAM_HEADER_SIZE).data());
  જો (Ret != LZMA_OK)
    પાછા createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  જો (આગત.size() < MinSize)
    પાછા createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        આગત.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      આગત.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, આગત.size());
  જો (Ret != LZMA_OK)
    પાછા createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  પાછા lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> આગત,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(આગત);
  જો (!UncompressedSize)
    પાછા UncompressedSize.takeError();

  જો (*UncompressedSize > std::numeric_limits<size_t>::max())
    પાછા createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, આગત.data(), &InPos, આગત.size(),
      Output.data(), &OutPos, Output.size());
  જો (Ret != LZMA_OK) {
    Output.clear();
    પાછા createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  પાછા Error::success();
}

#નહીંતર

તાર્કિક xz::isAvailable() { પાછા ખોટું; }
Error xz::decompress(ArrayRef<uint8_t> આગત,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
