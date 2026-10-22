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
स्थिर तार्किक isZlibHeader(ArrayRef<uint8_t> आगत) {
  यदि (आगत.size() < 2)
    फर्काऊ असत्य;
  अचिन्हित CMF = आगत[0];
  अचिन्हित FLG = आगत[1];
  यदि ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    फर्काऊ असत्य;
  फर्काऊ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
स्थिर तार्किक isZstdMagic(ArrayRef<uint8_t> आगत) {
  स्थिर constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  फर्काऊ आगत.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
स्थिर std::optional<Format> identifyFormat(ArrayRef<uint8_t> आगत) {
  यदि (isZstdMagic(आगत))
    फर्काऊ Format::Zstd;
  यदि (isZlibHeader(आगत))
    फर्काऊ Format::Zlib;
  फर्काऊ std::nullopt;
}

अचर वर्ण *compression::getReasonIfUnsupported(compression::Format F) {
  विकल्प (F) {
  पक्ष compression::Format::Zlib:
    यदि (zlib::isAvailable())
      फर्काऊ nullptr;
    फर्काऊ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  पक्ष compression::Format::Zstd:
    यदि (zstd::isAvailable())
      फर्काऊ nullptr;
    फर्काऊ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

अचर वर्ण *compression::getReasonIfUnsupported(ArrayRef<uint8_t> आगत) {
  यदि (std::optional<Format> F = identifyFormat(आगत))
    फर्काऊ getReasonIfUnsupported(*F);
  फर्काऊ "unknown compression format";
}

शून्य compression::compress(Params P, ArrayRef<uint8_t> आगत,
                           SmallVectorImpl<uint8_t> &Output) {
  विकल्प (P.format) {
  पक्ष compression::Format::Zlib:
    zlib::compress(आगत, Output, P.level);
    रोक;
  पक्ष compression::Format::Zstd:
    zstd::compress(आगत, Output, P.level, P.zstdEnableLdm);
    रोक;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> आगत,
                              uint8_t *Output, माप_प्रकार UncompressedSize) {
  विकल्प (formatFor(T)) {
  पक्ष compression::Format::Zlib:
    फर्काऊ zlib::decompress(आगत, Output, UncompressedSize);
  पक्ष compression::Format::Zstd:
    फर्काऊ zstd::decompress(आगत, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> आगत,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार UncompressedSize) {
  विकल्प (F) {
  पक्ष compression::Format::Zlib:
    फर्काऊ zlib::decompress(आगत, Output, UncompressedSize);
  पक्ष compression::Format::Zstd:
    फर्काऊ zstd::decompress(आगत, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> आगत,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार UncompressedSize) {
  फर्काऊ decompress(formatFor(T), आगत, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> आगत,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार UncompressedSize) {
  std::optional<Format> F = identifyFormat(आगत);
  यदि (अचर वर्ण *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    फर्काऊ createStringError(Reason);
  फर्काऊ decompress(*F, आगत, Output, UncompressedSize);
}

#यदि LLVM_ENABLE_ZLIB

स्थिर StringRef convertZlibCodeToString(पूर्णाङ्क Code) {
  विकल्प (Code) {
  पक्ष Z_MEM_ERROR:
    फर्काऊ "zlib error: Z_MEM_ERROR";
  पक्ष Z_BUF_ERROR:
    फर्काऊ "zlib error: Z_BUF_ERROR";
  पक्ष Z_STREAM_ERROR:
    फर्काऊ "zlib error: Z_STREAM_ERROR";
  पक्ष Z_DATA_ERROR:
    फर्काऊ "zlib error: Z_DATA_ERROR";
  पक्ष Z_OK:
  स्वभाव:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

तार्किक zlib::isAvailable() { फर्काऊ सत्य; }

शून्य zlib::compress(ArrayRef<uint8_t> आगत,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level) {
  अचिन्हित दीर्घ CompressedSize = ::compressBound(आगत.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  पूर्णाङ्क Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (अचर Bytef *)आगत.data(), आगत.size(), Level);
  यदि (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  प्रतिज्ञा(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  यदि (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> आगत, uint8_t *Output,
                       माप_प्रकार &UncompressedSize) {
  पूर्णाङ्क Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (अचर Bytef *)आगत.data(), आगत.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  फर्काऊ Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> आगत,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(आगत, Output.data(), UncompressedSize);
  यदि (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  फर्काऊ E;
}

#अन्यथा
तार्किक zlib::isAvailable() { फर्काऊ असत्य; }
शून्य zlib::compress(ArrayRef<uint8_t> आगत,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> आगत, uint8_t *UncompressedBuffer,
                       माप_प्रकार &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> आगत,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       माप_प्रकार UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#यदि LLVM_ENABLE_ZSTD

तार्किक zstd::isAvailable() { फर्काऊ सत्य; }

#अन्तर्भाव <zstd.h> // Ensure ZSTD library is included

शून्य zstd::compress(ArrayRef<uint8_t> आगत,
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

  अचिन्हित दीर्घ CompressedBufferSize = ZSTD_compressBound(आगत.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  माप_प्रकार अचर CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     आगत.data(), आगत.size());

  ZSTD_freeCCtx(Cctx);

  यदि (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  यदि (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> आगत, uint8_t *Output,
                       माप_प्रकार &UncompressedSize) {
  अचर माप_प्रकार Res = ::ZSTD_decompress(
      Output, UncompressedSize, (अचर uint8_t *)आगत.data(), आगत.size());
  UncompressedSize = Res;
  यदि (ZSTD_isError(Res))
    फर्काऊ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  फर्काऊ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> आगत,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(आगत, Output.data(), UncompressedSize);
  यदि (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  फर्काऊ E;
}

#अन्यथा
तार्किक zstd::isAvailable() { फर्काऊ असत्य; }
शून्य zstd::compress(ArrayRef<uint8_t> आगत,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णाङ्क Level,
                    तार्किक EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> आगत, uint8_t *Output,
                       माप_प्रकार &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> आगत,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#यदि LLVM_ENABLE_LZMA

तार्किक xz::isAvailable() { फर्काऊ सत्य; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
स्थिर अचर वर्ण *convertLZMACodeToString(lzma_ret Code) {
  विकल्प (Code) {
  पक्ष LZMA_STREAM_END:
    फर्काऊ "lzma error: LZMA_STREAM_END";
  पक्ष LZMA_NO_CHECK:
    फर्काऊ "lzma error: LZMA_NO_CHECK";
  पक्ष LZMA_UNSUPPORTED_CHECK:
    फर्काऊ "lzma error: LZMA_UNSUPPORTED_CHECK";
  पक्ष LZMA_GET_CHECK:
    फर्काऊ "lzma error: LZMA_GET_CHECK";
  पक्ष LZMA_MEM_ERROR:
    फर्काऊ "lzma error: LZMA_MEM_ERROR";
  पक्ष LZMA_MEMLIMIT_ERROR:
    फर्काऊ "lzma error: LZMA_MEMLIMIT_ERROR";
  पक्ष LZMA_FORMAT_ERROR:
    फर्काऊ "lzma error: LZMA_FORMAT_ERROR";
  पक्ष LZMA_OPTIONS_ERROR:
    फर्काऊ "lzma error: LZMA_OPTIONS_ERROR";
  पक्ष LZMA_DATA_ERROR:
    फर्काऊ "lzma error: LZMA_DATA_ERROR";
  पक्ष LZMA_BUF_ERROR:
    फर्काऊ "lzma error: LZMA_BUF_ERROR";
  पक्ष LZMA_PROG_ERROR:
    फर्काऊ "lzma error: LZMA_PROG_ERROR";
  स्वभाव:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
स्थिर Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> आगत) {
  यदि (आगत.size() < LZMA_STREAM_HEADER_SIZE)
    फर्काऊ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        आगत.size(), माप_प्रकार(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, आगत.take_back(LZMA_STREAM_HEADER_SIZE).data());
  यदि (Ret != LZMA_OK)
    फर्काऊ createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  यदि (आगत.size() < MinSize)
    फर्काऊ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        आगत.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  माप_प्रकार InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      आगत.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, आगत.size());
  यदि (Ret != LZMA_OK)
    फर्काऊ createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  फर्काऊ lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> आगत,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(आगत);
  यदि (!UncompressedSize)
    फर्काऊ UncompressedSize.takeError();

  यदि (*UncompressedSize > std::numeric_limits<माप_प्रकार>::max())
    फर्काऊ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<माप_प्रकार>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  माप_प्रकार InPos = 0;
  माप_प्रकार OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, आगत.data(), &InPos, आगत.size(),
      Output.data(), &OutPos, Output.size());
  यदि (Ret != LZMA_OK) {
    Output.clear();
    फर्काऊ createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  फर्काऊ Error::success();
}

#अन्यथा

तार्किक xz::isAvailable() { फर्काऊ असत्य; }
Error xz::decompress(ArrayRef<uint8_t> आगत,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
