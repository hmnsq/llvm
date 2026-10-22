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

#که_تعریف_شوی _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#د_که_پای

#إذا defined(HAVE_UNISTD_H)
#ضمن <unistd.h>
#وإلا
#که_نه_تعریف_شوی STDIN_FILENO
#تعریف_کړه STDIN_FILENO 0
#د_که_پای
#که_نه_تعریف_شوی STDOUT_FILENO
#تعریف_کړه STDOUT_FILENO 1
#د_که_پای
#که_نه_تعریف_شوی STDERR_FILENO
#تعریف_کړه STDERR_FILENO 2
#د_که_پای
#د_که_پای

ساكن فراغ RaiseLimits() {
#که_تعریف_شوی _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  خپلکاری Raise_حد = [](صحيح resource) {
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
  Raise_حد(RLIMIT_AS);
  // Heap size.
  Raise_حد(RLIMIT_DATA);
  // Stack size.
  Raise_حد(RLIMIT_STACK);
#که_تعریف_شوی RLIMIT_RSS
  // Resident set size.
  Raise_حد(RLIMIT_RSS);
#د_که_پای
#د_که_پای
}

فراغ CleanupStdHandles(فراغ *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

کارول نطاق llvm;
کارول نطاق llvm::sys;

InitLLVM::InitLLVM(صحيح &Argc, ثابت حرف **&Argv,
                   منطقي InstallPipeSignalExitHandler,
                   منطقي NeedsPOSIXUtilitySignalHandling) {
#که_نه_تعریف_شوی NDEBUG
  ساكن std::atomic<منطقي> Initialized{خطأ};
  أكد(!Initialized && "InitLLVM was already initialized!");
  Initialized = ریښتیا;
#د_که_پای

  // Bring stdin/stdout/stderr into a known state.
#که_تعریف_شوی _WIN32
  sys::جمع_کړه_Signal_Handler(CleanupStdHandles, تش_اشاره);
#وإلا
  sys::جمع_کړه_Signal_Handler(CleanupStdHandles, تش_اشاره,
                        NeedsPOSIXUtilitySignalHandling);
#د_که_پای

  إذا (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::سېټ_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  ډېری_Printer.emplace(Argc, Argv);
  sys::Print_ډېری_Trace_On_Error_Signal(Argv[0]);
  install_بهر_د_حافظه_new_handler();
  RaiseLimits();

#که_تعریف_شوی __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_کوډ_منزل_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_کوډ_منزل_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#د_که_پای

#که_تعریف_شوی _WIN32
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

  ExitOnErr(error_کوډ_منزل_Error(windows::Get_Command_کرښه_Arguments(دلیلونه, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  دلیلونه.ټېل_شا(تش_اشاره);

  Argc = دلیلونه.اندازه_2() - 1;
  Argv = دلیلونه.معلومات();
#د_که_پای
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(تش_اشاره);
  llvm_shutdown();
}
