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

#اگر_تعریف_شدہ __MVS__
#شامل <stdio.h>
#شامل <string.h>
#شامل <sys/resource.h>
#شامل <sys/wait.h>

ثابت حرف *signalName[] = {
    /*  0 */ خالی_اشاریہ,
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
    /* 36 */ خالی_اشاریہ,                    // n/a
    /* 37 */ "Toggle syscall trace",     // SIGTRACE
    /* 38 */ خالی_اشاریہ,                    // SIGDCE
    /* 39 */ "System dump",              // SIGDUMP
};

// z/OS Unix System Services does not have strsignal() support, so the
// strsignal() function is implemented here.
حرف *strsignal(صحیح sig) {
  اگر (static_cast<size_t>(sig) < (حجم(signalName) / حجم(signalName[0])) &&
      signalName[sig])
    واپس const_cast<حرف *>(signalName[sig]);
  ساکن حرف پیغام[256];
  snprintf(پیغام, حجم(پیغام), "Unknown signal %d", sig);
  // Ensure the message is NUL-terminated, just in case the NUL terminator
  // was overwritten due to a race condition on the static buffer.
  پیغام[حجم(پیغام) - 1] = '\0';
  واپس پیغام;
}

// z/OS Unix System Services does not have strnlen() support, so the strnlen()
// function is implemented here.
size_t strnlen(ثابت حرف *S, size_t MaxLen) {
  ثابت حرف *Ptr_مقصد_Null_Char =
      static_cast<ثابت حرف *>(memchr(S, '\0', MaxLen));
  واپس Ptr_مقصد_Null_Char ? Ptr_مقصد_Null_Char - S : MaxLen;
}
#اگر_ختم
