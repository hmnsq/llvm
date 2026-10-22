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

#شامل "llvm/Support/Signals.h"

#شامل "DebugOptions.h"

#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Config/llvm-config.h"
#شامل "llvm/Support/CommandLine.h"
#شامل "llvm/Support/ErrorOr.h"
#شامل "llvm/Support/FileSystem.h"
#شامل "llvm/Support/FileUtilities.h"
#شامل "llvm/Support/Format.h"
#شامل "llvm/Support/FormatVariadic.h"
#شامل "llvm/Support/IOSandbox.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/MemoryBuffer.h"
#شامل "llvm/Support/Path.h"
#شامل "llvm/Support/Program.h"
#شامل "llvm/Support/StringSaver.h"
#شامل "llvm/Support/raw_ostream.h"
#شامل <array>
#شامل <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

استعمال نامخلا llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ساکن منطقی DisableSymbolicationFlag = جھوٹ;
ساکن ManagedStatic<std::string> CrashDiagnosticsDirectory;
نامخلا {
ساخت CreateDisableSymbolication {
  ساکن خالی *call() {
    واپس نیا cl::opt<منطقی, سچ>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ساخت CreateCrashDiagnosticsDir {
  ساکن خالی *call() {
    واپس نیا cl::opt<std::string, سچ>(
        "crash-diagnostics-dir", cl::قدر_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
خالی llvm::init_اشارات_Options() {
  ساکن ManagedStatic<cl::opt<منطقی, سچ>, CreateDisableSymbolication>
      DisableSymbolication;
  ساکن ManagedStatic<cl::opt<std::string, سچ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr حرف DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr حرف LLVMSymbolizer_راستہ_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr حرف EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ساخت CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  خالی *Cookie;
  شمار جماعت حالت_2 { خالی_خانے_2, Initializing, Initialized, Executing };
  std::atomic<حالت_2> Flag;
};

ساکن constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ساکن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_مقصد_تسلسل() {
  ساکن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  واپس callbacks;
}

// Signal-safe.
خالی sys::تسلسل_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  خودکار BypassSandbox = sandbox::scopedDisable();

  برائے (CallbackAndCookie &تسلسل_Me : Call_Backs_مقصد_تسلسل()) {
    خودکار Expected = CallbackAndCookie::حالت_2::Initialized;
    خودکار Desired = CallbackAndCookie::حالت_2::Executing;
    اگر (!تسلسل_Me.Flag.compare_exchange_strong(Expected, Desired))
      جاری;
    (*تسلسل_Me.Callback)(تسلسل_Me.Cookie);
    تسلسل_Me.Callback = خالی_اشاریہ;
    تسلسل_Me.Cookie = خالی_اشاریہ;
    تسلسل_Me.Flag.store(CallbackAndCookie::حالت_2::خالی_خانے_2);
  }
}

// Signal-safe.
ساکن خالی داخل_کرو_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                خالی *Cookie) {
  برائے (CallbackAndCookie &سیٹ_Me : Call_Backs_مقصد_تسلسل()) {
    خودکار Expected = CallbackAndCookie::حالت_2::خالی_خانے_2;
    خودکار Desired = CallbackAndCookie::حالت_2::Initializing;
    اگر (!سیٹ_Me.Flag.compare_exchange_strong(Expected, Desired))
      جاری;
    سیٹ_Me.Callback = FnPtr;
    سیٹ_Me.Cookie = Cookie;
    سیٹ_Me.Flag.store(CallbackAndCookie::حالت_2::Initialized);
    واپس;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ساکن منطقی ڈھونڈو_Modules_And_Offsets(خالی **ڈھیر_Trace, صحیح گہرائی,
                                  ثابت حرف **Modules, intptr_t *Offsets,
                                  ثابت حرف *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ساکن Formatted_نمبر format_ptr(خالی *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  غیرعلامتی Ptr_چوڑائی = 2 + 2 * حجم(خالی *);
  واپس format_hex((uint64_t)PC, Ptr_چوڑائی);
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
std::optional<Small_سمتیہ<std::pair<غیرعلامتی, std::string>, 0>>
collect_پتہ_Symbols(خالی **پتہ_List, غیرعلامتی پتہ_گنتی,
                      ثابت حرف *MainExecutableName,
                      ثابت std::string &LLVMSymbolizer_راستہ) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_سمتیہ<ثابت حرف *, 0> Modules(پتہ_گنتی, خالی_اشاریہ);
  Small_سمتیہ<intptr_t, 0> Offsets(پتہ_گنتی, 0);
  اگر (!ڈھونڈو_Modules_And_Offsets(پتہ_List, پتہ_گنتی, Modules.ڈیٹا(),
                             Offsets.ڈیٹا(), MainExecutableName, StrPool))
    واپس {};
  صحیح داخلہ_FD;
  SmallString<32> داخلہ_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", داخلہ_FD, داخلہ_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover داخلہ_Remover(داخلہ_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream داخلہ(داخلہ_FD, سچ);
    برائے (غیرعلامتی AddrIdx = 0; AddrIdx < پتہ_گنتی; AddrIdx++) {
      اگر (Modules[AddrIdx])
        داخلہ << Modules[AddrIdx] << " " << (خالی *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {داخلہ_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef دلائل[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#اگر_تعریف_شدہ _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#اگر_ختم
                      "--demangle"};
  صحیح تسلسل_نتیجہ =
      sys::ExecuteAndWait(LLVMSymbolizer_راستہ, دلائل, std::nullopt, Redirects);
  اگر (تسلسل_نتیجہ != 0)
    واپس {};

  Small_سمتیہ<std::pair<غیرعلامتی, std::string>, 0> نتیجہ;
  خودکار OutputBuf = حافظہ_بفر::getFile(OutputFile.c_str());
  اگر (!OutputBuf)
    واپس {};
  StringRef Output = OutputBuf.get()->get_بفر();
  Small_سمتیہ<StringRef, 32> سطریں;
  Output.بانٹو(سطریں, "\n");
  خودکار *Cur_سطر = سطریں.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  برائے (غیرعلامتی AddrIdx = 0; AddrIdx < پتہ_گنتی; AddrIdx++) {
    اگر (!Modules[AddrIdx]) {
      خودکار &Symbolized_چوکھٹا = نتیجہ.emplace_عقب(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکھٹا.دوسرا_2);
      OS << format_ptr(پتہ_List[AddrIdx]);
      جاری;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    برائے (;;) {
      اگر (Cur_سطر == سطریں.انتہا())
        واپس {};
      StringRef FunctionName = *Cur_سطر++;
      اگر (FunctionName.خالی_خانے())
        توڑو;
      خودکار &Symbolized_چوکھٹا = نتیجہ.emplace_عقب(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکھٹا.دوسرا_2);
      OS << format_ptr(پتہ_List[AddrIdx]) << ' ';
      اگر (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      اگر (Cur_سطر == سطریں.انتہا())
        واپس {};
      StringRef File_سطر_Info = *Cur_سطر++;
      اگر (!File_سطر_Info.starts_with("??")) {
        OS << File_سطر_Info;
      } ورنہ {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  واپس نتیجہ;
}

ErrorOr<std::string> get_LLVMSymbolizer_راستہ(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_راستہ_Or_Err = std::error_رمز();
  اگر (ثابت حرف *راستہ = getenv(LLVMSymbolizer_راستہ_Env)) {
    LLVMSymbolizer_راستہ_Or_Err = sys::ڈھونڈو_پروگرام_By_Name(راستہ);
  } ورنہ اگر (!Argv0.خالی_خانے()) {
    StringRef والد_2 = llvm::sys::راستہ_2::والد_راستہ(Argv0);
    اگر (!والد_2.خالی_خانے())
      LLVMSymbolizer_راستہ_Or_Err =
          sys::ڈھونڈو_پروگرام_By_Name("llvm-symbolizer", والد_2);
  }
  اگر (!LLVMSymbolizer_راستہ_Or_Err)
    LLVMSymbolizer_راستہ_Or_Err = sys::ڈھونڈو_پروگرام_By_Name("llvm-symbolizer");
  واپس LLVMSymbolizer_راستہ_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_مستعمل
ساکن منطقی print_Symbolized_ڈھیر_Trace(StringRef Argv0, خالی **ڈھیر_Trace,
                                      صحیح گہرائی, llvm::raw_ostream &OS) {
  اگر (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    واپس جھوٹ;

  // Don't recursively invoke the llvm-symbolizer binary.
  اگر (Argv0.شامل_ہے("llvm-symbolizer"))
    واپس جھوٹ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_راستہ_Or_Err = get_LLVMSymbolizer_راستہ(Argv0);
  اگر (!LLVMSymbolizer_راستہ_Or_Err)
    واپس جھوٹ;
  ثابت std::string &LLVMSymbolizer_راستہ = *LLVMSymbolizer_راستہ_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(خالی_اشاریہ, خالی_اشاریہ);

  خودکار Symbolized_پتے_Opt = collect_پتہ_Symbols(
      ڈھیر_Trace, گہرائی, MainExecutableName.c_str(), LLVMSymbolizer_راستہ);
  اگر (!Symbolized_پتے_Opt)
    واپس جھوٹ;
  برائے (غیرعلامتی چوکھٹا_No = 0; چوکھٹا_No < Symbolized_پتے_Opt->سائز_2();
       ++چوکھٹا_No) {
    OS << دایاں_justify(formatv("#{0}", چوکھٹا_No).str(), std::log10(گہرائی) + 2)
       << ' ' << (*Symbolized_پتے_Opt)[چوکھٹا_No].دوسرا_2 << '\n';
  }
  واپس سچ;
}

#اگر LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
خالی sys::symbolize_پتے(پتہ_سیٹ &پتے,
                             Symbolized_پتہ_Map &Symbolized_پتے) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_سمتیہ<خالی *, 0> پتہ_List(پتے.begin(), پتے.انتہا());
  اگر (پتہ_List.خالی_خانے())
    واپس;
  llvm::ترتیب_دو(پتہ_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_راستہ_Or_Err = get_LLVMSymbolizer_راستہ();
  اگر (!LLVMSymbolizer_راستہ_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizer_راستہ = *LLVMSymbolizer_راستہ_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(خالی_اشاریہ, خالی_اشاریہ);

  خودکار Symbolized_پتے_Opt =
      collect_پتہ_Symbols(پتہ_List.begin(), پتہ_List.سائز_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_راستہ);
  اگر (!Symbolized_پتے_Opt)
    واپس;
  برائے (خودکار Symbolized_چوکھٹا : *Symbolized_پتے_Opt) {
    Small_سمتیہ<std::string, 0> &SymbolizedAddrs =
        Symbolized_پتے[پتہ_List[Symbolized_چوکھٹا.پہلا_2]];
    SymbolizedAddrs.ڈالو_عقب(Symbolized_چوکھٹا.دوسرا_2);
  }
  واپس;
}
#اگر_ختم

ساکن منطقی printMarkupContext(raw_ostream &OS, ثابت حرف *MainExecutableName);

LLVM_ATTRIBUTE_مستعمل
ساکن منطقی print_Markup_ڈھیر_Trace(StringRef Argv0, خالی **ڈھیر_Trace, صحیح گہرائی,
                                  raw_ostream &OS) {
  ثابت حرف *Env = getenv(EnableSymbolizerMarkupEnv);
  اگر (!Env || !*Env)
    واپس جھوٹ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(خالی_اشاریہ, خالی_اشاریہ);
  اگر (!printMarkupContext(OS, MainExecutableName.c_str()))
    واپس جھوٹ;
  برائے (صحیح I = 0; I < گہرائی; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ڈھیر_Trace[I]);
  واپس سچ;
}

// Include the platform-specific parts of this class.
#اگر_تعریف_شدہ LLVM_ON_UNIX
#شامل "Unix/Signals.inc"
#اگر_ختم
#اگر_تعریف_شدہ _WIN32
#شامل "Windows/Signals.inc"
#اگر_ختم
