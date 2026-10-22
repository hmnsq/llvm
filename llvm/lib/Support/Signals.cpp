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

उपयुज्य नामाकाश llvm;

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
        "crash-diagnostics-dir", cl::मानम्_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
शून्य llvm::init_सङ्केताः_Options() {
  स्थिर ManagedStatic<cl::opt<तार्किक, सत्य>, CreateDisableSymbolication>
      DisableSymbolication;
  स्थिर ManagedStatic<cl::opt<std::string, सत्य>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr वर्ण DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr वर्ण LLVMSymbolizer_मार्गः_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr वर्ण EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
संरचना CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  शून्य *Cookie;
  गणना वर्ग स्थितिः { रिक्तानि_2, Initializing, Initialized, Executing };
  std::atomic<स्थितिः> Flag;
};

स्थिर constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
स्थिर std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_गन्तव्यम्_सततम्() {
  स्थिर std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  प्रत्यागम callbacks;
}

// Signal-safe.
शून्य sys::सततम्_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  स्वचालित BypassSandbox = sandbox::scopedDisable();

  प्रति (CallbackAndCookie &सततम्_Me : Call_Backs_गन्तव्यम्_सततम्()) {
    स्वचालित Expected = CallbackAndCookie::स्थितिः::Initialized;
    स्वचालित Desired = CallbackAndCookie::स्थितिः::Executing;
    यदि (!सततम्_Me.Flag.compare_exchange_strong(Expected, Desired))
      अनुवर्त;
    (*सततम्_Me.Callback)(सततम्_Me.Cookie);
    सततम्_Me.Callback = शून्यसूचकः;
    सततम्_Me.Cookie = शून्यसूचकः;
    सततम्_Me.Flag.store(CallbackAndCookie::स्थितिः::रिक्तानि_2);
  }
}

// Signal-safe.
स्थिर शून्य निवेशय_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                शून्य *Cookie) {
  प्रति (CallbackAndCookie &समुच्चयः_Me : Call_Backs_गन्तव्यम्_सततम्()) {
    स्वचालित Expected = CallbackAndCookie::स्थितिः::रिक्तानि_2;
    स्वचालित Desired = CallbackAndCookie::स्थितिः::Initializing;
    यदि (!समुच्चयः_Me.Flag.compare_exchange_strong(Expected, Desired))
      अनुवर्त;
    समुच्चयः_Me.Callback = FnPtr;
    समुच्चयः_Me.Cookie = Cookie;
    समुच्चयः_Me.Flag.store(CallbackAndCookie::स्थितिः::Initialized);
    प्रत्यागम;
  }
  report_fatal_error("too many signal callbacks already registered");
}

