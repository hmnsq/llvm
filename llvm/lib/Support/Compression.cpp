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
#यदि_अन्तः
#यदि LLVM_ENABLE_ZSTD
#अन्तर्भाव <zstd.h>
#यदि_अन्तः
#यदि LLVM_ENABLE_LZMA
#अन्तर्भाव <lzma.h>
#यदि_अन्तः

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::compression;

// RFC 1950 section 2.2 zlib wrapper. Two-byte header CMF then FLG:
//   CMF: CM (bits 0-3) must be 8 (deflate). CINFO (bits 4-7) is
//        log2(windowSize)-8 and must be <= 7.
//   FLG: FCHECK (bits 0-4) is chosen so CMF*256+FLG is a multiple of 31;
//        FDICT (bit 5) marks a preset dictionary; FLEVEL (bits 6-7) is a
//        compressor hint. This only identifies the wrapper.
स्थिर तार्किक is_Zlib_शीर्षकम्(ArrayRef<uint8_t> निवेशः) {
  यदि (निवेशः.आकार_2() < 2)
    प्रत्यागम असत्य;
  अचिह्नित CMF = निवेशः[0];
  अचिह्नित FLG = निवेशः[1];
  यदि ((CMF & 0x0f) != 8 || (CMF >> 4) > 7)
    प्रत्यागम असत्य;
  प्रत्यागम (CMF * 256 + FLG) % 31 == 0;
}

// RFC 8878 section 3.1.1: Zstandard frame magic 0xFD2FB528, little-endian.
स्थिर तार्किक isZstdMagic(ArrayRef<uint8_t> निवेशः) {
  स्थिर constexpr uint8_t Magic[] = {0x28, 0xb5, 0x2f, 0xfd};
  प्रत्यागम निवेशः.take_front(4) == ArrayRef(Magic);
}

// Check zstd first: 0x28 is a valid zlib CMF (CINFO=2, 1KiB window).
स्थिर std::optional<Format> identifyFormat(ArrayRef<uint8_t> निवेशः) {
  यदि (isZstdMagic(निवेशः))
    प्रत्यागम Format::Zstd;
  यदि (is_Zlib_शीर्षकम्(निवेशः))
    प्रत्यागम Format::Zlib;
  प्रत्यागम std::nullopt;
}

ध्रुव वर्ण *compression::getReasonIfUnsupported(compression::Format F) {
  विकल्प (F) {
  पक्ष compression::Format::Zlib:
    यदि (zlib::is_उपलब्धम्())
      प्रत्यागम शून्यसूचकः;
    प्रत्यागम "LLVM was not built with LLVM_ENABLE_ZLIB or did not find zlib at "
           "build time";
  पक्ष compression::Format::Zstd:
    यदि (zstd::is_उपलब्धम्())
      प्रत्यागम शून्यसूचकः;
    प्रत्यागम "LLVM was not built with LLVM_ENABLE_ZSTD or did not find zstd at "
           "build time";
  }
  llvm_unreachable("");
}

ध्रुव वर्ण *compression::getReasonIfUnsupported(ArrayRef<uint8_t> निवेशः) {
  यदि (std::optional<Format> F = identifyFormat(निवेशः))
    प्रत्यागम getReasonIfUnsupported(*F);
  प्रत्यागम "unknown compression format";
}

शून्य compression::compress(Params P, ArrayRef<uint8_t> निवेशः,
                           Small_सदिशम्_Impl<uint8_t> &Output) {
  विकल्प (P.format) {
  पक्ष compression::Format::Zlib:
    zlib::compress(निवेशः, Output, P.level);
    भङ्ग;
  पक्ष compression::Format::Zstd:
    zstd::compress(निवेशः, Output, P.level, P.zstdEnableLdm);
    भङ्ग;
  }
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> निवेशः,
                              uint8_t *Output, size_t Uncompressed_आकार) {
  विकल्प (formatFor(T)) {
  पक्ष compression::Format::Zlib:
    प्रत्यागम zlib::decompress(निवेशः, Output, Uncompressed_आकार);
  पक्ष compression::Format::Zstd:
    प्रत्यागम zstd::decompress(निवेशः, Output, Uncompressed_आकार);
  }
  llvm_unreachable("");
}

Error compression::decompress(compression::Format F, ArrayRef<uint8_t> निवेशः,
                              Small_सदिशम्_Impl<uint8_t> &Output,
                              size_t Uncompressed_आकार) {
  विकल्प (F) {
  पक्ष compression::Format::Zlib:
    प्रत्यागम zlib::decompress(निवेशः, Output, Uncompressed_आकार);
  पक्ष compression::Format::Zstd:
    प्रत्यागम zstd::decompress(निवेशः, Output, Uncompressed_आकार);
  }
  llvm_unreachable("");
}

