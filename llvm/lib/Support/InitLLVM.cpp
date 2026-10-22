//===-- InitLLVM.cpp -----------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/InitLLVM.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringRef.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/AutoConvert.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ErrorHandling.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/ManagedStatic.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Signals.h"

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
#ಅನ್ತರ್ಭಾವ "llvm/Support/Windows/WindowsSupport.h"
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(HAVE_UNISTD_H)
#ಅನ್ತರ್ಭಾವ <unistd.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDIN_FILENO
#ವ್ಯಾಖ್ಯಾನಿಸು STDIN_FILENO 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDOUT_FILENO
#ವ್ಯಾಖ್ಯಾನಿಸು STDOUT_FILENO 1
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ STDERR_FILENO
#ವ್ಯಾಖ್ಯಾನಿಸು STDERR_FILENO 2
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ RaiseLimits() {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _AIX
  // AIX has restrictive memory soft-limits out-of-box, so raise them if needed.
  ಸ್ವಯಂಚಾಲಿತ Raise_ಎಲ್ಲೆ = [](ಪೂರ್ಣಾಂಕ resource) {
    ರಚನೆ rlimit r;
    getrlimit(resource, &r);

    // Increase the soft limit to the hard limit, if necessary and
    // possible.
    ಆದರೆ (r.rlim_cur != RLIM_INFINITY && r.rlim_cur != r.rlim_max) {
      r.rlim_cur = r.rlim_max;
      setrlimit(resource, &r);
    }
  };

  // Address space size.
  Raise_ಎಲ್ಲೆ(RLIMIT_AS);
  // Heap size.
  Raise_ಎಲ್ಲೆ(RLIMIT_DATA);
  // Stack size.
  Raise_ಎಲ್ಲೆ(RLIMIT_STACK);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ RLIMIT_RSS
  // Resident set size.
  Raise_ಎಲ್ಲೆ(RLIMIT_RSS);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ CleanupStdHandles(ಶೂನ್ಯ *Cookie) {
  llvm::raw_ostream *Outs = &llvm::outs(), *Errs = &llvm::errs();
  Outs->flush();
  Errs->flush();
  llvm::restoreStdHandleAutoConversion(STDIN_FILENO);
  llvm::restoreStdHandleAutoConversion(STDOUT_FILENO);
  llvm::restoreStdHandleAutoConversion(STDERR_FILENO);
}

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::sys;

InitLLVM::InitLLVM(ಪೂರ್ಣಾಂಕ &Argc, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **&Argv,
                   ತಾರ್ಕಿಕ InstallPipeSignalExitHandler,
                   ತಾರ್ಕಿಕ NeedsPOSIXUtilitySignalHandling) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ NDEBUG
  ಸ್ಥಿರ std::atomic<ತಾರ್ಕಿಕ> Initialized{ಸುಳ್ಳು};
  ಪ್ರತಿಜ್ಞಾ(!Initialized && "InitLLVM was already initialized!");
  Initialized = ನಿಜ;
#ಆದರೆ_ಅಂತ್ಯ

  // Bring stdin/stdout/stderr into a known state.
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
  sys::ಕೂಡಿಸು_Signal_Handler(CleanupStdHandles, ಶೂನ್ಯಸೂಚಕ);
#ಇಲ್ಲದಿದ್ದರೆ
  sys::ಕೂಡಿಸು_Signal_Handler(CleanupStdHandles, ಶೂನ್ಯಸೂಚಕ,
                        NeedsPOSIXUtilitySignalHandling);
#ಆದರೆ_ಅಂತ್ಯ

  ಆದರೆ (InstallPipeSignalExitHandler)
    // The pipe signal handler must be installed before any other handlers are
    // registered. This is because the Unix \ref RegisterHandlers function does
    // not perform a sigaction() for SIGPIPE unless a one-shot handler is
    // present, to allow long-lived processes (like lldb) to fully opt-out of
    // llvm's SIGPIPE handling and ignore the signal safely.
    sys::ಗಣ_One_Shot_Pipe_Signal_Function(sys::DefaultOneShotPipeSignalHandler);
  // Initialize the stack printer after installing the one-shot pipe signal
  // handler, so we can perform a sigaction() for SIGPIPE on Unix if requested.
  ರಾಶಿ_Printer.emplace(Argc, Argv);
  sys::Print_ರಾಶಿ_Trace_On_Error_Signal(Argv[0]);
  install_ಹೊರಹೋಗು_ದ_ಸ್ಮರಣೆ_new_handler();
  RaiseLimits();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __MVS__

  // We use UTF-8 as the internal character encoding. On z/OS, all external
  // output is encoded in EBCDIC. In order to be able to read all
  // error messages, we turn conversion to EBCDIC on for stderr fd.
  std::string Banner = std::string(Argv[0]) + ": ";
  ExitOnError ExitOnErr(Banner);

  // If turning on conversion for stderr fails then the error message
  // may be garbled. There is no solution to this problem.
  ExitOnErr(error_ಸಂಕೇತ_ಗಮ್ಯ_Error(llvm::enableAutoConversion(STDERR_FILENO)));
  ExitOnErr(error_ಸಂಕೇತ_ಗಮ್ಯ_Error(llvm::enableAutoConversion(STDOUT_FILENO)));
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ _WIN32
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

  ExitOnErr(error_ಸಂಕೇತ_ಗಮ್ಯ_Error(windows::Get_Command_ಸಾಲು_Arguments(ವಾದಗಳು, Alloc)));

  // GetCommandLineArguments doesn't terminate the vector with a
  // nullptr.  Do it to make it compatible with the real argv.
  ವಾದಗಳು.ತಳ್ಳು_ಹಿಂಭಾಗ(ಶೂನ್ಯಸೂಚಕ);

  Argc = ವಾದಗಳು.ಗಾತ್ರ_3() - 1;
  Argv = ವಾದಗಳು.ದತ್ತಾಂಶ();
#ಆದರೆ_ಅಂತ್ಯ
}

InitLLVM::~InitLLVM() {
  CleanupStdHandles(ಶೂನ್ಯಸೂಚಕ);
  llvm_shutdown();
}
