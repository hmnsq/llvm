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

فضاینام llvm {

using فضاینام sys;

LLVM_ABI ثابت نویسه llvm::detail::unit<std::ratio<3600>>::مقدار_2[] = "h";
LLVM_ABI ثابت نویسه llvm::detail::unit<std::ratio<60>>::مقدار_2[] = "m";
LLVM_ABI ثابت نویسه llvm::detail::unit<std::ratio<1>>::مقدار_2[] = "s";
LLVM_ABI ثابت نویسه llvm::detail::unit<std::milli>::مقدار_2[] = "ms";
LLVM_ABI ثابت نویسه llvm::detail::unit<std::micro>::مقدار_2[] = "us";
LLVM_ABI ثابت نویسه llvm::detail::unit<std::nano>::مقدار_2[] = "ns";

ایستا درونخطی ساختار tm getStructTM(TimePoint<> TP) {
  ساختار tm Storage;
  std::time_t OurTime = toTimeT(TP);

#اگر defined(LLVM_ON_UNIX)
  ساختار tm *LT = ::localtime_r(&OurTime, &Storage);
  أكد(LT);
  (تهی)LT;
#endif
#اگر defined(_WIN32)
  صحیح Error = ::localtime_s(&Storage, &OurTime);
  أكد(!Error);
  (تهی)Error;
#endif

  بازگشت Storage;
}

ایستا درونخطی ساختار tm getStructTMUtc(UtcTime<> TP) {
  ساختار tm Storage;
  std::time_t OurTime = toTimeT(TP);

#اگر defined(LLVM_ON_UNIX)
  ساختار tm *LT = ::gmtime_r(&OurTime, &Storage);
  أكد(LT);
  (تهی)LT;
#endif
#اگر defined(_WIN32)
  صحیح Error = ::gmtime_s(&Storage, &OurTime);
  أكد(!Error);
  (تهی)Error;
#endif

  بازگشت Storage;
}

raw_ostream &operator<<(raw_ostream &OS, TimePoint<> TP) {
  ساختار tm LT = getStructTM(TP);
  نویسه Buffer[اندازه("YYYY-MM-DD HH:MM:SS")];
  strftime(Buffer, اندازه(Buffer), "%Y-%m-%d %H:%M:%S", &LT);
  بازگشت OS << Buffer << '.'
            << format("%.9lu",
                      long((TP.time_since_epoch() % std::chrono::seconds(1))
                               .شمار()));
}

template <رده T>
ایستا تهی format(ثابت T &Fractional, ساختار tm &LT, raw_ostream &OS,
                   StringRef Style) {
  using فضاینام std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  اگر (Style.empty()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  برای (unsigned I = 0; I < Style.size(); ++I) {
    اگر (Style[I] == '%' && Style.size() > I + 1) انتخاب (Style[I + 1]) {
        حالت 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (long)duration_cast<milliseconds>(Fractional).شمار());
          ++I;
          ادامه;
        حالت 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (long)duration_cast<microseconds>(Fractional).شمار());
          ++I;
          ادامه;
        حالت 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (long)duration_cast<nanoseconds>(Fractional).شمار());
          ++I;
          ادامه;
        حالت '%':  // Consume %%, so %%f parses as (%%)f not %(%f)
          FStream << "%%";
          ++I;
          ادامه;
      }
    FStream << Style[I];
  }
  نویسه Buffer[256];  // Should be enough for anywhen.
  size_t Len = strftime(Buffer, اندازه(Buffer), Format.c_str(), &LT);
  OS << (Len ? Buffer : "BAD-DATE-FORMAT");
}

تهی format_provider<UtcTime<std::chrono::seconds>>::format(
    ثابت UtcTime<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  using فضاینام std::chrono;
  UtcTime<seconds> Truncated =
      UtcTime<seconds>(duration_cast<seconds>(T.time_since_epoch()));
  auto Fractional = T - Truncated;
  ساختار tm LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

تهی format_provider<TimePoint<>>::format(ثابت TimePoint<> &T, raw_ostream &OS,
                                          StringRef Style) {
  using فضاینام std::chrono;
  TimePoint<seconds> Truncated = time_point_cast<seconds>(T);
  auto Fractional = T - Truncated;
  ساختار tm LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
