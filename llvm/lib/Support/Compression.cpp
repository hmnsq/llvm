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

#అన్తర్భావ "llvm/Support/Compression.h"
#అన్తర్భావ "llvm/ADT/ScopeExit.h"
#అన్తర్భావ "llvm/ADT/SmallVector.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/Compiler.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ <limits>
#అన్తర్భావ <optional>
#ఒకవేళ LLVM_ENABLE_ZLIB
#అన్తర్భావ <zlib.h>
#అయితే_ముగింపు
#ఒకవేళ LLVM_ENABLE_ZSTD
#అన్తర్భావ <zstd.h>
#అయితే_ముగింపు
#ఒకవేళ LLVM_ENABLE_LZMA
#అన్తర్భావ <lzma.h>
#అయితే_ముగింపు

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
స్థిర తార్కిక is_Zlib_శీర్షిక(ArrayRef<uint8_t> ఆదానం) {
  ఒకవేళ (ఆదానం.కొలత_2() < 2)
    తిరిగి అబద్ధం;
  గుర్తులేని CMF = ఆదానం[0];
  గుర్తులేని FLG = ఆదానం[1];
  ఒకవేళ ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    తిరిగి అబద్ధం;
  తిరిగి (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
స్థిర తార్కిక isZstdMagic(ArrayRef<uint8_t> ఆదానం) {
  స్థిర constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  తిరిగి ఆదానం.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
స్థిర std::optional<Format> identifyFormat(ArrayRef<uint8_t> ఆదానం) {
  ఒకవేళ (isZstdMagic(ఆదానం))
    తిరిగి Format::Zstd;
  ఒకవేళ (is_Zlib_శీర్షిక(ఆదానం))
    తిరిగి Format::Zlib;
  తిరిగి std::nullopt;
}

స్థిరరాశి అక్షరం *compression::getReasonIfUnsupported(compression::Format F) {
  ఎంపిక (F) {
  స్థితి compression::Format::Zlib:
    ఒకవేళ (zlib::is_అందుబాటు())
      తిరిగి శూన్యసూచిక;
    తిరిగి "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  స్థితి compression::Format::Zstd:
    ఒకవేళ (zstd::is_అందుబాటు())
      తిరిగి శూన్యసూచిక;
    తిరిగి "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

స్థిరరాశి అక్షరం *compression::getReasonIfUnsupported(ArrayRef<uint8_t> ఆదానం) {
  ఒకవేళ (std::optional<Format> F = identifyFormat(ఆదానం))
    తిరిగి getReasonIfUnsupported(*F);
  తిరిగి "unknown compression format";
}

శూన్య compression::compress(Params P, ArrayRef<uint8_t> ఆదానం,
                           Small_సదిశ_Impl<uint8_t> &Output) {
  ఎంపిక (P.format) {
  స్థితి compression::Format::Zlib:
    zlib::compress(ఆదానం, Output, P.level);
    ఆపు;
  స్థితి compression::Format::Zstd:
    zstd::compress(ఆదానం, Output, P.level, P.zstdEnableLdm);
    ఆపు;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ఆదానం,
                              uint8_t *Output, size_t Uncompressed_కొలత) {
  ఎంపిక (formatFor(T)) {
  స్థితి compression::Format::Zlib:
    తిరిగి zlib::decompress(ఆదానం, Output, Uncompressed_కొలత);
  స్థితి compression::Format::Zstd:
    తిరిగి zstd::decompress(ఆదానం, Output, Uncompressed_కొలత);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> ఆదానం,
                              Small_సదిశ_Impl<uint8_t> &Output,
                              size_t Uncompressed_కొలత) {
  ఎంపిక (F) {
  స్థితి compression::Format::Zlib:
    తిరిగి zlib::decompress(ఆదానం, Output, Uncompressed_కొలత);
  స్థితి compression::Format::Zstd:
    తిరిగి zstd::decompress(ఆదానం, Output, Uncompressed_కొలత);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> ఆదానం,
                              Small_సదిశ_Impl<uint8_t> &Output,
                              size_t Uncompressed_కొలత) {
  తిరిగి decompress(formatFor(T), ఆదానం, Output, Uncompressed_కొలత);
}

Error compression::decompress(ArrayRef<uint8_t> ఆదానం,
                              Small_సదిశ_Impl<uint8_t> &Output,
                              size_t Uncompressed_కొలత) {
  std::optional<Format> F = identifyFormat(ఆదానం);
  ఒకవేళ (స్థిరరాశి అక్షరం *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    తిరిగి createStringError(Reason);
  తిరిగి decompress(*F, ఆదానం, Output, Uncompressed_కొలత);
}

#ఒకవేళ LLVM_ENABLE_ZLIB

స్థిర StringRef convert_Zlib_సంకేతం_గమ్యం_String(పూర్ణాంకం సంకేతం_2) {
  ఎంపిక (సంకేతం_2) {
  స్థితి Z_MEM_ERROR:
    తిరిగి "zlib error: Z_MEM_ERROR";
  స్థితి Z_BUF_ERROR:
    తిరిగి "zlib error: Z_BUF_ERROR";
  స్థితి Z_ప్రవాహం_ERROR:
    తిరిగి "zlib error: Z_STREAM_ERROR";
  స్థితి Z_దత్తాంశం_ERROR:
    తిరిగి "zlib error: Z_DATA_ERROR";
  స్థితి Z_OK:
  అప్రమేయ:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

తార్కిక zlib::is_అందుబాటు() { తిరిగి నిజం; }

శూన్య zlib::compress(ArrayRef<uint8_t> ఆదానం,
                    Small_సదిశ_Impl<uint8_t> &Compressed_బఫరు, పూర్ణాంకం Level) {
  గుర్తులేని దీర్ఘ Compressed_కొలత = ::compressBound(ఆదానం.కొలత_2());
  Compressed_బఫరు.resize_for_overwrite(Compressed_కొలత);
  పూర్ణాంకం Res = ::compress2((Bytef *)Compressed_బఫరు.దత్తాంశం(), &Compressed_కొలత,
                        (స్థిరరాశి Bytef *)ఆదానం.దత్తాంశం(), ఆదానం.కొలత_2(), Level);
  ఒకవేళ (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  ప్రతిజ్ఞా(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_బఫరు.దత్తాంశం(), Compressed_కొలత);
  ఒకవేళ (Compressed_కొలత < Compressed_బఫరు.కొలత_2())
    Compressed_బఫరు.truncate(Compressed_కొలత);
}

Error zlib::decompress(ArrayRef<uint8_t> ఆదానం, uint8_t *Output,
                       size_t &Uncompressed_కొలత) {
  పూర్ణాంకం Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_కొలత,
                         (స్థిరరాశి Bytef *)ఆదానం.దత్తాంశం(), ఆదానం.కొలత_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_కొలత);
  తిరిగి Res ? make_error<StringError>(convert_Zlib_సంకేతం_గమ్యం_String(Res),
                                       inconvertible_Error_సంకేతం())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> ఆదానం,
                       Small_సదిశ_Impl<uint8_t> &Output,
                       size_t Uncompressed_కొలత) {
  Output.resize_for_overwrite(Uncompressed_కొలత);
  Error E = zlib::decompress(ఆదానం, Output.దత్తాంశం(), Uncompressed_కొలత);
  ఒకవేళ (Uncompressed_కొలత < Output.కొలత_2())
    Output.truncate(Uncompressed_కొలత);
  తిరిగి E;
}

#లేకపోతే
తార్కిక zlib::is_అందుబాటు() { తిరిగి అబద్ధం; }
శూన్య zlib::compress(ArrayRef<uint8_t> ఆదానం,
                    Small_సదిశ_Impl<uint8_t> &Compressed_బఫరు, పూర్ణాంకం Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ఆదానం, uint8_t *Uncompressed_బఫరు,
                       size_t &Uncompressed_కొలత) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> ఆదానం,
                       Small_సదిశ_Impl<uint8_t> &Uncompressed_బఫరు,
                       size_t Uncompressed_కొలత) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#అయితే_ముగింపు

#ఒకవేళ LLVM_ENABLE_ZSTD

తార్కిక zstd::is_అందుబాటు() { తిరిగి నిజం; }

#అన్తర్భావ <zstd.h> // Ensure ZSTD library is included

శూన్య zstd::compress(ArrayRef<uint8_t> ఆదానం,
                    Small_సదిశ_Impl<uint8_t> &Compressed_బఫరు, పూర్ణాంకం Level,
                    తార్కిక EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  ఒకవేళ (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  ఒకవేళ (ZSTD_isError(ZSTD_CCtx_సమితి_Parameter(
          Cctx, ZSTD_c_enable_Long_దూరం_Matching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  ఒకవేళ (ZSTD_isError(
          ZSTD_CCtx_సమితి_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  గుర్తులేని దీర్ఘ Compressed_బఫరు_కొలత = ZSTD_compressBound(ఆదానం.కొలత_2());
  Compressed_బఫరు.resize_for_overwrite(Compressed_బఫరు_కొలత);

  size_t స్థిరరాశి Compressed_కొలత =
      ZSTD_compress2(Cctx, Compressed_బఫరు.దత్తాంశం(), Compressed_బఫరు_కొలత,
                     ఆదానం.దత్తాంశం(), ఆదానం.కొలత_2());

  ZSTD_freeCCtx(Cctx);

  ఒకవేళ (ZSTD_isError(Compressed_కొలత))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_బఫరు.దత్తాంశం(), Compressed_కొలత);
  ఒకవేళ (Compressed_కొలత < Compressed_బఫరు.కొలత_2())
    Compressed_బఫరు.truncate(Compressed_కొలత);
}

Error zstd::decompress(ArrayRef<uint8_t> ఆదానం, uint8_t *Output,
                       size_t &Uncompressed_కొలత) {
  స్థిరరాశి size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_కొలత, (స్థిరరాశి uint8_t *)ఆదానం.దత్తాంశం(), ఆదానం.కొలత_2());
  Uncompressed_కొలత = Res;
  ఒకవేళ (ZSTD_isError(Res))
    తిరిగి make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_సంకేతం());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_కొలత);
  తిరిగి Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> ఆదానం,
                       Small_సదిశ_Impl<uint8_t> &Output,
                       size_t Uncompressed_కొలత) {
  Output.resize_for_overwrite(Uncompressed_కొలత);
  Error E = zstd::decompress(ఆదానం, Output.దత్తాంశం(), Uncompressed_కొలత);
  ఒకవేళ (Uncompressed_కొలత < Output.కొలత_2())
    Output.truncate(Uncompressed_కొలత);
  తిరిగి E;
}

#లేకపోతే
తార్కిక zstd::is_అందుబాటు() { తిరిగి అబద్ధం; }
శూన్య zstd::compress(ArrayRef<uint8_t> ఆదానం,
                    Small_సదిశ_Impl<uint8_t> &Compressed_బఫరు, పూర్ణాంకం Level,
                    తార్కిక EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ఆదానం, uint8_t *Output,
                       size_t &Uncompressed_కొలత) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> ఆదానం,
                       Small_సదిశ_Impl<uint8_t> &Output,
                       size_t Uncompressed_కొలత) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#అయితే_ముగింపు

#ఒకవేళ LLVM_ENABLE_LZMA

తార్కిక xz::is_అందుబాటు() { తిరిగి నిజం; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
స్థిర స్థిరరాశి అక్షరం *convert_LZMACode_గమ్యం_String(lzma_ret సంకేతం_2) {
  ఎంపిక (సంకేతం_2) {
  స్థితి LZMA_ప్రవాహం_చివర:
    తిరిగి "lzma error: LZMA_STREAM_END";
  స్థితి LZMA_NO_CHECK:
    తిరిగి "lzma error: LZMA_NO_CHECK";
  స్థితి LZMA_UNSUPPORTED_CHECK:
    తిరిగి "lzma error: LZMA_UNSUPPORTED_CHECK";
  స్థితి LZMA_GET_CHECK:
    తిరిగి "lzma error: LZMA_GET_CHECK";
  స్థితి LZMA_MEM_ERROR:
    తిరిగి "lzma error: LZMA_MEM_ERROR";
  స్థితి LZMA_MEMLIMIT_ERROR:
    తిరిగి "lzma error: LZMA_MEMLIMIT_ERROR";
  స్థితి LZMA_FORMAT_ERROR:
    తిరిగి "lzma error: LZMA_FORMAT_ERROR";
  స్థితి LZMA_OPTIONS_ERROR:
    తిరిగి "lzma error: LZMA_OPTIONS_ERROR";
  స్థితి LZMA_దత్తాంశం_ERROR:
    తిరిగి "lzma error: LZMA_DATA_ERROR";
  స్థితి LZMA_BUF_ERROR:
    తిరిగి "lzma error: LZMA_BUF_ERROR";
  స్థితి LZMA_PROG_ERROR:
    తిరిగి "lzma error: LZMA_PROG_ERROR";
  అప్రమేయ:
    llvm_unreachable("unknown or unexpected lzma status code");
  }
}

/// Read the uncompressed size recorded in the xz stream's index.
స్థిర Expected<uint64_t> get_Uncompressed_కొలత(ArrayRef<uint8_t> ఆదానం) {
  ఒకవేళ (ఆదానం.కొలత_2() < LZMA_ప్రవాహం_శీర్షిక_కొలత)
    తిరిగి createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        ఆదానం.కొలత_2(), size_t(LZMA_ప్రవాహం_శీర్షిక_కొలత));

  // Decode the xz footer.
  lzma_ప్రవాహం_flags FooterFlags{};
  lzma_ret Ret = lzma_ప్రవాహం_footer_decode(
      &FooterFlags, ఆదానం.take_వెనుక(LZMA_ప్రవాహం_శీర్షిక_కొలత).దత్తాంశం());
  ఒకవేళ (Ret != LZMA_OK)
    తిరిగి createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_గమ్యం_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_కొలత = FooterFlags.backward_కొలత + 2 * LZMA_ప్రవాహం_శీర్షిక_కొలత;
  ఒకవేళ (ఆదానం.కొలత_2() < Min_కొలత)
    తిరిగి createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        ఆదానం.కొలత_2(), Min_కొలత);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_సూచీ *సూచీ_2 = శూన్యసూచిక;
  llvm::scope_exit Free_సూచీ([&] { lzma_సూచీ_చివర(సూచీ_2, శూన్యసూచిక); });
  uint64_t Mem_హద్దు = UINT64_MAX;
  size_t లోపల_Pos = 0;
  Ret = lzma_సూచీ_బఫరు_decode(
      &సూచీ_2, &Mem_హద్దు, శూన్యసూచిక,
      ఆదానం.take_వెనుక(LZMA_ప్రవాహం_శీర్షిక_కొలత + FooterFlags.backward_కొలత)
          .దత్తాంశం(),
      &లోపల_Pos, ఆదానం.కొలత_2());
  ఒకవేళ (Ret != LZMA_OK)
    తిరిగి createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_గమ్యం_String(Ret));

  తిరిగి lzma_సూచీ_uncompressed_కొలత(సూచీ_2);
}

Error xz::decompress(ArrayRef<uint8_t> ఆదానం,
                     Small_సదిశ_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_కొలత = get_Uncompressed_కొలత(ఆదానం);
  ఒకవేళ (!Uncompressed_కొలత)
    తిరిగి Uncompressed_కొలత.takeError();

  ఒకవేళ (*Uncompressed_కొలత > std::numeric_limits<size_t>::max())
    తిరిగి createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_కొలత);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_కొలత));
  uint64_t Mem_హద్దు = UINT64_MAX;
  size_t లోపల_Pos = 0;
  size_t బహిర్గతం_Pos = 0;
  lzma_ret Ret = lzma_ప్రవాహం_బఫరు_decode(
      &Mem_హద్దు, /*flags=*/0, శూన్యసూచిక, ఆదానం.దత్తాంశం(), &లోపల_Pos, ఆదానం.కొలత_2(),
      Output.దత్తాంశం(), &బహిర్గతం_Pos, Output.కొలత_2());
  ఒకవేళ (Ret != LZMA_OK) {
    Output.clear();
    తిరిగి createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_గమ్యం_String(Ret));
  }

  తిరిగి Error::success();
}

#లేకపోతే

తార్కిక xz::is_అందుబాటు() { తిరిగి అబద్ధం; }
Error xz::decompress(ArrayRef<uint8_t> ఆదానం,
                     Small_సదిశ_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#అయితే_ముగింపు
