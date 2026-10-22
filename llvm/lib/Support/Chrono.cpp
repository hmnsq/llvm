//===- Support/Chrono.cpp - Utilities for Timing Manipulation ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Chrono.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/Compiler.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/raw_ostream.h"

نامخلا llvm {

استعمال نامخلا sys;

LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<3600>>::قدر_2[] = "h";
LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<60>>::قدر_2[] = "m";
LLVM_ABI ثابت حرف llvm::detail::unit<std::تناسب<1>>::قدر_2[] = "s";
LLVM_ABI ثابت حرف llvm::detail::unit<std::milli>::قدر_2[] = "ms";
LLVM_ABI ثابت حرف llvm::detail::unit<std::micro>::قدر_2[] = "us";
LLVM_ABI ثابت حرف llvm::detail::unit<std::nano>::قدر_2[] = "ns";

ساکن اندرونی ساخت tm getStructTM(وقت_نقطہ<> TP) {
  ساخت tm Storage;
  std::time_t Our_وقت = مقصد_وقت_T(TP);

#اگر defined(LLVM_ON_UNIX)
  ساخت tm *LT = ::localtime_r(&Our_وقت, &Storage);
  أكد(LT);
  (خالی)LT;
#اگر_ختم
#اگر defined(_WIN32)
  صحیح Error = ::localtime_s(&Storage, &Our_وقت);
  أكد(!Error);
  (خالی)Error;
#اگر_ختم

  واپس Storage;
}

ساکن اندرونی ساخت tm getStructTMUtc(Utc_وقت<> TP) {
  ساخت tm Storage;
  std::time_t Our_وقت = مقصد_وقت_T(TP);

#اگر defined(LLVM_ON_UNIX)
  ساخت tm *LT = ::gmtime_r(&Our_وقت, &Storage);
  أكد(LT);
  (خالی)LT;
#اگر_ختم
#اگر defined(_WIN32)
  صحیح Error = ::gmtime_s(&Storage, &Our_وقت);
  أكد(!Error);
  (خالی)Error;
#اگر_ختم

  واپس Storage;
}

raw_ostream &عامل<<(raw_ostream &OS, وقت_نقطہ<> TP) {
  ساخت tm LT = getStructTM(TP);
  حرف بفر[حجم("YYYY-MM-DD HH:MM:SS")];
  strftime(بفر, حجم(بفر), "%Y-%m-%d %H:%M:%S", &LT);
  واپس OS << بفر << '.'
            << format("%.9lu",
                      طویل((TP.وقت_since_epoch() % std::chrono::seconds(1))
                               .گنتی()));
}

سانچہ <جماعت T>
ساکن خالی format(ثابت T &Fractional, ساخت tm &LT, raw_ostream &OS,
                   StringRef Style) {
  استعمال نامخلا std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  اگر (Style.خالی_خانے()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  برائے (غیرعلامتی I = 0; I < Style.سائز_2(); ++I) {
    اگر (Style[I] == '%' && Style.سائز_2() > I + 1) انتخاب (Style[I + 1]) {
        حالت 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (طویل)duration_cast<milliseconds>(Fractional).گنتی());
          ++I;
          جاری;
        حالت 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (طویل)duration_cast<microseconds>(Fractional).گنتی());
          ++I;
          جاری;
        حالت 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (طویل)duration_cast<nanoseconds>(Fractional).گنتی());
          ++I;
          جاری;
        حالت '%':  // Consume %%, so %%f parses as (%%)f not %(%f)
          FStream << "%%";
          ++I;
          جاری;
      }
    FStream << Style[I];
  }
  حرف بفر[256];  // Should be enough for anywhen.
  size_t Len = strftime(بفر, حجم(بفر), Format.c_str(), &LT);
  OS << (Len ? بفر : "BAD-DATE-FORMAT");
}

خالی format_provider<Utc_وقت<std::chrono::seconds>>::format(
    ثابت Utc_وقت<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  استعمال نامخلا std::chrono;
  Utc_وقت<seconds> Truncated =
      Utc_وقت<seconds>(duration_cast<seconds>(T.وقت_since_epoch()));
  خودکار Fractional = T - Truncated;
  ساخت tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

خالی format_provider<وقت_نقطہ<>>::format(ثابت وقت_نقطہ<> &T, raw_ostream &OS,
                                          StringRef Style) {
  استعمال نامخلا std::chrono;
  وقت_نقطہ<seconds> Truncated = وقت_نقطہ_cast<seconds>(T);
  خودکار Fractional = T - Truncated;
  ساخت tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
