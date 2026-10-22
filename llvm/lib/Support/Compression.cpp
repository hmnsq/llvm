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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compression.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ScopeExit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallVector.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Compiler.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorHandling.h"
#ଅନ୍ତର୍ଭାଵ <limits>
#ଅନ୍ତର୍ଭାଵ <optional>
#ଯଦି LLVM_ENABLE_ZLIB
#ଅନ୍ତର୍ଭାଵ <zlib.h>
#endif
#ଯଦି LLVM_ENABLE_ZSTD
#ଅନ୍ତର୍ଭାଵ <zstd.h>
#endif
#ଯଦି LLVM_ENABLE_LZMA
#ଅନ୍ତର୍ଭାଵ <lzma.h>
#endif

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ସ୍ଥିର ତାର୍କିକ isZlibHeader(ArrayRef<uint8_t> ନିବେଶ) {
  ଯଦି (ନିବେଶ.size() < 2)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  unsigned CMF = ନିବେଶ[0];
  unsigned FLG = ନିବେଶ[1];
  ଯଦି ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଫେରନ୍ତୁ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ସ୍ଥିର ତାର୍କିକ isZstdMagic(ArrayRef<uint8_t> ନିବେଶ) {
  ସ୍ଥିର constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ଫେରନ୍ତୁ ନିବେଶ.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ସ୍ଥିର std::optional<Format> identifyFormat(ArrayRef<uint8_t> ନିବେଶ) {
  ଯଦି (isZstdMagic(ନିବେଶ))
    ଫେରନ୍ତୁ Format::Zstd;
  ଯଦି (isZlibHeader(ନିବେଶ))
    ଫେରନ୍ତୁ Format::Zlib;
  ଫେରନ୍ତୁ std::nullopt;
}

ଅଚଳ ଅକ୍ଷର *compression::getReasonIfUnsupported(compression::Format F) {
  ବାଛ (F) {
  ସ୍ଥିତି compression::Format::Zlib:
    ଯଦି (zlib::isAvailable())
      ଫେରନ୍ତୁ nullptr;
    ଫେରନ୍ତୁ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  ସ୍ଥିତି compression::Format::Zstd:
    ଯଦି (zstd::isAvailable())
      ଫେରନ୍ତୁ nullptr;
    ଫେରନ୍ତୁ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ଅଚଳ ଅକ୍ଷର *compression::getReasonIfUnsupported(ArrayRef<uint8_t> ନିବେଶ) {
  ଯଦି (std::optional<Format> F = identifyFormat(ନିବେଶ))
    ଫେରନ୍ତୁ getReasonIfUnsupported(*F);
  ଫେରନ୍ତୁ "unknown compression format";
}

ଶୂନ୍ୟ compression::compress(Params P, ArrayRef<uint8_t> ନିବେଶ,
                           SmallVectorImpl<uint8_t> &Output) {
  ବାଛ (P.format) {
  ସ୍ଥିତି compression::Format::Zlib:
    zlib::compress(ନିବେଶ, Output, P.level);
    ଭାଙ୍ଗ;
  ସ୍ଥିତି compression::Format::Zstd:
    zstd::compress(ନିବେଶ, Output, P.level, P.zstdEnableLdm);
    ଭାଙ୍ଗ;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ନିବେଶ,
                              uint8_t *Output, size_t UncompressedSize) {
  ବାଛ (formatFor(T)) {
  ସ୍ଥିତି compression::Format::Zlib:
    ଫେରନ୍ତୁ zlib::decompress(ନିବେଶ, Output, UncompressedSize);
  ସ୍ଥିତି compression::Format::Zstd:
    ଫେରନ୍ତୁ zstd::decompress(ନିବେଶ, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> ନିବେଶ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ବାଛ (F) {
  ସ୍ଥିତି compression::Format::Zlib:
    ଫେରନ୍ତୁ zlib::decompress(ନିବେଶ, Output, UncompressedSize);
  ସ୍ଥିତି compression::Format::Zstd:
    ଫେରନ୍ତୁ zstd::decompress(ନିବେଶ, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ନିବେଶ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ଫେରନ୍ତୁ decompress(formatFor(T), ନିବେଶ, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> ନିବେଶ,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(ନିବେଶ);
  ଯଦି (ଅଚଳ ଅକ୍ଷର *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ଫେରନ୍ତୁ createStringError(Reason);
  ଫେରନ୍ତୁ decompress(*F, ନିବେଶ, Output, UncompressedSize);
}

#ଯଦି LLVM_ENABLE_ZLIB

ସ୍ଥିର StringRef convertZlibCodeToString(ପୂର୍ଣ୍ଣସଂଖ୍ୟା Code) {
  ବାଛ (Code) {
  ସ୍ଥିତି Z_MEM_ERROR:
    ଫେରନ୍ତୁ "zlib error: Z_MEM_ERROR";
  ସ୍ଥିତି Z_BUF_ERROR:
    ଫେରନ୍ତୁ "zlib error: Z_BUF_ERROR";
  ସ୍ଥିତି Z_STREAM_ERROR:
    ଫେରନ୍ତୁ "zlib error: Z_STREAM_ERROR";
  ସ୍ଥିତି Z_DATA_ERROR:
    ଫେରନ୍ତୁ "zlib error: Z_DATA_ERROR";
  ସ୍ଥିତି Z_OK:
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

ତାର୍କିକ zlib::isAvailable() { ଫେରନ୍ତୁ ସତ୍ୟ; }

ଶୂନ୍ୟ zlib::compress(ArrayRef<uint8_t> ନିବେଶ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Level) {
  unsigned long CompressedSize = ::compressBound(ନିବେଶ.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ଅଚଳ Bytef *)ନିବେଶ.data(), ନିବେଶ.size(), Level);
  ଯଦି (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  ପ୍ରତିଜ୍ଞା(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  ଯଦି (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> ନିବେଶ, uint8_t *Output,
                       size_t &UncompressedSize) {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ଅଚଳ Bytef *)ନିବେଶ.data(), ନିବେଶ.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ଫେରନ୍ତୁ Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> ନିବେଶ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(ନିବେଶ, Output.data(), UncompressedSize);
  ଯଦି (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ଫେରନ୍ତୁ E;
}

#ଅନ୍ୟଥା
ତାର୍କିକ zlib::isAvailable() { ଫେରନ୍ତୁ ମିଥ୍ୟା; }
ଶୂନ୍ୟ zlib::compress(ArrayRef<uint8_t> ନିବେଶ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ନିବେଶ, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ନିବେଶ,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#ଯଦି LLVM_ENABLE_ZSTD

ତାର୍କିକ zstd::isAvailable() { ଫେରନ୍ତୁ ସତ୍ୟ; }

#ଅନ୍ତର୍ଭାଵ <zstd.h> // Ensure ZSTD library is included

ଶୂନ୍ୟ zstd::compress(ArrayRef<uint8_t> ନିବେଶ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Level,
                    ତାର୍କିକ EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  ଯଦି (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  ଯଦି (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  ଯଦି (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(ନିବେଶ.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ଅଚଳ CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     ନିବେଶ.data(), ନିବେଶ.size());

  ZSTD_freeCCtx(Cctx);

  ଯଦି (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  ଯଦି (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> ନିବେଶ, uint8_t *Output,
                       size_t &UncompressedSize) {
  ଅଚଳ size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ଅଚଳ uint8_t *)ନିବେଶ.data(), ନିବେଶ.size());
  UncompressedSize = Res;
  ଯଦି (ZSTD_isError(Res))
    ଫେରନ୍ତୁ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ଫେରନ୍ତୁ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> ନିବେଶ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(ନିବେଶ, Output.data(), UncompressedSize);
  ଯଦି (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ଫେରନ୍ତୁ E;
}

#ଅନ୍ୟଥା
ତାର୍କିକ zstd::isAvailable() { ଫେରନ୍ତୁ ମିଥ୍ୟା; }
ଶୂନ୍ୟ zstd::compress(ArrayRef<uint8_t> ନିବେଶ,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Level,
                    ତାର୍କିକ EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ନିବେଶ, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ନିବେଶ,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#ଯଦି LLVM_ENABLE_LZMA

ତାର୍କିକ xz::isAvailable() { ଫେରନ୍ତୁ ସତ୍ୟ; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *convertLZMACodeToString(lzma_ret Code) {
  ବାଛ (Code) {
  ସ୍ଥିତି LZMA_STREAM_END:
    ଫେରନ୍ତୁ "lzma error: LZMA_STREAM_END";
  ସ୍ଥିତି LZMA_NO_CHECK:
    ଫେରନ୍ତୁ "lzma error: LZMA_NO_CHECK";
  ସ୍ଥିତି LZMA_UNSUPPORTED_CHECK:
    ଫେରନ୍ତୁ "lzma error: LZMA_UNSUPPORTED_CHECK";
  ସ୍ଥିତି LZMA_GET_CHECK:
    ଫେରନ୍ତୁ "lzma error: LZMA_GET_CHECK";
  ସ୍ଥିତି LZMA_MEM_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_MEM_ERROR";
  ସ୍ଥିତି LZMA_MEMLIMIT_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_MEMLIMIT_ERROR";
  ସ୍ଥିତି LZMA_FORMAT_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_FORMAT_ERROR";
  ସ୍ଥିତି LZMA_OPTIONS_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_OPTIONS_ERROR";
  ସ୍ଥିତି LZMA_DATA_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_DATA_ERROR";
  ସ୍ଥିତି LZMA_BUF_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_BUF_ERROR";
  ସ୍ଥିତି LZMA_PROG_ERROR:
    ଫେରନ୍ତୁ "lzma error: LZMA_PROG_ERROR";
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ସ୍ଥିର Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> ନିବେଶ) {
  ଯଦି (ନିବେଶ.size() < LZMA_STREAM_HEADER_SIZE)
    ଫେରନ୍ତୁ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        ନିବେଶ.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, ନିବେଶ.take_back(LZMA_STREAM_HEADER_SIZE).data());
  ଯଦି (Ret != LZMA_OK)
    ଫେରନ୍ତୁ createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  ଯଦି (ନିବେଶ.size() < MinSize)
    ଫେରନ୍ତୁ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        ନିବେଶ.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      ନିବେଶ.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, ନିବେଶ.size());
  ଯଦି (Ret != LZMA_OK)
    ଫେରନ୍ତୁ createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  ଫେରନ୍ତୁ lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> ନିବେଶ,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(ନିବେଶ);
  ଯଦି (!UncompressedSize)
    ଫେରନ୍ତୁ UncompressedSize.takeError();

  ଯଦି (*UncompressedSize > std::numeric_limits<size_t>::max())
    ଫେରନ୍ତୁ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, ନିବେଶ.data(), &InPos, ନିବେଶ.size(),
      Output.data(), &OutPos, Output.size());
  ଯଦି (Ret != LZMA_OK) {
    Output.clear();
    ଫେରନ୍ତୁ createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  ଫେରନ୍ତୁ Error::success();
}

#ଅନ୍ୟଥା

ତାର୍କିକ xz::isAvailable() { ଫେରନ୍ତୁ ମିଥ୍ୟା; }
Error xz::decompress(ArrayRef<uint8_t> ନିବେଶ,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
