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

#అన్తర్భావ "llvm/Support/Signals.h"

#అన్తర్భావ "DebugOptions.h"

#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Config/llvm-config.h"
#అన్తర్భావ "llvm/Support/CommandLine.h"
#అన్తర్భావ "llvm/Support/ErrorOr.h"
#అన్తర్భావ "llvm/Support/FileSystem.h"
#అన్తర్భావ "llvm/Support/FileUtilities.h"
#అన్తర్భావ "llvm/Support/Format.h"
#అన్తర్భావ "llvm/Support/FormatVariadic.h"
#అన్తర్భావ "llvm/Support/IOSandbox.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/MemoryBuffer.h"
#అన్తర్భావ "llvm/Support/Path.h"
#అన్తర్భావ "llvm/Support/Program.h"
#అన్తర్భావ "llvm/Support/StringSaver.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"
#అన్తర్భావ <array>
#అన్తర్భావ <cmath>

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only TRULY operating system
//===          independent code.
//===----------------------------------------------------------------------===//

ఉపయోగించి నామాకాశ llvm;

// Use explicit storage to avoid accessing cl::opt in a signal handler.
స్థిర తార్కిక DisableSymbolicationFlag = అబద్ధం;
స్థిర ManagedStatic<std::string> CrashDiagnosticsDirectory;
నామాకాశ {
నిర్మాణం CreateDisableSymbolication {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<తార్కిక, నిజం>(
        "disable-symbolication",
        cl::desc("Disable symbolizing crash backtraces."),
        cl::location(DisableSymbolicationFlag), cl::Hidden);
  }
};
నిర్మాణం CreateCrashDiagnosticsDir {
  స్థిర శూన్య *call() {
    తిరిగి కొత్త cl::opt<std::string, నిజం>(
        "crash-diagnostics-dir", cl::విలువ_desc("directory"),
        cl::desc("Directory for crash diagnostic files."),
        cl::location(*CrashDiagnosticsDirectory), cl::Hidden);
  }
};
} // namespace
శూన్య llvm::init_సంకేతాలు_Options() {
  స్థిర ManagedStatic<cl::opt<తార్కిక, నిజం>, CreateDisableSymbolication>
      DisableSymbolication;
  స్థిర ManagedStatic<cl::opt<std::string, నిజం>, CreateCrashDiagnosticsDir>
      CrashDiagnosticsDir;
  *DisableSymbolication;
  *CrashDiagnosticsDir;
}

constexpr అక్షరం DisableSymbolizationEnv[] = "LLVM_DISABLE_SYMBOLIZATION";
constexpr అక్షరం LLVMSymbolizer_మార్గం_Env[] = "LLVM_SYMBOLIZER_PATH";
constexpr అక్షరం EnableSymbolizerMarkupEnv[] = "LLVM_ENABLE_SYMBOLIZER_MARKUP";

// Callbacks to run in signal handler must be lock-free because a signal handler
// could be running as we add new callbacks. We don't add unbounded numbers of
// callbacks, an array is therefore sufficient.
నిర్మాణం CallbackAndCookie {
  sys::SignalHandlerCallback Callback;
  శూన్య *Cookie;
  గణన తరగతి స్థితి_2 { ఖాళీలు_2, Initializing, Initialized, Executing };
  std::atomic<స్థితి_2> Flag;
};

స్థిర constexpr size_t MaxSignalHandlerCallbacks = 8;

// A global array of CallbackAndCookie may not compile with
// -Werror=global-constructors in c++20 and above
స్థిర std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> &
Call_Backs_గమ్యం_నిరంతరం() {
  స్థిర std::array<CallbackAndCookie, MaxSignalHandlerCallbacks> callbacks;
  తిరిగి callbacks;
}

