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

کارول نطاق sys;

LLVM_ABI ثابت حرف llvm::detail::unit<std::نسبت<3600>>::ارزښت_2[] = "h";
LLVM_ABI ثابت حرف llvm::detail::unit<std::نسبت<60>>::ارزښت_2[] = "m";
LLVM_ABI ثابت حرف llvm::detail::unit<std::نسبت<1>>::ارزښت_2[] = "s";
LLVM_ABI ثابت حرف llvm::detail::unit<std::milli>::ارزښت_2[] = "ms";
LLVM_ABI ثابت حرف llvm::detail::unit<std::micro>::ارزښت_2[] = "us";
LLVM_ABI ثابت حرف llvm::detail::unit<std::nano>::ارزښت_2[] = "ns";

ساكن مضمن بنية tm getStructTM(وخت_ټکی<> TP) {
  بنية tm Storage;
  std::time_t Our_وخت = منزل_وخت_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::localtime_r(&Our_وخت, &Storage);
  أكد(LT);
  (فراغ)LT;
#د_که_پای
#إذا defined(_WIN32)
  صحيح Error = ::localtime_s(&Storage, &Our_وخت);
  أكد(!Error);
  (فراغ)Error;
#د_که_پای

  أرجع Storage;
}

ساكن مضمن بنية tm getStructTMUtc(Utc_وخت<> TP) {
  بنية tm Storage;
  std::time_t Our_وخت = منزل_وخت_T(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::gmtime_r(&Our_وخت, &Storage);
  أكد(LT);
  (فراغ)LT;
#د_که_پای
#إذا defined(_WIN32)
  صحيح Error = ::gmtime_s(&Storage, &Our_وخت);
  أكد(!Error);
  (فراغ)Error;
#د_که_پای

  أرجع Storage;
}

raw_ostream &چلوونکی<<(raw_ostream &OS, وخت_ټکی<> TP) {
  بنية tm LT = getStructTM(TP);
  حرف بفر[حجم("YYYY-MM-DD HH:MM:SS")];
  strftime(بفر, حجم(بفر), "%Y-%m-%d %H:%M:%S", &LT);
  أرجع OS << بفر << '.'
            << format("%.9lu",
                      اوږد((TP.وخت_since_epoch() % std::chrono::seconds(1))
                               .شمېر()));
}

کينډۍ <صنف T>
ساكن فراغ format(ثابت T &Fractional, بنية tm &LT, raw_ostream &OS,
                   StringRef Style) {
  کارول نطاق std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  إذا (Style.تشه()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  لكل (بینښه I = 0; I < Style.اندازه_2(); ++I) {
    إذا (Style[I] == '%' && Style.اندازه_2() > I + 1) بدل (Style[I + 1]) {
        حالة 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (اوږد)duration_cast<milliseconds>(Fractional).شمېر());
          ++I;
          استمر;
        حالة 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (اوږد)duration_cast<microseconds>(Fractional).شمېر());
          ++I;
          استمر;
        حالة 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (اوږد)duration_cast<nanoseconds>(Fractional).شمېر());
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

فراغ format_provider<Utc_وخت<std::chrono::seconds>>::format(
    ثابت Utc_وخت<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  کارول نطاق std::chrono;
  Utc_وخت<seconds> Truncated =
      Utc_وخت<seconds>(duration_cast<seconds>(T.وخت_since_epoch()));
  خپلکاری Fractional = T - Truncated;
  بنية tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

فراغ format_provider<وخت_ټکی<>>::format(ثابت وخت_ټکی<> &T, raw_ostream &OS,
                                          StringRef Style) {
  کارول نطاق std::chrono;
  وخت_ټکی<seconds> Truncated = وخت_ټکی_cast<seconds>(T);
  خپلکاری Fractional = T - Truncated;
  بنية tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
