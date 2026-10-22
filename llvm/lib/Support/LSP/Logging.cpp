//===- Logging.cpp --------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/LSP/Logging.h"
#અન્તર્ભાવ "llvm/Support/Chrono.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;
using નામાકાશ llvm::lsp;

શૂન્ય Logger::setLogLevel(Level LogLevel) { get().LogLevel = LogLevel; }

Logger &Logger::get() {
  સ્થિર Logger Logger;
  પાછા Logger;
}

શૂન્ય Logger::log(Level LogLevel, અચલ અક્ષર *Fmt,
                 અચલ llvm::formatv_object_base &Message) {
  Logger &Logger = get();

  // Ignore messages with log levels below the current setting in the logger.
  જો (LogLevel < Logger.LogLevel)
    પાછા;

  // An indicator character for each log level.
  અચલ અક્ષર *LogLevelIndicators = "DIE";

  // Format the message and print to errs.
  llvm::sys::TimePoint<> Timestamp = std::chrono::system_clock::now();
  std::lock_guard<std::mutex> LogGuard(Logger.Mutex);
  llvm::errs() << llvm::formatv(
      "{0}[{1:%H:%M:%S.%L}] {2}\n",
      LogLevelIndicators[static_cast<unsigned>(LogLevel)], Timestamp, Message);
  llvm::errs().flush();
}
