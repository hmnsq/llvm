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

#অন্তর্ভাৱ "llvm/Support/Signals.h"

#অন্তর্ভাৱ "DebugOptions.h"

#অন্তর্ভাৱ "llvm/ADT/StringRef.h"
#অন্তর্ভাৱ "llvm/Config/llvm-config.h"
#অন্তর্ভাৱ "llvm/Support/CommandLine.h"
#অন্তর্ভাৱ "llvm/Support/ErrorOr.h"
#অন্তর্ভাৱ "llvm/Support/FileSystem.h"
#অন্তর্ভাৱ "llvm/Support/FileUtilities.h"
#অন্তর্ভাৱ "llvm/Support/Format.h"
#অন্তর্ভাৱ "llvm/Support/FormatVariadic.h"
#অন্তর্ভাৱ "llvm/Support/IOSandbox.h"
#অন্তর্ভাৱ "llvm/Support/ManagedStatic.h"
#অন্তর্ভাৱ "llvm/Support/MemoryBuffer.h"
#অন্তর্ভাৱ "llvm/Support/Path.h"
#অন্তর্ভাৱ "llvm/Support/Program.h"
#অন্তর্ভাৱ "llvm/Support/StringSaver.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"
#অন্তর্ভাৱ <array>
#অন্তর্ভাৱ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using নামাকাশ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
স্থির তার্কিক DisableSymbolicationFlag = মিছা;
স্থির ManagedStatic<std::string> CrashDiagnosticsDirectory;
নামাকাশ {
সংরচনা CreateDisableSymbolication {
  স্থির শূন্য *call() {
    ঘূৰাই নূতন cl::opt<তার্কিক, সঁচা>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
সংরচনা CreateCrashDiagnosticsDir {
  স্থির শূন্য *call() {
    ঘূৰাই নূতন cl::opt<std::string, সঁচা>(
        "crash-diagnostics-dir", cl::মান_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
শূন্য llvm::initSignalsOptions() {
  স্থির ManagedStatic<cl::opt<তার্কিক, সঁচা>, CreateDisableSymbolication>
      DisableSymbolication;
  স্থির ManagedStatic<cl::opt<std::string, সঁচা>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr ৱর্ণ DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr ৱর্ণ LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr ৱর্ণ EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
সংরচনা CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  শূন্য *Cookie;
  গণনা শ্ৰেণী Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

স্থির constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
স্থির std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  স্থির std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  ঘূৰাই callbacks;
}

// Signal-safe.
শূন্য sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  বাবে (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    যদি (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      অনুৱর্ত;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
স্থির শূন্য ঢোকাও_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                শূন্য *Cookie) {
  বাবে (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    যদি (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      অনুৱর্ত;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    ঘূৰাই;
  }
  report_fatal_error("too many signal callbacks already registered");
}

স্থির তার্কিক findModulesAndOffsets(শূন্য **স্তূপ_Trace, পূর্ণাঙ্ক Depth,
                                  ধ্রুৱ ৱর্ণ **Modules, intptr_t *Offsets,
                                  ধ্রুৱ ৱর্ণ *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
স্থির Formatted_সংখ্যা format_ptr(শূন্য *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * পরিমাণ(শূন্য *);
  ঘূৰাই format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(শূন্য **AddressList, unsigned Address_গুনতি,
                      ধ্রুৱ ৱর্ণ *MainExecutableName,
                      ধ্রুৱ std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<ধ্রুৱ ৱর্ণ *, 0> Modules(Address_গুনতি, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_গুনতি, 0);
  যদি (!findModulesAndOffsets(AddressList, Address_গুনতি, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    ঘূৰাই {};
  পূর্ণাঙ্ক প্ৰবেশ_FD;
  SmallString<32> প্ৰবেশ_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", প্ৰবেশ_FD, প্ৰবেশ_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover প্ৰবেশ_Remover(প্ৰবেশ_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream প্ৰবেশ(প্ৰবেশ_FD, সঁচা);
    বাবে (unsigned AddrIdx = 0; AddrIdx < Address_গুনতি; AddrIdx++) {
      যদি (Modules[AddrIdx])
        প্ৰবেশ << Modules[AddrIdx] << " " << (শূন্য *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {প্ৰবেশ_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  পূর্ণাঙ্ক RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  যদি (RunResult != 0)
    ঘূৰাই {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  যদি (!OutputBuf)
    ঘূৰাই {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> সাৰিগুলি;
  Output.split(সাৰিগুলি, "\n");
  auto *Cur_সাৰি = সাৰিগুলি.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  বাবে (unsigned AddrIdx = 0; AddrIdx < Address_গুনতি; AddrIdx++) {
    যদি (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      অনুৱর্ত;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    বাবে (;;) {
      যদি (Cur_সাৰি == সাৰিগুলি.end())
        ঘূৰাই {};
      StringRef FunctionName = *Cur_সাৰি++;
      যদি (FunctionName.empty())
        ভঙ্গ;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      যদি (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      যদি (Cur_সাৰি == সাৰিগুলি.end())
        ঘূৰাই {};
      StringRef File_সাৰি_Info = *Cur_সাৰি++;
      যদি (!File_সাৰি_Info.starts_with("??")) {
        OS << File_সাৰি_Info;
      } অন্যথা {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  ঘূৰাই Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  যদি (ধ্রুৱ ৱর্ণ *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } অন্যথা যদি (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    যদি (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  যদি (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  ঘূৰাই LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
স্থির তার্কিক print_Symbolized_স্তূপ_Trace(StringRef Argv0, শূন্য **স্তূপ_Trace,
                                      পূর্ণাঙ্ক Depth, llvm::raw_ostream &OS) {
  যদি (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    ঘূৰাই মিছা;

  // Don't recursively invoke the llvm-symbolizer binary.
  যদি (Argv0.contains("llvm-symbolizer"))
    ঘূৰাই মিছা;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  যদি (!LLVMSymbolizerPathOrErr)
    ঘূৰাই মিছা;
  ধ্রুৱ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      স্তূপ_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  যদি (!SymbolizedAddressesOpt)
    ঘূৰাই মিছা;
  বাবে (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << ডান_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  ঘূৰাই সঁচা;
}

#যদি LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
শূন্য sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  প্রতিজ্ঞা(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<শূন্য *, 0> AddressList(Addresses.begin(), Addresses.end());
  যদি (AddressList.empty())
    ঘূৰাই;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  যদি (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ধ্রুৱ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  যদি (!SymbolizedAddressesOpt)
    ঘূৰাই;
  বাবে (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.ঠেলো_back(SymbolizedFrame.second);
  }
  ঘূৰাই;
}
#endif

স্থির তার্কিক printMarkupContext(raw_ostream &OS, ধ্রুৱ ৱর্ণ *MainExecutableName);

LLVM_ATTRIBUTE_USED
স্থির তার্কিক print_Markup_স্তূপ_Trace(StringRef Argv0, শূন্য **স্তূপ_Trace, পূর্ণাঙ্ক Depth,
                                  raw_ostream &OS) {
  ধ্রুৱ ৱর্ণ *Env = getenv(EnableSymbolizerMarkupEnv);
  যদি (!Env || !*Env)
    ঘূৰাই মিছা;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  যদি (!printMarkupContext(OS, MainExecutableName.c_str()))
    ঘূৰাই মিছা;
  বাবে (পূর্ণাঙ্ক I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, স্তূপ_Trace[I]);
  ঘূৰাই সঁচা;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#অন্তর্ভাৱ "Unix/Signals.inc"
#endif
#ifdef _WIN32
#অন্তর্ভাৱ "Windows/Signals.inc"
#endif
