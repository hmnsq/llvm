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

#جيڪڏهن_وصف_ٿيل _WIN32
#ضمن "llvm/Support/Windows/WindowsSupport.h"
#جيڪڏهن_پورو

#إذا defined(HAVE_UNISTD_H)
#ضمن <unistd.h>
#وإلا
#جيڪڏهن_وصف_نه_ٿيل STDIN_FILENO
#وصف_ڪريو STDIN_FILENO 0
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل STDOUT_FILENO
#وصف_ڪريو STDOUT_FILENO 1
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل STDERR_FILENO
#وصف_ڪريو STDERR_FILENO 2
#جيڪڏهن_پورو
#جيڪڏهن_پورو

ساكن فراغ RaiseLimits() {
#جيڪڏهن_وصف_ٿيل _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  خودڪار Raise_حد_بندي = [](صحيح resource) {
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
  Raise_حد_بندي(RLIMIT_AS);
  // Heap size.
  Raise_حد_بندي(RLIMIT_DATA);
  // Stack size.
  Raise_حد_بندي(RLIMIT_STACK);
#جيڪڏهن_وصف_ٿيل RLIMIT_RSS
  // Resident set size.
  Raise_حد_بندي(RLIMIT_RSS);
#جيڪڏهن_پورو
#جيڪڏهن_پورو
}

فراغ CleanupStdHandles(فراغ *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

استعمال نطاق llvm;
استعمال نطاق llvm::sys;

InitLLVM::InitLLVM(صحيح &Argc, ثابت حرف **&Argv,
                   منطقي InstallPipeSignalExitHandler,
                   منطقي NeedsPOSIXUtilitySignalHandling) {
#جيڪڏهن_وصف_نه_ٿيل NDEBUG
  ساكن std::atomic<منطقي> Initialized{خطأ};
  أكد(!Initialized && "InitLLVM was already initialized!");
  Initialized = سچ;
#جيڪڏهن_پورو

  // Bring stdin/stdout/stderr into a known state.
#جيڪڏهن_وصف_ٿيل _WIN32
  sys::جوڙيو_Signal_Handler(CleanupStdHandles, خالي_اشارو);
#وإلا
  sys::جوڙيو_Signal_Handler(CleanupStdHandles, خالي_اشارو,
                        NeedsPOSIXUtilitySignalHandling);
#جيڪڏهن_پورو

  إذا (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::سيٽ_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  ڍير_Printer.emplace(Argc, Argv);
  sys::Print_ڍير_Trace_On_Error_Signal(Argv[0]);
  install_ٻاهريون_جو_يادگيري_new_handler();
  RaiseLimits();

#جيڪڏهن_وصف_ٿيل __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_ڪوڊ_منزل_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_ڪوڊ_منزل_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل _WIN32
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

  ExitOnErr(error_ڪوڊ_منزل_Error(windows::Get_Command_سٽ_Arguments(دليلون, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  دليلون.ڌڪو_پٺ(خالي_اشارو);

  Argc = دليلون.سائيز_2() - 1;
  Argv = دليلون.معلومات();
#جيڪڏهن_پورو
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(خالي_اشارو);
  llvm_shutdown();
}
