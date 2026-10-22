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
    प्रत्यागम नूतन cl::opt<तार्किक, सत्य>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
संरचना CreateCrashDiagnosticsDir {
  स्थिर शून्य *call() {
    प्रत्यागम नूतन cl::opt<std::string, सत्य>(
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

constexpr वर्ण DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr वर्ण LLVMSymbolizerPathEnv[] = "LLVM_SYMBOLIZER_PATH";
constexpr वर्ण EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

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
  प्रत्यागम callbacks;
}

// Signal-safe.
शून्य sys::RunSignalHandlers() {
  // Let's not interfere with stack trace symbolication and friends.
  स्वतः BypassSandbox = sandbox::scopedDisable();

  प्रति (CallbackAndCookie &RunMe : CallBacksToRun()) {
    स्वतः Expected = CallbackAndCookie::Status::Initialized;
    स्वतः Desired = CallbackAndCookie::Status::Executing;
    यदि (!RunMe.Flag.compare_exchange_strong(Expected, Desired))
      अनुवर्त;
    (*RunMe.Callback)(RunMe.Cookie);
    RunMe.Callback = nullptr;
    RunMe.Cookie = nullptr;
    RunMe.Flag.store(CallbackAndCookie::Status::Empty);
  }
}

// Signal-safe.
स्थिर शून्य जोड़ो_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                शून्य *Cookie) {
  प्रति (CallbackAndCookie &SetMe : CallBacksToRun()) {
    स्वतः Expected = CallbackAndCookie::Status::Empty;
    स्वतः Desired = CallbackAndCookie::Status::Initializing;
    यदि (!SetMe.Flag.compare_exchange_strong(Expected, Desired))
      अनुवर्त;
    SetMe.Callback = FnPtr;
    SetMe.Cookie = Cookie;
    SetMe.Flag.store(CallbackAndCookie::Status::Initialized);
    प्रत्यागम;
  }
  report_fatal_error("too many signal callbacks already registered");
}

स्थिर तार्किक findModulesAndOffsets(शून्य **ढेर_Trace, पूर्णाङ्क Depth,
                                  ध्रुव वर्ण **Modules, intptr_t *Offsets,
                                  ध्रुव वर्ण *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
स्थिर Formatted_संख्या format_ptr(शून्य *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  अचिन्हित PtrWidth = 2 + 2 * परिमाण(शून्य *);
  प्रत्यागम format_hex((uint64_t)PC, PtrWidth);
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
collectAddressSymbols(शून्य **AddressList, अचिन्हित Address_गिनती,
                      ध्रुव वर्ण *MainExecutableName,
                      ध्रुव std::string &LLVMSymbolizerPath) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  SmallVector<ध्रुव वर्ण *, 0> Modules(Address_गिनती, nullptr);
  SmallVector<intptr_t, 0> Offsets(Address_गिनती, 0);
  यदि (!findModulesAndOffsets(AddressList, Address_गिनती, Modules.data(),
                             Offsets.data(), MainExecutableName, StrPool))
    प्रत्यागम {};
  पूर्णाङ्क निवेश_FD;
  SmallString<32> निवेश_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", निवेश_FD, निवेश_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover निवेश_Remover(निवेश_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream निवेश(निवेश_FD, सत्य);
    प्रति (अचिन्हित AddrIdx = 0; AddrIdx < Address_गिनती; AddrIdx++) {
      यदि (Modules[AddrIdx])
        निवेश << Modules[AddrIdx] << " " << (शून्य *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {निवेश_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef Args[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ifdef _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#endif
                      "--demangle"};
  पूर्णाङ्क RunResult =
      sys::ExecuteAndWait(LLVMSymbolizerPath, Args, std::nullopt, Redirects);
  यदि (RunResult != 0)
    प्रत्यागम {};

  SmallVector<std::pair<अचिन्हित, std::string>, 0> Result;
  स्वतः OutputBuf = MemoryBuffer::getFile(OutputFile.c_str());
  यदि (!OutputBuf)
    प्रत्यागम {};
  StringRef Output = OutputBuf.get()->getBuffer();
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
  प्रति (अचिन्हित AddrIdx = 0; AddrIdx < Address_गिनती; AddrIdx++) {
    यदि (!Modules[AddrIdx]) {
      स्वतः &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]);
      अनुवर्त;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    प्रति (;;) {
      यदि (Cur_पंक्ति == पंक्तियाँ.end())
        प्रत्यागम {};
      StringRef FunctionName = *Cur_पंक्ति++;
      यदि (FunctionName.empty())
        भङ्ग;
      स्वतः &SymbolizedFrame = Result.emplace_back(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(AddressList[AddrIdx]) << ' ';
      यदि (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      यदि (Cur_पंक्ति == पंक्तियाँ.end())
        प्रत्यागम {};
      StringRef File_पंक्ति_Info = *Cur_पंक्ति++;
      यदि (!File_पंक्ति_Info.starts_with("??")) {
        OS << File_पंक्ति_Info;
      } अन्यथा {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  प्रत्यागम Result;
}

ErrorOr<std::string> getLLVMSymbolizerPath(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = std::error_code();
  यदि (ध्रुव वर्ण *Path = दो_पर्या(LLVMSymbolizerPathEnv)) {
    LLVMSymbolizerPathOrErr = sys::findProgramByName(Path);
  } अन्यथा यदि (!Argv0.empty()) {
    StringRef Parent = llvm::sys::path::parent_path(Argv0);
    यदि (!Parent.empty())
      LLVMSymbolizerPathOrErr =
          sys::findProgramByName("llvm-symbolizer", Parent);
  }
  यदि (!LLVMSymbolizerPathOrErr)
    LLVMSymbolizerPathOrErr = sys::findProgramByName("llvm-symbolizer");
  प्रत्यागम LLVMSymbolizerPathOrErr;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_USED
स्थिर तार्किक print_Symbolized_ढेर_Trace(StringRef Argv0, शून्य **ढेर_Trace,
                                      पूर्णाङ्क Depth, llvm::raw_ostream &OS) {
  यदि (DisableSymbolicationFlag || दो_पर्या(DisableSymbolizationEnv))
    प्रत्यागम असत्य;

  // Don't recursively invoke the llvm-symbolizer binary.
  यदि (Argv0.contains("llvm-symbolizer"))
    प्रत्यागम असत्य;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath(Argv0);
  यदि (!LLVMSymbolizerPathOrErr)
    प्रत्यागम असत्य;
  ध्रुव std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);

  स्वतः SymbolizedAddressesOpt = collectAddressSymbols(
      ढेर_Trace, Depth, MainExecutableName.c_str(), LLVMSymbolizerPath);
  यदि (!SymbolizedAddressesOpt)
    प्रत्यागम असत्य;
  प्रति (अचिन्हित FrameNo = 0; FrameNo < SymbolizedAddressesOpt->size();
       ++FrameNo) {
    OS << दायाँ_justify(formatv("#{0}", FrameNo).str(), std::log10(Depth) + 2)
       << ' ' << (*SymbolizedAddressesOpt)[FrameNo].second << '\n';
  }
  प्रत्यागम सत्य;
}

#यदि LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
शून्य sys::symbolizeAddresses(AddressSet &Addresses,
                             SymbolizedAddressMap &SymbolizedAddresses) {
  प्रतिज्ञा(!DisableSymbolicationFlag && !दो_पर्या(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  स्वतः BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  SmallVector<शून्य *, 0> AddressList(Addresses.begin(), Addresses.end());
  यदि (AddressList.empty())
    प्रत्यागम;
  llvm::sort(AddressList);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizerPathOrErr = getLLVMSymbolizerPath();
  यदि (!LLVMSymbolizerPathOrErr)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ध्रुव std::string &LLVMSymbolizerPath = *LLVMSymbolizerPathOrErr;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(nullptr, nullptr);

  स्वतः SymbolizedAddressesOpt =
      collectAddressSymbols(AddressList.begin(), AddressList.size(),
                            MainExecutableName.c_str(), LLVMSymbolizerPath);
  यदि (!SymbolizedAddressesOpt)
    प्रत्यागम;
  प्रति (स्वतः SymbolizedFrame : *SymbolizedAddressesOpt) {
    SmallVector<std::string, 0> &SymbolizedAddrs =
        SymbolizedAddresses[AddressList[SymbolizedFrame.first]];
    SymbolizedAddrs.धकेलो_back(SymbolizedFrame.second);
  }
  प्रत्यागम;
}
#endif

स्थिर तार्किक printMarkupContext(raw_ostream &OS, ध्रुव वर्ण *MainExecutableName);

LLVM_ATTRIBUTE_USED
स्थिर तार्किक print_Markup_ढेर_Trace(StringRef Argv0, शून्य **ढेर_Trace, पूर्णाङ्क Depth,
                                  raw_ostream &OS) {
  ध्रुव वर्ण *Env = दो_पर्या(EnableSymbolizerMarkupEnv);
  यदि (!Env || !*Env)
    प्रत्यागम असत्य;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(nullptr, nullptr);
  यदि (!printMarkupContext(OS, MainExecutableName.c_str()))
    प्रत्यागम असत्य;
  प्रति (पूर्णाङ्क I = 0; I < Depth; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ढेर_Trace[I]);
  प्रत्यागम सत्य;
}

// Include the platform-specific parts of this class.
#ifdef LLVM_ON_UNIX
#अन्तर्भाव "Unix/Signals.inc"
#endif
#ifdef _WIN32
#अन्तर्भाव "Windows/Signals.inc"
#endif
