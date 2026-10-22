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

#अन्तर्भाव "llvm/Support/Signals.h"

#अन्तर्भाव "DebugOptions.h"

#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Config/llvm-config.h"
#अन्तर्भाव "llvm/Support/CommandLine.h"
#अन्तर्भाव "llvm/Support/ErrorOr.h"
#अन्तर्भाव "llvm/Support/FileSystem.h"
#अन्तर्भाव "llvm/Support/FileUtilities.h"
#अन्तर्भाव "llvm/Support/Format.h"
#अन्तर्भाव "llvm/Support/FormatVariadic.h"
#अन्तर्भाव "llvm/Support/IOSandbox.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/MemoryBuffer.h"
#अन्तर्भाव "llvm/Support/Path.h"
#अन्तर्भाव "llvm/Support/Program.h"
#अन्तर्भाव "llvm/Support/StringSaver.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"
#अन्तर्भाव <array>
#अन्तर्भाव <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

using नामाकाश llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
स्थिर तार्किक DisableSymbolicationFlag = असत्य;
स्थिर ManagedStatic<std::string> CrashDiagnosticsDirectory;
नामाकाश {
संरचना CreateDisableSymbolication {
  स्थिर शून्य *call() {
    परत नवीन cl::opt<तार्किक, सत्य>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
संरचना CreateCrashDiagnosticsDir {
  स्थिर शून्य *call() {
    परत नवीन cl::opt<std::string, सत्य>(
        "crash-diagnostics-dir", cl::मूल्य_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
शून्य llvm::initSignalsOptions() {
  स्थिर ManagedStatic<cl::opt<तार्किक, सत्य>, CreateDisableSymbolication>
      DisableSymbolication;
  स्थिर ManagedStatic<cl::opt<std::string, सत्य>, CreateCrashDiagnosticsDir>
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
संरचना CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  शून्य *Cookie;
  गणना वर्ग Status { Empty, Initializing, Initialized, Executing };
  std::atomic<Status> Flag;
};

स्थिर constexpr माप_प्रकार MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
स्थिर std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
CallBacksToRun() {
  स्थिर std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  परत callbacks;
}

// Signal-safe.
शून्य sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  स्वतः BypassSandbox = sandbox::scopedDisable();

  साठी (CallbackAndCookie &RunMe : CallBacksToRun()) {
    स्वतः Expected = CallbackAndCookie::Status::Initialized;
    स्वतः Desired = CallbackAndCookie::Status::Executing;
    जर (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      सुरूठेवा;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
स्थिर शून्य घाला_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                शून्य *Cookie) {
  साठी (CallbackAndCookie &SetMe : CallBacksToRun()) {
    स्वतः Expected = CallbackAndCookie::Status::Empty;
    स्वतः Desired = CallbackAndCookie::Status::Initializing;
    जर (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      सुरूठेवा;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    परत;
  }
  report_fatal_error("too many signal callbacks already registered");
}

स्थिर तार्किक findModulesAndOffsets(शून्य **चळत_Trace, पूर्णांक Depth,
                                  स्थिरांक अक्षर **Modules, intptr_t *Offsets,
                                  स्थिरांक अक्षर *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
स्थिर Formatted_संख्या format_ptr(शून्य *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  अचिन्हित PtrWidth = 2 + 2 * आकार(शून्य *);
  परत format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(शून्य **AddressList, अचिन्हित Address_मोजणी,
                      स्थिरांक अक्षर *MainExecutableName,
                      स्थिरांक std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<स्थिरांक अक्षर *, 0> Modules(Address_मोजणी, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_मोजणी, 0);
  जर (!findModulesAndOffsets(AddressList, Address_मोजणी, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    परत {};
  पूर्णांक आदान_FD;
  SmallString<32> आदान_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", आदान_FD, आदान_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover आदान_Remover(आदान_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream आदान(आदान_FD, सत्य);
    साठी (अचिन्हित AddrIdx = 0; AddrIdx < Address_मोजणी; AddrIdx++) {
      जर (Modules[AddrIdx])
        आदान << Modules[AddrIdx] << " " << (शून्य *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {आदान_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  पूर्णांक RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  जर (RunResult != 0)
    परत {};

  SmallVector<std::pair<अचिन्हित, std::string>, 0> Result;
  स्वतः OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  जर (!OutputBuf)
    परत {};
  StringRef Output = OutputBuf.get()->getBuffer();
  SmallVector<StringRef, 32> ओळी;
  Output.split(ओळी, "\n");
  स्वतः *Cur_ओळ = ओळी.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  साठी (अचिन्हित AddrIdx = 0; AddrIdx < Address_मोजणी; AddrIdx++) {
    जर (!Modules[AddrIdx]) {
      स्वतः &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      सुरूठेवा;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    साठी (;;) {
      जर (Cur_ओळ == ओळी.end())
        परत {};
      StringRef FunctionName = *Cur_ओळ++;
      जर (FunctionName.empty())
        थांबा;
      स्वतः &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      जर (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      जर (Cur_ओळ == ओळी.end())
        परत {};
      StringRef File_ओळ_Info = *Cur_ओळ++;
      जर (!File_ओळ_Info.starts_with("??")) {
        OS << File_ओळ_Info;
      } अन्यथा {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  परत Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  जर (स्थिरांक अक्षर *Path = दो_पर्या(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } अन्यथा जर (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    जर (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  जर (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  परत LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
स्थिर तार्किक print_Symbolized_चळत_Trace(StringRef Argv0, शून्य **चळत_Trace,
                                      पूर्णांक Depth, llvm::raw_ostream &OS) {
  जर (DisableSymbolicationFlag || दो_पर्या(DisableSymbolizationEnv))
    परत असत्य;

  // Don't recursively invoke the llvm-symbolizer binary.
  जर (Argv0.contains("llvm-symbolizer"))
    परत असत्य;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  जर (!LLVMSymbolizerPathOrErr)
    परत असत्य;
  स्थिरांक std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  स्वतः SymbolizedAddressesOpt = collectAddressSymbols(
      चळत_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  जर (!SymbolizedAddressesOpt)
    परत असत्य;
  साठी (अचिन्हित FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << उजवा_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  परत सत्य;
}

#जर LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
शून्य sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  प्रतिज्ञा(!DisableSymbolicationFlag && !दो_पर्या(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<शून्य *, 0> AddressList(Addresses.begin(), Addresses.end());
  जर (AddressList.empty())
    परत;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  जर (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  स्थिरांक std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  स्वतः SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  जर (!SymbolizedAddressesOpt)
    परत;
  साठी (स्वतः SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.ढकला_back(SymbolizedFrame.second);
  }
  परत;
}
#endif

स्थिर तार्किक printMarkupContext(raw_ostream &OS, स्थिरांक अक्षर *MainExecutableName);

LLVM_ATTRIBUTE_USED
स्थिर तार्किक print_Markup_चळत_Trace(StringRef Argv0, शून्य **चळत_Trace, पूर्णांक Depth,
                                  raw_ostream &OS) {
  स्थिरांक अक्षर *Env = दो_पर्या(EnableSymbolizerMarkupEnv);
  जर (!Env || !*Env)
    परत असत्य;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  जर (!printMarkupContext(OS, MainExecutableName.c_str()))
    परत असत्य;
  साठी (पूर्णांक I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, चळत_Trace[I]);
  परत सत्य;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Signals.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Signals.inc"
#endif
