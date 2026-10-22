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

استعمال نطاق llvm;

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
فراغ llvm::init_سگنل_Options() {
  ساكن ManagedStatic<cl::opt<منطقي, سچ>, CreateDisableSymbolication>
      DisableSymbolication;
  ساكن ManagedStatic<cl::opt<std::string, سچ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr حرف DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr حرف LLVMSymbolizer_رستو_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr حرف EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
بنية CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  فراغ *Cookie;
  تعداد صنف حالت { خالي_جايون_2, Initializing, Initialized, Executing };
  std::atomic<حالت> Flag;
};

ساكن constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_منزل_لاڳيتو() {
  ساكن std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  أرجع callbacks;
}

// Signal-safe.
فراغ sys::لاڳيتو_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  خودڪار BypassSandbox = sandbox::scopedDisable();

  لكل (CallbackAndCookie &لاڳيتو_Me : Call_Backs_منزل_لاڳيتو()) {
    خودڪار Expected = CallbackAndCookie::حالت::Initialized;
    خودڪار Desired = CallbackAndCookie::حالت::Executing;
    إذا (!لاڳيتو_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    (*لاڳيتو_Me.Callback)(لاڳيتو_Me.Cookie);
    لاڳيتو_Me.Callback = خالي_اشارو;
    لاڳيتو_Me.Cookie = خالي_اشارو;
    لاڳيتو_Me.Flag.store(CallbackAndCookie::حالت::خالي_جايون_2);
  }
}

