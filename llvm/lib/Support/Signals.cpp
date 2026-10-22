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

#include "llvm/Support/Signals.h"

#include "DebugOptions.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/Config/llvm-config.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FileUtilities.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/IOSandbox.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/raw_ostream.h"
#include <array>
#include <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using namespace llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
static bool DisableSymbolicationFlag = false;
static ManagedStatic<std::string> CrashDiagnosticsDirectory;
namespace {
struct CreateDisableSymbolication {
  static void *call() {
    반환 new cl::opt<bool, true>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
struct CreateCrashDiagnosticsDir {
  static void *call() {
    반환 new cl::opt<std::string, true>(
        "crash-diagnostics-dir", cl::value_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
void llvm::initSignalsOptions() {
  static ManagedStatic<cl::opt<bool, true>, CreateDisableSymbolication>
      DisableSymbolication;
  static ManagedStatic<cl::opt<std::string, true>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr char DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr char LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr char EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
struct CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  void *Cookie;
  enum class Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

static constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
static std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  static std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  반환 callbacks;
}

// Signal-safe.
void sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  auto BypassSandbox = sandbox::scopedDisable();

  위해 (CallbackAndCookie &RunMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Initialized;
    auto Desired = CallbackAndCookie::Status::Executing;
    만약 (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      continue;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
static void insertSignalHandler(sys::SignalHandlerCallback FnPtr,
                                void *Cookie) {
  위해 (CallbackAndCookie &SetMe : CallBacksToRun()) {
    auto Expected = CallbackAndCookie::Status::Empty;
    auto Desired = CallbackAndCookie::Status::Initializing;
    만약 (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      continue;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    반환;
  }
  report_fatal_error("too many signal callbacks already registered");
}

static bool findModulesAndOffsets(void **StackTrace, 정수 Depth,
                                  const char **Modules, intptr_t *Offsets,
                                  const char *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
static FormattedNumber format_ptr(void *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  unsigned PtrWidth = 2 + 2 * sizeof(void *);
  반환 format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(void **AddressList, unsigned AddressCount,
                      const char *MainExecutableName,
                      const std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<const char *, 0> Modules(AddressCount, nullptr);
  SmallVector<intptr_t, 0> Offsets(AddressCount, 0);
  만약 (!findModulesAndOffsets(AddressList, AddressCount, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    반환 {};
  정수 InputFD;
  SmallString<32> InputFile, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", InputFD, InputFile);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover InputRemover(InputFile.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream Input(InputFD, true);
    위해 (unsigned AddrIdx = 0; AddrIdx < AddressCount; AddrIdx++) {
      만약 (Modules[AddrIdx])
        Input << Modules[AddrIdx] << " " << (void *)Offsets[AddrIdx] << "\n";
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
  정수 RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  만약 (RunResult != 0)
    반환 {};

  SmallVector<std::pair<unsigned, std::string>, 0> Result;
  auto OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  만약 (!OutputBuf)
    반환 {};
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
  위해 (unsigned AddrIdx = 0; AddrIdx < AddressCount; AddrIdx++) {
    만약 (!Modules[AddrIdx]) {
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      continue;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    위해 (;;) {
      만약 (CurLine == Lines.end())
        반환 {};
      StringRef FunctionName = *CurLine++;
      만약 (FunctionName.empty())
        break;
      auto &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      만약 (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      만약 (CurLine == Lines.end())
        반환 {};
      StringRef FileLineInfo = *CurLine++;
      만약 (!FileLineInfo.starts_with("??")) {
        OS << FileLineInfo;
      } 아니면 {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  반환 Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  만약 (const char *Path = getenv(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } 아니면 만약 (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    만약 (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  만약 (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  반환 LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
static bool printSymbolizedStackTrace(StringRef Argv0, void **StackTrace,
                                      정수 Depth, llvm::raw_ostream &OS) {
  만약 (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    반환 false;

  // Don't recursively invoke the llvm-symbolizer binary.
  만약 (Argv0.contains("llvm-symbolizer"))
    반환 false;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  만약 (!LLVMSymbolizerPathOrErr)
    반환 false;
  const std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt = collectAddressSymbols(
      StackTrace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  만약 (!SymbolizedAddressesOpt)
    반환 false;
  위해 (unsigned FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << right_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  반환 true;
}

#만약 LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
void sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  assert(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  auto BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<void *, 0> AddressList(Addresses.begin(), Addresses.end());
  만약 (AddressList.empty())
    반환;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  만약 (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  const std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  auto SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  만약 (!SymbolizedAddressesOpt)
    반환;
  위해 (auto SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.push_back(SymbolizedFrame.second);
  }
  반환;
}
#endif

static bool printMarkupContext(raw_ostream &OS, const char *MainExecutableName);

LLVM_ATTRIBUTE_USED
static bool printMarkupStackTrace(StringRef Argv0, void **StackTrace, 정수 Depth,
                                  raw_ostream &OS) {
  const char *Env = getenv(EnableSymbolizerMarkupEnv);
  만약 (!Env || !*Env)
    반환 false;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  만약 (!printMarkupContext(OS, MainExecutableName.c_str()))
    반환 false;
  위해 (정수 I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, StackTrace[I]);
  반환 true;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#include "Unix/Signals.inc"
#endif
#ifdef _WIN32
#include "Windows/Signals.inc"
#endif
