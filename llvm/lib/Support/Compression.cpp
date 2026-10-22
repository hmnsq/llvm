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

#अन्तर्भाव "llvm/Support/Compression.h"
#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/ADT/SmallVector.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/Compiler.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव <limits>
#अन्तर्भाव <optional>
#यदि LLVM_ENABLE_ZLIB
#अन्तर्भाव <zlib.h>
#endif
#यदि LLVM_ENABLE_ZSTD
#अन्तर्भाव <zstd.h>
#endif
#यदि LLVM_ENABLE_LZMA
#अन्तर्भाव <lzma.h>
#endif

using नामाकाश llvm;
using नामाकाश llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
स्थिर तार्किक isZlibHeader(ArrayRef<uint8_t> आदान) {
  यदि (आदान.size() < 2)
    प्रत्यागम असत्य;
  unsigned CMF = आदान[0];
  unsigned FLG = आदान[1];
  यदि ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    प्रत्यागम असत्य;
  प्रत्यागम (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
स्थिर तार्किक isZstdMagic(ArrayRef<uint8_t> आदान) {
  स्थिर constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  प्रत्यागम आदान.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
स्थिर std::optional<Format> identifyFormat(ArrayRef<uint8_t> आदान) {
  यदि (isZstdMagic(आदान))
    प्रत्यागम Format::Zstd;
  यदि (isZlibHeader(आदान))
    प्रत्यागम Format::Zlib;
  प्रत्यागम std::nullopt;
}

ध्रुव वर्ण *compression::getReasonIfUnsupported(compression::Format F) {
  विकल्प (F) {
  पक्ष compression::Format::Zlib:
    यदि (zlib::isAvailable())
      प्रत्यागम nullptr;
    प्रत्यागम "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  पक्ष compression::Format::Zstd:
    यदि (zstd::isAvailable())
      प्रत्यागम nullptr;
    प्रत्यागम "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ध्रुव वर्ण *compression::getReasonIfUnsupported(ArrayRef<uint8_t> आदान) {
  यदि (std::optional<Format> F = identifyFormat(आदान))
    प्रत्यागम getReasonIfUnsupported(*F);
  प्रत्यागम "unknown compression format";
}

शून्य compression::compress(Params P, ArrayRef<uint8_t> आदान,
                           SmallVectorImpl<uint8_t> &Output) {
  विकल्प (P.format) {
  पक्ष compression::Format::Zlib:
    zlib::compress(आदान, Output, P.level);
    भङ्ग;
  पक्ष compression::Format::Zstd:
    zstd::compress(आदान, Output, P.level, P.zstdEnableLdm);
    भङ्ग;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> आदान,
                              uint8_t *Output, size_t UncompressedSize) {
  विकल्प (formatFor(T)) {
  पक्ष compression::Format::Zlib:
    प्रत्यागम zlib::decompress(आदान, Output, UncompressedSize);
  पक्ष compression::Format::Zstd:
    प्रत्यागम zstd::decompress(आदान, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> आदान,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  विकल्प (F) {
  पक्ष compression::Format::Zlib:
    प्रत्यागम zlib::decompress(आदान, Output, UncompressedSize);
  पक्ष compression::Format::Zstd:
    प्रत्यागम zstd::decompress(आदान, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> आदान,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  प्रत्यागम decompress(formatFor(T), आदान, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> आदान,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(आदान);
  यदि (ध्रुव वर्ण *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    प्रत्यागम createStringError(Reason);
  प्रत्यागम decompress(*F, आदान, Output, UncompressedSize);
}

#यदि LLVM_ENABLE_ZLIB

स्थिर StringRef convertZlibCodeToString(पूर्णाङ्क Code) {
  विकल्प (Code) {
  पक्ष Z_MEM_ERROR:
    प्रत्यागम "zlib error: Z_MEM_ERROR";
  पक्ष Z_BUF_ERROR:
    प्रत्यागम "zlib error: Z_BUF_ERROR";
  पक्ष Z_STREAM_ERROR:
    प्रत्यागम "zlib error: Z_STREAM_ERROR";
  पक्ष Z_DATA_ERROR:
    प्रत्यागम "zlib error: Z_DATA_ERROR";
  पक्ष Z_OK:
  स्वभाव:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

तार्किक zlib::isAvailable() { प्रत्यागम सत्य; }

शून्य zlib::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level) {
  unsigned long CompressedSize = ::compressBound(आदान.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  पूर्णाङ्क Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ध्रुव Bytef *)आदान.data(), आदान.size(), Level);
  यदि (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  प्रतिज्ञा(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  यदि (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> आदान, uint8_t *Output,
                       size_t &UncompressedSize) {
  पूर्णाङ्क Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ध्रुव Bytef *)आदान.data(), आदान.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  प्रत्यागम Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(आदान, Output.data(), UncompressedSize);
  यदि (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  प्रत्यागम E;
}

#अन्यथा
तार्किक zlib::isAvailable() { प्रत्यागम असत्य; }
शून्य zlib::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> आदान, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#यदि LLVM_ENABLE_ZSTD

तार्किक zstd::isAvailable() { प्रत्यागम सत्य; }

#अन्तर्भाव <zstd.h> // Ensure ZSTD library is included

शून्य zstd::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level,
                    तार्किक EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  यदि (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  यदि (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  यदि (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(आदान.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ध्रुव CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     आदान.data(), आदान.size());

  ZSTD_freeCCtx(Cctx);

  यदि (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  यदि (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> आदान, uint8_t *Output,
                       size_t &UncompressedSize) {
  ध्रुव size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ध्रुव uint8_t *)आदान.data(), आदान.size());
  UncompressedSize = Res;
  यदि (ZSTD_isError(Res))
    प्रत्यागम make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  प्रत्यागम Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(आदान, Output.data(), UncompressedSize);
  यदि (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  प्रत्यागम E;
}

#अन्यथा
तार्किक zstd::isAvailable() { प्रत्यागम असत्य; }
शून्य zstd::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level,
                    तार्किक EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> आदान, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#यदि LLVM_ENABLE_LZMA

तार्किक xz::isAvailable() { प्रत्यागम सत्य; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
स्थिर ध्रुव वर्ण *convertLZMACodeToString(lzma_ret Code) {
  विकल्प (Code) {
  पक्ष LZMA_STREAM_END:
    प्रत्यागम "lzma error: LZMA_STREAM_END";
  पक्ष LZMA_NO_CHECK:
    प्रत्यागम "lzma error: LZMA_NO_CHECK";
  पक्ष LZMA_UNSUPPORTED_CHECK:
    प्रत्यागम "lzma error: LZMA_UNSUPPORTED_CHECK";
  पक्ष LZMA_GET_CHECK:
    प्रत्यागम "lzma error: LZMA_GET_CHECK";
  पक्ष LZMA_MEM_ERROR:
    प्रत्यागम "lzma error: LZMA_MEM_ERROR";
  पक्ष LZMA_MEMLIMIT_ERROR:
    प्रत्यागम "lzma error: LZMA_MEMLIMIT_ERROR";
  पक्ष LZMA_FORMAT_ERROR:
    प्रत्यागम "lzma error: LZMA_FORMAT_ERROR";
  पक्ष LZMA_OPTIONS_ERROR:
    प्रत्यागम "lzma error: LZMA_OPTIONS_ERROR";
  पक्ष LZMA_DATA_ERROR:
    प्रत्यागम "lzma error: LZMA_DATA_ERROR";
  पक्ष LZMA_BUF_ERROR:
    प्रत्यागम "lzma error: LZMA_BUF_ERROR";
  पक्ष LZMA_PROG_ERROR:
    प्रत्यागम "lzma error: LZMA_PROG_ERROR";
  स्वभाव:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
स्थिर Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> आदान) {
  यदि (आदान.size() < LZMA_STREAM_HEADER_SIZE)
    प्रत्यागम createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        आदान.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, आदान.take_back(LZMA_STREAM_HEADER_SIZE).data());
  यदि (Ret != LZMA_OK)
    प्रत्यागम createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  यदि (आदान.size() < MinSize)
    प्रत्यागम createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        आदान.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      आदान.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, आदान.size());
  यदि (Ret != LZMA_OK)
    प्रत्यागम createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  प्रत्यागम lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> आदान,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(आदान);
  यदि (!UncompressedSize)
    प्रत्यागम UncompressedSize.takeError();

  यदि (*UncompressedSize > std::numeric_limits<size_t>::max())
    प्रत्यागम createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, आदान.data(), &InPos, आदान.size(),
      Output.data(), &OutPos, Output.size());
  यदि (Ret != LZMA_OK) {
    Output.clear();
    प्रत्यागम createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  प्रत्यागम Error::success();
}

#अन्यथा

तार्किक xz::isAvailable() { प्रत्यागम असत्य; }
Error xz::decompress(ArrayRef<uint8_t> आदान,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
