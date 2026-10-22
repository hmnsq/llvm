//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/InitLLVM.h"
#شامل "llvm/ADT/StringRef.h"
#شامل "llvm/Support/AutoConvert.h"
#شامل "llvm/Support/Error.h"
#شامل "llvm/Support/ErrorHandling.h"
#شامل "llvm/Support/ManagedStatic.h"
#شامل "llvm/Support/Signals.h"

#اگر_تعریف_شدہ _WIN32
#شامل "llvm/Support/Windows/WindowsSupport.h"
#اگر_ختم

#اگر defined(HAVE_UNISTD_H)
#شامل <unistd.h>
#ورنہ
#اگر_غیر_تعریف_شدہ STDIN_FILENO
#تعریف_کرو STDIN_FILENO 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ STDOUT_FILENO
#تعریف_کرو STDOUT_FILENO 1
#اگر_ختم
#اگر_غیر_تعریف_شدہ STDERR_FILENO
#تعریف_کرو STDERR_FILENO 2
#اگر_ختم
#اگر_ختم

ساکن خالی RaiseLimits() {
#اگر_تعریف_شدہ _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  خودکار Raise_حد_بندی = [](صحیح resource) {
    ساخت rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    اگر (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  Raise_حد_بندی(RLIMIT_AS);
  // Heap size.
  Raise_حد_بندی(RLIMIT_DATA);
  // Stack size.
  Raise_حد_بندی(RLIMIT_STACK);
#اگر_تعریف_شدہ RLIMIT_RSS
  // Resident set size.
  Raise_حد_بندی(RLIMIT_RSS);
#اگر_ختم
#اگر_ختم
}

خالی CleanupStdHandles(خالی *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

استعمال نامخلا llvm;
استعمال نامخلا llvm::sys;

InitLLVM::InitLLVM(صحیح &Argc, ثابت حرف **&Argv,
                   منطقی InstallPipeSignalExitHandler,
                   منطقی NeedsPOSIXUtilitySignalHandling) {
#اگر_غیر_تعریف_شدہ NDEBUG
  ساکن std::atomic<منطقی> Initialized{جھوٹ};
  أكد(!Initialized && "InitLLVM was already initialized!");
  Initialized = سچ;
#اگر_ختم

  // Bring stdin/stdout/stderr into a known state.
#اگر_تعریف_شدہ _WIN32
  sys::جوڑو_Signal_Handler(CleanupStdHandles, خالی_اشاریہ);
#ورنہ
  sys::جوڑو_Signal_Handler(CleanupStdHandles, خالی_اشاریہ,
                        NeedsPOSIXUtilitySignalHandling);
#اگر_ختم

  اگر (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::سیٹ_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  ڈھیر_Printer.emplace(Argc, Argv);
  sys::Print_ڈھیر_Trace_On_Error_Signal(Argv[0]);
  install_برآمد_کا_حافظہ_new_handler();
  RaiseLimits();

#اگر_تعریف_شدہ __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_رمز_مقصد_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_رمز_مقصد_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#اگر_ختم

#اگر_تعریف_شدہ _WIN32
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

  ExitOnErr(error_رمز_مقصد_Error(windows::Get_Command_سطر_Arguments(دلائل, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  دلائل.ڈالو_عقب(خالی_اشاریہ);

  Argc = دلائل.سائز_2() - 1;
  Argv = دلائل.ڈیٹا();
#اگر_ختم
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(خالی_اشاریہ);
  llvm_shutdown();
}
