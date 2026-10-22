//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/InitLLVM.h"
#ضمن "llvm/ADT/StringRef.h"
#ضمن "llvm/Support/AutoConvert.h"
#ضمن "llvm/Support/Error.h"
#ضمن "llvm/Support/ErrorHandling.h"
#ضمن "llvm/Support/ManagedStatic.h"
#ضمن "llvm/Support/Signals.h"

#ifdef _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#endif

#إذا defined(HAVE_UNISTD_H)
#ضمن <unistd.h>
#وإلا
#ifndef STDIN_FILENO
#define STDIN_FILENO 0
#endif
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif
#endif

ساكن فراغ RaiseLimits() {
#ifdef _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  auto RaiseLimit = [](صحيح resource) {
    بنية rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    إذا (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
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
#ifdef RLIMIT_RSS
  // Resident set size.
  RaiseLimit(RLIMIT_RSS);
#endif
#endif
}

فراغ CleanupStdHandles(فراغ *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

using نطاق llvm;
using نطاق llvm::sys;

InitLLVM::InitLLVM(صحيح &Argc, ثابت حرف **&Argv,
                   منطقي InstallPipeSignalExitHandler,
                   منطقي NeedsPOSIXUtilitySignalHandling) {
#ifndef NDEBUG
  ساكن std::atomic<منطقي> Initialized{خطأ};
  أكد(!Initialized && "InitLLVM was already initialized!");
  Initialized = true;
#endif

  // Bring stdin/stdout/stderr into a known state.
#ifdef _WIN32
  sys::AddSignalHandler(CleanupStdHandles, nullptr);
#وإلا
  sys::AddSignalHandler(CleanupStdHandles, nullptr,
                        NeedsPOSIXUtilitySignalHandling);
#endif

  إذا (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::SetOneShotPipeSignalFunction(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  دۆۋە_Printer.emplace(Argc, Argv);
  sys::Print_دۆۋە_Trace_On_Error_Signal(Argv[0]);
  install_out_of_memory_new_handler();
  RaiseLimits();

#ifdef __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(errorCodeToError(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(errorCodeToError(llvm::enableAutoConversion(STDOUT_FILENO)));
#endif

#ifdef _WIN32
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

  ExitOnErr(errorCodeToError(windows::Get_Command_قۇر_Arguments(Args, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  Args.ئىتتىرىش_back(nullptr);

  Argc = Args.size() - 1;
  Argv = Args.data();
#endif
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(nullptr);
  llvm_shutdown();
}
