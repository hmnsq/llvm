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

#അന്തര്ഭാവ "llvm/Support/Compression.h"
#അന്തര്ഭാവ "llvm/ADT/ScopeExit.h"
#അന്തര്ഭാവ "llvm/ADT/SmallVector.h"
#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/Compiler.h"
#അന്തര്ഭാവ "llvm/Support/Error.h"
#അന്തര്ഭാവ "llvm/Support/ErrorHandling.h"
#അന്തര്ഭാവ <limits>
#അന്തര്ഭാവ <optional>
#എങ്കിൽ LLVM_ENABLE_ZLIB
#അന്തര്ഭാവ <zlib.h>
#endif
#എങ്കിൽ LLVM_ENABLE_ZSTD
#അന്തര്ഭാവ <zstd.h>
#endif
#എങ്കിൽ LLVM_ENABLE_LZMA
#അന്തര്ഭാവ <lzma.h>
#endif

using നാമാകാശ llvm;
using നാമാകാശ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
സ്ഥിതം തർക്കം isZlibHeader(ArrayRef<uint8_t> നിവേശം) {
  എങ്കിൽ (നിവേശം.size() < 2)
    തിരികെ അസത്യം;
  unsigned CMF = നിവേശം[0];
  unsigned FLG = നിവേശം[1];
  എങ്കിൽ ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    തിരികെ അസത്യം;
  തിരികെ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
സ്ഥിതം തർക്കം isZstdMagic(ArrayRef<uint8_t> നിവേശം) {
  സ്ഥിതം constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  തിരികെ നിവേശം.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
സ്ഥിതം std::optional<Format> identifyFormat(ArrayRef<uint8_t> നിവേശം) {
  എങ്കിൽ (isZstdMagic(നിവേശം))
    തിരികെ Format::Zstd;
  എങ്കിൽ (isZlibHeader(നിവേശം))
    തിരികെ Format::Zlib;
  തിരികെ std::nullopt;
}

സ്ഥിരം അക്ഷരം *compression::getReasonIfUnsupported(compression::Format F) {
  തിരഞ്ഞെടുപ്പ് (F) {
  അവസ്ഥ compression::Format::Zlib:
    എങ്കിൽ (zlib::isAvailable())
      തിരികെ nullptr;
    തിരികെ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  അവസ്ഥ compression::Format::Zstd:
    എങ്കിൽ (zstd::isAvailable())
      തിരികെ nullptr;
    തിരികെ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

സ്ഥിരം അക്ഷരം *compression::getReasonIfUnsupported(ArrayRef<uint8_t> നിവേശം) {
  എങ്കിൽ (std::optional<Format> F = identifyFormat(നിവേശം))
    തിരികെ getReasonIfUnsupported(*F);
  തിരികെ "unknown compression format";
}

ശൂന്യം compression::compress(Params P, ArrayRef<uint8_t> നിവേശം,
                           SmallVectorImpl<uint8_t> &Output) {
  തിരഞ്ഞെടുപ്പ് (P.format) {
  അവസ്ഥ compression::Format::Zlib:
    zlib::compress(നിവേശം, Output, P.level);
    നിർത്തുക;
  അവസ്ഥ compression::Format::Zstd:
    zstd::compress(നിവേശം, Output, P.level, P.zstdEnableLdm);
    നിർത്തുക;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> നിവേശം,
                              uint8_t *Output, size_t UncompressedSize) {
  തിരഞ്ഞെടുപ്പ് (formatFor(T)) {
  അവസ്ഥ compression::Format::Zlib:
    തിരികെ zlib::decompress(നിവേശം, Output, UncompressedSize);
  അവസ്ഥ compression::Format::Zstd:
    തിരികെ zstd::decompress(നിവേശം, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> നിവേശം,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  തിരഞ്ഞെടുപ്പ് (F) {
  അവസ്ഥ compression::Format::Zlib:
    തിരികെ zlib::decompress(നിവേശം, Output, UncompressedSize);
  അവസ്ഥ compression::Format::Zstd:
    തിരികെ zstd::decompress(നിവേശം, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> നിവേശം,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  തിരികെ decompress(formatFor(T), നിവേശം, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> നിവേശം,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(നിവേശം);
  എങ്കിൽ (സ്ഥിരം അക്ഷരം *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    തിരികെ createStringError(Reason);
  തിരികെ decompress(*F, നിവേശം, Output, UncompressedSize);
}

#എങ്കിൽ LLVM_ENABLE_ZLIB

സ്ഥിതം StringRef convertZlibCodeToString(പൂർണ്ണസംഖ്യ Code) {
  തിരഞ്ഞെടുപ്പ് (Code) {
  അവസ്ഥ Z_MEM_ERROR:
    തിരികെ "zlib error: Z_MEM_ERROR";
  അവസ്ഥ Z_BUF_ERROR:
    തിരികെ "zlib error: Z_BUF_ERROR";
  അവസ്ഥ Z_STREAM_ERROR:
    തിരികെ "zlib error: Z_STREAM_ERROR";
  അവസ്ഥ Z_DATA_ERROR:
    തിരികെ "zlib error: Z_DATA_ERROR";
  അവസ്ഥ Z_OK:
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

തർക്കം zlib::isAvailable() { തിരികെ സത്യം; }

ശൂന്യം zlib::compress(ArrayRef<uint8_t> നിവേശം,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, പൂർണ്ണസംഖ്യ Level) {
  unsigned long CompressedSize = ::compressBound(നിവേശം.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  പൂർണ്ണസംഖ്യ Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (സ്ഥിരം Bytef *)നിവേശം.data(), നിവേശം.size(), Level);
  എങ്കിൽ (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  പ്രതിജ്ഞാ(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  എങ്കിൽ (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> നിവേശം, uint8_t *Output,
                       size_t &UncompressedSize) {
  പൂർണ്ണസംഖ്യ Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (സ്ഥിരം Bytef *)നിവേശം.data(), നിവേശം.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  തിരികെ Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> നിവേശം,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(നിവേശം, Output.data(), UncompressedSize);
  എങ്കിൽ (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  തിരികെ E;
}

#അല്ലെങ്കിൽ
തർക്കം zlib::isAvailable() { തിരികെ അസത്യം; }
ശൂന്യം zlib::compress(ArrayRef<uint8_t> നിവേശം,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, പൂർണ്ണസംഖ്യ Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> നിവേശം, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> നിവേശം,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#എങ്കിൽ LLVM_ENABLE_ZSTD

തർക്കം zstd::isAvailable() { തിരികെ സത്യം; }

#അന്തര്ഭാവ <zstd.h> // Ensure ZSTD library is included

ശൂന്യം zstd::compress(ArrayRef<uint8_t> നിവേശം,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, പൂർണ്ണസംഖ്യ Level,
                    തർക്കം EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  എങ്കിൽ (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  എങ്കിൽ (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  എങ്കിൽ (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(നിവേശം.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t സ്ഥിരം CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     നിവേശം.data(), നിവേശം.size());

  ZSTD_freeCCtx(Cctx);

  എങ്കിൽ (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  എങ്കിൽ (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> നിവേശം, uint8_t *Output,
                       size_t &UncompressedSize) {
  സ്ഥിരം size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (സ്ഥിരം uint8_t *)നിവേശം.data(), നിവേശം.size());
  UncompressedSize = Res;
  എങ്കിൽ (ZSTD_isError(Res))
    തിരികെ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  തിരികെ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> നിവേശം,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(നിവേശം, Output.data(), UncompressedSize);
  എങ്കിൽ (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  തിരികെ E;
}

#അല്ലെങ്കിൽ
തർക്കം zstd::isAvailable() { തിരികെ അസത്യം; }
ശൂന്യം zstd::compress(ArrayRef<uint8_t> നിവേശം,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, പൂർണ്ണസംഖ്യ Level,
                    തർക്കം EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> നിവേശം, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> നിവേശം,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#എങ്കിൽ LLVM_ENABLE_LZMA

തർക്കം xz::isAvailable() { തിരികെ സത്യം; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
സ്ഥിതം സ്ഥിരം അക്ഷരം *convertLZMACodeToString(lzma_ret Code) {
  തിരഞ്ഞെടുപ്പ് (Code) {
  അവസ്ഥ LZMA_STREAM_END:
    തിരികെ "lzma error: LZMA_STREAM_END";
  അവസ്ഥ LZMA_NO_CHECK:
    തിരികെ "lzma error: LZMA_NO_CHECK";
  അവസ്ഥ LZMA_UNSUPPORTED_CHECK:
    തിരികെ "lzma error: LZMA_UNSUPPORTED_CHECK";
  അവസ്ഥ LZMA_GET_CHECK:
    തിരികെ "lzma error: LZMA_GET_CHECK";
  അവസ്ഥ LZMA_MEM_ERROR:
    തിരികെ "lzma error: LZMA_MEM_ERROR";
  അവസ്ഥ LZMA_MEMLIMIT_ERROR:
    തിരികെ "lzma error: LZMA_MEMLIMIT_ERROR";
  അവസ്ഥ LZMA_FORMAT_ERROR:
    തിരികെ "lzma error: LZMA_FORMAT_ERROR";
  അവസ്ഥ LZMA_OPTIONS_ERROR:
    തിരികെ "lzma error: LZMA_OPTIONS_ERROR";
  അവസ്ഥ LZMA_DATA_ERROR:
    തിരികെ "lzma error: LZMA_DATA_ERROR";
  അവസ്ഥ LZMA_BUF_ERROR:
    തിരികെ "lzma error: LZMA_BUF_ERROR";
  അവസ്ഥ LZMA_PROG_ERROR:
    തിരികെ "lzma error: LZMA_PROG_ERROR";
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
സ്ഥിതം Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> നിവേശം) {
  എങ്കിൽ (നിവേശം.size() < LZMA_STREAM_HEADER_SIZE)
    തിരികെ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        നിവേശം.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, നിവേശം.take_back(LZMA_STREAM_HEADER_SIZE).data());
  എങ്കിൽ (Ret != LZMA_OK)
    തിരികെ createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  എങ്കിൽ (നിവേശം.size() < MinSize)
    തിരികെ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        നിവേശം.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      നിവേശം.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, നിവേശം.size());
  എങ്കിൽ (Ret != LZMA_OK)
    തിരികെ createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  തിരികെ lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> നിവേശം,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(നിവേശം);
  എങ്കിൽ (!UncompressedSize)
    തിരികെ UncompressedSize.takeError();

  എങ്കിൽ (*UncompressedSize > std::numeric_limits<size_t>::max())
    തിരികെ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, നിവേശം.data(), &InPos, നിവേശം.size(),
      Output.data(), &OutPos, Output.size());
  എങ്കിൽ (Ret != LZMA_OK) {
    Output.clear();
    തിരികെ createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  തിരികെ Error::success();
}

#അല്ലെങ്കിൽ

തർക്കം xz::isAvailable() { തിരികെ അസത്യം; }
Error xz::decompress(ArrayRef<uint8_t> നിവേശം,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
