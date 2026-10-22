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

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "DebugOptions.h"

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/llvm-config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/CommandLine.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorOr.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileSystem.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FileUtilities.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Format.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/FormatVariadic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/IOSandbox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/MemoryBuffer.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Path.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Program.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/StringSaver.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"
#ਅਨ੍ਤਰ੍ਭਾਵ <array>
#ਅਨ੍ਤਰ੍ਭਾਵ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
ਸਥਿਰ ਤਾਰਕਿਕ DisableSymbolicationFlag = ਝੂਠ;
ਸਥਿਰ ManagedStatic<std::string> CrashDiagnosticsDirectory;
ਨਾਮਾਕਾਸ਼ {
ਬਣਤਰ CreateDisableSymbolication {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<ਤਾਰਕਿਕ, ਸੱਚ>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
ਬਣਤਰ CreateCrashDiagnosticsDir {
  ਸਥਿਰ ਖਾਲੀ *call() {
    ਵਾਪਸ ਨਵਾਂ cl::opt<std::string, ਸੱਚ>(
        "crash-diagnostics-dir", cl::ਮੁੱਲ_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
ਖਾਲੀ llvm::init_ਸੰਕੇਤ_Options() {
  ਸਥਿਰ ManagedStatic<cl::opt<ਤਾਰਕਿਕ, ਸੱਚ>, CreateDisableSymbolication>
      DisableSymbolication;
  ਸਥਿਰ ManagedStatic<cl::opt<std::string, ਸੱਚ>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr ਅੱਖਰ DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr ਅੱਖਰ LLVMSymbolizer_ਰਸਤਾ_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr ਅੱਖਰ EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
ਬਣਤਰ CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  ਖਾਲੀ *Cookie;
  ਗਿਣਤੀ ਵਰਗ ਸਥਿਤੀ { ਖਾਲੀ_ਥਾਵਾਂ_2, Initializing, Initialized, Executing };
  std::atomic<ਸਥਿਤੀ> Flag;
};

ਸਥਿਰ constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
ਸਥਿਰ std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_ਮੰਜ਼ਿਲ_ਲੜੀਵਾਰ() {
  ਸਥਿਰ std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  ਵਾਪਸ callbacks;
}

// Signal-safe.
ਖਾਲੀ sys::ਲੜੀਵਾਰ_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  ਸਵੈਚਾਲਿਤ BypassSandbox = sandbox::scopedDisable();

  ਲਈ (CallbackAndCookie &ਲੜੀਵਾਰ_Me : Call_Backs_ਮੰਜ਼ਿਲ_ਲੜੀਵਾਰ()) {
    ਸਵੈਚਾਲਿਤ Expected = CallbackAndCookie::ਸਥਿਤੀ::Initialized;
    ਸਵੈਚਾਲਿਤ Desired = CallbackAndCookie::ਸਥਿਤੀ::Executing;
    ਜੇ (!ਲੜੀਵਾਰ_Me.Flag.compare_exchange_strong(Expected, Desired))
      ਜਾਰੀ;
    (*ਲੜੀਵਾਰ_Me.Callback)(ਲੜੀਵਾਰ_Me.Cookie);
    ਲੜੀਵਾਰ_Me.Callback = ਖਾਲੀ_ਸੂਚਕ;
    ਲੜੀਵਾਰ_Me.Cookie = ਖਾਲੀ_ਸੂਚਕ;
    ਲੜੀਵਾਰ_Me.Flag.store(CallbackAndCookie::ਸਥਿਤੀ::ਖਾਲੀ_ਥਾਵਾਂ_2);
  }
}

// Signal-safe.
ਸਥਿਰ ਖਾਲੀ ਪਾਓ_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                ਖਾਲੀ *Cookie) {
  ਲਈ (CallbackAndCookie &ਸੈੱਟ_Me : Call_Backs_ਮੰਜ਼ਿਲ_ਲੜੀਵਾਰ()) {
    ਸਵੈਚਾਲਿਤ Expected = CallbackAndCookie::ਸਥਿਤੀ::ਖਾਲੀ_ਥਾਵਾਂ_2;
    ਸਵੈਚਾਲਿਤ Desired = CallbackAndCookie::ਸਥਿਤੀ::Initializing;
    ਜੇ (!ਸੈੱਟ_Me.Flag.compare_exchange_strong(Expected, Desired))
      ਜਾਰੀ;
    ਸੈੱਟ_Me.Callback = FnPtr;
    ਸੈੱਟ_Me.Cookie = Cookie;
    ਸੈੱਟ_Me.Flag.store(CallbackAndCookie::ਸਥਿਤੀ::Initialized);
    ਵਾਪਸ;
  }
  report_fatal_error("too many signal callbacks already registered");
}

ਸਥਿਰ ਤਾਰਕਿਕ ਲੱਭੋ_Modules_And_Offsets(ਖਾਲੀ **ਢੇਰ_Trace, ਪੂਰਨ ਡੂੰਘਾਈ,
                                  ਅਚਲ ਅੱਖਰ **Modules, intptr_t *Offsets,
                                  ਅਚਲ ਅੱਖਰ *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
ਸਥਿਰ Formatted_ਸੰਖਿਆ format_ptr(ਖਾਲੀ *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  ਬਿਨਾਨਿਸ਼ਾਨੀ Ptr_ਚੌੜਾਈ = 2 + 2 * ਆਕਾਰ(ਖਾਲੀ *);
  ਵਾਪਸ format_hex((uint64_t)PC, Ptr_ਚੌੜਾਈ);
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
std::optional<Small_ਸਦਿਸ਼<std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, std::string>, 0>>
collect_ਪਤਾ_Symbols(ਖਾਲੀ **ਪਤਾ_List, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪਤਾ_ਗਿਣਤੀ,
                      ਅਚਲ ਅੱਖਰ *MainExecutableName,
                      ਅਚਲ std::string &LLVMSymbolizer_ਰਸਤਾ) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_ਸਦਿਸ਼<ਅਚਲ ਅੱਖਰ *, 0> Modules(ਪਤਾ_ਗਿਣਤੀ, ਖਾਲੀ_ਸੂਚਕ);
  Small_ਸਦਿਸ਼<intptr_t, 0> Offsets(ਪਤਾ_ਗਿਣਤੀ, 0);
  ਜੇ (!ਲੱਭੋ_Modules_And_Offsets(ਪਤਾ_List, ਪਤਾ_ਗਿਣਤੀ, Modules.ਅੰਕੜੇ(),
                             Offsets.ਅੰਕੜੇ(), MainExecutableName, StrPool))
    ਵਾਪਸ {};
  ਪੂਰਨ ਨਿਵੇਸ਼_FD;
  SmallString<32> ਨਿਵੇਸ਼_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", ਨਿਵੇਸ਼_FD, ਨਿਵੇਸ਼_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover ਨਿਵੇਸ਼_Remover(ਨਿਵੇਸ਼_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream ਨਿਵੇਸ਼(ਨਿਵੇਸ਼_FD, ਸੱਚ);
    ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ AddrIdx = 0; AddrIdx < ਪਤਾ_ਗਿਣਤੀ; AddrIdx++) {
      ਜੇ (Modules[AddrIdx])
        ਨਿਵੇਸ਼ << Modules[AddrIdx] << " " << (ਖਾਲੀ *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {ਨਿਵੇਸ਼_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef ਦਲੀਲਾਂ[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#ਜੇ_ਅੰਤ
                      "--demangle"};
  ਪੂਰਨ ਲੜੀਵਾਰ_ਨਤੀਜਾ =
      sys::ExecuteAndWait(LLVMSymbolizer_ਰਸਤਾ, ਦਲੀਲਾਂ, std::nullopt, Redirects);
  ਜੇ (ਲੜੀਵਾਰ_ਨਤੀਜਾ != 0)
    ਵਾਪਸ {};

  Small_ਸਦਿਸ਼<std::pair<ਬਿਨਾਨਿਸ਼ਾਨੀ, std::string>, 0> ਨਤੀਜਾ;
  ਸਵੈਚਾਲਿਤ OutputBuf = ਸਮ੍ਰਿਤੀ_ਬਫਰ::getFile(OutputFile.c_str());
  ਜੇ (!OutputBuf)
    ਵਾਪਸ {};
  StringRef Output = OutputBuf.get()->get_ਬਫਰ();
  Small_ਸਦਿਸ਼<StringRef, 32> ਸਤਰਾਂ;
  Output.ਵੰਡੋ(ਸਤਰਾਂ, "\n");
  ਸਵੈਚਾਲਿਤ *Cur_ਸਤਰ = ਸਤਰਾਂ.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ AddrIdx = 0; AddrIdx < ਪਤਾ_ਗਿਣਤੀ; AddrIdx++) {
    ਜੇ (!Modules[AddrIdx]) {
      ਸਵੈਚਾਲਿਤ &Symbolized_ਚੌਖਟਾ = ਨਤੀਜਾ.emplace_ਪਿੱਠ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_ਚੌਖਟਾ.ਦੂਜਾ_2);
      OS << format_ptr(ਪਤਾ_List[AddrIdx]);
      ਜਾਰੀ;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    ਲਈ (;;) {
      ਜੇ (Cur_ਸਤਰ == ਸਤਰਾਂ.ਸਿਰਾ())
        ਵਾਪਸ {};
      StringRef FunctionName = *Cur_ਸਤਰ++;
      ਜੇ (FunctionName.ਖਾਲੀ_ਥਾਵਾਂ())
        ਰੋਕੋ;
      ਸਵੈਚਾਲਿਤ &Symbolized_ਚੌਖਟਾ = ਨਤੀਜਾ.emplace_ਪਿੱਠ(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_ਚੌਖਟਾ.ਦੂਜਾ_2);
      OS << format_ptr(ਪਤਾ_List[AddrIdx]) << ' ';
      ਜੇ (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      ਜੇ (Cur_ਸਤਰ == ਸਤਰਾਂ.ਸਿਰਾ())
        ਵਾਪਸ {};
      StringRef File_ਸਤਰ_Info = *Cur_ਸਤਰ++;
      ਜੇ (!File_ਸਤਰ_Info.starts_with("??")) {
        OS << File_ਸਤਰ_Info;
      } ਨਹੀਂਤਾਂ {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  ਵਾਪਸ ਨਤੀਜਾ;
}

ErrorOr<std::string> get_LLVMSymbolizer_ਰਸਤਾ(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_ਰਸਤਾ_Or_Err = std::error_ਕੋਡ();
  ਜੇ (ਅਚਲ ਅੱਖਰ *ਰਸਤਾ = getenv(LLVMSymbolizer_ਰਸਤਾ_Env)) {
    LLVMSymbolizer_ਰਸਤਾ_Or_Err = sys::ਲੱਭੋ_ਪ੍ਰੋਗਰਾਮ_By_Name(ਰਸਤਾ);
  } ਨਹੀਂਤਾਂ ਜੇ (!Argv0.ਖਾਲੀ_ਥਾਵਾਂ()) {
    StringRef ਮਾਪਾ_2 = llvm::sys::ਰਸਤਾ_2::ਮਾਪਾ_ਰਸਤਾ(Argv0);
    ਜੇ (!ਮਾਪਾ_2.ਖਾਲੀ_ਥਾਵਾਂ())
      LLVMSymbolizer_ਰਸਤਾ_Or_Err =
          sys::ਲੱਭੋ_ਪ੍ਰੋਗਰਾਮ_By_Name("llvm-symbolizer", ਮਾਪਾ_2);
  }
  ਜੇ (!LLVMSymbolizer_ਰਸਤਾ_Or_Err)
    LLVMSymbolizer_ਰਸਤਾ_Or_Err = sys::ਲੱਭੋ_ਪ੍ਰੋਗਰਾਮ_By_Name("llvm-symbolizer");
  ਵਾਪਸ LLVMSymbolizer_ਰਸਤਾ_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_ਵਰਤਿਆ
ਸਥਿਰ ਤਾਰਕਿਕ print_Symbolized_ਢੇਰ_Trace(StringRef Argv0, ਖਾਲੀ **ਢੇਰ_Trace,
                                      ਪੂਰਨ ਡੂੰਘਾਈ, llvm::raw_ostream &OS) {
  ਜੇ (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    ਵਾਪਸ ਝੂਠ;

  // Don't recursively invoke the llvm-symbolizer binary.
  ਜੇ (Argv0.ਸ਼ਾਮਲ_ਹੈ("llvm-symbolizer"))
    ਵਾਪਸ ਝੂਠ;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_ਰਸਤਾ_Or_Err = get_LLVMSymbolizer_ਰਸਤਾ(Argv0);
  ਜੇ (!LLVMSymbolizer_ਰਸਤਾ_Or_Err)
    ਵਾਪਸ ਝੂਠ;
  ਅਚਲ std::string &LLVMSymbolizer_ਰਸਤਾ = *LLVMSymbolizer_ਰਸਤਾ_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ);

  ਸਵੈਚਾਲਿਤ Symbolized_ਪਤੇ_Opt = collect_ਪਤਾ_Symbols(
      ਢੇਰ_Trace, ਡੂੰਘਾਈ, MainExecutableName.c_str(), LLVMSymbolizer_ਰਸਤਾ);
  ਜੇ (!Symbolized_ਪਤੇ_Opt)
    ਵਾਪਸ ਝੂਠ;
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌਖਟਾ_No = 0; ਚੌਖਟਾ_No < Symbolized_ਪਤੇ_Opt->ਮਾਪ_2();
       ++ਚੌਖਟਾ_No) {
    OS << ਸੱਜਾ_justify(formatv("#{0}", ਚੌਖਟਾ_No).str(), std::log10(ਡੂੰਘਾਈ) + 2)
       << ' ' << (*Symbolized_ਪਤੇ_Opt)[ਚੌਖਟਾ_No].ਦੂਜਾ_2 << '\n';
  }
  ਵਾਪਸ ਸੱਚ;
}

#ਜੇ LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
ਖਾਲੀ sys::symbolize_ਪਤੇ(ਪਤਾ_ਸੈੱਟ &ਪਤੇ,
                             Symbolized_ਪਤਾ_Map &Symbolized_ਪਤੇ) {
  ਪ੍ਰਤਿਜ੍ਞਾ(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  ਸਵੈਚਾਲਿਤ BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_ਸਦਿਸ਼<ਖਾਲੀ *, 0> ਪਤਾ_List(ਪਤੇ.begin(), ਪਤੇ.ਸਿਰਾ());
  ਜੇ (ਪਤਾ_List.ਖਾਲੀ_ਥਾਵਾਂ())
    ਵਾਪਸ;
  llvm::ਛਾਂਟੋ(ਪਤਾ_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_ਰਸਤਾ_Or_Err = get_LLVMSymbolizer_ਰਸਤਾ();
  ਜੇ (!LLVMSymbolizer_ਰਸਤਾ_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  ਅਚਲ std::string &LLVMSymbolizer_ਰਸਤਾ = *LLVMSymbolizer_ਰਸਤਾ_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ);

  ਸਵੈਚਾਲਿਤ Symbolized_ਪਤੇ_Opt =
      collect_ਪਤਾ_Symbols(ਪਤਾ_List.begin(), ਪਤਾ_List.ਮਾਪ_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_ਰਸਤਾ);
  ਜੇ (!Symbolized_ਪਤੇ_Opt)
    ਵਾਪਸ;
  ਲਈ (ਸਵੈਚਾਲਿਤ Symbolized_ਚੌਖਟਾ : *Symbolized_ਪਤੇ_Opt) {
    Small_ਸਦਿਸ਼<std::string, 0> &SymbolizedAddrs =
        Symbolized_ਪਤੇ[ਪਤਾ_List[Symbolized_ਚੌਖਟਾ.ਪਹਿਲਾ_2]];
    SymbolizedAddrs.ਧੱਕੋ_ਪਿੱਠ(Symbolized_ਚੌਖਟਾ.ਦੂਜਾ_2);
  }
  ਵਾਪਸ;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਤਾਰਕਿਕ printMarkupContext(raw_ostream &OS, ਅਚਲ ਅੱਖਰ *MainExecutableName);

LLVM_ATTRIBUTE_ਵਰਤਿਆ
ਸਥਿਰ ਤਾਰਕਿਕ print_Markup_ਢੇਰ_Trace(StringRef Argv0, ਖਾਲੀ **ਢੇਰ_Trace, ਪੂਰਨ ਡੂੰਘਾਈ,
                                  raw_ostream &OS) {
  ਅਚਲ ਅੱਖਰ *Env = getenv(EnableSymbolizerMarkupEnv);
  ਜੇ (!Env || !*Env)
    ਵਾਪਸ ਝੂਠ;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(ਖਾਲੀ_ਸੂਚਕ, ਖਾਲੀ_ਸੂਚਕ);
  ਜੇ (!printMarkupContext(OS, MainExecutableName.c_str()))
    ਵਾਪਸ ਝੂਠ;
  ਲਈ (ਪੂਰਨ I = 0; I < ਡੂੰਘਾਈ; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, ਢੇਰ_Trace[I]);
  ਵਾਪਸ ਸੱਚ;
}

// Include the platform-specific parts of this class.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ LLVM_ON_UNIX
#ਅਨ੍ਤਰ੍ਭਾਵ "Unix/Signals.inc"
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "Windows/Signals.inc"
#ਜੇ_ਅੰਤ
