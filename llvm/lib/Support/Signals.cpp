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

#અન્તર્ભાવ "llvm/Support/Signals.h"

#અન્તર્ભાવ "DebugOptions.h"

#અન્તર્ભાવ "llvm/ADT/StringRef.h"
#અન્તર્ભાવ "llvm/Config/llvm-config.h"
#અન્તર્ભાવ "llvm/Support/CommandLine.h"
#અન્તર્ભાવ "llvm/Support/ErrorOr.h"
#અન્તર્ભાવ "llvm/Support/FileSystem.h"
#અન્તર્ભાવ "llvm/Support/FileUtilities.h"
#અન્તર્ભાવ "llvm/Support/Format.h"
#અન્તર્ભાવ "llvm/Support/FormatVariadic.h"
#અન્તર્ભાવ "llvm/Support/IOSandbox.h"
#અન્તર્ભાવ "llvm/Support/ManagedStatic.h"
#અન્તર્ભાવ "llvm/Support/MemoryBuffer.h"
#અન્તર્ભાવ "llvm/Support/Path.h"
#અન્તર્ભાવ "llvm/Support/Program.h"
#અન્તર્ભાવ "llvm/Support/StringSaver.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"
#અન્તર્ભાવ <array>
#અન્તર્ભાવ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using નામાકાશ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
સ્થિર તાર્કિક DisableSymbolicationFlag = ખોટું;
સ્થિર ManagedStatic<std::string> CrashDiagnosticsDirectory;
નામાકાશ {
સંરચના CreateDisableSymbolication {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<તાર્કિક, સાચું>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
સંરચના CreateCrashDiagnosticsDir {
  સ્થિર શૂન્ય *call() {
    પાછા નવું cl::opt<std::string, સાચું>(
        "crash-diagnostics-dir", cl::કિંમત_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
શૂન્ય llvm::initSignalsOptions() {
  સ્થિર ManagedStatic<cl::opt<તાર્કિક, સાચું>, CreateDisableSymbolication>
      DisableSymbolication;
  સ્થિર ManagedStatic<cl::opt<std::string, સાચું>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr અક્ષર DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr અક્ષર LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr અક્ષર EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
સંરચના CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  શૂન્ય *Cookie;
  ગણના વર્ગ Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

સ્થિર constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
સ્થિર std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  સ્થિર std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  પાછા callbacks;
}

// Signal-safe.
શૂન્ય sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  માટે (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    જો (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      ચાલુ;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
સ્થિર શૂન્ય ઉમેરો_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                શૂન્ય *Cookie) {
  માટે (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    જો (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      ચાલુ;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    પાછા;
  }
  report_fatal_error("too many signal callbacks already registered");
}

સ્થિર તાર્કિક findModulesAndOffsets(શૂન્ય **ઢગલો_Trace, પૂર્ણાંક Depth,
                                  અચલ અક્ષર **Modules, intptr_t *Offsets,
                                  અચલ અક્ષર *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
સ્થિર Formatted_સંખ્યા format_ptr(શૂન્ય *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * કદ(શૂન્ય *);
  પાછા format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(શૂન્ય **AddressList, unsigned Address_ગણતરી,
                      અચલ અક્ષર *MainExecutableName,
                      અચલ std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<અચલ અક્ષર *, 0> Modules(Address_ગણતરી, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_ગણતરી, 0);
  જો (!findModulesAndOffsets(AddressList, Address_ગણતરી, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    પાછા {};
  પૂર્ણાંક આગત_FD;
  SmallString<32> આગત_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", આગત_FD, આગત_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover આગત_Remover(આગત_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream આગત(આગત_FD, સાચું);
    માટે (unsigned AddrIdx = 0; AddrIdx < Address_ગણતરી; AddrIdx++) {
      જો (Modules[AddrIdx])
        આગત << Modules[AddrIdx] << " " << (શૂન્ય *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {આગત_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  પૂર્ણાંક RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  જો (RunResult != 0)
    પાછા {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  જો (!OutputBuf)
    પાછા {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> લીટીઓ;
  Output.split(લીટીઓ, "\n");
  auto *Cur_લીટી = લીટીઓ.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  માટે (unsigned AddrIdx = 0; AddrIdx < Address_ગણતરી; AddrIdx++) {
    જો (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      ચાલુ;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    માટે (;;) {
      જો (Cur_લીટી == લીટીઓ.end())
        પાછા {};
      StringRef FunctionName = *Cur_લીટી++;
      જો (FunctionName.empty())
        તોડો;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      જો (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      જો (Cur_લીટી == લીટીઓ.end())
        પાછા {};
      StringRef File_લીટી_Info = *Cur_લીટી++;
      જો (!File_લીટી_Info.starts_with("??")) {
        OS << File_લીટી_Info;
      } નહીંતર {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  પાછા Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  જો (અચલ અક્ષર *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } નહીંતર જો (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    જો (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  જો (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  પાછા LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
સ્થિર તાર્કિક print_Symbolized_ઢગલો_Trace(StringRef Argv0, શૂન્ય **ઢગલો_Trace,
                                      પૂર્ણાંક Depth, llvm::raw_ostream &OS) {
  જો (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    પાછા ખોટું;

  // Don't recursively invoke the llvm-symbolizer binary.
  જો (Argv0.contains("llvm-symbolizer"))
    પાછા ખોટું;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  જો (!LLVMSymbolizerPathOrErr)
    પાછા ખોટું;
  અચલ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      ઢગલો_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  જો (!SymbolizedAddressesOpt)
    પાછા ખોટું;
  માટે (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << જમણું_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  પાછા સાચું;
}

#જો LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
શૂન્ય sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  પ્રતિજ્ઞા(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<શૂન્ય *, 0> AddressList(Addresses.begin(), Addresses.end());
  જો (AddressList.empty())
    પાછા;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  જો (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  અચલ std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  જો (!SymbolizedAddressesOpt)
    પાછા;
  માટે (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.ધકેલો_back(SymbolizedFrame.second);
  }
  પાછા;
}
#endif

સ્થિર તાર્કિક printMarkupContext(raw_ostream &OS, અચલ અક્ષર *MainExecutableName);

LLVM_ATTRIBUTE_USED
સ્થિર તાર્કિક print_Markup_ઢગલો_Trace(StringRef Argv0, શૂન્ય **ઢગલો_Trace, પૂર્ણાંક Depth,
                                  raw_ostream &OS) {
  અચલ અક્ષર *Env = getenv(EnableSymbolizerMarkupEnv);
  જો (!Env || !*Env)
    પાછા ખોટું;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  જો (!printMarkupContext(OS, MainExecutableName.c_str()))
    પાછા ખોટું;
  માટે (પૂર્ણાંક I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ઢગલો_Trace[I]);
  પાછા સાચું;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#અન્તર્ભાવ "Unix/Signals.inc"
#endif
#ifdef _WIN32
#અન્તર્ભાવ "Windows/Signals.inc"
#endif