// Signal-safe.
ساكن فراغ وجهو_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                فراغ *Cookie) {
  لكل (CallbackAndCookie &سيٽ_Me : Call_Backs_منزل_لاڳيتو()) {
    خودڪار Expected = CallbackAndCookie::حالت::خالي_جايون_2;
    خودڪار Desired = CallbackAndCookie::حالت::Initializing;
    إذا (!سيٽ_Me.Flag.compare_exchange_strong(Expected, Desired))
      استمر;
    سيٽ_Me.Callback = FnPtr;
    سيٽ_Me.Cookie = Cookie;
    سيٽ_Me.Flag.store(CallbackAndCookie::حالت::Initialized);
    أرجع;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ساكن منطقي ڳوليو_Modules_And_Offsets(فراغ **ڍير_Trace, صحيح اونهائي,
                                  ثابت حرف **Modules, intptr_t *Offsets,
                                  ثابت حرف *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ساكن Formatted_عدد format_ptr(فراغ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  بنانشان Ptr_ويڪر = 2 + 2 * حجم(فراغ *);
  أرجع format_hex((uint64_t)PC, Ptr_ويڪر);
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
std::optional<Small_ويڪٽر<std::pair<بنانشان, std::string>, 0>>
collect_پتو_Symbols(فراغ **پتو_List, بنانشان پتو_ڳڻپ,
                      ثابت حرف *MainExecutableName,
                      ثابت std::string &LLVMSymbolizer_رستو) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_ويڪٽر<ثابت حرف *, 0> Modules(پتو_ڳڻپ, خالي_اشارو);
  Small_ويڪٽر<intptr_t, 0> Offsets(پتو_ڳڻپ, 0);
  إذا (!ڳوليو_Modules_And_Offsets(پتو_List, پتو_ڳڻپ, Modules.معلومات(),
                             Offsets.معلومات(), MainExecutableName, StrPool))
    أرجع {};
  صحيح داخلا_FD;
  SmallString<32> داخلا_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", داخلا_FD, داخلا_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover داخلا_Remover(داخلا_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream داخلا(داخلا_FD, سچ);
    لكل (بنانشان AddrIdx = 0; AddrIdx < پتو_ڳڻپ; AddrIdx++) {
      إذا (Modules[AddrIdx])
        داخلا << Modules[AddrIdx] << " " << (فراغ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {داخلا_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef دليلون[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#جيڪڏهن_وصف_ٿيل _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#جيڪڏهن_پورو
                      "--demangle"};
  صحيح لاڳيتو_نتيجو =
      sys::ExecuteAndWait(LLVMSymbolizer_رستو, دليلون, std::nullopt, Redirects);
  إذا (لاڳيتو_نتيجو != 0)
    أرجع {};

  Small_ويڪٽر<std::pair<بنانشان, std::string>, 0> نتيجو;
  خودڪار OutputBuf = يادگيري_بفر::getFile(OutputFile.c_str());
  إذا (!OutputBuf)
    أرجع {};
  StringRef Output = OutputBuf.get()->get_بفر();
  Small_ويڪٽر<StringRef, 32> سٽون;
  Output.ورهايو(سٽون, "\n");
  خودڪار *Cur_سٽ = سٽون.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  لكل (بنانشان AddrIdx = 0; AddrIdx < پتو_ڳڻپ; AddrIdx++) {
    إذا (!Modules[AddrIdx]) {
      خودڪار &Symbolized_چوکٺو = نتيجو.emplace_پٺ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکٺو.ٻيو_2);
      OS << format_ptr(پتو_List[AddrIdx]);
      استمر;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    لكل (;;) {
      إذا (Cur_سٽ == سٽون.پڇاڙي())
        أرجع {};
      StringRef FunctionName = *Cur_سٽ++;
      إذا (FunctionName.خالي_جايون())
        اكسر;
      خودڪار &Symbolized_چوکٺو = نتيجو.emplace_پٺ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_چوکٺو.ٻيو_2);
      OS << format_ptr(پتو_List[AddrIdx]) << ' ';
      إذا (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      إذا (Cur_سٽ == سٽون.پڇاڙي())
        أرجع {};
      StringRef File_سٽ_Info = *Cur_سٽ++;
      إذا (!File_سٽ_Info.starts_with("??")) {
        OS << File_سٽ_Info;
      } وإلا {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  أرجع نتيجو;
}

ErrorOr<std::string> get_LLVMSymbolizer_رستو(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_رستو_Or_Err = std::error_ڪوڊ();
  إذا (ثابت حرف *رستو = getenv(LLVMSymbolizer_رستو_Env)) {
    LLVMSymbolizer_رستو_Or_Err = sys::ڳوليو_پروگرام_By_Name(رستو);
  } وإلا إذا (!Argv0.خالي_جايون()) {
    StringRef والد_2 = llvm::sys::رستو_2::والد_رستو(Argv0);
    إذا (!والد_2.خالي_جايون())
      LLVMSymbolizer_رستو_Or_Err =
          sys::ڳوليو_پروگرام_By_Name("llvm-symbolizer", والد_2);
  }
  إذا (!LLVMSymbolizer_رستو_Or_Err)
    LLVMSymbolizer_رستو_Or_Err = sys::ڳوليو_پروگرام_By_Name("llvm-symbolizer");
  أرجع LLVMSymbolizer_رستو_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_استعمال_ٿيل
ساكن منطقي print_Symbolized_ڍير_Trace(StringRef Argv0, فراغ **ڍير_Trace,
                                      صحيح اونهائي, llvm::raw_ostream &OS) {
  إذا (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    أرجع خطأ;

  // Don't recursively invoke the llvm-symbolizer binary.
  إذا (Argv0.شامل_آهي("llvm-symbolizer"))
    أرجع خطأ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_رستو_Or_Err = get_LLVMSymbolizer_رستو(Argv0);
  إذا (!LLVMSymbolizer_رستو_Or_Err)
    أرجع خطأ;
  ثابت std::string &LLVMSymbolizer_رستو = *LLVMSymbolizer_رستو_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(خالي_اشارو, خالي_اشارو);

  خودڪار Symbolized_پتا_Opt = collect_پتو_Symbols(
      ڍير_Trace, اونهائي, MainExecutableName.c_str(), LLVMSymbolizer_رستو);
  إذا (!Symbolized_پتا_Opt)
    أرجع خطأ;
  لكل (بنانشان چوکٺو_No = 0; چوکٺو_No < Symbolized_پتا_Opt->سائيز_2();
       ++چوکٺو_No) {
    OS << ساڄو_justify(formatv("#{0}", چوکٺو_No).str(), std::log10(اونهائي) + 2)
       << ' ' << (*Symbolized_پتا_Opt)[چوکٺو_No].ٻيو_2 << '\n';
  }
  أرجع سچ;
}

#إذا LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
فراغ sys::symbolize_پتا(پتو_سيٽ &پتا,
                             Symbolized_پتو_Map &Symbolized_پتا) {
  أكد(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  خودڪار BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_ويڪٽر<فراغ *, 0> پتو_List(پتا.begin(), پتا.پڇاڙي());
  إذا (پتو_List.خالي_جايون())
    أرجع;
  llvm::ترتيب_ڏيو(پتو_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_رستو_Or_Err = get_LLVMSymbolizer_رستو();
  إذا (!LLVMSymbolizer_رستو_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ثابت std::string &LLVMSymbolizer_رستو = *LLVMSymbolizer_رستو_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(خالي_اشارو, خالي_اشارو);

  خودڪار Symbolized_پتا_Opt =
      collect_پتو_Symbols(پتو_List.begin(), پتو_List.سائيز_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_رستو);
  إذا (!Symbolized_پتا_Opt)
    أرجع;
  لكل (خودڪار Symbolized_چوکٺو : *Symbolized_پتا_Opt) {
    Small_ويڪٽر<std::string, 0> &SymbolizedAddrs =
        Symbolized_پتا[پتو_List[Symbolized_چوکٺو.پهريون_2]];
    SymbolizedAddrs.ڌڪو_پٺ(Symbolized_چوکٺو.ٻيو_2);
  }
  أرجع;
}
#جيڪڏهن_پورو

ساكن منطقي printMarkupContext(raw_ostream &OS, ثابت حرف *MainExecutableName);

LLVM_ATTRIBUTE_استعمال_ٿيل
ساكن منطقي print_Markup_ڍير_Trace(StringRef Argv0, فراغ **ڍير_Trace, صحيح اونهائي,
                                  raw_ostream &OS) {
  ثابت حرف *Env = getenv(EnableSymbolizerMarkupEnv);
  إذا (!Env || !*Env)
    أرجع خطأ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(خالي_اشارو, خالي_اشارو);
  إذا (!printMarkupContext(OS, MainExecutableName.c_str()))
    أرجع خطأ;
  لكل (صحيح I = 0; I < اونهائي; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ڍير_Trace[I]);
  أرجع سچ;
}

// Include the platform-specific parts of this class.
#جيڪڏهن_وصف_ٿيل LLVM_ON_UNIX
#ضمن "Unix/Signals.inc"
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "Windows/Signals.inc"
#جيڪڏهن_پورو
