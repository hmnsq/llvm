//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/Support/Jobserver.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"
#अन्तर्भाव "llvm/Support/Error.h"

#अन्तर्भाव <atomic>
#अन्तर्भाव <memory>
#अन्तर्भाव <mutex>
#अन्तर्भाव <new>

#define DEBUG_TYPE "jobserver"

using नामाकाश llvm;

नामाकाश {
संरचना FdPair {
  पूर्णांक Read = -1;
  पूर्णांक Write = -1;
  तार्किक isValid() स्थिरांक { परत Read >= 0 && Write >= 0; }
};

संरचना JobserverConfig {
  गणना Mode {
    None,
    PosixFifo,
    PosixPipe,
    Win32Semaphore,
  };
  Mode TheMode = None;
  std::string Path;
  FdPair PipeFDs;
};

/// A helper function that checks if `Input` starts with `Prefix`.
/// If it does, it removes the prefix from `Input`, assigns the remainder to
/// `Value`, and returns true. Otherwise, it returns false.
तार्किक get_Prefixed_मूल्य(StringRef आदान, StringRef Prefix, StringRef &मूल्य) {
  जर (आदान.consume_front(Prefix)) {
    मूल्य = आदान;
    परत सत्य;
  }
  परत असत्य;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
स्थिर std::optional<FdPair> getFileDescriptorPair(StringRef आदान) {
  FdPair FDs;
  जर (आदान.consumeInteger(10, FDs.Read))
    परत std::nullopt;
  जर (!आदान.consume_front(","))
    परत std::nullopt;
  जर (आदान.consumeInteger(10, FDs.Write))
    परत std::nullopt;
  जर (!आदान.empty() || !FDs.isValid())
    परत std::nullopt;
  परत FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  जर (MakeFlags.empty())
    परत Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  जर (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    परत Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  साठी (StringRef Arg : Args) {
    StringRef मूल्य;
    जर (get_Prefixed_मूल्य(Arg, "--jobserver-auth=", मूल्य)) {
      // Try to parse as a file descriptor pair first.
      जर (स्वतः FDPair = getFileDescriptorPair(मूल्य)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        जर (get_Prefixed_मूल्य(मूल्य, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } अन्यथा {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = मूल्य.str();
        }
      }
    } अन्यथा जर (get_Prefixed_मूल्य(Arg, "--jobserver-fds=", मूल्य)) {
      // This is an alternative, older syntax for the pipe-based server.
      जर (स्वतः FDPair = getFileDescriptorPair(मूल्य)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        परत createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  जर (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    परत createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#अन्यथा
  जर (Config.TheMode == JobserverConfig::Win32Semaphore)
    परत createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  परत Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

नामाकाश llvm {
वर्ग JobserverClientImpl : सार्वजनिक JobserverClient {
  तार्किक IsInitialized = असत्य;
  std::atomic<तार्किक> HasImplicitSlot{सत्य};
  अचिन्हित अंक_Jobs = 0;

सार्वजनिक:
  JobserverClientImpl(स्थिरांक JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  शून्य release(JobSlot Slot) override;
  अचिन्हित get_अंक_Jobs() स्थिरांक override { परत अंक_Jobs; }

  तार्किक isValid() स्थिरांक { परत IsInitialized; }

खाजगी:
#जर defined(LLVM_ON_UNIX)
  पूर्णांक ReadFD = -1;
  पूर्णांक WriteFD = -1;
  std::string FifoPath;
#किंवाजर defined(_WIN32)
  शून्य *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#जर defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Jobserver.inc"
#किंवाजर defined(_WIN32)
#अन्तर्भाव "Windows/Jobserver.inc"
#अन्यथा
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(स्थिरांक JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = पूर्वनिर्धारित;
JobSlot JobserverClientImpl::tryAcquire() { परत JobSlot(); }
शून्य JobserverClientImpl::release(JobSlot Slot) {}
#endif

नामाकाश llvm {
JobserverClient::~JobserverClient() = पूर्वनिर्धारित;

uint8_t JobSlot::get_Explicit_मूल्य() स्थिरांक {
  प्रतिज्ञा(isExplicit() && "Cannot get value of implicit or invalid slot");
  परत static_cast<uint8_t>(मूल्य);
}

/// This is the main entry point for acquiring a jobserver client. It uses a
/// std::call_once to ensure the singleton `GJobserver` instance is created
/// safely in a multi-threaded environment. On first call, it reads the
/// `MAKEFLAGS` environment variable, parses it, and attempts to construct and
/// initialize a `JobserverClientImpl`. If successful, the global instance is
/// stored in `GJobserver`. Subsequent calls will return the existing instance.
JobserverClient *JobserverClient::getInstance() {
  std::call_once(GJobserverOnceFlag, []() {
    LLVM_DEBUG(
        dbgs()
        << "JobserverClient::getInstance() called for the first time.\n");
    स्थिरांक अक्षर *MakeFlagsEnv = दो_पर्या("MAKEFLAGS");
    जर (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      परत;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    स्वतः ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    जर (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      परत;
    }

    JobserverConfig Config = *ConfigOrErr;
    जर (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      परत;
    }

    जर (Config.TheMode == JobserverConfig::PosixPipe) {
#जर defined(LLVM_ON_UNIX)
      जर (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        परत;
      }
#endif
    }

    स्वतः Client = std::make_unique<JobserverClientImpl>(Config);
    जर (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } अन्यथा
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  परत GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
शून्य JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  नवीन (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
