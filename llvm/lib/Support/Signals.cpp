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

#ضمن "llvm/Support/Signals.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/FileUtilities.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Program.h"
#ضمن "llvm/Support/StringSaver.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <array>
#ضمن <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using نطاق llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ساكن منطقي DisableSymbolicationFlag = שקר;
ساكن ManagedStatic<std::string> CrashDiagnosticsDirectory;
نطاق {
بنية CreateDisableSymbolication {
  ساكن فراغ *call() {
    החזר جديد cl::opt<منطقي, אמת>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
بنية CreateCrashDiagnosticsDir {
  ساكن فراغ *call() {
    החזר جديد cl::opt<std::string, אמת>(
        "crash-diagnostics-dir", cl::value_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
فراغ llvm::initSignalsOptions() {
  ساكن ManagedStatic<cl::opt<منطقي, אמת>, CreateDisableSymbolication>
      DisableSymbolication;
  ساكن ManagedStatic<cl::opt<std::string, אמת>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr حرف DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr حرف LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr حرف EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
بنية CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  فراغ *Cookie;
  تعداد מחלקה Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

ساكن constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  החזר callbacks;
}

// Signal-safe.
فراغ sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  עבור (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    אם (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
ساكن فراغ insertSignalHandler(sys::SignalHandlerCallback FnPtr,
                                فراغ *Cookie) {
  עבור (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    אם (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    החזר;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ساكن منطقي findModulesAndOffsets(فراغ **StackTrace, صحيح Depth,
                                  ثابت حرف **Modules, intptr_t *Offsets,
                                  ثابت حرف *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ساكن FormattedNumber format_ptr(فراغ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * حجم(فراغ *);
  החזר format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(فراغ **AddressList, unsigned AddressCount,
                      ثابت حرف *MainExecutableName,
                      ثابت std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<ثابت حرف *, 0> Modules(AddressCount, nullptr);
  SmallVector<intptr_t, 0> Offsets(AddressCount, 0);
  אם (!findModulesAndOffsets(AddressList, AddressCount, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    החזר {};
  صحيح InputFD;
  SmallString<32> InputFile, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", InputFD, InputFile);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover InputRemover(InputFile.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream Input(InputFD, אמת);
    עבור (unsigned AddrIdx = 0; AddrIdx < AddressCount; AddrIdx++) {
      אם (Modules[AddrIdx])
        Input << Modules[AddrIdx] << " " << (فراغ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {InputFile.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  صحيح RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  אם (RunResult != 0)
    החזר {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  אם (!OutputBuf)
    החזר {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> Lines;
  Output.split(Lines, "\n");
  auto *CurLine = Lines.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  עבור (unsigned AddrIdx = 0; AddrIdx < AddressCount; AddrIdx++) {
    אם (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      استمر;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    עבור (;;) {
      אם (CurLine == Lines.end())
        החזר {};
      StringRef FunctionName = *CurLine++;
      אם (FunctionName.empty())
        اكسر;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      אם (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      אם (CurLine == Lines.end())
        החזר {};
      StringRef FileLineInfo = *CurLine++;
      אם (!FileLineInfo.starts_with("??")) {
        OS << FileLineInfo;
      } אחרת {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  החזר Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  אם (ثابت حرف *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } אחרת אם (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    אם (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  אם (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  החזר LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
ساكن منطقي printSymbolizedStackTrace(StringRef Argv0, فراغ **StackTrace,
                                      صحيح Depth, llvm::raw_ostream &OS) {
  אם (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    החזר שקר;

  // Don't recursively invoke the llvm-symbolizer binary.
  אם (Argv0.contains("llvm-symbolizer"))
    החזר שקר;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  אם (!LLVMSymbolizerPathOrErr)
    החזר שקר;
  ثابت std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      StackTrace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  אם (!SymbolizedAddressesOpt)
    החזר שקר;
  עבור (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << right_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  החזר אמת;
}

#אם LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
فراغ sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<فراغ *, 0> AddressList(Addresses.begin(), Addresses.end());
  אם (AddressList.empty())
    החזר;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  אם (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  אם (!SymbolizedAddressesOpt)
    החזר;
  עבור (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.push_back(SymbolizedFrame.second);
  }
  החזר;
}
#endif

ساكن منطقي printMarkupContext(raw_ostream &OS, ثابت حرف *MainExecutableName);

LLVM_ATTRIBUTE_USED
ساكن منطقي printMarkupStackTrace(StringRef Argv0, فراغ **StackTrace, صحيح Depth,
                                  raw_ostream &OS) {
  ثابت حرف *Env = getenv(EnableSymbolizerMarkupEnv);
  אם (!Env || !*Env)
    החזר שקר;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  אם (!printMarkupContext(OS, MainExecutableName.c_str()))
    החזר שקר;
  עבור (صحيح I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, StackTrace[I]);
  החזר אמת;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ضمن "Unix/Signals.inc"
#endif
#ifdef _WIN32
#ضمن "Windows/Signals.inc"
#endif
