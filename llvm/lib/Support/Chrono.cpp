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

using نطاق sys;

LLVM_ABI ثابت حرف llvm::detail::unit<std::ratio<3600>>::قيمة_2[] = "h";
LLVM_ABI ثابت حرف llvm::detail::unit<std::ratio<60>>::قيمة_2[] = "m";
LLVM_ABI ثابت حرف llvm::detail::unit<std::ratio<1>>::قيمة_2[] = "s";
LLVM_ABI ثابت حرف llvm::detail::unit<std::milli>::قيمة_2[] = "ms";
LLVM_ABI ثابت حرف llvm::detail::unit<std::micro>::قيمة_2[] = "us";
LLVM_ABI ثابت حرف llvm::detail::unit<std::nano>::قيمة_2[] = "ns";

ساكن مضمن بنية tm getStructTM(TimePoint<> TP) {
  بنية tm Storage;
  std::time_t OurTime = toTimeT(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::localtime_r(&OurTime, &Storage);
  أكد(LT);
  (فراغ)LT;
#endif
#إذا defined(_WIN32)
  صحيح Error = ::localtime_s(&Storage, &OurTime);
  أكد(!Error);
  (فراغ)Error;
#endif

  أرجع Storage;
}

ساكن مضمن بنية tm getStructTMUtc(UtcTime<> TP) {
  بنية tm Storage;
  std::time_t OurTime = toTimeT(TP);

#إذا defined(LLVM_ON_UNIX)
  بنية tm *LT = ::gmtime_r(&OurTime, &Storage);
  أكد(LT);
  (فراغ)LT;
#endif
#إذا defined(_WIN32)
  صحيح Error = ::gmtime_s(&Storage, &OurTime);
  أكد(!Error);
  (فراغ)Error;
#endif

  أرجع Storage;
}

raw_ostream &operator<<(raw_ostream &OS, TimePoint<> TP) {
  بنية tm LT = getStructTM(TP);
  حرف Buffer[حجم("YYYY-MM-DD HH:MM:SS")];
  strftime(Buffer, حجم(Buffer), "%Y-%m-%d %H:%M:%S", &LT);
  أرجع OS << Buffer << '.'
            << format("%.9lu",
                      long((TP.time_since_epoch() % std::chrono::seconds(1))
                               .عدد()));
}

template <صنف T>
ساكن فراغ format(ثابت T &Fractional, بنية tm &LT, raw_ostream &OS,
                   StringRef Style) {
  using نطاق std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  إذا (Style.empty()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  لكل (unsigned I = 0; I < Style.size(); ++I) {
    إذا (Style[I] == '%' && Style.size() > I + 1) بدل (Style[I + 1]) {
        حالة 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (long)duration_cast<milliseconds>(Fractional).عدد());
          ++I;
          استمر;
        حالة 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (long)duration_cast<microseconds>(Fractional).عدد());
          ++I;
          استمر;
        حالة 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (long)duration_cast<nanoseconds>(Fractional).عدد());
          ++I;
          استمر;
        حالة '%':  // Consume %%, so %%f parses as (%%)f not %(%f)
          FStream << "%%";
          ++I;
          استمر;
      }
    FStream << Style[I];
  }
  حرف Buffer[256];  // Should be enough for anywhen.
  size_t Len = strftime(Buffer, حجم(Buffer), Format.c_str(), &LT);
  OS << (Len ? Buffer : "BAD-DATE-FORMAT");
}

فراغ format_provider<UtcTime<std::chrono::seconds>>::format(
    ثابت UtcTime<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  using نطاق std::chrono;
  UtcTime<seconds> Truncated =
      UtcTime<seconds>(duration_cast<seconds>(T.time_since_epoch()));
  auto Fractional = T - Truncated;
  بنية tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

فراغ format_provider<TimePoint<>>::format(ثابت TimePoint<> &T, raw_ostream &OS,
                                          StringRef Style) {
  using نطاق std::chrono;
  TimePoint<seconds> Truncated = time_point_cast<seconds>(T);
  auto Fractional = T - Truncated;
  بنية tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
