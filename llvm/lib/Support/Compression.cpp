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

#समावेश "llvm/Support/Compression.h"
#समावेश "llvm/ADT/ScopeExit.h"
#समावेश "llvm/ADT/SmallVector.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश <limits>
#समावेश <optional>
#अगर LLVM_ENABLE_ZLIB
#समावेश <zlib.h>
#पूर्ण_अगर
#अगर LLVM_ENABLE_ZSTD
#समावेश <zstd.h>
#पूर्ण_अगर
#अगर LLVM_ENABLE_LZMA
#समावेश <lzma.h>
#पूर्ण_अगर

using namespace llvm;
using namespace llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
अटल bool isZlibHeader(ArrayRef<uint8_t> निवेश) {
  अगर (निवेश.आकार_2() < 2)
    वापस false;
  अचिन्हित CMF = निवेश[0];
  अचिन्हित FLG = निवेश[1];
  अगर ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    वापस false;
  वापस (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
अटल bool isZstdMagic(ArrayRef<uint8_t> निवेश) {
  अटल constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  वापस निवेश.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
अटल std::optional<Format> identifyFormat(ArrayRef<uint8_t> निवेश) {
  अगर (isZstdMagic(निवेश))
    वापस Format::Zstd;
  अगर (isZlibHeader(निवेश))
    वापस Format::Zlib;
  वापस std::nullopt;
}

स्थिर अक्षर *compression::getReasonIfUnsupported(compression::Format F) {
  चयन (F) {
  हाल compression::Format::Zlib:
    अगर (zlib::isAvailable())
      वापस nullptr;
    वापस "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  हाल compression::Format::Zstd:
    अगर (zstd::isAvailable())
      वापस nullptr;
    वापस "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

स्थिर अक्षर *compression::getReasonIfUnsupported(ArrayRef<uint8_t> निवेश) {
  अगर (std::optional<Format> F = identifyFormat(निवेश))
    वापस getReasonIfUnsupported(*F);
  वापस "unknown compression format";
}

व्योम compression::compress(Params P, ArrayRef<uint8_t> निवेश,
                           SmallVectorImpl<uint8_t> &Output) {
  चयन (P.format) {
  हाल compression::Format::Zlib:
    zlib::compress(निवेश, Output, P.level);
    अवरोध;
  हाल compression::Format::Zstd:
    zstd::compress(निवेश, Output, P.level, P.zstdEnableLdm);
    अवरोध;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> निवेश,
                              uint8_t *Output, माप_प्रकार Uncompressed_आकार) {
  चयन (formatFor(T)) {
  हाल compression::Format::Zlib:
    वापस zlib::decompress(निवेश, Output, Uncompressed_आकार);
  हाल compression::Format::Zstd:
    वापस zstd::decompress(निवेश, Output, Uncompressed_आकार);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> निवेश,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार Uncompressed_आकार) {
  चयन (F) {
  हाल compression::Format::Zlib:
    वापस zlib::decompress(निवेश, Output, Uncompressed_आकार);
  हाल compression::Format::Zstd:
    वापस zstd::decompress(निवेश, Output, Uncompressed_आकार);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> निवेश,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार Uncompressed_आकार) {
  वापस decompress(formatFor(T), निवेश, Output, Uncompressed_आकार);
}

Error compression::decompress(ArrayRef<uint8_t> निवेश,
                              SmallVectorImpl<uint8_t> &Output,
                              माप_प्रकार Uncompressed_आकार) {
  std::optional<Format> F = identifyFormat(निवेश);
  अगर (स्थिर अक्षर *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    वापस createStringError(Reason);
  वापस decompress(*F, निवेश, Output, Uncompressed_आकार);
}

#अगर LLVM_ENABLE_ZLIB

अटल StringRef convertZlibCodeToString(पूर्णांक Code) {
  चयन (Code) {
  हाल Z_MEM_ERROR:
    वापस "zlib error: Z_MEM_ERROR";
  हाल Z_BUF_ERROR:
    वापस "zlib error: Z_BUF_ERROR";
  हाल Z_STREAM_ERROR:
    वापस "zlib error: Z_STREAM_ERROR";
  हाल Z_तथ्य_ERROR:
    वापस "zlib error: Z_DATA_ERROR";
  हाल Z_OK:
  शेष:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

bool zlib::isAvailable() { वापस true; }

व्योम zlib::compress(ArrayRef<uint8_t> निवेश,
                    SmallVectorImpl<uint8_t> &Compressed_बफर, पूर्णांक Level) {
  अचिन्हित दीर्घ Compressed_आकार = ::compressBound(निवेश.आकार_2());
  Compressed_बफर.resize_for_overwrite(Compressed_आकार);
  पूर्णांक Res = ::compress2((Bytef *)Compressed_बफर.तथ्य(), &Compressed_आकार,
                        (स्थिर Bytef *)निवेश.तथ्य(), निवेश.आकार_2(), Level);
  अगर (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  निश्चित(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_बफर.तथ्य(), Compressed_आकार);
  अगर (Compressed_आकार < Compressed_बफर.आकार_2())
    Compressed_बफर.truncate(Compressed_आकार);
}

Error zlib::decompress(ArrayRef<uint8_t> निवेश, uint8_t *Output,
                       माप_प्रकार &Uncompressed_आकार) {
  पूर्णांक Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_आकार,
                         (स्थिर Bytef *)निवेश.तथ्य(), निवेश.आकार_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_आकार);
  वापस Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> निवेश,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार Uncompressed_आकार) {
  Output.resize_for_overwrite(Uncompressed_आकार);
  Error E = zlib::decompress(निवेश, Output.तथ्य(), Uncompressed_आकार);
  अगर (Uncompressed_आकार < Output.आकार_2())
    Output.truncate(Uncompressed_आकार);
  वापस E;
}

#अन्यथा
bool zlib::isAvailable() { वापस false; }
व्योम zlib::compress(ArrayRef<uint8_t> निवेश,
                    SmallVectorImpl<uint8_t> &Compressed_बफर, पूर्णांक Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> निवेश, uint8_t *Uncompressed_बफर,
                       माप_प्रकार &Uncompressed_आकार) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> निवेश,
                       SmallVectorImpl<uint8_t> &Uncompressed_बफर,
                       माप_प्रकार Uncompressed_आकार) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#पूर्ण_अगर

#अगर LLVM_ENABLE_ZSTD

bool zstd::isAvailable() { वापस true; }

#समावेश <zstd.h> // Ensure ZSTD library is included

व्योम zstd::compress(ArrayRef<uint8_t> निवेश,
                    SmallVectorImpl<uint8_t> &Compressed_बफर, पूर्णांक Level,
                    bool EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  अगर (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  अगर (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  अगर (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  अचिन्हित दीर्घ Compressed_बफर_आकार = ZSTD_compressBound(निवेश.आकार_2());
  Compressed_बफर.resize_for_overwrite(Compressed_बफर_आकार);

  माप_प्रकार स्थिर Compressed_आकार =
      ZSTD_compress2(Cctx, Compressed_बफर.तथ्य(), Compressed_बफर_आकार,
                     निवेश.तथ्य(), निवेश.आकार_2());

  ZSTD_freeCCtx(Cctx);

  अगर (ZSTD_isError(Compressed_आकार))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_बफर.तथ्य(), Compressed_आकार);
  अगर (Compressed_आकार < Compressed_बफर.आकार_2())
    Compressed_बफर.truncate(Compressed_आकार);
}

Error zstd::decompress(ArrayRef<uint8_t> निवेश, uint8_t *Output,
                       माप_प्रकार &Uncompressed_आकार) {
  स्थिर माप_प्रकार Res = ::ZSTD_decompress(
      Output, Uncompressed_आकार, (स्थिर uint8_t *)निवेश.तथ्य(), निवेश.आकार_2());
  Uncompressed_आकार = Res;
  अगर (ZSTD_isError(Res))
    वापस make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_आकार);
  वापस Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> निवेश,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार Uncompressed_आकार) {
  Output.resize_for_overwrite(Uncompressed_आकार);
  Error E = zstd::decompress(निवेश, Output.तथ्य(), Uncompressed_आकार);
  अगर (Uncompressed_आकार < Output.आकार_2())
    Output.truncate(Uncompressed_आकार);
  वापस E;
}

#अन्यथा
bool zstd::isAvailable() { वापस false; }
व्योम zstd::compress(ArrayRef<uint8_t> निवेश,
                    SmallVectorImpl<uint8_t> &Compressed_बफर, पूर्णांक Level,
                    bool EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> निवेश, uint8_t *Output,
                       माप_प्रकार &Uncompressed_आकार) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> निवेश,
                       SmallVectorImpl<uint8_t> &Output,
                       माप_प्रकार Uncompressed_आकार) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#पूर्ण_अगर

#अगर LLVM_ENABLE_LZMA

bool xz::isAvailable() { वापस true; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
अटल स्थिर अक्षर *convertLZMACodeToString(lzma_ret Code) {
  चयन (Code) {
  हाल LZMA_STREAM_END:
    वापस "lzma error: LZMA_STREAM_END";
  हाल LZMA_NO_CHECK:
    वापस "lzma error: LZMA_NO_CHECK";
  हाल LZMA_UNSUPPORTED_CHECK:
    वापस "lzma error: LZMA_UNSUPPORTED_CHECK";
  हाल LZMA_GET_CHECK:
    वापस "lzma error: LZMA_GET_CHECK";
  हाल LZMA_MEM_ERROR:
    वापस "lzma error: LZMA_MEM_ERROR";
  हाल LZMA_MEMLIMIT_ERROR:
    वापस "lzma error: LZMA_MEMLIMIT_ERROR";
  हाल LZMA_FORMAT_ERROR:
    वापस "lzma error: LZMA_FORMAT_ERROR";
  हाल LZMA_OPTIONS_ERROR:
    वापस "lzma error: LZMA_OPTIONS_ERROR";
  हाल LZMA_तथ्य_ERROR:
    वापस "lzma error: LZMA_DATA_ERROR";
  हाल LZMA_BUF_ERROR:
    वापस "lzma error: LZMA_BUF_ERROR";
  हाल LZMA_PROG_ERROR:
    वापस "lzma error: LZMA_PROG_ERROR";
  शेष:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
अटल Expected<uint64_t> get_Uncompressed_आकार(ArrayRef<uint8_t> निवेश) {
  अगर (निवेश.आकार_2() < LZMA_STREAM_HEADER_आकार)
    वापस createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        निवेश.आकार_2(), माप_प्रकार(LZMA_STREAM_HEADER_आकार));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, निवेश.take_back(LZMA_STREAM_HEADER_आकार).तथ्य());
  अगर (Ret != LZMA_OK)
    वापस createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_आकार = FooterFlags.backward_आकार + 2 * LZMA_STREAM_HEADER_आकार;
  अगर (निवेश.आकार_2() < Min_आकार)
    वापस createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        निवेश.आकार_2(), Min_आकार);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_सूचकांक *सूचकांक_2 = nullptr;
  llvm::scope_exit Free_सूचकांक([&] { lzma_सूचकांक_end(सूचकांक_2, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  माप_प्रकार InPos = 0;
  Ret = lzma_सूचकांक_बफर_decode(
      &सूचकांक_2, &MemLimit, nullptr,
      निवेश.take_back(LZMA_STREAM_HEADER_आकार + FooterFlags.backward_आकार)
          .तथ्य(),
      &InPos, निवेश.आकार_2());
  अगर (Ret != LZMA_OK)
    वापस createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  वापस lzma_सूचकांक_uncompressed_आकार(सूचकांक_2);
}

Error xz::decompress(ArrayRef<uint8_t> निवेश,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.सफा();

  Expected<uint64_t> Uncompressed_आकार = get_Uncompressed_आकार(निवेश);
  अगर (!Uncompressed_आकार)
    वापस Uncompressed_आकार.takeError();

  अगर (*Uncompressed_आकार > std::numeric_limits<माप_प्रकार>::max())
    वापस createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_आकार);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<माप_प्रकार>(*Uncompressed_आकार));
  uint64_t MemLimit = UINT64_MAX;
  माप_प्रकार InPos = 0;
  माप_प्रकार OutPos = 0;
  lzma_ret Ret = lzma_stream_बफर_decode(
      &MemLimit, /*flags=*/0, nullptr, निवेश.तथ्य(), &InPos, निवेश.आकार_2(),
      Output.तथ्य(), &OutPos, Output.आकार_2());
  अगर (Ret != LZMA_OK) {
    Output.सफा();
    वापस createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  वापस Error::success();
}

#अन्यथा

bool xz::isAvailable() { वापस false; }
Error xz::decompress(ArrayRef<uint8_t> निवेश,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#पूर्ण_अगर
