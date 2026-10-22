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
#ಯದಿ LLVM_ENABLE_ZLIB
#ಅನ್ತರ್ಭಾವ <zlib.h>
#endif
#ಯದಿ LLVM_ENABLE_ZSTD
#ಅನ್ತರ್ಭಾವ <zstd.h>
#endif
#ಯದಿ LLVM_ENABLE_LZMA
#ಅನ್ತರ್ಭಾವ <lzma.h>
#endif

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isZlibHeader(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಯದಿ (ಒಳಹರಿವು.size() < 2)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  unsigned CMF = ಒಳಹರಿವು[0];
  unsigned FLG = ಒಳಹರಿವು[1];
  ಯದಿ ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತ್ಯಾಗಮ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ಸ್ಥಿರ ತಾರ್ಕಿಕ isZstdMagic(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಸ್ಥಿರ constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ಪ್ರತ್ಯಾಗಮ ಒಳಹರಿವು.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ಸ್ಥಿರ std::optional<Format> identifyFormat(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಯದಿ (isZstdMagic(ಒಳಹರಿವು))
    ಪ್ರತ್ಯಾಗಮ Format::Zstd;
  ಯದಿ (isZlibHeader(ಒಳಹರಿವು))
    ಪ್ರತ್ಯಾಗಮ Format::Zlib;
  ಪ್ರತ್ಯಾಗಮ std::nullopt;
}

ಧ್ರುವ ವರ್ಣ *compression::getReasonIfUnsupported(compression::Format F) {
  ವಿಕಲ್ಪ (F) {
  ಪಕ್ಷ compression::Format::Zlib:
    ಯದಿ (zlib::isAvailable())
      ಪ್ರತ್ಯಾಗಮ nullptr;
    ಪ್ರತ್ಯಾಗಮ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  ಪಕ್ಷ compression::Format::Zstd:
    ಯದಿ (zstd::isAvailable())
      ಪ್ರತ್ಯಾಗಮ nullptr;
    ಪ್ರತ್ಯಾಗಮ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ಧ್ರುವ ವರ್ಣ *compression::getReasonIfUnsupported(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಯದಿ (std::optional<Format> F = identifyFormat(ಒಳಹರಿವು))
    ಪ್ರತ್ಯಾಗಮ getReasonIfUnsupported(*F);
  ಪ್ರತ್ಯಾಗಮ "unknown compression format";
}

ಶೂನ್ಯ compression::compress(Params P, ArrayRef<uint8_t> ಒಳಹರಿವು,
                           SmallVectorImpl<uint8_t> &Output) {
  ವಿಕಲ್ಪ (P.format) {
  ಪಕ್ಷ compression::Format::Zlib:
    zlib::compress(ಒಳಹರಿವು, Output, P.level);
    ಭಙ್ಗ;
  ಪಕ್ಷ compression::Format::Zstd:
    zstd::compress(ಒಳಹರಿವು, Output, P.level, P.zstdEnableLdm);
    ಭಙ್ಗ;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ಒಳಹರಿವು,
                              uint8_t *Output, size_t UncompressedSize) {
  ವಿಕಲ್ಪ (formatFor(T)) {
  ಪಕ್ಷ compression::Format::Zlib:
    ಪ್ರತ್ಯಾಗಮ zlib::decompress(ಒಳಹರಿವು, Output, UncompressedSize);
  ಪಕ್ಷ compression::Format::Zstd:
    ಪ್ರತ್ಯಾಗಮ zstd::decompress(ಒಳಹರಿವು, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> ಒಳಹರಿವು,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ವಿಕಲ್ಪ (F) {
  ಪಕ್ಷ compression::Format::Zlib:
    ಪ್ರತ್ಯಾಗಮ zlib::decompress(ಒಳಹರಿವು, Output, UncompressedSize);
  ಪಕ್ಷ compression::Format::Zstd:
    ಪ್ರತ್ಯಾಗಮ zstd::decompress(ಒಳಹರಿವು, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ಒಳಹರಿವು,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ಪ್ರತ್ಯಾಗಮ decompress(formatFor(T), ಒಳಹರಿವು, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(ಒಳಹರಿವು);
  ಯದಿ (ಧ್ರುವ ವರ್ಣ *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ಪ್ರತ್ಯಾಗಮ createStringError(Reason);
  ಪ್ರತ್ಯಾಗಮ decompress(*F, ಒಳಹರಿವು, Output, UncompressedSize);
}

#ಯದಿ LLVM_ENABLE_ZLIB

ಸ್ಥಿರ StringRef convertZlibCodeToString(ಪೂರ್ಣಾಙ್ಕ Code) {
  ವಿಕಲ್ಪ (Code) {
  ಪಕ್ಷ Z_MEM_ERROR:
    ಪ್ರತ್ಯಾಗಮ "zlib error: Z_MEM_ERROR";
  ಪಕ್ಷ Z_BUF_ERROR:
    ಪ್ರತ್ಯಾಗಮ "zlib error: Z_BUF_ERROR";
  ಪಕ್ಷ Z_STREAM_ERROR:
    ಪ್ರತ್ಯಾಗಮ "zlib error: Z_STREAM_ERROR";
  ಪಕ್ಷ Z_DATA_ERROR:
    ಪ್ರತ್ಯಾಗಮ "zlib error: Z_DATA_ERROR";
  ಪಕ್ಷ Z_OK:
  ಸ್ವಭಾವ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

ತಾರ್ಕಿಕ zlib::isAvailable() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }

ಶೂನ್ಯ zlib::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ಪೂರ್ಣಾಙ್ಕ Level) {
  unsigned long CompressedSize = ::compressBound(ಒಳಹರಿವು.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  ಪೂರ್ಣಾಙ್ಕ Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ಧ್ರುವ Bytef *)ಒಳಹರಿವು.data(), ಒಳಹರಿವು.size(), Level);
  ಯದಿ (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  ಪ್ರತಿಜ್ಞಾ(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  ಯದಿ (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Output,
                       size_t &UncompressedSize) {
  ಪೂರ್ಣಾಙ್ಕ Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ಧ್ರುವ Bytef *)ಒಳಹರಿವು.data(), ಒಳಹರಿವು.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ಪ್ರತ್ಯಾಗಮ Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(ಒಳಹರಿವು, Output.data(), UncompressedSize);
  ಯದಿ (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ಪ್ರತ್ಯಾಗಮ E;
}

#ಅನ್ಯಥಾ
ತಾರ್ಕಿಕ zlib::isAvailable() { ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; }
ಶೂನ್ಯ zlib::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ಪೂರ್ಣಾಙ್ಕ Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#ಯದಿ LLVM_ENABLE_ZSTD

ತಾರ್ಕಿಕ zstd::isAvailable() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }

#ಅನ್ತರ್ಭಾವ <zstd.h> // Ensure ZSTD library is included

ಶೂನ್ಯ zstd::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ಪೂರ್ಣಾಙ್ಕ Level,
                    ತಾರ್ಕಿಕ EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  ಯದಿ (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  ಯದಿ (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  ಯದಿ (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(ಒಳಹರಿವು.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ಧ್ರುವ CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     ಒಳಹರಿವು.data(), ಒಳಹರಿವು.size());

  ZSTD_freeCCtx(Cctx);

  ಯದಿ (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  ಯದಿ (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Output,
                       size_t &UncompressedSize) {
  ಧ್ರುವ size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ಧ್ರುವ uint8_t *)ಒಳಹರಿವು.data(), ಒಳಹರಿವು.size());
  UncompressedSize = Res;
  ಯದಿ (ZSTD_isError(Res))
    ಪ್ರತ್ಯಾಗಮ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ಪ್ರತ್ಯಾಗಮ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(ಒಳಹರಿವು, Output.data(), UncompressedSize);
  ಯದಿ (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ಪ್ರತ್ಯಾಗಮ E;
}

#ಅನ್ಯಥಾ
ತಾರ್ಕಿಕ zstd::isAvailable() { ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; }
ಶೂನ್ಯ zstd::compress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ಪೂರ್ಣಾಙ್ಕ Level,
                    ತಾರ್ಕಿಕ EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#ಯದಿ LLVM_ENABLE_LZMA

ತಾರ್ಕಿಕ xz::isAvailable() { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *convertLZMACodeToString(lzma_ret Code) {
  ವಿಕಲ್ಪ (Code) {
  ಪಕ್ಷ LZMA_STREAM_END:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_STREAM_END";
  ಪಕ್ಷ LZMA_NO_CHECK:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_NO_CHECK";
  ಪಕ್ಷ LZMA_UNSUPPORTED_CHECK:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_UNSUPPORTED_CHECK";
  ಪಕ್ಷ LZMA_GET_CHECK:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_GET_CHECK";
  ಪಕ್ಷ LZMA_MEM_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_MEM_ERROR";
  ಪಕ್ಷ LZMA_MEMLIMIT_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_MEMLIMIT_ERROR";
  ಪಕ್ಷ LZMA_FORMAT_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_FORMAT_ERROR";
  ಪಕ್ಷ LZMA_OPTIONS_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_OPTIONS_ERROR";
  ಪಕ್ಷ LZMA_DATA_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_DATA_ERROR";
  ಪಕ್ಷ LZMA_BUF_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_BUF_ERROR";
  ಪಕ್ಷ LZMA_PROG_ERROR:
    ಪ್ರತ್ಯಾಗಮ "lzma error: LZMA_PROG_ERROR";
  ಸ್ವಭಾವ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ಸ್ಥಿರ Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> ಒಳಹರಿವು) {
  ಯದಿ (ಒಳಹರಿವು.size() < LZMA_STREAM_HEADER_SIZE)
    ಪ್ರತ್ಯಾಗಮ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        ಒಳಹರಿವು.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, ಒಳಹರಿವು.take_back(LZMA_STREAM_HEADER_SIZE).data());
  ಯದಿ (Ret != LZMA_OK)
    ಪ್ರತ್ಯಾಗಮ createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  ಯದಿ (ಒಳಹರಿವು.size() < MinSize)
    ಪ್ರತ್ಯಾಗಮ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        ಒಳಹರಿವು.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      ಒಳಹರಿವು.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, ಒಳಹರಿವು.size());
  ಯದಿ (Ret != LZMA_OK)
    ಪ್ರತ್ಯಾಗಮ createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  ಪ್ರತ್ಯಾಗಮ lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(ಒಳಹರಿವು);
  ಯದಿ (!UncompressedSize)
    ಪ್ರತ್ಯಾಗಮ UncompressedSize.takeError();

  ಯದಿ (*UncompressedSize > std::numeric_limits<size_t>::max())
    ಪ್ರತ್ಯಾಗಮ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, ಒಳಹರಿವು.data(), &InPos, ಒಳಹರಿವು.size(),
      Output.data(), &OutPos, Output.size());
  ಯದಿ (Ret != LZMA_OK) {
    Output.clear();
    ಪ್ರತ್ಯಾಗಮ createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  ಪ್ರತ್ಯಾಗಮ Error::success();
}

#ಅನ್ಯಥಾ

ತಾರ್ಕಿಕ xz::isAvailable() { ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ; }
Error xz::decompress(ArrayRef<uint8_t> ಒಳಹರಿವು,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
