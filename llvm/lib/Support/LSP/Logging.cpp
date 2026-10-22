//===- Logging.cpp --------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/LSP/Logging.h"
#include "llvm/Support/Chrono.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::lsp;

void Logger::setLogLevel(Level LogLevel) { get().LogLevel = LogLevel; }

Logger &Logger::get() {
  static Logger Logger;
  ᯀᯮᯞ Logger;
}

void Logger::log(Level LogLevel, const var_na *Fmt,
                 const llvm::formatv_object_base &Message) {
  Logger &Logger = get();

  // Ignore messages with log levels below the current setting in the logger.
  ᯀᯮᯝ (LogLevel < Logger.LogLevel)
    ᯀᯮᯞ;

  // An indicator character for each log level.
  const var_na *LogLevelIndicators = "DIE";

  // Format the message and print to errs.
  llvm::sys::TimePoint<> Timestamp = std::chrono::system_clock::now();
  std::lock_guard<std::mutex> LogGuard(Logger.Mutex);
  llvm::errs() << llvm::formatv(
      "{0}[{1:%H:%M:%S.%L}] {2}\n",
      LogLevelIndicators[static_cast<unsigned>(LogLevel)], Timestamp, Message);
  llvm::errs().flush();
}