स्थिर तार्किक findModulesAndOffsets(शून्य **राशिः_Trace, पूर्णाङ्क गभीरता,
                                  ध्रुव वर्ण **Modules, intptr_t *Offsets,
                                  ध्रुव वर्ण *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
स्थिर Formatted_सङ्ख्या format_ptr(शून्य *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  अचिह्नित Ptr_विस्तारः = 2 + 2 * परिमाण(शून्य *);
  प्रत्यागम format_hex((uint64_t)PC, Ptr_विस्तारः);
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
std::optional<Small_सदिशम्<std::pair<अचिह्नित, std::string>, 0>>
collect_स्थानाङ्कः_Symbols(शून्य **स्थानाङ्कः_List, अचिह्नित स्थानाङ्कः_गणनम्,
                      ध्रुव वर्ण *MainExecutableName,
                      ध्रुव std::string &LLVMSymbolizer_मार्गः) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_सदिशम्<ध्रुव वर्ण *, 0> Modules(स्थानाङ्कः_गणनम्, शून्यसूचकः);
  Small_सदिशम्<intptr_t, 0> Offsets(स्थानाङ्कः_गणनम्, 0);
  यदि (!findModulesAndOffsets(स्थानाङ्कः_List, स्थानाङ्कः_गणनम्, Modules.दत्त(),
                             Offsets.दत्त(), MainExecutableName, StrPool))
    प्रत्यागम {};
  पूर्णाङ्क निवेशः_FD;
  SmallString<32> निवेशः_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", निवेशः_FD, निवेशः_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover निवेशः_Remover(निवेशः_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream निवेशः(निवेशः_FD, सत्य);
    प्रति (अचिह्नित AddrIdx = 0; AddrIdx < स्थानाङ्कः_गणनम्; AddrIdx++) {
      यदि (Modules[AddrIdx])
        निवेशः << Modules[AddrIdx] << " " << (शून्य *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {निवेशः_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef तर्काः[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#यदि_परिभाषितम् _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#यदि_अन्तः
                      "--demangle"};
  पूर्णाङ्क सततम्_परिणाम =
      sys::ExecuteAndWait(LLVMSymbolizer_मार्गः, तर्काः, std::nullopt, Redirects);
  यदि (सततम्_परिणाम != 0)
    प्रत्यागम {};

  Small_सदिशम्<std::pair<अचिह्नित, std::string>, 0> परिणाम;
  स्वचालित OutputBuf = स्मृतिः_बफर::getFile(OutputFile.c_str());
  यदि (!OutputBuf)
    प्रत्यागम {};
  StringRef Output = OutputBuf.get()->get_बफर();
  Small_सदिशम्<StringRef, 32> पङ्क्तयः;
  Output.विभज(पङ्क्तयः, "\n");
  स्वचालित *Cur_पङ्क्तिः = पङ्क्तयः.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  प्रति (अचिह्नित AddrIdx = 0; AddrIdx < स्थानाङ्कः_गणनम्; AddrIdx++) {
    यदि (!Modules[AddrIdx]) {
      स्वचालित &SymbolizedFrame = परिणाम.emplace_पृष्ठभागः(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(स्थानाङ्कः_List[AddrIdx]);
      अनुवर्त;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    प्रति (;;) {
      यदि (Cur_पङ्क्तिः == पङ्क्तयः.अन्तः())
        प्रत्यागम {};
      StringRef FunctionName = *Cur_पङ्क्तिः++;
      यदि (FunctionName.रिक्तानि())
        भङ्ग;
      स्वचालित &SymbolizedFrame = परिणाम.emplace_पृष्ठभागः(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(SymbolizedFrame.second);
      OS << format_ptr(स्थानाङ्कः_List[AddrIdx]) << ' ';
      यदि (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      यदि (Cur_पङ्क्तिः == पङ्क्तयः.अन्तः())
        प्रत्यागम {};
      StringRef File_पङ्क्तिः_Info = *Cur_पङ्क्तिः++;
      यदि (!File_पङ्क्तिः_Info.starts_with("??")) {
        OS << File_पङ्क्तिः_Info;
      } अन्यथा {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  प्रत्यागम परिणाम;
}

ErrorOr<std::string> get_LLVMSymbolizer_मार्गः(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_मार्गः_Or_Err = std::error_संहिता();
  यदि (ध्रुव वर्ण *मार्गः = getenv(LLVMSymbolizer_मार्गः_Env)) {
    LLVMSymbolizer_मार्गः_Or_Err = sys::find_क्रमादेशः_By_Name(मार्गः);
  } अन्यथा यदि (!Argv0.रिक्तानि()) {
    StringRef Parent = llvm::sys::मार्गः_2::parent_मार्गः(Argv0);
    यदि (!Parent.रिक्तानि())
      LLVMSymbolizer_मार्गः_Or_Err =
          sys::find_क्रमादेशः_By_Name("llvm-symbolizer", Parent);
  }
  यदि (!LLVMSymbolizer_मार्गः_Or_Err)
    LLVMSymbolizer_मार्गः_Or_Err = sys::find_क्रमादेशः_By_Name("llvm-symbolizer");
  प्रत्यागम LLVMSymbolizer_मार्गः_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_प्रयुक्तम्
स्थिर तार्किक print_Symbolized_राशिः_Trace(StringRef Argv0, शून्य **राशिः_Trace,
                                      पूर्णाङ्क गभीरता, llvm::raw_ostream &OS) {
  यदि (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    प्रत्यागम असत्य;

  // Don't recursively invoke the llvm-symbolizer binary.
  यदि (Argv0.अन्तर्भवति("llvm-symbolizer"))
    प्रत्यागम असत्य;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_मार्गः_Or_Err = get_LLVMSymbolizer_मार्गः(Argv0);
  यदि (!LLVMSymbolizer_मार्गः_Or_Err)
    प्रत्यागम असत्य;
  ध्रुव std::string &LLVMSymbolizer_मार्गः = *LLVMSymbolizer_मार्गः_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(शून्यसूचकः, शून्यसूचकः);

  स्वचालित Symbolized_स्थानाङ्काः_Opt = collect_स्थानाङ्कः_Symbols(
      राशिः_Trace, गभीरता, MainExecutableName.c_str(), LLVMSymbolizer_मार्गः);
  यदि (!Symbolized_स्थानाङ्काः_Opt)
    प्रत्यागम असत्य;
  प्रति (अचिह्नित FrameNo = 0; FrameNo < Symbolized_स्थानाङ्काः_Opt->आकार_2();
       ++FrameNo) {
    OS << दक्षिणः_justify(formatv("#{0}", FrameNo).str(), std::log10(गभीरता) + 2)
       << ' ' << (*Symbolized_स्थानाङ्काः_Opt)[FrameNo].second << '\n';
  }
  प्रत्यागम सत्य;
}

#यदि LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
शून्य sys::symbolize_स्थानाङ्काः(स्थानाङ्कः_समुच्चयः &स्थानाङ्काः,
                             Symbolized_स्थानाङ्कः_Map &Symbolized_स्थानाङ्काः) {
  प्रतिज्ञा(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  स्वचालित BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_सदिशम्<शून्य *, 0> स्थानाङ्कः_List(स्थानाङ्काः.begin(), स्थानाङ्काः.अन्तः());
  यदि (स्थानाङ्कः_List.रिक्तानि())
    प्रत्यागम;
  llvm::क्रमय(स्थानाङ्कः_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_मार्गः_Or_Err = get_LLVMSymbolizer_मार्गः();
  यदि (!LLVMSymbolizer_मार्गः_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ध्रुव std::string &LLVMSymbolizer_मार्गः = *LLVMSymbolizer_मार्गः_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(शून्यसूचकः, शून्यसूचकः);

  स्वचालित Symbolized_स्थानाङ्काः_Opt =
      collect_स्थानाङ्कः_Symbols(स्थानाङ्कः_List.begin(), स्थानाङ्कः_List.आकार_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_मार्गः);
  यदि (!Symbolized_स्थानाङ्काः_Opt)
    प्रत्यागम;
  प्रति (स्वचालित SymbolizedFrame : *Symbolized_स्थानाङ्काः_Opt) {
    Small_सदिशम्<std::string, 0> &SymbolizedAddrs =
        Symbolized_स्थानाङ्काः[स्थानाङ्कः_List[SymbolizedFrame.first]];
    SymbolizedAddrs.प्रक्षिप_पृष्ठभागः(SymbolizedFrame.second);
  }
  प्रत्यागम;
}
#यदि_अन्तः

स्थिर तार्किक printMarkupContext(raw_ostream &OS, ध्रुव वर्ण *MainExecutableName);

LLVM_ATTRIBUTE_प्रयुक्तम्
स्थिर तार्किक print_Markup_राशिः_Trace(StringRef Argv0, शून्य **राशिः_Trace, पूर्णाङ्क गभीरता,
                                  raw_ostream &OS) {
  ध्रुव वर्ण *Env = getenv(EnableSymbolizerMarkupEnv);
  यदि (!Env || !*Env)
    प्रत्यागम असत्य;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(शून्यसूचकः, शून्यसूचकः);
  यदि (!printMarkupContext(OS, MainExecutableName.c_str()))
    प्रत्यागम असत्य;
  प्रति (पूर्णाङ्क I = 0; I < गभीरता; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, राशिः_Trace[I]);
  प्रत्यागम सत्य;
}

// Include the platform-specific parts of this class.
#यदि_परिभाषितम् LLVM_ON_UNIX
#अन्तर्भाव "Unix/Signals.inc"
#यदि_अन्तः
#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "Windows/Signals.inc"
#यदि_अन्तः
