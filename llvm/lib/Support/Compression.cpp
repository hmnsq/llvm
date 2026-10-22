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

#ضمن "llvm/Support/Compression.h"
#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/ADT/SmallVector.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن <limits>
#ضمن <optional>
#إذا LLVM_ENABLE_ZLIB
#ضمن <zlib.h>
#endif
#إذا LLVM_ENABLE_ZSTD
#ضمن <zstd.h>
#endif
#إذا LLVM_ENABLE_LZMA
#ضمن <lzma.h>
#endif

using نطاق llvm;
using نطاق llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
ساكن منطقي isZlibHeader(ArrayRef<uint8_t> كىرگۈزۈش) {
  إذا (كىرگۈزۈش.size() < 2)
    أرجع خطأ;
  unsigned CMF = كىرگۈزۈش[0];
  unsigned FLG = كىرگۈزۈش[1];
  إذا ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    أرجع خطأ;
  أرجع (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
ساكن منطقي isZstdMagic(ArrayRef<uint8_t> كىرگۈزۈش) {
  ساكن constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  أرجع كىرگۈزۈش.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
ساكن std::optional<Format> identifyFormat(ArrayRef<uint8_t> كىرگۈزۈش) {
  إذا (isZstdMagic(كىرگۈزۈش))
    أرجع Format::Zstd;
  إذا (isZlibHeader(كىرگۈزۈش))
    أرجع Format::Zlib;
  أرجع std::nullopt;
}

ثابت حرف *compression::getReasonIfUnsupported(compression::Format F) {
  بدل (F) {
  حالة compression::Format::Zlib:
    إذا (zlib::isAvailable())
      أرجع nullptr;
    أرجع "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  حالة compression::Format::Zstd:
    إذا (zstd::isAvailable())
      أرجع nullptr;
    أرجع "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ثابت حرف *compression::getReasonIfUnsupported(ArrayRef<uint8_t> كىرگۈزۈش) {
  إذا (std::optional<Format> F = identifyFormat(كىرگۈزۈش))
    أرجع getReasonIfUnsupported(*F);
  أرجع "unknown compression format";
}

فراغ compression::compress(Params P, ArrayRef<uint8_t> كىرگۈزۈش,
                           SmallVectorImpl<uint8_t> &Output) {
  بدل (P.format) {
  حالة compression::Format::Zlib:
    zlib::compress(كىرگۈزۈش, Output, P.level);
    اكسر;
  حالة compression::Format::Zstd:
    zstd::compress(كىرگۈزۈش, Output, P.level, P.zstdEnableLdm);
    اكسر;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> كىرگۈزۈش,
                              uint8_t *Output, size_t UncompressedSize) {
  بدل (formatFor(T)) {
  حالة compression::Format::Zlib:
    أرجع zlib::decompress(كىرگۈزۈش, Output, UncompressedSize);
  حالة compression::Format::Zstd:
    أرجع zstd::decompress(كىرگۈزۈش, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> كىرگۈزۈش,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  بدل (F) {
  حالة compression::Format::Zlib:
    أرجع zlib::decompress(كىرگۈزۈش, Output, UncompressedSize);
  حالة compression::Format::Zstd:
    أرجع zstd::decompress(كىرگۈزۈش, Output, UncompressedSize);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> كىرگۈزۈش,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  أرجع decompress(formatFor(T), كىرگۈزۈش, Output, UncompressedSize);
}

Error compression::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                              SmallVectorImpl<uint8_t> &Output,
                              size_t UncompressedSize) {
  std::optional<Format> F = identifyFormat(كىرگۈزۈش);
  إذا (ثابت حرف *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    أرجع createStringError(Reason);
  أرجع decompress(*F, كىرگۈزۈش, Output, UncompressedSize);
}

#إذا LLVM_ENABLE_ZLIB

ساكن StringRef convertZlibCodeToString(صحيح Code) {
  بدل (Code) {
  حالة Z_MEM_ERROR:
    أرجع "zlib error: Z_MEM_ERROR";
  حالة Z_BUF_ERROR:
    أرجع "zlib error: Z_BUF_ERROR";
  حالة Z_STREAM_ERROR:
    أرجع "zlib error: Z_STREAM_ERROR";
  حالة Z_DATA_ERROR:
    أرجع "zlib error: Z_DATA_ERROR";
  حالة Z_OK:
  افتراضي:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

منطقي zlib::isAvailable() { أرجع true; }

فراغ zlib::compress(ArrayRef<uint8_t> كىرگۈزۈش,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحيح Level) {
  unsigned long CompressedSize = ::compressBound(كىرگۈزۈش.size());
  CompressedBuffer.resize_for_overwrite(CompressedSize);
  صحيح Res = ::compress2((Bytef *)CompressedBuffer.data(), &CompressedSize,
                        (ثابت Bytef *)كىرگۈزۈش.data(), كىرگۈزۈش.size(), Level);
  إذا (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  أكد(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  إذا (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zlib::decompress(ArrayRef<uint8_t> كىرگۈزۈش, uint8_t *Output,
                       size_t &UncompressedSize) {
  صحيح Res = ::uncompress((Bytef *)Output, (uLongf *)&UncompressedSize,
                         (ثابت Bytef *)كىرگۈزۈش.data(), كىرگۈزۈش.size());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  أرجع Res ? make_error<StringError>(convertZlibCodeToString(Res),
                                       inconvertibleErrorCode())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zlib::decompress(كىرگۈزۈش, Output.data(), UncompressedSize);
  إذا (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  أرجع E;
}

#وإلا
منطقي zlib::isAvailable() { أرجع خطأ; }
فراغ zlib::compress(ArrayRef<uint8_t> كىرگۈزۈش,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحيح Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> كىرگۈزۈش, uint8_t *UncompressedBuffer,
                       size_t &UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                       SmallVectorImpl<uint8_t> &UncompressedBuffer,
                       size_t UncompressedSize) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#endif

#إذا LLVM_ENABLE_ZSTD

منطقي zstd::isAvailable() { أرجع true; }

#ضمن <zstd.h> // Ensure ZSTD library is included

فراغ zstd::compress(ArrayRef<uint8_t> كىرگۈزۈش,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحيح Level,
                    منطقي EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  إذا (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  إذا (ZSTD_isError(ZSTD_CCtx_setParameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  إذا (ZSTD_isError(
          ZSTD_CCtx_setParameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  unsigned long CompressedBufferSize = ZSTD_compressBound(كىرگۈزۈش.size());
  CompressedBuffer.resize_for_overwrite(CompressedBufferSize);

  size_t ثابت CompressedSize =
      ZSTD_compress2(Cctx, CompressedBuffer.data(), CompressedBufferSize,
                     كىرگۈزۈش.data(), كىرگۈزۈش.size());

  ZSTD_freeCCtx(Cctx);

  إذا (ZSTD_isError(CompressedSize))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(CompressedBuffer.data(), CompressedSize);
  إذا (CompressedSize < CompressedBuffer.size())
    CompressedBuffer.truncate(CompressedSize);
}

Error zstd::decompress(ArrayRef<uint8_t> كىرگۈزۈش, uint8_t *Output,
                       size_t &UncompressedSize) {
  ثابت size_t Res = ::ZSTD_decompress(
      Output, UncompressedSize, (ثابت uint8_t *)كىرگۈزۈش.data(), كىرگۈزۈش.size());
  UncompressedSize = Res;
  إذا (ZSTD_isError(Res))
    أرجع make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertibleErrorCode());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, UncompressedSize);
  أرجع Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  Output.resize_for_overwrite(UncompressedSize);
  Error E = zstd::decompress(كىرگۈزۈش, Output.data(), UncompressedSize);
  إذا (UncompressedSize < Output.size())
    Output.truncate(UncompressedSize);
  أرجع E;
}

#وإلا
منطقي zstd::isAvailable() { أرجع خطأ; }
فراغ zstd::compress(ArrayRef<uint8_t> كىرگۈزۈش,
                    SmallVectorImpl<uint8_t> &CompressedBuffer, صحيح Level,
                    منطقي EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> كىرگۈزۈش, uint8_t *Output,
                       size_t &UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                       SmallVectorImpl<uint8_t> &Output,
                       size_t UncompressedSize) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#endif

#إذا LLVM_ENABLE_LZMA

منطقي xz::isAvailable() { أرجع true; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
ساكن ثابت حرف *convertLZMACodeToString(lzma_ret Code) {
  بدل (Code) {
  حالة LZMA_STREAM_END:
    أرجع "lzma error: LZMA_STREAM_END";
  حالة LZMA_NO_CHECK:
    أرجع "lzma error: LZMA_NO_CHECK";
  حالة LZMA_UNSUPPORTED_CHECK:
    أرجع "lzma error: LZMA_UNSUPPORTED_CHECK";
  حالة LZMA_GET_CHECK:
    أرجع "lzma error: LZMA_GET_CHECK";
  حالة LZMA_MEM_ERROR:
    أرجع "lzma error: LZMA_MEM_ERROR";
  حالة LZMA_MEMLIMIT_ERROR:
    أرجع "lzma error: LZMA_MEMLIMIT_ERROR";
  حالة LZMA_FORMAT_ERROR:
    أرجع "lzma error: LZMA_FORMAT_ERROR";
  حالة LZMA_OPTIONS_ERROR:
    أرجع "lzma error: LZMA_OPTIONS_ERROR";
  حالة LZMA_DATA_ERROR:
    أرجع "lzma error: LZMA_DATA_ERROR";
  حالة LZMA_BUF_ERROR:
    أرجع "lzma error: LZMA_BUF_ERROR";
  حالة LZMA_PROG_ERROR:
    أرجع "lzma error: LZMA_PROG_ERROR";
  افتراضي:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
ساكن Expected<uint64_t> getUncompressedSize(ArrayRef<uint8_t> كىرگۈزۈش) {
  إذا (كىرگۈزۈش.size() < LZMA_STREAM_HEADER_SIZE)
    أرجع createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        كىرگۈزۈش.size(), size_t(LZMA_STREAM_HEADER_SIZE));

  // Decode the xz footer.
  lzma_stream_flags FooterFlags{};
  lzma_ret Ret = lzma_stream_footer_decode(
      &FooterFlags, كىرگۈزۈش.take_back(LZMA_STREAM_HEADER_SIZE).data());
  إذا (Ret != LZMA_OK)
    أرجع createStringError("lzma_stream_footer_decode()=%s",
                             convertLZMACodeToString(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t MinSize = FooterFlags.backward_size + 2 * LZMA_STREAM_HEADER_SIZE;
  إذا (كىرگۈزۈش.size() < MinSize)
    أرجع createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        كىرگۈزۈش.size(), MinSize);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_index *Index = nullptr;
  llvm::scope_exit FreeIndex([&] { lzma_index_end(Index, nullptr); });
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  Ret = lzma_index_buffer_decode(
      &Index, &MemLimit, nullptr,
      كىرگۈزۈش.take_back(LZMA_STREAM_HEADER_SIZE + FooterFlags.backward_size)
          .data(),
      &InPos, كىرگۈزۈش.size());
  إذا (Ret != LZMA_OK)
    أرجع createStringError("lzma_index_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));

  أرجع lzma_index_uncompressed_size(Index);
}

Error xz::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                     SmallVectorImpl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> UncompressedSize = getUncompressedSize(كىرگۈزۈش);
  إذا (!UncompressedSize)
    أرجع UncompressedSize.takeError();

  إذا (*UncompressedSize > std::numeric_limits<size_t>::max())
    أرجع createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *UncompressedSize);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*UncompressedSize));
  uint64_t MemLimit = UINT64_MAX;
  size_t InPos = 0;
  size_t OutPos = 0;
  lzma_ret Ret = lzma_stream_buffer_decode(
      &MemLimit, /*flags=*/0, nullptr, كىرگۈزۈش.data(), &InPos, كىرگۈزۈش.size(),
      Output.data(), &OutPos, Output.size());
  إذا (Ret != LZMA_OK) {
    Output.clear();
    أرجع createStringError("lzma_stream_buffer_decode()=%s",
                             convertLZMACodeToString(Ret));
  }

  أرجع Error::success();
}

#وإلا

منطقي xz::isAvailable() { أرجع خطأ; }
Error xz::decompress(ArrayRef<uint8_t> كىرگۈزۈش,
                     SmallVectorImpl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#endif
