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

#அந்தர்பாவ "llvm/Support/Signals.h"

#அந்தர்பாவ "DebugOptions.h"

#அந்தர்பாவ "llvm/ADT/StringRef.h"
#அந்தர்பாவ "llvm/Config/llvm-config.h"
#அந்தர்பாவ "llvm/Support/CommandLine.h"
#அந்தர்பாவ "llvm/Support/ErrorOr.h"
#அந்தர்பாவ "llvm/Support/FileSystem.h"
#அந்தர்பாவ "llvm/Support/FileUtilities.h"
#அந்தர்பாவ "llvm/Support/Format.h"
#அந்தர்பாவ "llvm/Support/FormatVariadic.h"
#அந்தர்பாவ "llvm/Support/IOSandbox.h"
#அந்தர்பாவ "llvm/Support/ManagedStatic.h"
#அந்தர்பாவ "llvm/Support/MemoryBuffer.h"
#அந்தர்பாவ "llvm/Support/Path.h"
#அந்தர்பாவ "llvm/Support/Program.h"
#அந்தர்பாவ "llvm/Support/StringSaver.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"
#அந்தர்பாவ <array>
#அந்தர்பாவ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using நாமாகாஶ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
நிலையான தருக்க DisableSymbolicationFlag = பொய்;
நிலையான ManagedStatic<std::string> CrashDiagnosticsDirectory;
நாமாகாஶ {
கட்டமைப்பு CreateDisableSymbolication {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<தருக்க, உண்மை>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
கட்டமைப்பு CreateCrashDiagnosticsDir {
  நிலையான வெறுமை *call() {
    திரும்பு புதிய cl::opt<std::string, உண்மை>(
        "crash-diagnostics-dir", cl::மதிப்பு_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
வெறுமை llvm::initSignalsOptions() {
  நிலையான ManagedStatic<cl::opt<தருக்க, உண்மை>, CreateDisableSymbolication>
      DisableSymbolication;
  நிலையான ManagedStatic<cl::opt<std::string, உண்மை>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr எழுத்து DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr எழுத்து LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr எழுத்து EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
கட்டமைப்பு CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  வெறுமை *Cookie;
  எண்ணிக்கை வகுப்பு Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

நிலையான constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
நிலையான std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  நிலையான std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  திரும்பு callbacks;
}

// Signal-safe.
வெறுமை sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  க்கு (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    எனில் (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      தொடர்;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
நிலையான வெறுமை செருகு_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                வெறுமை *Cookie) {
  க்கு (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    எனில் (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      தொடர்;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    திரும்பு;
  }
  report_fatal_error("too many signal callbacks already registered");
}

நிலையான தருக்க findModulesAndOffsets(வெறுமை **அடுக்கு_Trace, முழு Depth,
                                  மாறிலி எழுத்து **Modules, intptr_t *Offsets,
                                  மாறிலி எழுத்து *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
நிலையான Formatted_எண் format_ptr(வெறுமை *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * அளவு(வெறுமை *);
  திரும்பு format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(வெறுமை **AddressList, unsigned Address_மொத்தம்,
                      மாறிலி எழுத்து *MainExecutableName,
                      மாறிலி std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<மாறிலி எழுத்து *, 0> Modules(Address_மொத்தம், nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_மொத்தம், 0);
  எனில் (!findModulesAndOffsets(AddressList, Address_மொத்தம், Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    திரும்பு {};
  முழு உள்ளீடு_FD;
  SmallString<32> உள்ளீடு_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", உள்ளீடு_FD, உள்ளீடு_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover உள்ளீடு_Remover(உள்ளீடு_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream உள்ளீடு(உள்ளீடு_FD, உண்மை);
    க்கு (unsigned AddrIdx = 0; AddrIdx < Address_மொத்தம்; AddrIdx++) {
      எனில் (Modules[AddrIdx])
        உள்ளீடு << Modules[AddrIdx] << " " << (வெறுமை *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {உள்ளீடு_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  முழு RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  எனில் (RunResult != 0)
    திரும்பு {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  எனில் (!OutputBuf)
    திரும்பு {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> வரிகள்;
  Output.split(வரிகள், "\n");
  auto *Cur_வரி = வரிகள்.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  க்கு (unsigned AddrIdx = 0; AddrIdx < Address_மொத்தம்; AddrIdx++) {
    எனில் (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      தொடர்;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    க்கு (;;) {
      எனில் (Cur_வரி == வரிகள்.end())
        திரும்பு {};
      StringRef FunctionName = *Cur_வரி++;
      எனில் (FunctionName.empty())
        நிறுத்து;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      எனில் (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      எனில் (Cur_வரி == வரிகள்.end())
        திரும்பு {};
      StringRef File_வரி_Info = *Cur_வரி++;
      எனில் (!File_வரி_Info.starts_with("??")) {
        OS << File_வரி_Info;
      } இல்லையெனில் {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  திரும்பு Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  எனில் (மாறிலி எழுத்து *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } இல்லையெனில் எனில் (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    எனில் (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  எனில் (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  திரும்பு LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
நிலையான தருக்க print_Symbolized_அடுக்கு_Trace(StringRef Argv0, வெறுமை **அடுக்கு_Trace,
                                      முழு Depth, llvm::raw_ostream &OS) {
  எனில் (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    திரும்பு பொய்;

  // Don't recursively invoke the llvm-symbolizer binary.
  எனில் (Argv0.contains("llvm-symbolizer"))
    திரும்பு பொய்;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  எனில் (!LLVMSymbolizerPathOrErr)
    திரும்பு பொய்;
  மாறிலி std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      அடுக்கு_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  எனில் (!SymbolizedAddressesOpt)
    திரும்பு பொய்;
  க்கு (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << வலது_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  திரும்பு உண்மை;
}

#எனில் LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
வெறுமை sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  ப்ரதிஜ்ஞா(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<வெறுமை *, 0> AddressList(Addresses.begin(), Addresses.end());
  எனில் (AddressList.empty())
    திரும்பு;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  எனில் (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  மாறிலி std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  எனில் (!SymbolizedAddressesOpt)
    திரும்பு;
  க்கு (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.தள்ளு_back(SymbolizedFrame.second);
  }
  திரும்பு;
}
#endif

நிலையான தருக்க printMarkupContext(raw_ostream &OS, மாறிலி எழுத்து *MainExecutableName);

LLVM_ATTRIBUTE_USED
நிலையான தருக்க print_Markup_அடுக்கு_Trace(StringRef Argv0, வெறுமை **அடுக்கு_Trace, முழு Depth,
                                  raw_ostream &OS) {
  மாறிலி எழுத்து *Env = getenv(EnableSymbolizerMarkupEnv);
  எனில் (!Env || !*Env)
    திரும்பு பொய்;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  எனில் (!printMarkupContext(OS, MainExecutableName.c_str()))
    திரும்பு பொய்;
  க்கு (முழு I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, அடுக்கு_Trace[I]);
  திரும்பு உண்மை;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#அந்தர்பாவ "Unix/Signals.inc"
#endif
#ifdef _WIN32
#அந்தர்பாவ "Windows/Signals.inc"
#endif
