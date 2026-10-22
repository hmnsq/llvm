//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/InitLLVM.h"
#अन्तर्भाव "llvm/ADT/StringRef.h"
#अन्तर्भाव "llvm/Support/AutoConvert.h"
#अन्तर्भाव "llvm/Support/Error.h"
#अन्तर्भाव "llvm/Support/ErrorHandling.h"
#अन्तर्भाव "llvm/Support/ManagedStatic.h"
#अन्तर्भाव "llvm/Support/Signals.h"

#यदि_परिभाषितम् _WIN32
#अन्तर्भाव "llvm/Support/Windows/WindowsSupport.h"
#यदि_अन्तः

#यदि defined(HAVE_UNISTD_H)
#अन्तर्भाव <unistd.h>
#अन्यथा
#यदि_अपरिभाषितम् STDIN_FILENO
#परिभाषय STDIN_FILENO 0
#यदि_अन्तः
#यदि_अपरिभाषितम् STDOUT_FILENO
#परिभाषय STDOUT_FILENO 1
#यदि_अन्तः
#यदि_अपरिभाषितम् STDERR_FILENO
#परिभाषय STDERR_FILENO 2
#यदि_अन्तः
#यदि_अन्तः

स्थिर शून्य RaiseLimits() {
#यदि_परिभाषितम् _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  स्वचालित Raise_सीमा = [](पूर्णाङ्क resource) {
    संरचना rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    यदि (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  Raise_सीमा(RLIMIT_AS);
  // Heap size.
  Raise_सीमा(RLIMIT_DATA);
  // Stack size.
  Raise_सीमा(RLIMIT_STACK);
#यदि_परिभाषितम् RLIMIT_RSS
  // Resident set size.
  Raise_सीमा(RLIMIT_RSS);
#यदि_अन्तः
#यदि_अन्तः
}

शून्य CleanupStdHandles(शून्य *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::sys;

InitLLVM::InitLLVM(पूर्णाङ्क &Argc, ध्रुव वर्ण **&Argv,
                   तार्किक InstallPipeSignalExitHandler,
                   तार्किक NeedsPOSIXUtilitySignalHandling) {
#यदि_अपरिभाषितम् NDEBUG
  स्थिर std::atomic<तार्किक> Initialized{असत्य};
  प्रतिज्ञा(!Initialized && "InitLLVM was already initialized!");
  Initialized = सत्य;
#यदि_अन्तः

  // Bring stdin/stdout/stderr into a known state.
#यदि_परिभाषितम् _WIN32
  sys::सङ्कलय_Signal_Handler(CleanupStdHandles, शून्यसूचकः);
#अन्यथा
  sys::सङ्कलय_Signal_Handler(CleanupStdHandles, शून्यसूचकः,
                        NeedsPOSIXUtilitySignalHandling);
#यदि_अन्तः

  यदि (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::समुच्चयः_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  राशिः_Printer.emplace(Argc, Argv);
  sys::Print_राशिः_Trace_On_Error_Signal(Argv[0]);
  install_बहिर्गमनम्_सम्बन्धि_स्मृतिः_new_handler();
  RaiseLimits();

#यदि_परिभाषितम् __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_संहिता_गन्तव्यम्_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_संहिता_गन्तव्यम्_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#यदि_अन्तः

#यदि_परिभाषितम् _WIN32
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

  ExitOnErr(error_संहिता_गन्तव्यम्_Error(windows::Get_Command_पङ्क्तिः_Arguments(तर्काः, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  तर्काः.प्रक्षिप_पृष्ठभागः(शून्यसूचकः);

  Argc = तर्काः.आकार_2() - 1;
  Argv = तर्काः.दत्त();
#यदि_अन्तः
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(शून्यसूचकः);
  llvm_shutdown();
}
