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

ورتوں نطاق llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ساكن منطقي DisableSymbolicationFlag = خطأ;
ساكن ManagedStatic<std::string> CrashDiagnosticsDirectory;
نطاق {
بنية CreateDisableSymbolication {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<منطقي, سچ>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
بنية CreateCrashDiagnosticsDir {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<std::string, سچ>(
        "crash-diagnostics-dir", cl::قدر_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
فراغ llvm::init_اشارے_Options() {
  ساكن ManagedStatic<cl::opt<منطقي, سچ>, CreateDisableSymbolication>
      DisableSymbolication;
  ساكن ManagedStatic<cl::opt<std::string, سچ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr حرف DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr حرف LLVMSymbolizer_رستہ_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr حرف EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
بنية CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  فراغ *Cookie;
  تعداد صنف حالت { خالی_تھاواں_2, Initializing, Initialized, Executing };
  std::atomic<حالت> Flag;
};

ساكن constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_منزل_لگاتار() {
  ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  أرجع callbacks;
}

// Signal-safe.
فراغ sys::لگاتار_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  خودکار BypassSandbox = sandbox::scopedDisable();

  لكل (CallbackAndCookie &لگاتار_Me : Call_Backs_منزل_لگاتار()) {
    خودکار Expected = CallbackAndCookie::حالت::Initialized;
    خودکار Desired = CallbackAndCookie::حالت::Executing;
    إذا (!لگاتار_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    (*لگاتار_Me.Callback)(لگاتار_Me.Cookie);
    لگاتار_Me.Callback = خالی_سوچک;
    لگاتار_Me.Cookie = خالی_سوچک;
    لگاتار_Me.Flag.store(CallbackAndCookie::حالت::خالی_تھاواں_2);
  }
}

// Signal-safe.
ساكن فراغ داخل_کرو_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                فراغ *Cookie) {
  لكل (CallbackAndCookie &سیٹ_Me : Call_Backs_منزل_لگاتار()) {
    خودکار Expected = CallbackAndCookie::حالت::خالی_تھاواں_2;
    خودکار Desired = CallbackAndCookie::حالت::Initializing;
    إذا (!سیٹ_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    سیٹ_Me.Callback = FnPtr;
    سیٹ_Me.Cookie = Cookie;
    سیٹ_Me.Flag.store(CallbackAndCookie::حالت::Initialized);
    أرجع;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ساكن منطقي لبھو_Modules_And_Offsets(فراغ **ڈھیر_Trace, صحيح ڈونگھائی,
                                  ثابت حرف **Modules, intptr_t *Offsets,
                                  ثابت حرف *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ساكن Formatted_نمبر format_ptr(فراغ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  بنانشانی Ptr_چوڑائی = 2 + 2 * حجم(فراغ *);
  أرجع format_hex((uint64_t)PC, Ptr_چوڑائی);
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
std::optional<Small_سمتیہ<std::pair<بنانشانی, std::string>, 0>>
collect_پتہ_Symbols(فراغ **پتہ_List, بنانشانی پتہ_گنتی,
                      ثابت حرف *MainExecutableName,
                      ثابت std::string &LLVMSymbolizer_رستہ) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_سمتیہ<ثابت حرف *, 0> Modules(پتہ_گنتی, خالی_سوچک);
  Small_سمتیہ<intptr_t, 0> Offsets(پتہ_گنتی, 0);
  إذا (!لبھو_Modules_And_Offsets(پتہ_List, پتہ_گنتی, Modules.انکڑے(),
                             Offsets.انکڑے(), MainExecutableName, StrPool))
    أرجع {};
  صحيح داخلہ_FD;
  SmallString<32> داخلہ_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", داخلہ_FD, داخلہ_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover داخلہ_Remover(داخلہ_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream داخلہ(داخلہ_FD, سچ);
    لكل (بنانشانی AddrIdx = 0; AddrIdx < پتہ_گنتی; AddrIdx++) {
      إذا (Modules[AddrIdx])
        داخلہ << Modules[AddrIdx] << " " << (فراغ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {داخلہ_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef دلیلاں[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#جے_تعریف_شدہ _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#جے_انت
                      "--demangle"};
  صحيح لگاتار_نتیجہ =
      sys::ExecuteAndWait(LLVMSymbolizer_رستہ, دلیلاں, std::nullopt, Redirects);
  إذا (لگاتار_نتیجہ != 0)
    أرجع {};

  Small_سمتیہ<std::pair<بنانشانی, std::string>, 0> نتیجہ;
  خودکار OutputBuf = حافظہ_بفر::getFile(OutputFile.c_str());
  إذا (!OutputBuf)
    أرجع {};
  StringRef Output = OutputBuf.get()->get_بفر();
  Small_سمتیہ<StringRef, 32> سطریں;
  Output.ونڈو(سطریں, "\n");
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
  لكل (بنانشانی AddrIdx = 0; AddrIdx < پتہ_گنتی; AddrIdx++) {
    إذا (!Modules[AddrIdx]) {
      خودکار &Symbolized_چوکھٹا = نتیجہ.emplace_پٹھ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکھٹا.دوجا_2);
      OS << format_ptr(پتہ_List[AddrIdx]);
      استمر;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    لكل (;;) {
      إذا (Cur_سطر == سطریں.سرا())
        أرجع {};
      StringRef FunctionName = *Cur_سطر++;
      إذا (FunctionName.خالی_تھاواں())
        اكسر;
      خودکار &Symbolized_چوکھٹا = نتیجہ.emplace_پٹھ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکھٹا.دوجا_2);
      OS << format_ptr(پتہ_List[AddrIdx]) << ' ';
      إذا (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      إذا (Cur_سطر == سطریں.سرا())
        أرجع {};
      StringRef File_سطر_Info = *Cur_سطر++;
      إذا (!File_سطر_Info.starts_with("??")) {
        OS << File_سطر_Info;
      } وإلا {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  أرجع نتیجہ;
}

ErrorOr<std::string> get_LLVMSymbolizer_رستہ(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_رستہ_Or_Err = std::error_کوڈ();
  إذا (ثابت حرف *رستہ = getenv(LLVMSymbolizer_رستہ_Env)) {
    LLVMSymbolizer_رستہ_Or_Err = sys::لبھو_پروگرام_By_Name(رستہ);
  } وإلا إذا (!Argv0.خالی_تھاواں()) {
    StringRef ماپا_2 = llvm::sys::رستہ_2::ماپا_رستہ(Argv0);
    إذا (!ماپا_2.خالی_تھاواں())
      LLVMSymbolizer_رستہ_Or_Err =
          sys::لبھو_پروگرام_By_Name("llvm-symbolizer", ماپا_2);
  }
  إذا (!LLVMSymbolizer_رستہ_Or_Err)
    LLVMSymbolizer_رستہ_Or_Err = sys::لبھو_پروگرام_By_Name("llvm-symbolizer");
  أرجع LLVMSymbolizer_رستہ_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_ورتیا
ساكن منطقي print_Symbolized_ڈھیر_Trace(StringRef Argv0, فراغ **ڈھیر_Trace,
                                      صحيح ڈونگھائی, llvm::raw_ostream &OS) {
  إذا (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    أرجع خطأ;

  // Don't recursively invoke the llvm-symbolizer binary.
  إذا (Argv0.شامل_اے("llvm-symbolizer"))
    أرجع خطأ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_رستہ_Or_Err = get_LLVMSymbolizer_رستہ(Argv0);
  إذا (!LLVMSymbolizer_رستہ_Or_Err)
    أرجع خطأ;
  ثابت std::string &LLVMSymbolizer_رستہ = *LLVMSymbolizer_رستہ_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(خالی_سوچک, خالی_سوچک);

  خودکار Symbolized_پتے_Opt = collect_پتہ_Symbols(
      ڈھیر_Trace, ڈونگھائی, MainExecutableName.c_str(), LLVMSymbolizer_رستہ);
  إذا (!Symbolized_پتے_Opt)
    أرجع خطأ;
  لكل (بنانشانی چوکھٹا_No = 0; چوکھٹا_No < Symbolized_پتے_Opt->ناپ_2();
       ++چوکھٹا_No) {
    OS << دایاں_justify(formatv("#{0}", چوکھٹا_No).str(), std::log10(ڈونگھائی) + 2)
       << ' ' << (*Symbolized_پتے_Opt)[چوکھٹا_No].دوجا_2 << '\n';
  }
  أرجع سچ;
}

#إذا LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
فراغ sys::symbolize_پتے(پتہ_سیٹ &پتے,
                             Symbolized_پتہ_Map &Symbolized_پتے) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_سمتیہ<فراغ *, 0> پتہ_List(پتے.begin(), پتے.سرا());
  إذا (پتہ_List.خالی_تھاواں())
    أرجع;
  llvm::ترتیب_دیو(پتہ_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_رستہ_Or_Err = get_LLVMSymbolizer_رستہ();
  إذا (!LLVMSymbolizer_رستہ_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizer_رستہ = *LLVMSymbolizer_رستہ_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(خالی_سوچک, خالی_سوچک);

  خودکار Symbolized_پتے_Opt =
      collect_پتہ_Symbols(پتہ_List.begin(), پتہ_List.ناپ_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_رستہ);
  إذا (!Symbolized_پتے_Opt)
    أرجع;
  لكل (خودکار Symbolized_چوکھٹا : *Symbolized_پتے_Opt) {
    Small_سمتیہ<std::string, 0> &SymbolizedAddrs =
        Symbolized_پتے[پتہ_List[Symbolized_چوکھٹا.پہلا_2]];
    SymbolizedAddrs.ڈالو_پٹھ(Symbolized_چوکھٹا.دوجا_2);
  }
  أرجع;
}
#جے_انت

ساكن منطقي printMarkupContext(raw_ostream &OS, ثابت حرف *MainExecutableName);

LLVM_ATTRIBUTE_ورتیا
ساكن منطقي print_Markup_ڈھیر_Trace(StringRef Argv0, فراغ **ڈھیر_Trace, صحيح ڈونگھائی,
                                  raw_ostream &OS) {
  ثابت حرف *Env = getenv(EnableSymbolizerMarkupEnv);
  إذا (!Env || !*Env)
    أرجع خطأ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(خالی_سوچک, خالی_سوچک);
  إذا (!printMarkupContext(OS, MainExecutableName.c_str()))
    أرجع خطأ;
  لكل (صحيح I = 0; I < ڈونگھائی; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ڈھیر_Trace[I]);
  أرجع سچ;
}

// Include the platform-specific parts of this class.
#جے_تعریف_شدہ LLVM_ON_UNIX
#ضمن "Unix/Signals.inc"
#جے_انت
#جے_تعریف_شدہ _WIN32
#ضمن "Windows/Signals.inc"
#جے_انت