// Signal-safe.
శూన్య sys::నిరంతరం_Signal_Handlers() {
  // Let's not interfere with stack trace symbolication and friends.
  స్వయంచాలక BypassSandbox = sandbox::scopedDisable();

  కొరకు (CallbackAndCookie &నిరంతరం_Me : Call_Backs_గమ్యం_నిరంతరం()) {
    స్వయంచాలక Expected = CallbackAndCookie::స్థితి_2::Initialized;
    స్వయంచాలక Desired = CallbackAndCookie::స్థితి_2::Executing;
    ఒకవేళ (!నిరంతరం_Me.Flag.compare_exchange_strong(Expected, Desired))
      కొనసాగు;
    (*నిరంతరం_Me.Callback)(నిరంతరం_Me.Cookie);
    నిరంతరం_Me.Callback = శూన్యసూచిక;
    నిరంతరం_Me.Cookie = శూన్యసూచిక;
    నిరంతరం_Me.Flag.store(CallbackAndCookie::స్థితి_2::ఖాళీలు_2);
  }
}

// Signal-safe.
స్థిర శూన్య చొప్పించు_Signal_Handler(sys::SignalHandlerCallback FnPtr,
                                శూన్య *Cookie) {
  కొరకు (CallbackAndCookie &సమితి_Me : Call_Backs_గమ్యం_నిరంతరం()) {
    స్వయంచాలక Expected = CallbackAndCookie::స్థితి_2::ఖాళీలు_2;
    స్వయంచాలక Desired = CallbackAndCookie::స్థితి_2::Initializing;
    ఒకవేళ (!సమితి_Me.Flag.compare_exchange_strong(Expected, Desired))
      కొనసాగు;
    సమితి_Me.Callback = FnPtr;
    సమితి_Me.Cookie = Cookie;
    సమితి_Me.Flag.store(CallbackAndCookie::స్థితి_2::Initialized);
    తిరిగి;
  }
  report_fatal_error("too many signal callbacks already registered");
}

స్థిర తార్కిక కనుగొను_Modules_And_Offsets(శూన్య **దొంతర_Trace, పూర్ణాంకం లోతు,
                                  స్థిరరాశి అక్షరం **Modules, intptr_t *Offsets,
                                  స్థిరరాశి అక్షరం *MainExecutableName,
                                  StringSaver &StrPool);

