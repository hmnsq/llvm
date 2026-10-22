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

استعمال نطاق sys;

LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<3600>>::قدر_2[] = "h";
LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<60>>::قدر_2[] = "m";
LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<1>>::قدر_2[] = "s";
LLVM_ABI ثابت حرف llvm::detail::unit<std::milli>::قدر_2[] = "ms";
LLVM_ABI ثابت حرف llvm::detail::unit<std::micro>::قدر_2[] = "us";
LLVM_ABI ثابت حرف llvm::detail::unit<std::nano>::قدر_2[] = "ns";

ساكن مضمن بنية tm getStructTM(وقت_نقطو<> TP) {
  بنية tm Storage;
  std::time_t Our_وقت = منزل_وقت_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::localtime_r(&Our_وقت, &Storage);
  أكد(LT);
  (فراغ)LT;
#جيڪڏهن_پورو
#إذا defined(_WIN32)
  صحيح Error = ::localtime_s(&Storage, &Our_وقت);
  أكد(!Error);
  (فراغ)Error;
#جيڪڏهن_پورو

  أرجع Storage;
}

ساكن مضمن بنية tm getStructTMUtc(Utc_وقت<> TP) {
  بنية tm Storage;
  std::time_t Our_وقت = منزل_وقت_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::gmtime_r(&Our_وقت, &Storage);
  أكد(LT);
  (فراغ)LT;
#جيڪڏهن_پورو
#إذا defined(_WIN32)
  صحيح Error = ::gmtime_s(&Storage, &Our_وقت);
  أكد(!Error);
  (فراغ)Error;
#جيڪڏهن_پورو

  أرجع Storage;
}

raw_ostream &عامل<<(raw_ostream &OS, وقت_نقطو<> TP) {
  بنية tm LT = getStructTM(TP);
  حرف بفر[حجم("YYYY-MM-DD HH:MM:SS")];
  strftime(بفر, حجم(بفر), "%Y-%m-%d %H:%M:%S", &LT);
  أرجع OS << بفر << '.'
            << format("%.9lu",
                      ڊگهو((TP.وقت_since_epoch() % std::chrono::seconds(1))
                               .ڳڻپ()));
}

سانچو <صنف T>
ساكن فراغ format(ثابت T &Fractional, بنية tm &LT, raw_ostream &OS,
                   StringRef Style) {
  استعمال نطاق std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  إذا (Style.خالي_جايون()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  لكل (بنانشان I = 0; I < Style.سائيز_2(); ++I) {
    إذا (Style[I] == '%' && Style.سائيز_2() > I + 1) بدل (Style[I + 1]) {
        حالة 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (ڊگهو)duration_cast<milliseconds>(Fractional).ڳڻپ());
          ++I;
          استمر;
        حالة 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (ڊگهو)duration_cast<microseconds>(Fractional).ڳڻپ());
          ++I;
          استمر;
        حالة 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (ڊگهو)duration_cast<nanoseconds>(Fractional).ڳڻپ());
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

فراغ format_provider<Utc_وقت<std::chrono::seconds>>::format(
    ثابت Utc_وقت<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  استعمال نطاق std::chrono;
  Utc_وقت<seconds> Truncated =
      Utc_وقت<seconds>(duration_cast<seconds>(T.وقت_since_epoch()));
  خودڪار Fractional = T - Truncated;
  بنية tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

فراغ format_provider<وقت_نقطو<>>::format(ثابت وقت_نقطو<> &T, raw_ostream &OS,
                                          StringRef Style) {
  استعمال نطاق std::chrono;
  وقت_نقطو<seconds> Truncated = وقت_نقطو_cast<seconds>(T);
  خودڪار Fractional = T - Truncated;
  بنية tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
