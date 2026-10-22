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
#जर LLVM_ENABLE_ZLIB
#अन्तर्भाव <zlib.h>
#endif
#जर LLVM_ENABLE_ZSTD
#अन्तर्भाव <zstd.h>
#endif
#जर LLVM_ENABLE_LZMA
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
  जर (आदान.size() < 2)
    परत असत्य;
  अचिन्हित CMF = आदान[0];
  अचिन्हित FLG = आदान[1];
  जर ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    परत असत्य;
  परत (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
स्थिर तार्किक isZstdMagic(ArrayRef<uint8_t> आदान) {
  स्थिर constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  परत आदान.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
स्थिर std::optional<Format> identifyFormat(ArrayRef<uint8_t> आदान) {
  जर (isZstdMagic(आदान))
    परत Format::Zstd;
  जर (isZlibHeader(आदान))
    परत Format::Zlib;
  परत std::nullopt;
}

स्थिरांक अक्षर *compression::getReasonIfUnsupported(compression::Format F) {
  निवड (F) {
  स्थिती compression::Format::Zlib:
    जर (zlib::isAvailable())
      परत nullptr;
    परत "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  स्थिती compression::Format::Zstd:
    जर (zstd::isAvailable())
      परत nullptr;
    परत "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

स्थिरांक अक्षर *compression::getReasonIfUnsupported(ArrayRef<uint8_t> आदान) {
  जर (std::optional<Format> F = identifyFormat(आदान))
    परत getReasonIfUnsupported(*F);
  परत "unknown compression format";
}

शून्य compression::compress(Params P, ArrayRef<uint8_t> आदान,
                           SmallVectorImpl<uint8_t> &Output) {
  निवड (P.format) {
  स्थिती compression::Format::Zlib:
    zlib::compress(आदान, Output, P.level);
    थांबा;
  स्थिती compression::Format::Zstd:
    zstd::compress(आदान, Output, P.level, P.zstdEnableLdm);
    थांबा;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> आदान,
                              uint8_t *Output, माप_प्रकार UncompressedSize) {
  निवड (formatFor(T)) {
  स्थिती compression::Format::Zlib:
    परत zlib::decompress(आदान, Output, UncompressedSize);
  स्थिती compression::Format::Zstd:
    परत zstd::decompress(आदान, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> आदान,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार UncompressedSize) {
  निवड (F) {
  स्थिती compression::Format::Zlib:
    परत zlib::decompress(आदान, Output, UncompressedSize);
  स्थिती compression::Format::Zstd:
    परत zstd::decompress(आदान, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> आदान,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार UncompressedSize) {
  परत decompress(formatFor(T), आदान, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> आदान,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार UncompressedSize) {
  std::optional<Format> F = identifyFormat(आदान);
  जर (स्थिरांक अक्षर *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    परत createStringError(Reason);
  परत decompress(*F, आदान, Output, UncompressedSize);
}

#जर LLVM_ENABLE_ZLIB

स्थिर StringRef convertZlibCodeToString(पूर्णांक Code) {
  निवड (Code) {
  स्थिती Z_MEM_ERROR:
    परत "zlib error: Z_MEM_ERROR";
  स्थिती Z_BUF_ERROR:
    परत "zlib error: Z_BUF_ERROR";
  स्थिती Z_STREAM_ERROR:
    परत "zlib error: Z_STREAM_ERROR";
  स्थिती Z_DATA_ERROR:
    परत "zlib error: Z_DATA_ERROR";
  स्थिती Z_OK:
  पूर्वनिर्धारित:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

तार्किक zlib::isAvailable() { परत सत्य; }

शून्य zlib::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णांक Level) {
  अचिन्हित दीर्घ CompressedSize = ::compressBound(आदान.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  पूर्णांक Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (स्थिरांक Bytef *)आदान.data(), आदान.size(), Level);
  जर (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  प्रतिज्ञा(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  जर (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> आदान, uint8_t *Output,
                       माप_प्रकार &UncompressedSize) {
  पूर्णांक Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (स्थिरांक Bytef *)आदान.data(), आदान.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  परत Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(आदान, Output.data(), UncompressedSize);
  जर (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  परत E;
}

#अन्यथा
तार्किक zlib::isAvailable() { परत असत्य; }
शून्य zlib::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णांक Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> आदान, uint8_t *UncompressedBuffer,
                       माप_प्रकार &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       माप_प्रकार UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#जर LLVM_ENABLE_ZSTD

तार्किक zstd::isAvailable() { परत सत्य; }

#अन्तर्भाव <zstd.h> // Ensure ZSTD library is included

शून्य zstd::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णांक Level,
                    तार्किक EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  जर (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  जर (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  जर (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  अचिन्हित दीर्घ CompressedBufferSize = ZSTD_compressBound(आदान.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  माप_प्रकार स्थिरांक CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     आदान.data(), आदान.size());

  ZSTD_freeCCtx(Cctx);

  जर (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  जर (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> आदान, uint8_t *Output,
                       माप_प्रकार &UncompressedSize) {
  स्थिरांक माप_प्रकार Res = ::ZSTD_decompress(
      Output, UncompressedSize, (स्थिरांक uint8_t *)आदान.data(), आदान.size());
  UncompressedSize = Res;
  जर (ZSTD_isError(Res))
    परत make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  परत Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(आदान, Output.data(), UncompressedSize);
  जर (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  परत E;
}

#अन्यथा
तार्किक zstd::isAvailable() { परत असत्य; }
शून्य zstd::compress(ArrayRef<uint8_t> आदान,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, पूर्णांक Level,
                    तार्किक EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> आदान, uint8_t *Output,
                       माप_प्रकार &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> आदान,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#जर LLVM_ENABLE_LZMA

तार्किक xz::isAvailable() { परत सत्य; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
स्थिर स्थिरांक अक्षर *convertLZMACodeToString(lzma_ret Code) {
  निवड (Code) {
  स्थिती LZMA_STREAM_END:
    परत "lzma error: LZMA_STREAM_END";
  स्थिती LZMA_NO_CHECK:
    परत "lzma error: LZMA_NO_CHECK";
  स्थिती LZMA_UNSUPPORTED_CHECK:
    परत "lzma error: LZMA_UNSUPPORTED_CHECK";
  स्थिती LZMA_GET_CHECK:
    परत "lzma error: LZMA_GET_CHECK";
  स्थिती LZMA_MEM_ERROR:
    परत "lzma error: LZMA_MEM_ERROR";
  स्थिती LZMA_MEMLIMIT_ERROR:
    परत "lzma error: LZMA_MEMLIMIT_ERROR";
  स्थिती LZMA_FORMAT_ERROR:
    परत "lzma error: LZMA_FORMAT_ERROR";
  स्थिती LZMA_OPTIONS_ERROR:
    परत "lzma error: LZMA_OPTIONS_ERROR";
  स्थिती LZMA_DATA_ERROR:
    परत "lzma error: LZMA_DATA_ERROR";
  स्थिती LZMA_BUF_ERROR:
    परत "lzma error: LZMA_BUF_ERROR";
  स्थिती LZMA_PROG_ERROR:
    परत "lzma error: LZMA_PROG_ERROR";
  पूर्वनिर्धारित:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
स्थिर Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> आदान) {
  जर (आदान.size() < LZMA_STREAM_HEADER_SIZE)
    परत createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        आदान.size(), माप_प्रकार(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, आदान.take_back(LZMA_STREAM_HEADER_SIZE).data());
  जर (Ret != LZMA_OK)
    परत createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  जर (आदान.size() < MinSize)
    परत createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        आदान.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  माप_प्रकार InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      आदान.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, आदान.size());
  जर (Ret != LZMA_OK)
    परत createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  परत lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> आदान,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(आदान);
  जर (!UncompressedSize)
    परत UncompressedSize.takeError();

  जर (*UncompressedSize > std::numeric_limits<माप_प्रकार>::max())
    परत createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<माप_प्रकार>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  माप_प्रकार InPos = 0;
  माप_प्रकार OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, आदान.data(), &InPos, आदान.size(),
      Output.data(), &OutPos, Output.size());
  जर (Ret != LZMA_OK) {
    Output.clear();
    परत createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  परत Error::success();
}

#अन्यथा

तार्किक xz::isAvailable() { परत असत्य; }
Error xz::decompress(ArrayRef<uint8_t> आदान,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
