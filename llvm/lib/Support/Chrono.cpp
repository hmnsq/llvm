//===- Support/Chrono.cpp - Utilities for Timing Manipulation ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/Chrono.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/Compiler.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/raw_ostream.h"

namespace llvm {

using namespace sys;

LLVM_ABI स्थिर अक्षर llvm::detail::unit<std::ratio<3600>>::मूल्य_2[] = "h";
LLVM_ABI स्थिर अक्षर llvm::detail::unit<std::ratio<60>>::मूल्य_2[] = "m";
LLVM_ABI स्थिर अक्षर llvm::detail::unit<std::ratio<1>>::मूल्य_2[] = "s";
LLVM_ABI स्थिर अक्षर llvm::detail::unit<std::milli>::मूल्य_2[] = "ms";
LLVM_ABI स्थिर अक्षर llvm::detail::unit<std::micro>::मूल्य_2[] = "us";
LLVM_ABI स्थिर अक्षर llvm::detail::unit<std::nano>::मूल्य_2[] = "ns";

अटल अंतरभूत काष्ठा पंचांग getStructTM(TimePoint<> TP) {
  काष्ठा पंचांग Storage;
  std::समय_प्रकार OurTime = toTimeT(TP);

#अगर defined(LLVM_ON_UNIX)
  काष्ठा पंचांग *LT = ::localtime_r(&OurTime, &Storage);
  निश्चित(LT);
  (व्योम)LT;
#पूर्ण_अगर
#अगर defined(_WIN32)
  पूर्णांक Error = ::localtime_s(&Storage, &OurTime);
  निश्चित(!Error);
  (व्योम)Error;
#पूर्ण_अगर

  वापस Storage;
}

अटल अंतरभूत काष्ठा पंचांग getStructTMUtc(UtcTime<> TP) {
  काष्ठा पंचांग Storage;
  std::समय_प्रकार OurTime = toTimeT(TP);

#अगर defined(LLVM_ON_UNIX)
  काष्ठा पंचांग *LT = ::gmtime_r(&OurTime, &Storage);
  निश्चित(LT);
  (व्योम)LT;
#पूर्ण_अगर
#अगर defined(_WIN32)
  पूर्णांक Error = ::gmtime_s(&Storage, &OurTime);
  निश्चित(!Error);
  (व्योम)Error;
#पूर्ण_अगर

  वापस Storage;
}

raw_ostream &चालक<<(raw_ostream &OS, TimePoint<> TP) {
  काष्ठा पंचांग LT = getStructTM(TP);
  अक्षर बफर[माप("YYYY-MM-DD HH:MM:SS")];
  स_माला(बफर, माप(बफर), "%Y-%m-%d %H:%M:%S", &LT);
  वापस OS << बफर << '.'
            << format("%.9lu",
                      दीर्घ((TP.time_since_epoch() % std::chrono::seconds(1))
                               .गिनती()));
}

ढाँचा <श्रेणी T>
अटल व्योम format(स्थिर T &Fractional, काष्ठा पंचांग &LT, raw_ostream &OS,
                   StringRef Style) {
  using namespace std::chrono;
  // Handle extensions first. strftime mangles unknown %x on some platforms.
  अगर (Style.empty()) Style = "%Y-%m-%d %H:%M:%S.%N";
  std::string Format;
  raw_string_ostream FStream(Format);
  क्रम (अचिन्हित I = 0; I < Style.आकार_2(); ++I) {
    अगर (Style[I] == '%' && Style.आकार_2() > I + 1) चयन (Style[I + 1]) {
        हाल 'L':  // Milliseconds, from Ruby.
          FStream << llvm::format(
              "%.3lu", (दीर्घ)duration_cast<milliseconds>(Fractional).गिनती());
          ++I;
          जारी;
        हाल 'f':  // Microseconds, from Python.
          FStream << llvm::format(
              "%.6lu", (दीर्घ)duration_cast<microseconds>(Fractional).गिनती());
          ++I;
          जारी;
        हाल 'N':  // Nanoseconds, from date(1).
          FStream << llvm::format(
              "%.9lu", (दीर्घ)duration_cast<nanoseconds>(Fractional).गिनती());
          ++I;
          जारी;
        हाल '%':  // Consume %%, so %%f parses as (%%)f not %(%f)
          FStream << "%%";
          ++I;
          जारी;
      }
    FStream << Style[I];
  }
  अक्षर बफर[256];  // Should be enough for anywhen.
  माप_प्रकार Len = स_माला(बफर, माप(बफर), Format.c_str(), &LT);
  OS << (Len ? बफर : "BAD-DATE-FORMAT");
}

व्योम format_provider<UtcTime<std::chrono::seconds>>::format(
    स्थिर UtcTime<std::chrono::seconds> &T, raw_ostream &OS, StringRef Style) {
  using namespace std::chrono;
  UtcTime<seconds> Truncated =
      UtcTime<seconds>(duration_cast<seconds>(T.time_since_epoch()));
  स्वतः Fractional = T - Truncated;
  काष्ठा पंचांग LT = getStructTMUtc(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

व्योम format_provider<TimePoint<>>::format(स्थिर TimePoint<> &T, raw_ostream &OS,
                                          StringRef Style) {
  using namespace std::chrono;
  TimePoint<seconds> Truncated = time_point_cast<seconds>(T);
  स्वतः Fractional = T - Truncated;
  काष्ठा पंचांग LT = getStructTM(Truncated);
  llvm::format(Fractional, LT, OS, Style);
}

} // namespace llvm
