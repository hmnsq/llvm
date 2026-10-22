//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/Support/InitLLVM.h"
#అన్తర్భావ "llvm/ADT/StringRef.h"
#అన్తర్భావ "llvm/Support/AutoConvert.h"
#అన్తర్భావ "llvm/Support/Error.h"
#అన్తర్భావ "llvm/Support/ErrorHandling.h"
#అన్తర్భావ "llvm/Support/ManagedStatic.h"
#అన్తర్భావ "llvm/Support/Signals.h"

#నిర్వచితమైతే _WIN32
#అన్తర్భావ "llvm/Support/Windows/WindowsSupport.h"
#అయితే_ముగింపు

#ఒకవేళ defined(HAVE_UNISTD_H)
#అన్తర్భావ <unistd.h>
#లేకపోతే
#నిర్వచితం_కాకపోతే STDIN_FILENO
#నిర్వచించు STDIN_FILENO 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే STDOUT_FILENO
#నిర్వచించు STDOUT_FILENO 1
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే STDERR_FILENO
#నిర్వచించు STDERR_FILENO 2
#అయితే_ముగింపు
#అయితే_ముగింపు

స్థిర శూన్య RaiseLimits() {
#నిర్వచితమైతే _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  స్వయంచాలక Raise_హద్దు = [](పూర్ణాంకం resource) {
    నిర్మాణం rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    ఒకవేళ (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  Raise_హద్దు(RLIMIT_AS);
  // Heap size.
  Raise_హద్దు(RLIMIT_DATA);
  // Stack size.
  Raise_హద్దు(RLIMIT_STACK);
#నిర్వచితమైతే RLIMIT_RSS
  // Resident set size.
  Raise_హద్దు(RLIMIT_RSS);
#అయితే_ముగింపు
#అయితే_ముగింపు
}

శూన్య CleanupStdHandles(శూన్య *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::sys;

InitLLVM::InitLLVM(పూర్ణాంకం &Argc, స్థిరరాశి అక్షరం **&Argv,
                   తార్కిక InstallPipeSignalExitHandler,
                   తార్కిక NeedsPOSIXUtilitySignalHandling) {
#నిర్వచితం_కాకపోతే NDEBUG
  స్థిర std::atomic<తార్కిక> Initialized{అబద్ధం};
  ప్రతిజ్ఞా(!Initialized && "InitLLVM was already initialized!");
  Initialized = నిజం;
#అయితే_ముగింపు

  // Bring stdin/stdout/stderr into a known state.
#నిర్వచితమైతే _WIN32
  sys::కూడు_Signal_Handler(CleanupStdHandles, శూన్యసూచిక);
#లేకపోతే
  sys::కూడు_Signal_Handler(CleanupStdHandles, శూన్యసూచిక,
                        NeedsPOSIXUtilitySignalHandling);
#అయితే_ముగింపు

  ఒకవేళ (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::సమితి_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  దొంతర_Printer.emplace(Argc, Argv);
  sys::Print_దొంతర_Trace_On_Error_Signal(Argv[0]);
  install_బహిర్గతం_యొక్క_జ్ఞాపకం_new_handler();
  RaiseLimits();

#నిర్వచితమైతే __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_సంకేతం_గమ్యం_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_సంకేతం_గమ్యం_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#అయితే_ముగింపు

#నిర్వచితమైతే _WIN32
  // We use UTF-8 as the internal character encoding. On Windows,
  // arguments passed to main() may not be encoded in UTF-8. In order
  // to reliably detect encoding of command line arguments, we use an
  // Windows API to obtain arguments, convert them to UTF-8, and then
  // write them back to the Argv vector.
  //
  // There's probably other way to do the same thing (e.g. using
  // wmain() instead of main()), but this way seems less intrusive
  // than that.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  ExitOnErr(error_సంకేతం_గమ్యం_Error(windows::Get_Command_పంక్తి_Arguments(వాదనలు, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  వాదనలు.నెట్టు_వెనుక(శూన్యసూచిక);

  Argc = వాదనలు.కొలత_2() - 1;
  Argv = వాదనలు.దత్తాంశం();
#అయితే_ముగింపు
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(శూన్యసూచిక);
  llvm_shutdown();
}
