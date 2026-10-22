//===- Signals.cpp - Signal Handling support --------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines some helpful functions for dealing with the possibility of
// Unix signals occurring while your program is running.
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Signals.h"

#شامل "DebugOptions.h"

#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/CommandLine.h"
#شامل "llvm/Support/ErrorOr.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/FileUtilities.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Program.h"
#شامل "llvm/Support/StringSaver.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <array>
#شامل <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using فضاینام llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ایستا منطقی DisableSymbolicationFlag = نادرست;
ایستا ManagedStatic<std::string> CrashDiagnosticsDirectory;
فضاینام {
ساختار CreateDisableSymbolication {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<منطقی, درست>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ساختار CreateCrashDiagnosticsDir {
  ایستا تهی *call() {
    بازگشت جدید cl::opt<std::string, درست>(
        "crash-diagnostics-dir", cl::مقدار_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
تهی llvm::initSignalsOptions() {
  ایستا ManagedStatic<cl::opt<منطقی, درست>, CreateDisableSymbolication>
      DisableSymbolication;
  ایستا ManagedStatic<cl::opt<std::string, درست>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr نویسه DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr نویسه LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr نویسه EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ساختار CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  تهی *Cookie;
  شمارش رده Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

ایستا constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ایستا std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  ایستا std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  بازگشت callbacks;
}

// Signal-safe.
تهی sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  برای (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    اگر (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      ادامه;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
ایستا تهی درج_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                تهی *Cookie) {
  برای (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    اگر (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      ادامه;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    بازگشت;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ایستا منطقی findModulesAndOffsets(تهی **پشته_Trace, صحیح Depth,
                                  ثابت نویسه **Modules, intptr_t *Offsets,
                                  ثابت نویسه *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ایستا Formatted_عدد format_ptr(تهی *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * اندازه(تهی *);
  بازگشت format_hex((uint64_t)PC, PtrWidth);
}

/// Reads a file \p Filename written by llvm-symbolizer containing function
/// names and source locations for the addresses in \p AddressList and returns
/// the strings in a vector of pairs, where the first pair element is the index
/// of the corresponding entry in AddressList and the second is the symbolized
/// frame, in a format based on the sanitizer stack trace printer, with the
/// exception that it does not write out frame numbers (i.e. "#2 " for the
/// third address), as it is not assumed that \p AddressList corresponds to a
/// single stack trace.
/// There may be multiple returned entries for a single \p AddressList entry if
/// that frame address corresponds to one or more inlined frames; in this case,
/// all frames for an address will appear contiguously and in-order.
std::optional<SmallVector<std::pair<unsigned, std::string>, 0>>
collectAddressSymbols(تهی **AddressList, unsigned Address_شمار,
                      ثابت نویسه *MainExecutableName,
                      ثابت std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<ثابت نویسه *, 0> Modules(Address_شمار, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_شمار, 0);
  اگر (!findModulesAndOffsets(AddressList, Address_شمار, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    بازگشت {};
  صحیح درون_داد_FD;
  SmallString<32> درون_داد_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", درون_داد_FD, درون_داد_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover درون_داد_Remover(درون_داد_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream درون_داد(درون_داد_FD, درست);
    برای (unsigned AddrIdx = 0; AddrIdx < Address_شمار; AddrIdx++) {
      اگر (Modules[AddrIdx])
        درون_داد << Modules[AddrIdx] << " " << (تهی *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {درون_داد_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  صحیح RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  اگر (RunResult != 0)
    بازگشت {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  اگر (!OutputBuf)
    بازگشت {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> سطرها;
  Output.split(سطرها, "\n");
  auto *Cur_سطر = سطرها.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  برای (unsigned AddrIdx = 0; AddrIdx < Address_شمار; AddrIdx++) {
    اگر (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      ادامه;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    برای (;;) {
      اگر (Cur_سطر == سطرها.end())
        بازگشت {};
      StringRef FunctionName = *Cur_سطر++;
      اگر (FunctionName.empty())
        بشکن;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      اگر (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      اگر (Cur_سطر == سطرها.end())
        بازگشت {};
      StringRef File_سطر_Info = *Cur_سطر++;
      اگر (!File_سطر_Info.starts_with("??")) {
        OS << File_سطر_Info;
      } وگرنه {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  بازگشت Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  اگر (ثابت نویسه *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } وگرنه اگر (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    اگر (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  اگر (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  بازگشت LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
ایستا منطقی print_Symbolized_پشته_Trace(StringRef Argv0, تهی **پشته_Trace,
                                      صحیح Depth, llvm::raw_ostream &OS) {
  اگر (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    بازگشت نادرست;

  // Don't recursively invoke the llvm-symbolizer binary.
  اگر (Argv0.contains("llvm-symbolizer"))
    بازگشت نادرست;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  اگر (!LLVMSymbolizerPathOrErr)
    بازگشت نادرست;
  ثابت std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      پشته_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  اگر (!SymbolizedAddressesOpt)
    بازگشت نادرست;
  برای (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << راست_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  بازگشت درست;
}

#اگر LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
تهی sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<تهی *, 0> AddressList(Addresses.begin(), Addresses.end());
  اگر (AddressList.empty())
    بازگشت;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  اگر (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  اگر (!SymbolizedAddressesOpt)
    بازگشت;
  برای (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.افزودن_back(SymbolizedFrame.second);
  }
  بازگشت;
}
#endif

ایستا منطقی printMarkupContext(raw_ostream &OS, ثابت نویسه *MainExecutableName);

LLVM_ATTRIBUTE_USED
ایستا منطقی print_Markup_پشته_Trace(StringRef Argv0, تهی **پشته_Trace, صحیح Depth,
                                  raw_ostream &OS) {
  ثابت نویسه *Env = getenv(EnableSymbolizerMarkupEnv);
  اگر (!Env || !*Env)
    بازگشت نادرست;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  اگر (!printMarkupContext(OS, MainExecutableName.c_str()))
    بازگشت نادرست;
  برای (صحیح I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, پشته_Trace[I]);
  بازگشت درست;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#شامل "Unix/Signals.inc"
#endif
#ifdef _WIN32
#شامل "Windows/Signals.inc"
#endif
