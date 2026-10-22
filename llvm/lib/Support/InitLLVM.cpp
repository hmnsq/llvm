//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/Support/InitLLVM.h"
#समावेश "llvm/ADT/StringRef.h"
#समावेश "llvm/Support/AutoConvert.h"
#समावेश "llvm/Support/Error.h"
#समावेश "llvm/Support/ErrorHandling.h"
#समावेश "llvm/Support/ManagedStatic.h"
#समावेश "llvm/Support/Signals.h"

#अगर_घोषित _WIN32
#समावेश "llvm/Support/Windows/WindowsSupport.h"
#पूर्ण_अगर

#अगर defined(HAVE_UNISTD_H)
#समावेश <unistd.h>
#अन्यथा
#अगर_अघोषित STDIN_FILENO
#घोषणा STDIN_FILENO 0
#पूर्ण_अगर
#अगर_अघोषित STDOUT_FILENO
#घोषणा STDOUT_FILENO 1
#पूर्ण_अगर
#अगर_अघोषित STDERR_FILENO
#घोषणा STDERR_FILENO 2
#पूर्ण_अगर
#पूर्ण_अगर

अटल व्योम RaiseLimits() {
#अगर_घोषित _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  स्वतः RaiseLimit = [](पूर्णांक resource) {
    काष्ठा rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    अगर (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  RaiseLimit(RLIMIT_AS);
  // Heap size.
  RaiseLimit(RLIMIT_DATA);
  // Stack size.
  RaiseLimit(RLIMIT_STACK);
#अगर_घोषित RLIMIT_RSS
  // Resident set size.
  RaiseLimit(RLIMIT_RSS);
#पूर्ण_अगर
#पूर्ण_अगर
}

व्योम CleanupStdHandles(व्योम *Cookie) {
  llvm::raw_ostream *Outs = &llvm::म_दो(), *Errs = &llvm::errs();
  Outs->सफाई();
  Errs->सफाई();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

using namespace llvm;
using namespace llvm::sys;

InitLLVM::InitLLVM(पूर्णांक &Argc, स्थिर अक्षर **&Argv,
                   bool InstallPipeSignalExitHandler,
                   bool NeedsPOSIXUtilitySignalHandling) {
#अगर_अघोषित न_संशोधन
  अटल std::atomic<bool> Initialized{false};
  निश्चित(!Initialized && "InitLLVM was already initialized!");
  Initialized = true;
#पूर्ण_अगर

  // Bring stdin/stdout/stderr into a known state.
#अगर_घोषित _WIN32
  sys::AddSignalHandler(CleanupStdHandles, nullptr);
#अन्यथा
  sys::AddSignalHandler(CleanupStdHandles, nullptr,
                        NeedsPOSIXUtilitySignalHandling);
#पूर्ण_अगर

  अगर (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::SetOneShotPipeSignalFunction(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  ढेर_Printer.emplace(Argc, Argv);
  sys::Print_ढेर_Trace_On_Error_Signal(Argv[0]);
  install_out_of_memory_new_handler();
  RaiseLimits();

#अगर_घोषित __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(errorCodeToError(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(errorCodeToError(llvm::enableAutoConversion(STDOUT_FILENO)));
#पूर्ण_अगर

#अगर_घोषित _WIN32
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

  ExitOnErr(errorCodeToError(windows::Get_Command_पंक्ति_Arguments(Args, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  Args.धकेलो_back(nullptr);

  Argc = Args.आकार_2() - 1;
  Argv = Args.तथ्य();
#पूर्ण_अगर
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(nullptr);
  llvm_shutdown();
}
