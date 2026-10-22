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

استفاده نطاق llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ساكن منطقي DisableSymbolicationFlag = خطأ;
ساكن ManagedStatic<std::string> CrashDiagnosticsDirectory;
نطاق {
بنية CreateDisableSymbolication {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<منطقي, درست>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
بنية CreateCrashDiagnosticsDir {
  ساكن فراغ *call() {
    أرجع جديد cl::opt<std::string, درست>(
        "crash-diagnostics-dir", cl::مقدار_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
فراغ llvm::init_سیگنالها_Options() {
  ساكن ManagedStatic<cl::opt<منطقي, درست>, CreateDisableSymbolication>
      DisableSymbolication;
  ساكن ManagedStatic<cl::opt<std::string, درست>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr حرف DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr حرف LLVMSymbolizer_مسیر_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr حرف EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
بنية CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  فراغ *Cookie;
  تعداد صنف وضعیت { خالی_2, Initializing, Initialized, Executing };
  std::atomic<وضعیت> Flag;
};

ساكن constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_مقصد_پیاپی() {
  ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  أرجع callbacks;
}

// Signal-safe.
فراغ sys::پیاپی_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  خودکار BypassSandbox = sandbox::scopedDisable();

  لكل (CallbackAndCookie &پیاپی_Me : Call_Backs_مقصد_پیاپی()) {
    خودکار Expected = CallbackAndCookie::وضعیت::Initialized;
    خودکار Desired = CallbackAndCookie::وضعیت::Executing;
    إذا (!پیاپی_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    (*پیاپی_Me.Callback)(پیاپی_Me.Cookie);
    پیاپی_Me.Callback = اشاره_تهی;
    پیاپی_Me.Cookie = اشاره_تهی;
    پیاپی_Me.Flag.store(CallbackAndCookie::وضعیت::خالی_2);
  }
}

// Signal-safe.
ساكن فراغ درج_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                فراغ *Cookie) {
  لكل (CallbackAndCookie &مجموعه_Me : Call_Backs_مقصد_پیاپی()) {
    خودکار Expected = CallbackAndCookie::وضعیت::خالی_2;
    خودکار Desired = CallbackAndCookie::وضعیت::Initializing;
    إذا (!مجموعه_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    مجموعه_Me.Callback = FnPtr;
    مجموعه_Me.Cookie = Cookie;
    مجموعه_Me.Flag.store(CallbackAndCookie::وضعیت::Initialized);
    أرجع;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ساكن منطقي پیدا_کن_Modules_And_Offsets(فراغ **پشته_Trace, صحيح عمق,
                                  ثابت حرف **Modules, intptr_t *Offsets,
                                  ثابت حرف *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ساكن Formatted_عدد format_ptr(فراغ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  بیعلامت Ptr_عرض = 2 + 2 * حجم(فراغ *);
  أرجع format_hex((uint64_t)PC, Ptr_عرض);
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
std::optional<Small_وکتور<std::pair<بیعلامت, std::string>, 0>>
collect_آدرس_Symbols(فراغ **آدرس_List, بیعلامت آدرس_شمار,
                      ثابت حرف *MainExecutableName,
                      ثابت std::string &LLVMSymbolizer_مسیر) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_وکتور<ثابت حرف *, 0> Modules(آدرس_شمار, اشاره_تهی);
  Small_وکتور<intptr_t, 0> Offsets(آدرس_شمار, 0);
  إذا (!پیدا_کن_Modules_And_Offsets(آدرس_List, آدرس_شمار, Modules.داده(),
                             Offsets.داده(), MainExecutableName, StrPool))
    أرجع {};
  صحيح درون_داد_FD;
  SmallString<32> درون_داد_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", درون_داد_FD, درون_داد_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover درون_داد_Remover(درون_داد_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream درون_داد(درون_داد_FD, درست);
    لكل (بیعلامت AddrIdx = 0; AddrIdx < آدرس_شمار; AddrIdx++) {
      إذا (Modules[AddrIdx])
        درون_داد << Modules[AddrIdx] << " " << (فراغ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {درون_داد_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef آرگومانها[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#اگر_تعریف_شده _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#پایان_اگر
                      "--demangle"};
  صحيح پیاپی_نتیجه =
      sys::ExecuteAndWait(LLVMSymbolizer_مسیر, آرگومانها, std::nullopt, Redirects);
  إذا (پیاپی_نتیجه != 0)
    أرجع {};

  Small_وکتور<std::pair<بیعلامت, std::string>, 0> نتیجه;
  خودکار OutputBuf = حافظه_میانگیر::getFile(OutputFile.c_str());
  إذا (!OutputBuf)
    أرجع {};
  StringRef Output = OutputBuf.get()->get_میانگیر();
  Small_وکتور<StringRef, 32> سطرها;
  Output.تقسیم(سطرها, "\n");
  خودکار *Cur_سطر = سطرها.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  لكل (بیعلامت AddrIdx = 0; AddrIdx < آدرس_شمار; AddrIdx++) {
    إذا (!Modules[AddrIdx]) {
      خودکار &Symbolized_قاب = نتیجه.emplace_پشت(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_قاب.دومی_2);
      OS << format_ptr(آدرس_List[AddrIdx]);
      استمر;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    لكل (;;) {
      إذا (Cur_سطر == سطرها.انتها())
        أرجع {};
      StringRef FunctionName = *Cur_سطر++;
      إذا (FunctionName.خالی())
        اكسر;
      خودکار &Symbolized_قاب = نتیجه.emplace_پشت(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_قاب.دومی_2);
      OS << format_ptr(آدرس_List[AddrIdx]) << ' ';
      إذا (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      إذا (Cur_سطر == سطرها.انتها())
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
  أرجع نتیجه;
}

ErrorOr<std::string> get_LLVMSymbolizer_مسیر(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_مسیر_Or_Err = std::error_کد();
  إذا (ثابت حرف *مسیر = getenv(LLVMSymbolizer_مسیر_Env)) {
    LLVMSymbolizer_مسیر_Or_Err = sys::پیدا_کن_برنامه_By_Name(مسیر);
  } وإلا إذا (!Argv0.خالی()) {
    StringRef والد_2 = llvm::sys::مسیر_2::والد_مسیر(Argv0);
    إذا (!والد_2.خالی())
      LLVMSymbolizer_مسیر_Or_Err =
          sys::پیدا_کن_برنامه_By_Name("llvm-symbolizer", والد_2);
  }
  إذا (!LLVMSymbolizer_مسیر_Or_Err)
    LLVMSymbolizer_مسیر_Or_Err = sys::پیدا_کن_برنامه_By_Name("llvm-symbolizer");
  أرجع LLVMSymbolizer_مسیر_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_استفاده_شده
ساكن منطقي print_Symbolized_پشته_Trace(StringRef Argv0, فراغ **پشته_Trace,
                                      صحيح عمق, llvm::raw_ostream &OS) {
  إذا (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    أرجع خطأ;

  // Don't recursively invoke the llvm-symbolizer binary.
  إذا (Argv0.شامل_است("llvm-symbolizer"))
    أرجع خطأ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_مسیر_Or_Err = get_LLVMSymbolizer_مسیر(Argv0);
  إذا (!LLVMSymbolizer_مسیر_Or_Err)
    أرجع خطأ;
  ثابت std::string &LLVMSymbolizer_مسیر = *LLVMSymbolizer_مسیر_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(اشاره_تهی, اشاره_تهی);

  خودکار Symbolized_آدرسها_Opt = collect_آدرس_Symbols(
      پشته_Trace, عمق, MainExecutableName.c_str(), LLVMSymbolizer_مسیر);
  إذا (!Symbolized_آدرسها_Opt)
    أرجع خطأ;
  لكل (بیعلامت قاب_No = 0; قاب_No < Symbolized_آدرسها_Opt->اندازه_2();
       ++قاب_No) {
    OS << راست_justify(formatv("#{0}", قاب_No).str(), std::log10(عمق) + 2)
       << ' ' << (*Symbolized_آدرسها_Opt)[قاب_No].دومی_2 << '\n';
  }
  أرجع درست;
}

#إذا LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
فراغ sys::symbolize_آدرسها(آدرس_مجموعه &آدرسها,
                             Symbolized_آدرس_Map &Symbolized_آدرسها) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  خودکار BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_وکتور<فراغ *, 0> آدرس_List(آدرسها.begin(), آدرسها.انتها());
  إذا (آدرس_List.خالی())
    أرجع;
  llvm::مرتب_سازی(آدرس_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_مسیر_Or_Err = get_LLVMSymbolizer_مسیر();
  إذا (!LLVMSymbolizer_مسیر_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizer_مسیر = *LLVMSymbolizer_مسیر_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(اشاره_تهی, اشاره_تهی);

  خودکار Symbolized_آدرسها_Opt =
      collect_آدرس_Symbols(آدرس_List.begin(), آدرس_List.اندازه_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_مسیر);
  إذا (!Symbolized_آدرسها_Opt)
    أرجع;
  لكل (خودکار Symbolized_قاب : *Symbolized_آدرسها_Opt) {
    Small_وکتور<std::string, 0> &SymbolizedAddrs =
        Symbolized_آدرسها[آدرس_List[Symbolized_قاب.اولی_2]];
    SymbolizedAddrs.افزودن_پشت(Symbolized_قاب.دومی_2);
  }
  أرجع;
}
#پایان_اگر

ساكن منطقي printMarkupContext(raw_ostream &OS, ثابت حرف *MainExecutableName);

LLVM_ATTRIBUTE_استفاده_شده
ساكن منطقي print_Markup_پشته_Trace(StringRef Argv0, فراغ **پشته_Trace, صحيح عمق,
                                  raw_ostream &OS) {
  ثابت حرف *Env = getenv(EnableSymbolizerMarkupEnv);
  إذا (!Env || !*Env)
    أرجع خطأ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(اشاره_تهی, اشاره_تهی);
  إذا (!printMarkupContext(OS, MainExecutableName.c_str()))
    أرجع خطأ;
  لكل (صحيح I = 0; I < عمق; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, پشته_Trace[I]);
  أرجع درست;
}

// Include the platform-specific parts of this class.
#اگر_تعریف_شده LLVM_ON_UNIX
#ضمن "Unix/Signals.inc"
#پایان_اگر
#اگر_تعریف_شده _WIN32
#ضمن "Windows/Signals.inc"
#پایان_اگر
