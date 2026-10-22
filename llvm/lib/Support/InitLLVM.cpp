//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/InitLLVM.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringRef.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/AutoConvert.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Error.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ErrorHandling.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/ManagedStatic.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Signals.h"

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Windows/WindowsSupport.h"
#ਜੇ_ਅੰਤ

#ਜੇ defined(HAVE_UNISTD_H)
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>
#ਨਹੀਂਤਾਂ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDIN_FILENO
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STDIN_FILENO 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDOUT_FILENO
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STDOUT_FILENO 1
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ STDERR_FILENO
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STDERR_FILENO 2
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ RaiseLimits() {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  ਸਵੈਚਾਲਿਤ Raise_ਹੱਦਬੰਦੀ = [](ਪੂਰਨ resource) {
    ਬਣਤਰ rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    ਜੇ (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  Raise_ਹੱਦਬੰਦੀ(RLIMIT_AS);
  // Heap size.
  Raise_ਹੱਦਬੰਦੀ(RLIMIT_DATA);
  // Stack size.
  Raise_ਹੱਦਬੰਦੀ(RLIMIT_STACK);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ RLIMIT_RSS
  // Resident set size.
  Raise_ਹੱਦਬੰਦੀ(RLIMIT_RSS);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ CleanupStdHandles(ਖਾਲੀ *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::sys;

InitLLVM::InitLLVM(ਪੂਰਨ &Argc, ਅਚਲ ਅੱਖਰ **&Argv,
                   ਤਾਰਕਿਕ InstallPipeSignalExitHandler,
                   ਤਾਰਕਿਕ NeedsPOSIXUtilitySignalHandling) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ NDEBUG
  ਸਥਿਰ std::atomic<ਤਾਰਕਿਕ> Initialized{ਝੂਠ};
  ਪ੍ਰਤਿਜ੍ਞਾ(!Initialized && "InitLLVM was already initialized!");
  Initialized = ਸੱਚ;
#ਜੇ_ਅੰਤ

  // Bring stdin/stdout/stderr into a known state.
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
  sys::ਜੋੜੋ_Signal_Handler(CleanupStdHandles, ਖਾਲੀ_ਸੂਚਕ);
#ਨਹੀਂਤਾਂ
  sys::ਜੋੜੋ_Signal_Handler(CleanupStdHandles, ਖਾਲੀ_ਸੂਚਕ,
                        NeedsPOSIXUtilitySignalHandling);
#ਜੇ_ਅੰਤ

  ਜੇ (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::ਸੈੱਟ_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  ਢੇਰ_Printer.emplace(Argc, Argv);
  sys::Print_ਢੇਰ_Trace_On_Error_Signal(Argv[0]);
  install_ਬਾਹਰਲਾ_ਦਾ_ਸਮ੍ਰਿਤੀ_new_handler();
  RaiseLimits();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ _WIN32
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

  ExitOnErr(error_ਕੋਡ_ਮੰਜ਼ਿਲ_Error(windows::Get_Command_ਸਤਰ_Arguments(ਦਲੀਲਾਂ, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  ਦਲੀਲਾਂ.ਧੱਕੋ_ਪਿੱਠ(ਖਾਲੀ_ਸੂਚਕ);

  Argc = ਦਲੀਲਾਂ.ਮਾਪ_2() - 1;
  Argv = ਦਲੀਲਾਂ.ਅੰਕੜੇ();
#ਜੇ_ਅੰਤ
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(ਖਾਲੀ_ਸੂਚਕ);
  llvm_shutdown();
}
