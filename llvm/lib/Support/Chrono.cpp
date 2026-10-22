//===- Support/Chrono.cpp - Utilities for Timing Manipulation ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Chrono.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/Compiler.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/raw_ostream.h"

نطاق llvm {

ورتوں نطاق sys;

LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<3600>>::قدر_2[] = "h";
LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<60>>::قدر_2[] = "m";
LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<1>>::قدر_2[] = "s";
LLVM_ABI ثابت حرف llvm::detail::unit<std::milli>::قدر_2[] = "ms";
LLVM_ABI ثابت حرف llvm::detail::unit<std::micro>::قدر_2[] = "us";
LLVM_ABI ثابت حرف llvm::detail::unit<std::nano>::قدر_2[] = "ns";

ساكن مضمن بنية tm getStructTM(ویلا_نقطہ<> TP) {
  بنية tm Storage;
  std::time_t Our_ویلا = منزل_ویلا_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::localtime_r(&Our_ویلا, &Storage);
  أكد(LT);
  (فراغ)LT;
#جے_انت
#إذا defined(_WIN32)
  صحيح Error = ::localtime_s(&Storage, &Our_ویلا);
  أكد(!Error);
  (فراغ)Error;
#جے_انت

  أرجع Storage;
}

ساكن مضمن بنية tm getStructTMUtc(Utc_ویلا<> TP) {
  بنية tm Storage;
  std::time_t Our_ویلا = منزل_ویلا_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::gmtime_r(&Our_ویلا, &Storage);
  أكد(LT);
  (فراغ)LT;
#جے_انت
#إذا defined(_WIN32)
  صحيح Error = ::gmtime_s(&Storage, &Our_ویلا);
  أكد(!Error);
  (فراغ)Error;
#جے_انت

  أرجع Storage;
}

raw_ostream &عامل<<(raw_ostream &OS, ویلا_نقطہ<> TP) {
  بنية tm LT = getStructTM(TP);
  حرف بفر[حجم("YYYY-MM-DD HH:MM:SS")];
  strftime(بفر, حجم(بفر), "%Y-%m-%d %H:%M:%S", &LT);
  أرجع OS << بفر << '.'
            << format("%.9lu",
                      لما((TP.ویلا_since_epoch() % std::chrono::seconds(1))
                               .گنتی()));
}

سانچا <صنف T>
ساكن فراغ format(ثابت T &Fractional, بنية tm &LT, raw_ostream &OS,
                   StringRef Style) {
  ورتوں نطاق std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  إذا (Style.خالی_تھاواں()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  لكل (بنانشانی I = 0; I < Style.ناپ_2(); ++I) {
    إذا (Style[I] == '%' && Style.ناپ_2() > I + 1) بدل (Style[I + 1]) {
        حالة 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (لما)duration_cast<milliseconds>(Fractional).گنتی());
          ++I;
          استمر;
        حالة 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (لما)duration_cast<microseconds>(Fractional).گنتی());
          ++I;
          استمر;
        حالة 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (لما)duration_cast<nanoseconds>(Fractional).گنتی());
          ++I;
          استمر;
        حالة '%':  // Consume %%, so %%f parses as (%%)f not %(%f)
          FStream << "%%";
          ++I;
          استمر;
      }
    FStream << Style[I];
  }
  حرف بفر[256];  // Should be enough for anywhen.
  size_t Len = strftime(بفر, حجم(بفر), Format.c_str(), &LT);
  OS << (Len ? بفر : "BAD-DATE-FORMAT");
}

فراغ format_provider<Utc_ویلا<std::chrono::seconds>>::format(
    ثابت Utc_ویلا<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  ورتوں نطاق std::chrono;
  Utc_ویلا<seconds> Truncated =
      Utc_ویلا<seconds>(duration_cast<seconds>(T.ویلا_since_epoch()));
  خودکار Fractional = T - Truncated;
  بنية tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

فراغ format_provider<ویلا_نقطہ<>>::format(ثابت ویلا_نقطہ<> &T, raw_ostream &OS,
                                          StringRef Style) {
  ورتوں نطاق std::chrono;
  ویلا_نقطہ<seconds> Truncated = ویلا_نقطہ_cast<seconds>(T);
  خودکار Fractional = T - Truncated;
  بنية tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
