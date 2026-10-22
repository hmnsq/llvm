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

#ضمن "llvm/Support/Signals.h"

#ضمن "DebugOptions.h"

#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Config/llvm-config.h"
#ضمن "llvm/Support/CommandLine.h"
#ضمن "llvm/Support/ErrorOr.h"
#ضمن "llvm/Support/FileSystem.h"
#ضمن "llvm/Support/FileUtilities.h"
#ضمن "llvm/Support/Format.h"
#ضمن "llvm/Support/FormatVariadic.h"
#ضمن "llvm/Support/IOSandbox.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/MemoryBuffer.h"
#ضمن "llvm/Support/Path.h"
#ضمن "llvm/Support/Program.h"
#ضمن "llvm/Support/StringSaver.h"
#ضمن "llvm/Support/raw_ostream.h"
#ضمن <array>
#ضمن <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

کارول نطاق llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ساكن منطقي DisableSymbolicationFlag = خطأ;
ساكن ManagedStatic<std::string> CrashDiagnosticsDirectory;
نطاق {
بنية CreateDisableSymbolication {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<منطقي, ریښتیا>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
بنية CreateCrashDiagnosticsDir {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<std::string, ریښتیا>(
        "crash-diagnostics-dir", cl::ارزښت_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
فراغ llvm::init_سیګنالونه_Options() {
  ساكن ManagedStatic<cl::opt<منطقي, ریښتیا>, CreateDisableSymbolication>
      DisableSymbolication;
  ساكن ManagedStatic<cl::opt<std::string, ریښتیا>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr حرف DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr حرف LLVMSymbolizer_لار_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr حرف EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
بنية CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  فراغ *Cookie;
  تعداد صنف حالت { تشه_2, Initializing, Initialized, Executing };
  std::atomic<حالت> Flag;
};

ساكن constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_منزل_پرلپسې() {
  ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  أرجع callbacks;
}

// Signal-safe.
فراغ sys::پرلپسې_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  خپلکاری BypassSandbox = sandbox::scopedDisable();

  لكل (CallbackAndCookie &پرلپسې_Me : Call_Backs_منزل_پرلپسې()) {
    خپلکاری Expected = CallbackAndCookie::حالت::Initialized;
    خپلکاری Desired = CallbackAndCookie::حالت::Executing;
    إذا (!پرلپسې_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    (*پرلپسې_Me.Callback)(پرلپسې_Me.Cookie);
    پرلپسې_Me.Callback = تش_اشاره;
    پرلپسې_Me.Cookie = تش_اشاره;
    پرلپسې_Me.Flag.store(CallbackAndCookie::حالت::تشه_2);
  }
}

// Signal-safe.
ساكن فراغ ورننباسه_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                فراغ *Cookie) {
  لكل (CallbackAndCookie &سېټ_Me : Call_Backs_منزل_پرلپسې()) {
    خپلکاری Expected = CallbackAndCookie::حالت::تشه_2;
    خپلکاری Desired = CallbackAndCookie::حالت::Initializing;
    إذا (!سېټ_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    سېټ_Me.Callback = FnPtr;
    سېټ_Me.Cookie = Cookie;
    سېټ_Me.Flag.store(CallbackAndCookie::حالت::Initialized);
    أرجع;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ساكن منطقي پیدا_کړه_Modules_And_Offsets(فراغ **ډېری_Trace, صحيح ژوروالی,
                                  ثابت حرف **Modules, intptr_t *Offsets,
                                  ثابت حرف *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ساكن Formatted_شمېره format_ptr(فراغ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  بینښه Ptr_پلنوالی = 2 + 2 * حجم(فراغ *);
  أرجع format_hex((uint64_t)PC, Ptr_پلنوالی);
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
std::optional<Small_ویکټور<std::pair<بینښه, std::string>, 0>>
collect_پته_Symbols(فراغ **پته_List, بینښه پته_شمېر,
                      ثابت حرف *MainExecutableName,
                      ثابت std::string &LLVMSymbolizer_لار) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_ویکټور<ثابت حرف *, 0> Modules(پته_شمېر, تش_اشاره);
  Small_ویکټور<intptr_t, 0> Offsets(پته_شمېر, 0);
  إذا (!پیدا_کړه_Modules_And_Offsets(پته_List, پته_شمېر, Modules.معلومات(),
                             Offsets.معلومات(), MainExecutableName, StrPool))
    أرجع {};
  صحيح ننوت_FD;
  SmallString<32> ننوت_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", ننوت_FD, ننوت_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover ننوت_Remover(ننوت_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream ننوت(ننوت_FD, ریښتیا);
    لكل (بینښه AddrIdx = 0; AddrIdx < پته_شمېر; AddrIdx++) {
      إذا (Modules[AddrIdx])
        ننوت << Modules[AddrIdx] << " " << (فراغ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {ننوت_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef دلیلونه[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#که_تعریف_شوی _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#د_که_پای
                      "--demangle"};
  صحيح پرلپسې_پایله =
      sys::ExecuteAndWait(LLVMSymbolizer_لار, دلیلونه, std::nullopt, Redirects);
  إذا (پرلپسې_پایله != 0)
    أرجع {};

  Small_ویکټور<std::pair<بینښه, std::string>, 0> پایله;
  خپلکاری OutputBuf = حافظه_بفر::getFile(OutputFile.c_str());
  إذا (!OutputBuf)
    أرجع {};
  StringRef Output = OutputBuf.get()->get_بفر();
  Small_ویکټور<StringRef, 32> کرښې;
  Output.وېشل(کرښې, "\n");
  خپلکاری *Cur_کرښه = کرښې.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  لكل (بینښه AddrIdx = 0; AddrIdx < پته_شمېر; AddrIdx++) {
    إذا (!Modules[AddrIdx]) {
      خپلکاری &Symbolized_چوکاټ = پایله.emplace_شا(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکاټ.دوهم_2);
      OS << format_ptr(پته_List[AddrIdx]);
      استمر;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    لكل (;;) {
      إذا (Cur_کرښه == کرښې.پای_ټکی())
        أرجع {};
      StringRef FunctionName = *Cur_کرښه++;
      إذا (FunctionName.تشه())
        اكسر;
      خپلکاری &Symbolized_چوکاټ = پایله.emplace_شا(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکاټ.دوهم_2);
      OS << format_ptr(پته_List[AddrIdx]) << ' ';
      إذا (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      إذا (Cur_کرښه == کرښې.پای_ټکی())
        أرجع {};
      StringRef File_کرښه_Info = *Cur_کرښه++;
      إذا (!File_کرښه_Info.starts_with("??")) {
        OS << File_کرښه_Info;
      } وإلا {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  أرجع پایله;
}

ErrorOr<std::string> get_LLVMSymbolizer_لار(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_لار_Or_Err = std::error_کوډ();
  إذا (ثابت حرف *لار = getenv(LLVMSymbolizer_لار_Env)) {
    LLVMSymbolizer_لار_Or_Err = sys::پیدا_کړه_پروګرام_By_Name(لار);
  } وإلا إذا (!Argv0.تشه()) {
    StringRef پلار_2 = llvm::sys::لار_2::پلار_لار(Argv0);
    إذا (!پلار_2.تشه())
      LLVMSymbolizer_لار_Or_Err =
          sys::پیدا_کړه_پروګرام_By_Name("llvm-symbolizer", پلار_2);
  }
  إذا (!LLVMSymbolizer_لار_Or_Err)
    LLVMSymbolizer_لار_Or_Err = sys::پیدا_کړه_پروګرام_By_Name("llvm-symbolizer");
  أرجع LLVMSymbolizer_لار_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_کارول_شوی
ساكن منطقي print_Symbolized_ډېری_Trace(StringRef Argv0, فراغ **ډېری_Trace,
                                      صحيح ژوروالی, llvm::raw_ostream &OS) {
  إذا (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    أرجع خطأ;

  // Don't recursively invoke the llvm-symbolizer binary.
  إذا (Argv0.لري("llvm-symbolizer"))
    أرجع خطأ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_لار_Or_Err = get_LLVMSymbolizer_لار(Argv0);
  إذا (!LLVMSymbolizer_لار_Or_Err)
    أرجع خطأ;
  ثابت std::string &LLVMSymbolizer_لار = *LLVMSymbolizer_لار_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(تش_اشاره, تش_اشاره);

  خپلکاری Symbolized_پتې_Opt = collect_پته_Symbols(
      ډېری_Trace, ژوروالی, MainExecutableName.c_str(), LLVMSymbolizer_لار);
  إذا (!Symbolized_پتې_Opt)
    أرجع خطأ;
  لكل (بینښه چوکاټ_No = 0; چوکاټ_No < Symbolized_پتې_Opt->اندازه_2();
       ++چوکاټ_No) {
    OS << ښی_justify(formatv("#{0}", چوکاټ_No).str(), std::log10(ژوروالی) + 2)
       << ' ' << (*Symbolized_پتې_Opt)[چوکاټ_No].دوهم_2 << '\n';
  }
  أرجع ریښتیا;
}

#إذا LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
فراغ sys::symbolize_پتې(پته_سېټ &پتې,
                             Symbolized_پته_Map &Symbolized_پتې) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  خپلکاری BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_ویکټور<فراغ *, 0> پته_List(پتې.begin(), پتې.پای_ټکی());
  إذا (پته_List.تشه())
    أرجع;
  llvm::ترتیبول(پته_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_لار_Or_Err = get_LLVMSymbolizer_لار();
  إذا (!LLVMSymbolizer_لار_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizer_لار = *LLVMSymbolizer_لار_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(تش_اشاره, تش_اشاره);

  خپلکاری Symbolized_پتې_Opt =
      collect_پته_Symbols(پته_List.begin(), پته_List.اندازه_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_لار);
  إذا (!Symbolized_پتې_Opt)
    أرجع;
  لكل (خپلکاری Symbolized_چوکاټ : *Symbolized_پتې_Opt) {
    Small_ویکټور<std::string, 0> &SymbolizedAddrs =
        Symbolized_پتې[پته_List[Symbolized_چوکاټ.لومړی_2]];
    SymbolizedAddrs.ټېل_شا(Symbolized_چوکاټ.دوهم_2);
  }
  أرجع;
}
#د_که_پای

ساكن منطقي printMarkupContext(raw_ostream &OS, ثابت حرف *MainExecutableName);

LLVM_ATTRIBUTE_کارول_شوی
ساكن منطقي print_Markup_ډېری_Trace(StringRef Argv0, فراغ **ډېری_Trace, صحيح ژوروالی,
                                  raw_ostream &OS) {
  ثابت حرف *Env = getenv(EnableSymbolizerMarkupEnv);
  إذا (!Env || !*Env)
    أرجع خطأ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(تش_اشاره, تش_اشاره);
  إذا (!printMarkupContext(OS, MainExecutableName.c_str()))
    أرجع خطأ;
  لكل (صحيح I = 0; I < ژوروالی; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ډېری_Trace[I]);
  أرجع ریښتیا;
}

// Include the platform-specific parts of this class.
#که_تعریف_شوی LLVM_ON_UNIX
#ضمن "Unix/Signals.inc"
#د_که_پای
#که_تعریف_شوی _WIN32
#ضمن "Windows/Signals.inc"
#د_که_پای