Error compression::decompress(DebugCompressionType T, ArrayRef<uint8_t> निवेशः,
                              Small_सदिशम्_Impl<uint8_t> &Output,
                              size_t Uncompressed_आकार) {
  प्रत्यागम decompress(formatFor(T), निवेशः, Output, Uncompressed_आकार);
}

Error compression::decompress(ArrayRef<uint8_t> निवेशः,
                              Small_सदिशम्_Impl<uint8_t> &Output,
                              size_t Uncompressed_आकार) {
  std::optional<Format> F = identifyFormat(निवेशः);
  यदि (ध्रुव वर्ण *Reason =
          F ? getReasonIfUnsupported(*F) : "unknown compression format")
    प्रत्यागम createStringError(Reason);
  प्रत्यागम decompress(*F, निवेशः, Output, Uncompressed_आकार);
}

#यदि LLVM_ENABLE_ZLIB

स्थिर StringRef convert_Zlib_संहिता_गन्तव्यम्_String(पूर्णाङ्क संहिता) {
  विकल्प (संहिता) {
  पक्ष Z_MEM_ERROR:
    प्रत्यागम "zlib error: Z_MEM_ERROR";
  पक्ष Z_BUF_ERROR:
    प्रत्यागम "zlib error: Z_BUF_ERROR";
  पक्ष Z_प्रवाहः_ERROR:
    प्रत्यागम "zlib error: Z_STREAM_ERROR";
  पक्ष Z_दत्त_ERROR:
    प्रत्यागम "zlib error: Z_DATA_ERROR";
  पक्ष Z_OK:
  स्वभाव:
    llvm_unreachable("unknown or unexpected zlib status code");
  }
}

तार्किक zlib::is_उपलब्धम्() { प्रत्यागम सत्य; }

शून्य zlib::compress(ArrayRef<uint8_t> निवेशः,
                    Small_सदिशम्_Impl<uint8_t> &Compressed_बफर, पूर्णाङ्क Level) {
  अचिह्नित दीर्घ Compressed_आकार = ::compressBound(निवेशः.आकार_2());
  Compressed_बफर.resize_for_overwrite(Compressed_आकार);
  पूर्णाङ्क Res = ::compress2((Bytef *)Compressed_बफर.दत्त(), &Compressed_आकार,
                        (ध्रुव Bytef *)निवेशः.दत्त(), निवेशः.आकार_2(), Level);
  यदि (Res == Z_MEM_ERROR)
    report_bad_alloc_error("Allocation failed");
  प्रतिज्ञा(Res == Z_OK);
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Compressed_बफर.दत्त(), Compressed_आकार);
  यदि (Compressed_आकार < Compressed_बफर.आकार_2())
    Compressed_बफर.truncate(Compressed_आकार);
}

Error zlib::decompress(ArrayRef<uint8_t> निवेशः, uint8_t *Output,
                       size_t &Uncompressed_आकार) {
  पूर्णाङ्क Res = ::uncompress((Bytef *)Output, (uLongf *)&Uncompressed_आकार,
                         (ध्रुव Bytef *)निवेशः.दत्त(), निवेशः.आकार_2());
  // Tell MemorySanitizer that zlib output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_आकार);
  प्रत्यागम Res ? make_error<StringError>(convert_Zlib_संहिता_गन्तव्यम्_String(Res),
                                       inconvertible_Error_संहिता())
             : Error::success();
}

Error zlib::decompress(ArrayRef<uint8_t> निवेशः,
                       Small_सदिशम्_Impl<uint8_t> &Output,
                       size_t Uncompressed_आकार) {
  Output.resize_for_overwrite(Uncompressed_आकार);
  Error E = zlib::decompress(निवेशः, Output.दत्त(), Uncompressed_आकार);
  यदि (Uncompressed_आकार < Output.आकार_2())
    Output.truncate(Uncompressed_आकार);
  प्रत्यागम E;
}

