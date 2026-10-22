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

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Signals.h"

#ଅନ୍ତର୍ଭାଵ "DebugOptions.h"

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringRef.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/llvm-config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/CommandLine.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ErrorOr.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileSystem.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FileUtilities.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Format.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/FormatVariadic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/IOSandbox.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/ManagedStatic.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/MemoryBuffer.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Path.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Program.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/StringSaver.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"
#ଅନ୍ତର୍ଭାଵ <array>
#ଅନ୍ତର୍ଭାଵ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using ନାମାକାଶ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ସ୍ଥିର ତାର୍କିକ DisableSymbolicationFlag = ମିଥ୍ୟା;
ସ୍ଥିର ManagedStatic<std::string> CrashDiagnosticsDirectory;
ନାମାକାଶ {
ସଂରଚନା CreateDisableSymbolication {
  ସ୍ଥିର ଶୂନ୍ୟ *call() {
    ଫେରନ୍ତୁ ନୂତନ cl::opt<ତାର୍କିକ, ସତ୍ୟ>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ସଂରଚନା CreateCrashDiagnosticsDir {
  ସ୍ଥିର ଶୂନ୍ୟ *call() {
    ଫେରନ୍ତୁ ନୂତନ cl::opt<std::string, ସତ୍ୟ>(
        "crash-diagnostics-dir", cl::ମୂଲ୍ୟ_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
ଶୂନ୍ୟ llvm::initSignalsOptions() {
  ସ୍ଥିର ManagedStatic<cl::opt<ତାର୍କିକ, ସତ୍ୟ>, CreateDisableSymbolication>
      DisableSymbolication;
  ସ୍ଥିର ManagedStatic<cl::opt<std::string, ସତ୍ୟ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr ଅକ୍ଷର DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr ଅକ୍ଷର LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr ଅକ୍ଷର EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ସଂରଚନା CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  ଶୂନ୍ୟ *Cookie;
  ଗଣନା ଶ୍ରେଣୀ Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

ସ୍ଥିର constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ସ୍ଥିର std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  ସ୍ଥିର std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  ଫେରନ୍ତୁ callbacks;
}

// Signal-safe.
ଶୂନ୍ୟ sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  ପାଇଁ (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    ଯଦି (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      ଜାରି;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
ସ୍ଥିର ଶୂନ୍ୟ ଭର୍ତ୍ତି_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                ଶୂନ୍ୟ *Cookie) {
  ପାଇଁ (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    ଯଦି (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      ଜାରି;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    ଫେରନ୍ତୁ;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ସ୍ଥିର ତାର୍କିକ findModulesAndOffsets(ଶୂନ୍ୟ **ଗଦା_Trace, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Depth,
                                  ଅଚଳ ଅକ୍ଷର **Modules, intptr_t *Offsets,
                                  ଅଚଳ ଅକ୍ଷର *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ସ୍ଥିର Formatted_ସଂଖ୍ୟା format_ptr(ଶୂନ୍ୟ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * ଆକାର(ଶୂନ୍ୟ *);
  ଫେରନ୍ତୁ format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(ଶୂନ୍ୟ **AddressList, unsigned Address_ଗଣନା,
                      ଅଚଳ ଅକ୍ଷର *MainExecutableName,
                      ଅଚଳ std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<ଅଚଳ ଅକ୍ଷର *, 0> Modules(Address_ଗଣନା, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_ଗଣନା, 0);
  ଯଦି (!findModulesAndOffsets(AddressList, Address_ଗଣନା, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    ଫେରନ୍ତୁ {};
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ନିବେଶ_FD;
  SmallString<32> ନିବେଶ_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", ନିବେଶ_FD, ନିବେଶ_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover ନିବେଶ_Remover(ନିବେଶ_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream ନିବେଶ(ନିବେଶ_FD, ସତ୍ୟ);
    ପାଇଁ (unsigned AddrIdx = 0; AddrIdx < Address_ଗଣନା; AddrIdx++) {
      ଯଦି (Modules[AddrIdx])
        ନିବେଶ << Modules[AddrIdx] << " " << (ଶୂନ୍ୟ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {ନିବେଶ_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  ଯଦି (RunResult != 0)
    ଫେରନ୍ତୁ {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  ଯଦି (!OutputBuf)
    ଫେରନ୍ତୁ {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> ଧାଡ଼ିଗୁଡ଼ିକ;
  Output.split(ଧାଡ଼ିଗୁଡ଼ିକ, "\n");
  auto *Cur_ଧାଡ଼ି = ଧାଡ଼ିଗୁଡ଼ିକ.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  ପାଇଁ (unsigned AddrIdx = 0; AddrIdx < Address_ଗଣନା; AddrIdx++) {
    ଯଦି (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      ଜାରି;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    ପାଇଁ (;;) {
      ଯଦି (Cur_ଧାଡ଼ି == ଧାଡ଼ିଗୁଡ଼ିକ.end())
        ଫେରନ୍ତୁ {};
      StringRef FunctionName = *Cur_ଧାଡ଼ି++;
      ଯଦି (FunctionName.empty())
        ଭାଙ୍ଗ;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      ଯଦି (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      ଯଦି (Cur_ଧାଡ଼ି == ଧାଡ଼ିଗୁଡ଼ିକ.end())
        ଫେରନ୍ତୁ {};
      StringRef File_ଧାଡ଼ି_Info = *Cur_ଧାଡ଼ି++;
      ଯଦି (!File_ଧାଡ଼ି_Info.starts_with("??")) {
        OS << File_ଧାଡ଼ି_Info;
      } ଅନ୍ୟଥା {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  ଫେରନ୍ତୁ Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  ଯଦି (ଅଚଳ ଅକ୍ଷର *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } ଅନ୍ୟଥା ଯଦି (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    ଯଦି (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  ଯଦି (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  ଫେରନ୍ତୁ LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
ସ୍ଥିର ତାର୍କିକ print_Symbolized_ଗଦା_Trace(StringRef Argv0, ଶୂନ୍ୟ **ଗଦା_Trace,
                                      ପୂର୍ଣ୍ଣସଂଖ୍ୟା Depth, llvm::raw_ostream &OS) {
  ଯଦି (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // Don't recursively invoke the llvm-symbolizer binary.
  ଯଦି (Argv0.contains("llvm-symbolizer"))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  ଯଦି (!LLVMSymbolizerPathOrErr)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ଅଚଳ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      ଗଦା_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  ଯଦି (!SymbolizedAddressesOpt)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ପାଇଁ (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << ଡାହାଣ_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

#ଯଦି LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
ଶୂନ୍ୟ sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  ପ୍ରତିଜ୍ଞା(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<ଶୂନ୍ୟ *, 0> AddressList(Addresses.begin(), Addresses.end());
  ଯଦି (AddressList.empty())
    ଫେରନ୍ତୁ;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  ଯଦି (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ଅଚଳ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  ଯଦି (!SymbolizedAddressesOpt)
    ଫେରନ୍ତୁ;
  ପାଇଁ (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.ଠେଲ_back(SymbolizedFrame.second);
  }
  ଫେରନ୍ତୁ;
}
#endif

ସ୍ଥିର ତାର୍କିକ printMarkupContext(raw_ostream &OS, ଅଚଳ ଅକ୍ଷର *MainExecutableName);

LLVM_ATTRIBUTE_USED
ସ୍ଥିର ତାର୍କିକ print_Markup_ଗଦା_Trace(StringRef Argv0, ଶୂନ୍ୟ **ଗଦା_Trace, ପୂର୍ଣ୍ଣସଂଖ୍ୟା Depth,
                                  raw_ostream &OS) {
  ଅଚଳ ଅକ୍ଷର *Env = getenv(EnableSymbolizerMarkupEnv);
  ଯଦି (!Env || !*Env)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  ଯଦି (!printMarkupContext(OS, MainExecutableName.c_str()))
    ଫେରନ୍ତୁ ମିଥ୍ୟା;
  ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ଗଦା_Trace[I]);
  ଫେରନ୍ତୁ ସତ୍ୟ;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#ଅନ୍ତର୍ଭାଵ "Unix/Signals.inc"
#endif
#ifdef _WIN32
#ଅନ୍ତର୍ଭାଵ "Windows/Signals.inc"
#endif
