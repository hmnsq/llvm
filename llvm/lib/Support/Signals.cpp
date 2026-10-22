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

#समावेश "llvm/Support/Signals.h"

#समावेश "DebugOptions.h"

#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Config/llvm-config.h"
#समावेश "llvm/Support/CommandLine.h"
#समावेश "llvm/Support/ErrorOr.h"
#समावेश "llvm/Support/FileSystem.h"
#समावेश "llvm/Support/FileUtilities.h"
#समावेश "llvm/Support/Format.h"
#समावेश "llvm/Support/FormatVariadic.h"
#समावेश "llvm/Support/IOSandbox.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/MemoryBuffer.h"
#समावेश "llvm/Support/Path.h"
#समावेश "llvm/Support/Program.h"
#समावेश "llvm/Support/StringSaver.h"
#समावेश "llvm/Support/raw_ostream.h"
#समावेश <array>
#समावेश <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using namespace llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
अटल bool DisableSymbolicationFlag = false;
अटल ManagedStatic<std::string> CrashDiagnosticsDirectory;
namespace {
काष्ठा CreateDisableSymbolication {
  अटल व्योम *call() {
    वापस नया cl::opt<bool, true>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
काष्ठा CreateCrashDiagnosticsDir {
  अटल व्योम *call() {
    वापस नया cl::opt<std::string, true>(
        "crash-diagnostics-dir", cl::मूल्य_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
व्योम llvm::initSignalsOptions() {
  अटल ManagedStatic<cl::opt<bool, true>, CreateDisableSymbolication>
      DisableSymbolication;
  अटल ManagedStatic<cl::opt<std::string, true>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr अक्षर DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr अक्षर LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr अक्षर EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
काष्ठा CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  व्योम *Cookie;
  क्रमागत श्रेणी Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

अटल constexpr माप_प्रकार MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
अटल std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  अटल std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  वापस callbacks;
}

// Signal-safe.
व्योम sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  स्वतः BypassSandbox = sandbox::scopedDisable();

  क्रम (CallbackAndCookie &RunMe : CallBacksToRun()) {
    स्वतः Expected = CallbackAndCookie::Status::Initialized;
    स्वतः Desired = CallbackAndCookie::Status::Executing;
    अगर (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      जारी;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
अटल व्योम जोड़ो_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                व्योम *Cookie) {
  क्रम (CallbackAndCookie &SetMe : CallBacksToRun()) {
    स्वतः Expected = CallbackAndCookie::Status::Empty;
    स्वतः Desired = CallbackAndCookie::Status::Initializing;
    अगर (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      जारी;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    वापस;
  }
  report_fatal_error("too many signal callbacks already registered");
}

अटल bool findModulesAndOffsets(व्योम **ढेर_Trace, पूर्णांक Depth,
                                  स्थिर अक्षर **Modules, intptr_t *Offsets,
                                  स्थिर अक्षर *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
अटल Formatted_संख्या format_ptr(व्योम *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  अचिन्हित PtrWidth = 2 + 2 * माप(व्योम *);
  वापस format_hex((uint64_t)PC, PtrWidth);
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
std::optional<SmallVector<std::pair<अचिन्हित, std::string>, 0>>
collectAddressSymbols(व्योम **AddressList, अचिन्हित Address_गिनती,
                      स्थिर अक्षर *MainExecutableName,
                      स्थिर std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<स्थिर अक्षर *, 0> Modules(Address_गिनती, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_गिनती, 0);
  अगर (!findModulesAndOffsets(AddressList, Address_गिनती, Modules.तथ्य(),
                             Offsets.तथ्य(), MainExecutableName, StrPool))
    वापस {};
  पूर्णांक निवेश_FD;
  SmallString<32> निवेश_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", निवेश_FD, निवेश_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover निवेश_Remover(निवेश_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream निवेश(निवेश_FD, true);
    क्रम (अचिन्हित AddrIdx = 0; AddrIdx < Address_गिनती; AddrIdx++) {
      अगर (Modules[AddrIdx])
        निवेश << Modules[AddrIdx] << " " << (व्योम *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {निवेश_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#अगर_घोषित _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#पूर्ण_अगर
                      "--demangle"};
  पूर्णांक Run_परिणाम =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  अगर (Run_परिणाम != 0)
    वापस {};

  SmallVector<std::pair<अचिन्हित, std::string>, 0> परिणाम;
  स्वतः OutputBuf = Memory_बफर::getFile(OutputFile.c_str());
  अगर (!OutputBuf)
    वापस {};
  StringRef Output = OutputBuf.लो()->get_बफर();
  SmallVector<StringRef, 32> पंक्तियाँ;
  Output.split(पंक्तियाँ, "\n");
  स्वतः *Cur_पंक्ति = पंक्तियाँ.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  क्रम (अचिन्हित AddrIdx = 0; AddrIdx < Address_गिनती; AddrIdx++) {
    अगर (!Modules[AddrIdx]) {
      स्वतः &SymbolizedFrame = परिणाम.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      जारी;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    क्रम (;;) {
      अगर (Cur_पंक्ति == पंक्तियाँ.end())
        वापस {};
      StringRef FunctionName = *Cur_पंक्ति++;
      अगर (FunctionName.empty())
        अवरोध;
      स्वतः &SymbolizedFrame = परिणाम.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      अगर (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      अगर (Cur_पंक्ति == पंक्तियाँ.end())
        वापस {};
      StringRef File_पंक्ति_Info = *Cur_पंक्ति++;
      अगर (!File_पंक्ति_Info.starts_with("??")) {
        OS << File_पंक्ति_Info;
      } अन्यथा {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  वापस परिणाम;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  अगर (स्थिर अक्षर *Path = दो_पर्या(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } अन्यथा अगर (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    अगर (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  अगर (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  वापस LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
अटल bool print_Symbolized_ढेर_Trace(StringRef Argv0, व्योम **ढेर_Trace,
                                      पूर्णांक Depth, llvm::raw_ostream &OS) {
  अगर (DisableSymbolicationFlag || दो_पर्या(DisableSymbolizationEnv))
    वापस false;

  // Don't recursively invoke the llvm-symbolizer binary.
  अगर (Argv0.contains("llvm-symbolizer"))
    वापस false;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  अगर (!LLVMSymbolizerPathOrErr)
    वापस false;
  स्थिर std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  स्वतः SymbolizedAddressesOpt = collectAddressSymbols(
      ढेर_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  अगर (!SymbolizedAddressesOpt)
    वापस false;
  क्रम (अचिन्हित FrameNo = 0; FrameNo < SymbolizedAddressesOpt->आकार_2();
       ++FrameNo) {
    OS << दायाँ_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  वापस true;
}

#अगर LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
व्योम sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  निश्चित(!DisableSymbolicationFlag && !दो_पर्या(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<व्योम *, 0> AddressList(Addresses.begin(), Addresses.end());
  अगर (AddressList.empty())
    वापस;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  अगर (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  स्थिर std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  स्वतः SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.आकार_2(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  अगर (!SymbolizedAddressesOpt)
    वापस;
  क्रम (स्वतः SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.धकेलो_back(SymbolizedFrame.second);
  }
  वापस;
}
#पूर्ण_अगर

अटल bool printMarkupContext(raw_ostream &OS, स्थिर अक्षर *MainExecutableName);

LLVM_ATTRIBUTE_USED
अटल bool print_Markup_ढेर_Trace(StringRef Argv0, व्योम **ढेर_Trace, पूर्णांक Depth,
                                  raw_ostream &OS) {
  स्थिर अक्षर *Env = दो_पर्या(EnableSymbolizerMarkupEnv);
  अगर (!Env || !*Env)
    वापस false;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  अगर (!printMarkupContext(OS, MainExecutableName.c_str()))
    वापस false;
  क्रम (पूर्णांक I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ढेर_Trace[I]);
  वापस true;
}

// Include the platform-specific parts of this class.
#अगर_घोषित LLVM_ON_UNIX
#समावेश "Unix/Signals.inc"
#पूर्ण_अगर
#अगर_घोषित _WIN32
#समावेश "Windows/Signals.inc"
#पूर्ण_अगर
