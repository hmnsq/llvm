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

استفاده نطاق sys;

LLVM_ABI ثابت حرف llvm::detail::unit<std::نسبت<3600>>::مقدار_2[] = "h";
LLVM_ABI ثابت حرف llvm::detail::unit<std::نسبت<60>>::مقدار_2[] = "m";
LLVM_ABI ثابت حرف llvm::detail::unit<std::نسبت<1>>::مقدار_2[] = "s";
LLVM_ABI ثابت حرف llvm::detail::unit<std::milli>::مقدار_2[] = "ms";
LLVM_ABI ثابت حرف llvm::detail::unit<std::micro>::مقدار_2[] = "us";
LLVM_ABI ثابت حرف llvm::detail::unit<std::nano>::مقدار_2[] = "ns";

ساكن مضمن بنية tm getStructTM(زمان_نقطه<> TP) {
  بنية tm Storage;
  std::time_t Our_زمان = مقصد_زمان_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::localtime_r(&Our_زمان, &Storage);
  أكد(LT);
  (فراغ)LT;
#پایان_اگر
#إذا defined(_WIN32)
  صحيح Error = ::localtime_s(&Storage, &Our_زمان);
  أكد(!Error);
  (فراغ)Error;
#پایان_اگر

  أرجع Storage;
}

ساكن مضمن بنية tm getStructTMUtc(Utc_زمان<> TP) {
  بنية tm Storage;
  std::time_t Our_زمان = مقصد_زمان_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::gmtime_r(&Our_زمان, &Storage);
  أكد(LT);
  (فراغ)LT;
#پایان_اگر
#إذا defined(_WIN32)
  صحيح Error = ::gmtime_s(&Storage, &Our_زمان);
  أكد(!Error);
  (فراغ)Error;
#پایان_اگر

  أرجع Storage;
}

raw_ostream &عملگر<<(raw_ostream &OS, زمان_نقطه<> TP) {
  بنية tm LT = getStructTM(TP);
  حرف میانگیر[حجم("YYYY-MM-DD HH:MM:SS")];
  strftime(میانگیر, حجم(میانگیر), "%Y-%m-%d %H:%M:%S", &LT);
  أرجع OS << میانگیر << '.'
            << format("%.9lu",
                      دراز((TP.زمان_since_epoch() % std::chrono::seconds(1))
                               .شمار()));
}

قالب <صنف T>
ساكن فراغ format(ثابت T &Fractional, بنية tm &LT, raw_ostream &OS,
                   StringRef Style) {
  استفاده نطاق std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  إذا (Style.خالی()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  لكل (بیعلامت I = 0; I < Style.اندازه_2(); ++I) {
    إذا (Style[I] == '%' && Style.اندازه_2() > I + 1) بدل (Style[I + 1]) {
        حالة 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (دراز)duration_cast<milliseconds>(Fractional).شمار());
          ++I;
          استمر;
        حالة 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (دراز)duration_cast<microseconds>(Fractional).شمار());
          ++I;
          استمر;
        حالة 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (دراز)duration_cast<nanoseconds>(Fractional).شمار());
          ++I;
          استمر;
        حالة '%':  // Consume %%, so %%f parses as (%%)f not %(%f)
          FStream << "%%";
          ++I;
          استمر;
      }
    FStream << Style[I];
  }
  حرف میانگیر[256];  // Should be enough for anywhen.
  size_t Len = strftime(میانگیر, حجم(میانگیر), Format.c_str(), &LT);
  OS << (Len ? میانگیر : "BAD-DATE-FORMAT");
}

فراغ format_provider<Utc_زمان<std::chrono::seconds>>::format(
    ثابت Utc_زمان<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  استفاده نطاق std::chrono;
  Utc_زمان<seconds> Truncated =
      Utc_زمان<seconds>(duration_cast<seconds>(T.زمان_since_epoch()));
  خودکار Fractional = T - Truncated;
  بنية tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

فراغ format_provider<زمان_نقطه<>>::format(ثابت زمان_نقطه<> &T, raw_ostream &OS,
                                          StringRef Style) {
  استفاده نطاق std::chrono;
  زمان_نقطه<seconds> Truncated = زمان_نقطه_cast<seconds>(T);
  خودکار Fractional = T - Truncated;
  بنية tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