#अन्यथा
तार्किक zlib::is_उपलब्धम्() { प्रत्यागम असत्य; }
शून्य zlib::compress(ArrayRef<uint8_t> निवेशः,
                    Small_सदिशम्_Impl<uint8_t> &Compressed_बफर, पूर्णाङ्क Level) {
  llvm_unreachable("zlib::compress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> निवेशः, uint8_t *Uncompressed_बफर,
                       size_t &Uncompressed_आकार) {
  llvm_unreachable("zlib::decompress is unavailable");
}
Error zlib::decompress(ArrayRef<uint8_t> निवेशः,
                       Small_सदिशम्_Impl<uint8_t> &Uncompressed_बफर,
                       size_t Uncompressed_आकार) {
  llvm_unreachable("zlib::decompress is unavailable");
}
#यदि_अन्तः

#यदि LLVM_ENABLE_ZSTD

तार्किक zstd::is_उपलब्धम्() { प्रत्यागम सत्य; }

#अन्तर्भाव <zstd.h> // Ensure ZSTD library is included

शून्य zstd::compress(ArrayRef<uint8_t> निवेशः,
                    Small_सदिशम्_Impl<uint8_t> &Compressed_बफर, पूर्णाङ्क Level,
                    तार्किक EnableLdm) {
  ZSTD_CCtx *Cctx = ZSTD_createCCtx();
  यदि (!Cctx)
    report_bad_alloc_error("Failed to create ZSTD_CCtx");

  यदि (ZSTD_isError(ZSTD_CCtx_समुच्चयः_Parameter(
          Cctx, ZSTD_c_enableLongDistanceMatching, EnableLdm ? 1 : 0))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_enableLongDistanceMatching");
  }

  यदि (ZSTD_isError(
          ZSTD_CCtx_समुच्चयः_Parameter(Cctx, ZSTD_c_compressionLevel, Level))) {
    ZSTD_freeCCtx(Cctx);
    report_bad_alloc_error("Failed to set ZSTD_c_compressionLevel");
  }

  अचिह्नित दीर्घ Compressed_बफर_आकार = ZSTD_compressBound(निवेशः.आकार_2());
  Compressed_बफर.resize_for_overwrite(Compressed_बफर_आकार);

  size_t ध्रुव Compressed_आकार =
      ZSTD_compress2(Cctx, Compressed_बफर.दत्त(), Compressed_बफर_आकार,
                     निवेशः.दत्त(), निवेशः.आकार_2());

  ZSTD_freeCCtx(Cctx);

  यदि (ZSTD_isError(Compressed_आकार))
    report_bad_alloc_error("Compression failed");

  __msan_unpoison(Compressed_बफर.दत्त(), Compressed_आकार);
  यदि (Compressed_आकार < Compressed_बफर.आकार_2())
    Compressed_बफर.truncate(Compressed_आकार);
}

Error zstd::decompress(ArrayRef<uint8_t> निवेशः, uint8_t *Output,
                       size_t &Uncompressed_आकार) {
  ध्रुव size_t Res = ::ZSTD_decompress(
      Output, Uncompressed_आकार, (ध्रुव uint8_t *)निवेशः.दत्त(), निवेशः.आकार_2());
  Uncompressed_आकार = Res;
  यदि (ZSTD_isError(Res))
    प्रत्यागम make_error<StringError>(ZSTD_getErrorName(Res),
                                   inconvertible_Error_संहिता());
  // Tell MemorySanitizer that zstd output buffer is fully initialized.
  // This avoids a false report when running LLVM with uninstrumented ZLib.
  __msan_unpoison(Output, Uncompressed_आकार);
  प्रत्यागम Error::success();
}

Error zstd::decompress(ArrayRef<uint8_t> निवेशः,
                       Small_सदिशम्_Impl<uint8_t> &Output,
                       size_t Uncompressed_आकार) {
  Output.resize_for_overwrite(Uncompressed_आकार);
  Error E = zstd::decompress(निवेशः, Output.दत्त(), Uncompressed_आकार);
  यदि (Uncompressed_आकार < Output.आकार_2())
    Output.truncate(Uncompressed_आकार);
  प्रत्यागम E;
}

#अन्यथा
तार्किक zstd::is_उपलब्धम्() { प्रत्यागम असत्य; }
शून्य zstd::compress(ArrayRef<uint8_t> निवेशः,
                    Small_सदिशम्_Impl<uint8_t> &Compressed_बफर, पूर्णाङ्क Level,
                    तार्किक EnableLdm) {
  llvm_unreachable("zstd::compress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> निवेशः, uint8_t *Output,
                       size_t &Uncompressed_आकार) {
  llvm_unreachable("zstd::decompress is unavailable");
}
Error zstd::decompress(ArrayRef<uint8_t> निवेशः,
                       Small_सदिशम्_Impl<uint8_t> &Output,
                       size_t Uncompressed_आकार) {
  llvm_unreachable("zstd::decompress is unavailable");
}
#यदि_अन्तः

#यदि LLVM_ENABLE_LZMA

तार्किक xz::is_उपलब्धम्() { प्रत्यागम सत्य; }

// Returns a C string rather than a StringRef because every caller feeds the
// result to a printf-style "%s", which requires NUL termination.
स्थिर ध्रुव वर्ण *convert_LZMACode_गन्तव्यम्_String(lzma_ret संहिता) {
  विकल्प (संहिता) {
  पक्ष LZMA_प्रवाहः_अन्तः:
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
  पक्ष LZMA_दत्त_ERROR:
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
स्थिर Expected<uint64_t> get_Uncompressed_आकार(ArrayRef<uint8_t> निवेशः) {
  यदि (निवेशः.आकार_2() < LZMA_प्रवाहः_शीर्षकम्_आकार)
    प्रत्यागम createStringError(
        "size of xz-compressed blob (%zu bytes) is smaller than the "
        "LZMA_STREAM_HEADER_SIZE (%zu bytes)",
        निवेशः.आकार_2(), size_t(LZMA_प्रवाहः_शीर्षकम्_आकार));

  // Decode the xz footer.
  lzma_प्रवाहः_flags FooterFlags{};
  lzma_ret Ret = lzma_प्रवाहः_footer_decode(
      &FooterFlags, निवेशः.take_पृष्ठभागः(LZMA_प्रवाहः_शीर्षकम्_आकार).दत्त());
  यदि (Ret != LZMA_OK)
    प्रत्यागम createStringError("lzma_stream_footer_decode()=%s",
                             convert_LZMACode_गन्तव्यम्_String(Ret));

  // A stream is the header, block data, index and stream footer
  uint64_t Min_आकार = FooterFlags.backward_आकार + 2 * LZMA_प्रवाहः_शीर्षकम्_आकार;
  यदि (निवेशः.आकार_2() < Min_आकार)
    प्रत्यागम createStringError(
        "xz-compressed buffer size (%zu bytes) too small (required at "
        "least %" PRIu64 " bytes)",
        निवेशः.आकार_2(), Min_आकार);

  // Decode xz index.
  // liblzma stores null on failure, and lzma_index_end() ignores null.
  lzma_सूचकांक *सूचकांक_2 = शून्यसूचकः;
  llvm::scope_exit Free_सूचकांक([&] { lzma_सूचकांक_अन्तः(सूचकांक_2, शून्यसूचकः); });
  uint64_t Mem_सीमा = UINT64_MAX;
  size_t अन्तर्_Pos = 0;
  Ret = lzma_सूचकांक_बफर_decode(
      &सूचकांक_2, &Mem_सीमा, शून्यसूचकः,
      निवेशः.take_पृष्ठभागः(LZMA_प्रवाहः_शीर्षकम्_आकार + FooterFlags.backward_आकार)
          .दत्त(),
      &अन्तर्_Pos, निवेशः.आकार_2());
  यदि (Ret != LZMA_OK)
    प्रत्यागम createStringError("lzma_index_buffer_decode()=%s",
                             convert_LZMACode_गन्तव्यम्_String(Ret));

  प्रत्यागम lzma_सूचकांक_uncompressed_आकार(सूचकांक_2);
}

Error xz::decompress(ArrayRef<uint8_t> निवेशः,
                     Small_सदिशम्_Impl<uint8_t> &Output) {
  // Hand back nothing unless the whole stream decodes.
  Output.clear();

  Expected<uint64_t> Uncompressed_आकार = get_Uncompressed_आकार(निवेशः);
  यदि (!Uncompressed_आकार)
    प्रत्यागम Uncompressed_आकार.takeError();

  यदि (*Uncompressed_आकार > std::numeric_limits<size_t>::max())
    प्रत्यागम createStringError("xz uncompressed size (%" PRIu64
                             " bytes) exceeds addressable memory",
                             *Uncompressed_आकार);

  // Concatenated streams are unsupported: liblzma decodes only the first and
  // still reports LZMA_OK, leaving the rest of Output zero-filled.
  Output.resize(static_cast<size_t>(*Uncompressed_आकार));
  uint64_t Mem_सीमा = UINT64_MAX;
  size_t अन्तर्_Pos = 0;
  size_t बहिर्गमनम्_Pos = 0;
  lzma_ret Ret = lzma_प्रवाहः_बफर_decode(
      &Mem_सीमा, /*flags=*/0, शून्यसूचकः, निवेशः.दत्त(), &अन्तर्_Pos, निवेशः.आकार_2(),
      Output.दत्त(), &बहिर्गमनम्_Pos, Output.आकार_2());
  यदि (Ret != LZMA_OK) {
    Output.clear();
    प्रत्यागम createStringError("lzma_stream_buffer_decode()=%s",
                             convert_LZMACode_गन्तव्यम्_String(Ret));
  }

  प्रत्यागम Error::success();
}

#अन्यथा

तार्किक xz::is_उपलब्धम्() { प्रत्यागम असत्य; }
Error xz::decompress(ArrayRef<uint8_t> निवेशः,
                     Small_सदिशम्_Impl<uint8_t> &Output) {
  llvm_unreachable("xz::decompress is unavailable");
}

#यदि_अन्तः
