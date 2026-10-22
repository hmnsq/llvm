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

#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"

#ಅನ್ತರ್ಭಾವ "DebugOptions.h"

#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/llvm-config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/CommandLine.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorOr.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileSystem.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FileUtilities.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Format.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/FormatVariadic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/IOSandbox.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/MemoryBuffer.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Path.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Program.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/StringSaver.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"
#ಅನ್ತರ್ಭಾವ <array>
#ಅನ್ತರ್ಭಾವ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using ನಾಮಾಕಾಶ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ಸ್ಥಿರ ತಾರ್ಕಿಕ DisableSymbolicationFlag = ಅಸತ್ಯ;
ಸ್ಥಿರ ManagedStatic<std::string> CrashDiagnosticsDirectory;
ನಾಮಾಕಾಶ {
ಸಂರಚನಾ CreateDisableSymbolication {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಪ್ರತ್ಯಾಗಮ ನೂತನ cl::opt<ತಾರ್ಕಿಕ, ಸತ್ಯ>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ಸಂರಚನಾ CreateCrashDiagnosticsDir {
  ಸ್ಥಿರ ಶೂನ್ಯ *call() {
    ಪ್ರತ್ಯಾಗಮ ನೂತನ cl::opt<std::string, ಸತ್ಯ>(
        "crash-diagnostics-dir", cl::ಮೌಲ್ಯ_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
ಶೂನ್ಯ llvm::initSignalsOptions() {
  ಸ್ಥಿರ ManagedStatic<cl::opt<ತಾರ್ಕಿಕ, ಸತ್ಯ>, CreateDisableSymbolication>
      DisableSymbolication;
  ಸ್ಥಿರ ManagedStatic<cl::opt<std::string, ಸತ್ಯ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr ವರ್ಣ DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr ವರ್ಣ LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr ವರ್ಣ EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ಸಂರಚನಾ CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  ಶೂನ್ಯ *Cookie;
  ಗಣನಾ ವರ್ಗ Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

ಸ್ಥಿರ constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ಸ್ಥಿರ std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  ಸ್ಥಿರ std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  ಪ್ರತ್ಯಾಗಮ callbacks;
}

// Signal-safe.
ಶೂನ್ಯ sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  ಪ್ರತಿ (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    ಯದಿ (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      ಅನುವರ್ತ;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                ಶೂನ್ಯ *Cookie) {
  ಪ್ರತಿ (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    ಯದಿ (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      ಅನುವರ್ತ;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    ಪ್ರತ್ಯಾಗಮ;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ಸ್ಥಿರ ತಾರ್ಕಿಕ findModulesAndOffsets(ಶೂನ್ಯ **ರಾಶಿ_Trace, ಪೂರ್ಣಾಙ್ಕ Depth,
                                  ಧ್ರುವ ವರ್ಣ **Modules, intptr_t *Offsets,
                                  ಧ್ರುವ ವರ್ಣ *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ಸ್ಥಿರ Formatted_ಸಂಖ್ಯೆ format_ptr(ಶೂನ್ಯ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * ಪರಿಮಾಣ(ಶೂನ್ಯ *);
  ಪ್ರತ್ಯಾಗಮ format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(ಶೂನ್ಯ **AddressList, unsigned Address_ಎಣಿಕೆ,
                      ಧ್ರುವ ವರ್ಣ *MainExecutableName,
                      ಧ್ರುವ std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<ಧ್ರುವ ವರ್ಣ *, 0> Modules(Address_ಎಣಿಕೆ, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_ಎಣಿಕೆ, 0);
  ಯದಿ (!findModulesAndOffsets(AddressList, Address_ಎಣಿಕೆ, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    ಪ್ರತ್ಯಾಗಮ {};
  ಪೂರ್ಣಾಙ್ಕ ಒಳಹರಿವು_FD;
  SmallString<32> ಒಳಹರಿವು_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", ಒಳಹರಿವು_FD, ಒಳಹರಿವು_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover ಒಳಹರಿವು_Remover(ಒಳಹರಿವು_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream ಒಳಹರಿವು(ಒಳಹರಿವು_FD, ಸತ್ಯ);
    ಪ್ರತಿ (unsigned AddrIdx = 0; AddrIdx < Address_ಎಣಿಕೆ; AddrIdx++) {
      ಯದಿ (Modules[AddrIdx])
        ಒಳಹರಿವು << Modules[AddrIdx] << " " << (ಶೂನ್ಯ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {ಒಳಹರಿವು_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  ಪೂರ್ಣಾಙ್ಕ RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  ಯದಿ (RunResult != 0)
    ಪ್ರತ್ಯಾಗಮ {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  ಯದಿ (!OutputBuf)
    ಪ್ರತ್ಯಾಗಮ {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> ಸಾಲುಗಳು;
  Output.split(ಸಾಲುಗಳು, "\n");
  auto *Cur_ಸಾಲು = ಸಾಲುಗಳು.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  ಪ್ರತಿ (unsigned AddrIdx = 0; AddrIdx < Address_ಎಣಿಕೆ; AddrIdx++) {
    ಯದಿ (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      ಅನುವರ್ತ;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    ಪ್ರತಿ (;;) {
      ಯದಿ (Cur_ಸಾಲು == ಸಾಲುಗಳು.end())
        ಪ್ರತ್ಯಾಗಮ {};
      StringRef FunctionName = *Cur_ಸಾಲು++;
      ಯದಿ (FunctionName.empty())
        ಭಙ್ಗ;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      ಯದಿ (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      ಯದಿ (Cur_ಸಾಲು == ಸಾಲುಗಳು.end())
        ಪ್ರತ್ಯಾಗಮ {};
      StringRef File_ಸಾಲು_Info = *Cur_ಸಾಲು++;
      ಯದಿ (!File_ಸಾಲು_Info.starts_with("??")) {
        OS << File_ಸಾಲು_Info;
      } ಅನ್ಯಥಾ {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  ಪ್ರತ್ಯಾಗಮ Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  ಯದಿ (ಧ್ರುವ ವರ್ಣ *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } ಅನ್ಯಥಾ ಯದಿ (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    ಯದಿ (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  ಯದಿ (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  ಪ್ರತ್ಯಾಗಮ LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
ಸ್ಥಿರ ತಾರ್ಕಿಕ print_Symbolized_ರಾಶಿ_Trace(StringRef Argv0, ಶೂನ್ಯ **ರಾಶಿ_Trace,
                                      ಪೂರ್ಣಾಙ್ಕ Depth, llvm::raw_ostream &OS) {
  ಯದಿ (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // Don't recursively invoke the llvm-symbolizer binary.
  ಯದಿ (Argv0.contains("llvm-symbolizer"))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  ಯದಿ (!LLVMSymbolizerPathOrErr)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಧ್ರುವ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      ರಾಶಿ_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  ಯದಿ (!SymbolizedAddressesOpt)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತಿ (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << ಬಲ_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

#ಯದಿ LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
ಶೂನ್ಯ sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  ಪ್ರತಿಜ್ಞಾ(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<ಶೂನ್ಯ *, 0> AddressList(Addresses.begin(), Addresses.end());
  ಯದಿ (AddressList.empty())
    ಪ್ರತ್ಯಾಗಮ;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  ಯದಿ (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ಧ್ರುವ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  ಯದಿ (!SymbolizedAddressesOpt)
    ಪ್ರತ್ಯಾಗಮ;
  ಪ್ರತಿ (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.ತಳ್ಳು_back(SymbolizedFrame.second);
  }
  ಪ್ರತ್ಯಾಗಮ;
}
#endif

ಸ್ಥಿರ ತಾರ್ಕಿಕ printMarkupContext(raw_ostream &OS, ಧ್ರುವ ವರ್ಣ *MainExecutableName);

LLVM_ATTRIBUTE_USED
ಸ್ಥಿರ ತಾರ್ಕಿಕ print_Markup_ರಾಶಿ_Trace(StringRef Argv0, ಶೂನ್ಯ **ರಾಶಿ_Trace, ಪೂರ್ಣಾಙ್ಕ Depth,
                                  raw_ostream &OS) {
  ಧ್ರುವ ವರ್ಣ *Env = getenv(EnableSymbolizerMarkupEnv);
  ಯದಿ (!Env || !*Env)
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  ಯದಿ (!printMarkupContext(OS, MainExecutableName.c_str()))
    ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
  ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ರಾಶಿ_Trace[I]);
  ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ಅನ್ತರ್ಭಾವ "Unix/Signals.inc"
#endif
#ifdef _WIN32
#ಅನ್ತರ್ಭಾವ "Windows/Signals.inc"
#endif
