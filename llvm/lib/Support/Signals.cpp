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

#അന്തര്ഭാവ "llvm/Support/Signals.h"

#അന്തര്ഭാവ "DebugOptions.h"

#അന്തര്ഭാവ "llvm/ADT/StringRef.h"
#അന്തര്ഭാവ "llvm/Config/llvm-config.h"
#അന്തര്ഭാവ "llvm/Support/CommandLine.h"
#അന്തര്ഭാവ "llvm/Support/ErrorOr.h"
#അന്തര്ഭാവ "llvm/Support/FileSystem.h"
#അന്തര്ഭാവ "llvm/Support/FileUtilities.h"
#അന്തര്ഭാവ "llvm/Support/Format.h"
#അന്തര്ഭാവ "llvm/Support/FormatVariadic.h"
#അന്തര്ഭാവ "llvm/Support/IOSandbox.h"
#അന്തര്ഭാവ "llvm/Support/ManagedStatic.h"
#അന്തര്ഭാവ "llvm/Support/MemoryBuffer.h"
#അന്തര്ഭാവ "llvm/Support/Path.h"
#അന്തര്ഭാവ "llvm/Support/Program.h"
#അന്തര്ഭാവ "llvm/Support/StringSaver.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"
#അന്തര്ഭാവ <array>
#അന്തര്ഭാവ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using നാമാകാശ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
സ്ഥിതം തർക്കം DisableSymbolicationFlag = അസത്യം;
സ്ഥിതം ManagedStatic<std::string> CrashDiagnosticsDirectory;
നാമാകാശ {
ഘടന CreateDisableSymbolication {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<തർക്കം, സത്യം>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ഘടന CreateCrashDiagnosticsDir {
  സ്ഥിതം ശൂന്യം *call() {
    തിരികെ പുതിയ cl::opt<std::string, സത്യം>(
        "crash-diagnostics-dir", cl::മൂല്യം_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
ശൂന്യം llvm::initSignalsOptions() {
  സ്ഥിതം ManagedStatic<cl::opt<തർക്കം, സത്യം>, CreateDisableSymbolication>
      DisableSymbolication;
  സ്ഥിതം ManagedStatic<cl::opt<std::string, സത്യം>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr അക്ഷരം DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr അക്ഷരം LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr അക്ഷരം EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ഘടന CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  ശൂന്യം *Cookie;
  എണ്ണൽ വർഗ്ഗം Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

സ്ഥിതം constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
സ്ഥിതം std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  സ്ഥിതം std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  തിരികെ callbacks;
}

// Signal-safe.
ശൂന്യം sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  വേണ്ടി (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    എങ്കിൽ (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      തുടരുക;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
സ്ഥിതം ശൂന്യം തിരുകുക_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                ശൂന്യം *Cookie) {
  വേണ്ടി (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    എങ്കിൽ (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      തുടരുക;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    തിരികെ;
  }
  report_fatal_error("too many signal callbacks already registered");
}

സ്ഥിതം തർക്കം findModulesAndOffsets(ശൂന്യം **അട്ടി_Trace, പൂർണ്ണസംഖ്യ Depth,
                                  സ്ഥിരം അക്ഷരം **Modules, intptr_t *Offsets,
                                  സ്ഥിരം അക്ഷരം *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
സ്ഥിതം Formatted_സംഖ്യ format_ptr(ശൂന്യം *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * വലുപ്പം(ശൂന്യം *);
  തിരികെ format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(ശൂന്യം **AddressList, unsigned Address_എണ്ണം,
                      സ്ഥിരം അക്ഷരം *MainExecutableName,
                      സ്ഥിരം std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<സ്ഥിരം അക്ഷരം *, 0> Modules(Address_എണ്ണം, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_എണ്ണം, 0);
  എങ്കിൽ (!findModulesAndOffsets(AddressList, Address_എണ്ണം, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    തിരികെ {};
  പൂർണ്ണസംഖ്യ നിവേശം_FD;
  SmallString<32> നിവേശം_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", നിവേശം_FD, നിവേശം_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover നിവേശം_Remover(നിവേശം_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream നിവേശം(നിവേശം_FD, സത്യം);
    വേണ്ടി (unsigned AddrIdx = 0; AddrIdx < Address_എണ്ണം; AddrIdx++) {
      എങ്കിൽ (Modules[AddrIdx])
        നിവേശം << Modules[AddrIdx] << " " << (ശൂന്യം *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {നിവേശം_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  പൂർണ്ണസംഖ്യ RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  എങ്കിൽ (RunResult != 0)
    തിരികെ {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  എങ്കിൽ (!OutputBuf)
    തിരികെ {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> വരികൾ;
  Output.split(വരികൾ, "\n");
  auto *Cur_വരി = വരികൾ.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  വേണ്ടി (unsigned AddrIdx = 0; AddrIdx < Address_എണ്ണം; AddrIdx++) {
    എങ്കിൽ (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      തുടരുക;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    വേണ്ടി (;;) {
      എങ്കിൽ (Cur_വരി == വരികൾ.end())
        തിരികെ {};
      StringRef FunctionName = *Cur_വരി++;
      എങ്കിൽ (FunctionName.empty())
        നിർത്തുക;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      എങ്കിൽ (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      എങ്കിൽ (Cur_വരി == വരികൾ.end())
        തിരികെ {};
      StringRef File_വരി_Info = *Cur_വരി++;
      എങ്കിൽ (!File_വരി_Info.starts_with("??")) {
        OS << File_വരി_Info;
      } അല്ലെങ്കിൽ {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  തിരികെ Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  എങ്കിൽ (സ്ഥിരം അക്ഷരം *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } അല്ലെങ്കിൽ എങ്കിൽ (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    എങ്കിൽ (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  എങ്കിൽ (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  തിരികെ LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
സ്ഥിതം തർക്കം print_Symbolized_അട്ടി_Trace(StringRef Argv0, ശൂന്യം **അട്ടി_Trace,
                                      പൂർണ്ണസംഖ്യ Depth, llvm::raw_ostream &OS) {
  എങ്കിൽ (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    തിരികെ അസത്യം;

  // Don't recursively invoke the llvm-symbolizer binary.
  എങ്കിൽ (Argv0.contains("llvm-symbolizer"))
    തിരികെ അസത്യം;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  എങ്കിൽ (!LLVMSymbolizerPathOrErr)
    തിരികെ അസത്യം;
  സ്ഥിരം std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      അട്ടി_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  എങ്കിൽ (!SymbolizedAddressesOpt)
    തിരികെ അസത്യം;
  വേണ്ടി (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << വലത്_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  തിരികെ സത്യം;
}

#എങ്കിൽ LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
ശൂന്യം sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  പ്രതിജ്ഞാ(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<ശൂന്യം *, 0> AddressList(Addresses.begin(), Addresses.end());
  എങ്കിൽ (AddressList.empty())
    തിരികെ;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  എങ്കിൽ (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  സ്ഥിരം std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  എങ്കിൽ (!SymbolizedAddressesOpt)
    തിരികെ;
  വേണ്ടി (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.തള്ളുക_back(SymbolizedFrame.second);
  }
  തിരികെ;
}
#endif

സ്ഥിതം തർക്കം printMarkupContext(raw_ostream &OS, സ്ഥിരം അക്ഷരം *MainExecutableName);

LLVM_ATTRIBUTE_USED
സ്ഥിതം തർക്കം print_Markup_അട്ടി_Trace(StringRef Argv0, ശൂന്യം **അട്ടി_Trace, പൂർണ്ണസംഖ്യ Depth,
                                  raw_ostream &OS) {
  സ്ഥിരം അക്ഷരം *Env = getenv(EnableSymbolizerMarkupEnv);
  എങ്കിൽ (!Env || !*Env)
    തിരികെ അസത്യം;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  എങ്കിൽ (!printMarkupContext(OS, MainExecutableName.c_str()))
    തിരികെ അസത്യം;
  വേണ്ടി (പൂർണ്ണസംഖ്യ I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, അട്ടി_Trace[I]);
  തിരികെ സത്യം;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#അന്തര്ഭാവ "Unix/Signals.inc"
#endif
#ifdef _WIN32
#അന്തര്ഭാവ "Windows/Signals.inc"
#endif
