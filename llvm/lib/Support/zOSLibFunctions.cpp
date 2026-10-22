//===-- zOSLibFunctions.cpp -----------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//
// This file defines z/OS implementations for common functions.
//
//===----------------------------------------------------------------------===//

#अगर_घोषित __MVS__
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <sys/resource.h>
#समावेश <sys/wait.h>

स्थिर अक्षर *signalName[] = {
    /*  0 */ nullptr,
    /*  1 */ "Hangup",                   // SIGHUP
    /*  2 */ "Interrupt",                // SIGINT
    /*  3 */ "Aborted",                  // SIGABRT
    /*  4 */ "Illegal instruction",      // SIGILL
    /*  5 */ "Polling event",            // SIGPOLL
    /*  6 */ "Socket data available",    // SIGURG
    /*  7 */ "Stopped (signal)",         // SIGSTOP
    /*  8 */ "Floating point exception", // SIGFPE
    /*  9 */ "Killed",                   // SIGKILL
    /* 10 */ "Bus error",                // SIGBUS
    /* 11 */ "Segmentation fault",       // SIGSEGV
    /* 12 */ "Bad system call",          // SIGSYS
    /* 13 */ "Broken pipe",              // SIGPIPE
    /* 14 */ "Alarm clock",              // SIGALRM
    /* 15 */ "Terminated",               // SIGTERM
    /* 16 */ "User defined signal 1",    // SIGUSR1
    /* 17 */ "User defined signal 2",    // SIGUSR2
    /* 18 */ "Abend",                    // SIGABND
    /* 19 */ "Continued",                // SIGCONT
    /* 20 */ "Child exited",             // SIGCHLD
    /* 21 */ "Stopped (tty input)",      // SIGTTIN
    /* 22 */ "Stopped (tty output)",     // SIGTTOU
    /* 23 */ "I/O complete",             // SIGIO
    /* 24 */ "Quit",                     // SIGQUIT
    /* 25 */ "Stopped",                  // SIGTSTP
    /* 26 */ "Trace/breakpoint trap",    // SIGTRAP
    /* 27 */ "I/O error",                // SIGIOERR
    /* 28 */ "Window changed",           // SIGWINCH
    /* 29 */ "CPU time limit exceeded",  // SIGXCPU
    /* 30 */ "File size limit exceeded", // SIGXFSZ
    /* 31 */ "Virtual timer expired",    // SIGVTALRM
    /* 32 */ "Profiling timer expired",  // SIGPROF
    /* 33 */ "OMVS subsystem shutdown",  // SIGDANGER
    /* 34 */ "Thread stop",              // SIGTHSTOP
    /* 35 */ "Thread resume",            // SIGTHCONT
    /* 36 */ nullptr,                    // n/a
    /* 37 */ "Toggle syscall trace",     // SIGTRACE
    /* 38 */ nullptr,                    // SIGDCE
    /* 39 */ "System dump",              // SIGDUMP
};

// z/OS Unix System Services does not have strsignal() support, so the
// strsignal() function is implemented here.
अक्षर *strsignal(पूर्णांक sig) {
  अगर (static_cast<माप_प्रकार>(sig) < (माप(signalName) / माप(signalName[0])) &&
      signalName[sig])
    वापस const_cast<अक्षर *>(signalName[sig]);
  अटल अक्षर संदेश[256];
  snprintf(संदेश, माप(संदेश), "Unknown signal %d", sig);
  // Ensure the message is NUL-terminated, just in case the NUL terminator
  // was overwritten due to a race condition on the static buffer.
  संदेश[माप(संदेश) - 1] = '\0';
  वापस संदेश;
}

// z/OS Unix System Services does not have strnlen() support, so the strnlen()
// function is implemented here.
माप_प्रकार strnlen(स्थिर अक्षर *S, माप_प्रकार MaxLen) {
  स्थिर अक्षर *PtrToNullChar =
      static_cast<स्थिर अक्षर *>(स_प्रथम(S, '\0', MaxLen));
  वापस PtrToNullChar ? PtrToNullChar - S : MaxLen;
}
#पूर्ण_अगर
