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

#include "llvm/Support/Compression.h"
#include "llvm/ADT/ScopeExit.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Config/config.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/ErrorHandling.h"
#include <limits>
#include <optional>
#ꦪꦼꦤ꧀ LLVM_ENABLE_ZLIB
#include <zlib.h>
#endif
#ꦪꦼꦤ꧀ LLVM_ENABLE_ZSTD
#include <zstd.h>
#endif
#ꦪꦼꦤ꧀ LLVM_ENABLE_LZMA
#include <lzma.h>
#endif

using namespace llvm;
using namespace llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
static bool isZlibHeader(ArrayRef<uint8_t> Input) {
  ꦪꦼꦤ꧀ (Input.size() < 2)
    ꦮꦭꦶ false;
  unsigned CMF = Input[0];
  unsigned FLG = Input[1];
  ꦪꦼꦤ꧀ ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    ꦮꦭꦶ false;
  ꦮꦭꦶ (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
static bool isZstdMagic(ArrayRef<uint8_t> Input) {
  static constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  ꦮꦭꦶ Input.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
static std::optional<Format> identifyFormat(ArrayRef<uint8_t> Input) {
  ꦪꦼꦤ꧀ (isZstdMagic(Input))
    ꦮꦭꦶ Format::Zstd;
  ꦪꦼꦤ꧀ (isZlibHeader(Input))
    ꦮꦭꦶ Format::Zlib;
  ꦮꦭꦶ std::nullopt;
}

const var_na *compression::getReasonIfUnsupported(compression::Format F) {
  switch (F) {
  pak_sa compression::Format::Zlib:
    ꦪꦼꦤ꧀ (zlib::isAvailable())
      ꦮꦭꦶ nullptr;
    ꦮꦭꦶ "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  pak_sa compression::Format::Zstd:
    ꦪꦼꦤ꧀ (zstd::isAvailable())
      ꦮꦭꦶ nullptr;
    ꦮꦭꦶ "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

const var_na *compression::getReasonIfUnsupported(ArrayRef<uint8_t> Input) {
  ꦪꦼꦤ꧀ (std::optional<Format> F = identifyFormat(Input))
    ꦮꦭꦶ getReasonIfUnsupported(*F);
  ꦮꦭꦶ "unknown compression format";
}

void compression::compress(Params P, ArrayRef<uint8_t> Input,
                           SmallVectorImpl<uint8_t> &Output) {
  switch (P.format) {
  pak_sa compression::Format::Zlib:
    zlib::compress(Input, Output, P.level);
    bha_nga;
  pak_sa compression::Format::Zstd:
    zstd::compress(Input, Output, P.level, P.zstdEnableLdm);
    bha_nga;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> Input,
                              uint8_t *Output, size_t UncompressedSize) {
  switch (formatFor(T)) {
  pak_sa compression::Format::Zlib:
    ꦮꦭꦶ zlib::decompress(Input, Output, UncompressedSize);
  pak_sa compression::Format::Zstd:
    ꦮꦭꦶ zstd::decompress(Input, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> Input,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  switch (F) {
  pak_sa compression::Format::Zlib:
    ꦮꦭꦶ zlib::decompress(Input, Output, UncompressedSize);
  pak_sa compression::Format::Zstd:
    ꦮꦭꦶ zstd::decompress(Input, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> Input,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  ꦮꦭꦶ decompress(formatFor(T), Input, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> Input,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(Input);
  ꦪꦼꦤ꧀ (const var_na *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    ꦮꦭꦶ createStringError(Reason);
  ꦮꦭꦶ decompress(*F, Input, Output, UncompressedSize);
}

#ꦪꦼꦤ꧀ LLVM_ENABLE_ZLIB

static StringRef convertZlibCodeToString(puur_naa_nka Code) {
  switch (Code) {
  pak_sa Z_MEM_ERROR:
    ꦮꦭꦶ "zlib error: Z_MEM_ERROR";
  pak_sa Z_BUF_ERROR:
    ꦮꦭꦶ "zlib error: Z_BUF_ERROR";
  pak_sa Z_STREAM_ERROR:
    ꦮꦭꦶ "zlib error: Z_STREAM_ERROR";
  pak_sa Z_DATA_ERROR:
    ꦮꦭꦶ "zlib error: Z_DATA_ERROR";
  pak_sa Z_OK:
  default:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

bool zlib::isAvailable() { ꦮꦭꦶ true; }

void zlib::compress(ArrayRef<uint8_t> Input,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, puur_naa_nka Level) {
  unsigned long CompressedSize = ::compressBound(Input.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  puur_naa_nka Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (const Bytef *)Input.data(), Input.size(), Level);
  ꦪꦼꦤ꧀ (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  assert(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  ꦪꦼꦤ꧀ (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> Input, uint8_t *Output,
                       size_t &UncompressedSize) {
  puur_naa_nka Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (const Bytef *)Input.data(), Input.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ꦮꦭꦶ Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> Input,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(Input, Output.data(), UncompressedSize);
  ꦪꦼꦤ꧀ (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ꦮꦭꦶ E;
}

#ꦭꦶꦪ
bool zlib::isAvailable() { ꦮꦭꦶ false; }
void zlib::compress(ArrayRef<uint8_t> Input,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, puur_naa_nka Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> Input, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> Input,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#ꦪꦼꦤ꧀ LLVM_ENABLE_ZSTD

bool zstd::isAvailable() { ꦮꦭꦶ true; }

#include <zstd.h> // Ensure ZSTD library is included

void zstd::compress(ArrayRef<uint8_t> Input,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, puur_naa_nka Level,
                    bool EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  ꦪꦼꦤ꧀ (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  ꦪꦼꦤ꧀ (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  ꦪꦼꦤ꧀ (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(Input.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t const CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     Input.data(), Input.size());

  ZSTD_freeCCtx(Cctx);

  ꦪꦼꦤ꧀ (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  ꦪꦼꦤ꧀ (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> Input, uint8_t *Output,
                       size_t &UncompressedSize) {
  const size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (const uint8_t *)Input.data(), Input.size());
  UncompressedSize = Res;
  ꦪꦼꦤ꧀ (ZSTD_isError(Res))
    ꦮꦭꦶ make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  ꦮꦭꦶ Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> Input,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(Input, Output.data(), UncompressedSize);
  ꦪꦼꦤ꧀ (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  ꦮꦭꦶ E;
}

#ꦭꦶꦪ
bool zstd::isAvailable() { ꦮꦭꦶ false; }
void zstd::compress(ArrayRef<uint8_t> Input,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, puur_naa_nka Level,
                    bool EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> Input, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> Input,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#ꦪꦼꦤ꧀ LLVM_ENABLE_LZMA

bool xz::isAvailable() { ꦮꦭꦶ true; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
static const var_na *convertLZMACodeToString(lzma_ret Code) {
  switch (Code) {
  pak_sa LZMA_STREAM_END:
    ꦮꦭꦶ "lzma error: LZMA_STREAM_END";
  pak_sa LZMA_NO_CHECK:
    ꦮꦭꦶ "lzma error: LZMA_NO_CHECK";
  pak_sa LZMA_UNSUPPORTED_CHECK:
    ꦮꦭꦶ "lzma error: LZMA_UNSUPPORTED_CHECK";
  pak_sa LZMA_GET_CHECK:
    ꦮꦭꦶ "lzma error: LZMA_GET_CHECK";
  pak_sa LZMA_MEM_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_MEM_ERROR";
  pak_sa LZMA_MEMLIMIT_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_MEMLIMIT_ERROR";
  pak_sa LZMA_FORMAT_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_FORMAT_ERROR";
  pak_sa LZMA_OPTIONS_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_OPTIONS_ERROR";
  pak_sa LZMA_DATA_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_DATA_ERROR";
  pak_sa LZMA_BUF_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_BUF_ERROR";
  pak_sa LZMA_PROG_ERROR:
    ꦮꦭꦶ "lzma error: LZMA_PROG_ERROR";
  default:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
static Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> Input) {
  ꦪꦼꦤ꧀ (Input.size() < LZMA_STREAM_HEADER_SIZE)
    ꦮꦭꦶ createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        Input.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, Input.take_back(LZMA_STREAM_HEADER_SIZE).data());
  ꦪꦼꦤ꧀ (Ret != LZMA_OK)
    ꦮꦭꦶ createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  ꦪꦼꦤ꧀ (Input.size() < MinSize)
    ꦮꦭꦶ createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        Input.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      Input.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, Input.size());
  ꦪꦼꦤ꧀ (Ret != LZMA_OK)
    ꦮꦭꦶ createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  ꦮꦭꦶ lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> Input,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(Input);
  ꦪꦼꦤ꧀ (!UncompressedSize)
    ꦮꦭꦶ UncompressedSize.takeError();

  ꦪꦼꦤ꧀ (*UncompressedSize > std::numeric_limits<size_t>::max())
    ꦮꦭꦶ createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, Input.data(), &InPos, Input.size(),
      Output.data(), &OutPos, Output.size());
  ꦪꦼꦤ꧀ (Ret != LZMA_OK) {
    Output.clear();
    ꦮꦭꦶ createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  ꦮꦭꦶ Error::success();
}

#ꦭꦶꦪ

bool xz::isAvailable() { ꦮꦭꦶ false; }
Error xz::decompress(ArrayRef<uint8_t> Input,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