/// Format a pointer value as hexadecimal. Zero pad it out so its always the
/// same width.
స్థిర Formatted_సంఖ్య format_ptr(శూన్య *PC) {
  // Each byte is two hex digits plus 2 for the 0x prefix.
  గుర్తులేని Ptr_వెడల్పు = 2 + 2 * పరిమాణం(శూన్య *);
  తిరిగి format_hex((uint64_t)PC, Ptr_వెడల్పు);
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
std::optional<Small_సదిశ<std::pair<గుర్తులేని, std::string>, 0>>
collect_చిరునామా_Symbols(శూన్య **చిరునామా_List, గుర్తులేని చిరునామా_లెక్క,
                      స్థిరరాశి అక్షరం *MainExecutableName,
                      స్థిరరాశి std::string &LLVMSymbolizer_మార్గం) {
  BumpPtrAllocator Allocator;
  StringSaver StrPool(Allocator);
  Small_సదిశ<స్థిరరాశి అక్షరం *, 0> Modules(చిరునామా_లెక్క, శూన్యసూచిక);
  Small_సదిశ<intptr_t, 0> Offsets(చిరునామా_లెక్క, 0);
  ఒకవేళ (!కనుగొను_Modules_And_Offsets(చిరునామా_List, చిరునామా_లెక్క, Modules.దత్తాంశం(),
                             Offsets.దత్తాంశం(), MainExecutableName, StrPool))
    తిరిగి {};
  పూర్ణాంకం ఆదానం_FD;
  SmallString<32> ఆదానం_File, OutputFile;
  sys::fs::createTemporaryFile("symbolizer-input", "", ఆదానం_FD, ఆదానం_File);
  sys::fs::createTemporaryFile("symbolizer-output", "", OutputFile);
  FileRemover ఆదానం_Remover(ఆదానం_File.c_str());
  FileRemover OutputRemover(OutputFile.c_str());

  {
    raw_fd_ostream ఆదానం(ఆదానం_FD, నిజం);
    కొరకు (గుర్తులేని AddrIdx = 0; AddrIdx < చిరునామా_లెక్క; AddrIdx++) {
      ఒకవేళ (Modules[AddrIdx])
        ఆదానం << Modules[AddrIdx] << " " << (శూన్య *)Offsets[AddrIdx] << "\n";
    }
  }

  std::optional<StringRef> Redirects[] = {ఆదానం_File.str(), OutputFile.str(),
                                          StringRef("")};
  StringRef వాదనలు[] = {"llvm-symbolizer", "--functions=linkage", "--inlining",
#నిర్వచితమైతే _WIN32
                      // Pass --relative-address on Windows so that we don't
                      // have to add ImageBase from PE file.
                      // FIXME: Make this the default for llvm-symbolizer.
                      "--relative-address",
#అయితే_ముగింపు
                      "--demangle"};
  పూర్ణాంకం నిరంతరం_ఫలితం =
      sys::ExecuteAndWait(LLVMSymbolizer_మార్గం, వాదనలు, std::nullopt, Redirects);
  ఒకవేళ (నిరంతరం_ఫలితం != 0)
    తిరిగి {};

  Small_సదిశ<std::pair<గుర్తులేని, std::string>, 0> ఫలితం;
  స్వయంచాలక OutputBuf = జ్ఞాపకం_బఫరు::getFile(OutputFile.c_str());
  ఒకవేళ (!OutputBuf)
    తిరిగి {};
  StringRef Output = OutputBuf.get()->get_బఫరు();
  Small_సదిశ<StringRef, 32> పంక్తులు;
  Output.విభజించు(పంక్తులు, "\n");
  స్వయంచాలక *Cur_పంక్తి = పంక్తులు.begin();
  // Lines contains the output from llvm-symbolizer, which should contain for
  // each address with a module in order of appearance, one or more lines
  // containing the function name and line associated with that address,
  // followed by an empty line.
  // For each address, adds an output entry for every real or inlined frame at
  // that address. For addresses without known modules, we have a single entry
  // containing just the formatted address; for all other output entries, we
  // output the function entry if it is known, and either the line number if it
  // is known or the module+address offset otherwise.
  కొరకు (గుర్తులేని AddrIdx = 0; AddrIdx < చిరునామా_లెక్క; AddrIdx++) {
    ఒకవేళ (!Modules[AddrIdx]) {
      స్వయంచాలక &Symbolized_చట్రం = ఫలితం.emplace_వెనుక(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_చట్రం.రెండవ_2);
      OS << format_ptr(చిరునామా_List[AddrIdx]);
      కొనసాగు;
    }
    // Read pairs of lines (function name and file/line info) until we
    // encounter empty line.
    కొరకు (;;) {
      ఒకవేళ (Cur_పంక్తి == పంక్తులు.చివర())
        తిరిగి {};
      StringRef FunctionName = *Cur_పంక్తి++;
      ఒకవేళ (FunctionName.ఖాళీలు())
        ఆపు;
      స్వయంచాలక &Symbolized_చట్రం = ఫలితం.emplace_వెనుక(std::make_pair(AddrIdx, ""));
      raw_string_ostream OS(Symbolized_చట్రం.రెండవ_2);
      OS << format_ptr(చిరునామా_List[AddrIdx]) << ' ';
      ఒకవేళ (!FunctionName.starts_with("??"))
        OS << FunctionName << ' ';
      ఒకవేళ (Cur_పంక్తి == పంక్తులు.చివర())
        తిరిగి {};
      StringRef File_పంక్తి_Info = *Cur_పంక్తి++;
      ఒకవేళ (!File_పంక్తి_Info.starts_with("??")) {
        OS << File_పంక్తి_Info;
      } లేకపోతే {
        OS << "(" << Modules[AddrIdx] << '+' << format_hex(Offsets[AddrIdx], 0)
           << ")";
      }
    }
  }
  తిరిగి ఫలితం;
}

ErrorOr<std::string> get_LLVMSymbolizer_మార్గం(StringRef Argv0 = {}) {
  ErrorOr<std::string> LLVMSymbolizer_మార్గం_Or_Err = std::error_సంకేతం();
  ఒకవేళ (స్థిరరాశి అక్షరం *మార్గం = getenv(LLVMSymbolizer_మార్గం_Env)) {
    LLVMSymbolizer_మార్గం_Or_Err = sys::కనుగొను_కార్యక్రమం_By_Name(మార్గం);
  } లేకపోతే ఒకవేళ (!Argv0.ఖాళీలు()) {
    StringRef మాతృ_2 = llvm::sys::మార్గం_2::మాతృ_మార్గం(Argv0);
    ఒకవేళ (!మాతృ_2.ఖాళీలు())
      LLVMSymbolizer_మార్గం_Or_Err =
          sys::కనుగొను_కార్యక్రమం_By_Name("llvm-symbolizer", మాతృ_2);
  }
  ఒకవేళ (!LLVMSymbolizer_మార్గం_Or_Err)
    LLVMSymbolizer_మార్గం_Or_Err = sys::కనుగొను_కార్యక్రమం_By_Name("llvm-symbolizer");
  తిరిగి LLVMSymbolizer_మార్గం_Or_Err;
}

/// Helper that launches llvm-symbolizer and symbolizes a backtrace.
LLVM_ATTRIBUTE_వాడిన
స్థిర తార్కిక print_Symbolized_దొంతర_Trace(StringRef Argv0, శూన్య **దొంతర_Trace,
                                      పూర్ణాంకం లోతు, llvm::raw_ostream &OS) {
  ఒకవేళ (DisableSymbolicationFlag || getenv(DisableSymbolizationEnv))
    తిరిగి అబద్ధం;

  // Don't recursively invoke the llvm-symbolizer binary.
  ఒకవేళ (Argv0.కలిగియుంది("llvm-symbolizer"))
    తిరిగి అబద్ధం;

  // FIXME: Subtract necessary number from StackTrace entries to turn return
  // addresses into actual instruction addresses.
  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_మార్గం_Or_Err = get_LLVMSymbolizer_మార్గం(Argv0);
  ఒకవేళ (!LLVMSymbolizer_మార్గం_Or_Err)
    తిరిగి అబద్ధం;
  స్థిరరాశి std::string &LLVMSymbolizer_మార్గం = *LLVMSymbolizer_మార్గం_Or_Err;

  // If we don't know argv0 or the address of main() at this point, try
  // to guess it anyway (it's possible on some platforms).
  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(శూన్యసూచిక, శూన్యసూచిక);

  స్వయంచాలక Symbolized_చిరునామాలు_Opt = collect_చిరునామా_Symbols(
      దొంతర_Trace, లోతు, MainExecutableName.c_str(), LLVMSymbolizer_మార్గం);
  ఒకవేళ (!Symbolized_చిరునామాలు_Opt)
    తిరిగి అబద్ధం;
  కొరకు (గుర్తులేని చట్రం_No = 0; చట్రం_No < Symbolized_చిరునామాలు_Opt->కొలత_2();
       ++చట్రం_No) {
    OS << కుడి_justify(formatv("#{0}", చట్రం_No).str(), std::log10(లోతు) + 2)
       << ' ' << (*Symbolized_చిరునామాలు_Opt)[చట్రం_No].రెండవ_2 << '\n';
  }
  తిరిగి నిజం;
}

#ఒకవేళ LLVM_ENABLE_DEBUGLOC_TRACKING_ORIGIN
శూన్య sys::symbolize_చిరునామాలు(చిరునామా_సమితి &చిరునామాలు,
                             Symbolized_చిరునామా_Map &Symbolized_చిరునామాలు) {
  ప్రతిజ్ఞా(!DisableSymbolicationFlag && !getenv(DisableSymbolizationEnv) &&
         "Debugify origin stacktraces require symbolization to be enabled.");

  // This function deals with temporary files for the purposes of symbolization
  // only, not formal compiler output.
  స్వయంచాలక BypassSandbox = sys::sandbox::scopedDisable();

  // Convert Set of Addresses to ordered list.
  Small_సదిశ<శూన్య *, 0> చిరునామా_List(చిరునామాలు.begin(), చిరునామాలు.చివర());
  ఒకవేళ (చిరునామా_List.ఖాళీలు())
    తిరిగి;
  llvm::క్రమీకరించు(చిరునామా_List);

  // Use llvm-symbolizer tool to symbolize the stack traces. First look for it
  // alongside our binary, then in $PATH.
  ErrorOr<std::string> LLVMSymbolizer_మార్గం_Or_Err = get_LLVMSymbolizer_మార్గం();
  ఒకవేళ (!LLVMSymbolizer_మార్గం_Or_Err)
    report_fatal_error("Debugify origin stacktraces require llvm-symbolizer");
  స్థిరరాశి std::string &LLVMSymbolizer_మార్గం = *LLVMSymbolizer_మార్గం_Or_Err;

  // Try to guess the main executable name, since we don't have argv0 available
  // here.
  std::string MainExecutableName = sys::fs::getMainExecutable(శూన్యసూచిక, శూన్యసూచిక);

  స్వయంచాలక Symbolized_చిరునామాలు_Opt =
      collect_చిరునామా_Symbols(చిరునామా_List.begin(), చిరునామా_List.కొలత_2(),
                            MainExecutableName.c_str(), LLVMSymbolizer_మార్గం);
  ఒకవేళ (!Symbolized_చిరునామాలు_Opt)
    తిరిగి;
  కొరకు (స్వయంచాలక Symbolized_చట్రం : *Symbolized_చిరునామాలు_Opt) {
    Small_సదిశ<std::string, 0> &SymbolizedAddrs =
        Symbolized_చిరునామాలు[చిరునామా_List[Symbolized_చట్రం.మొదటి_2]];
    SymbolizedAddrs.నెట్టు_వెనుక(Symbolized_చట్రం.రెండవ_2);
  }
  తిరిగి;
}
#అయితే_ముగింపు

స్థిర తార్కిక printMarkupContext(raw_ostream &OS, స్థిరరాశి అక్షరం *MainExecutableName);

LLVM_ATTRIBUTE_వాడిన
స్థిర తార్కిక print_Markup_దొంతర_Trace(StringRef Argv0, శూన్య **దొంతర_Trace, పూర్ణాంకం లోతు,
                                  raw_ostream &OS) {
  స్థిరరాశి అక్షరం *Env = getenv(EnableSymbolizerMarkupEnv);
  ఒకవేళ (!Env || !*Env)
    తిరిగి అబద్ధం;

  std::string MainExecutableName =
      sys::fs::exists(Argv0) ? std::string(Argv0)
                             : sys::fs::getMainExecutable(శూన్యసూచిక, శూన్యసూచిక);
  ఒకవేళ (!printMarkupContext(OS, MainExecutableName.c_str()))
    తిరిగి అబద్ధం;
  కొరకు (పూర్ణాంకం I = 0; I < లోతు; I++)
    OS << format("{{{bt:%d:%#016x}}}\n", I, దొంతర_Trace[I]);
  తిరిగి నిజం;
}

// Include the platform-specific parts of this class.
#నిర్వచితమైతే LLVM_ON_UNIX
#అన్తర్భావ "Unix/Signals.inc"
#అయితే_ముగింపు
#నిర్వచితమైతే _WIN32
#అన్తర్భావ "Windows/Signals.inc"
#అయితే_ముగింపు
