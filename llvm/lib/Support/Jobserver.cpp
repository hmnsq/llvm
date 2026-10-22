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
  पूर्णाङ्क Read = -1;
  पूर्णाङ्क Write = -1;
  तार्किक isValid() ध्रुव { पटिनिवत्त Read >= 0 && Write >= 0; }
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
तार्किक get_Prefixed_मानम्(StringRef निवेशः, StringRef Prefix, StringRef &मानम्) {
  सचे (निवेशः.consume_front(Prefix)) {
    मानम् = निवेशः;
    पटिनिवत्त सच्च;
  }
  पटिनिवत्त मिच्छा;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
स्थिर std::optional<FdPair> getFileDescriptorPair(StringRef निवेशः) {
  FdPair FDs;
  सचे (निवेशः.consumeInteger(10, FDs.Read))
    पटिनिवत्त std::nullopt;
  सचे (!निवेशः.consume_front(","))
    पटिनिवत्त std::nullopt;
  सचे (निवेशः.consumeInteger(10, FDs.Write))
    पटिनिवत्त std::nullopt;
  सचे (!निवेशः.empty() || !FDs.isValid())
    पटिनिवत्त std::nullopt;
  पटिनिवत्त FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  सचे (MakeFlags.empty())
    पटिनिवत्त Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  सचे (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    पटिनिवत्त Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  अत्थाय (StringRef Arg : Args) {
    StringRef मानम्;
    सचे (get_Prefixed_मानम्(Arg, "--jobserver-auth=", मानम्)) {
      // Try to parse as a file descriptor pair first.
      सचे (auto FDPair = getFileDescriptorPair(मानम्)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अञ्ञथा {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        सचे (get_Prefixed_मानम्(मानम्, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } अञ्ञथा {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = मानम्.str();
        }
      }
    } अञ्ञथा सचे (get_Prefixed_मानम्(Arg, "--jobserver-fds=", मानम्)) {
      // This is an alternative, older syntax for the pipe-based server.
      सचे (auto FDPair = getFileDescriptorPair(मानम्)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अञ्ञथा {
        पटिनिवत्त createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  सचे (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    पटिनिवत्त createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#अञ्ञथा
  सचे (Config.TheMode == JobserverConfig::Win32Semaphore)
    पटिनिवत्त createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  पटिनिवत्त Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

नामाकाश llvm {
वग्ग JobserverClientImpl : सार्वजनिक JobserverClient {
  तार्किक IsInitialized = मिच्छा;
  std::atomic<तार्किक> HasImplicitSlot{सच्च};
  unsigned अङ्कः_Jobs = 0;

सार्वजनिक:
  JobserverClientImpl(ध्रुव JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  शून्य release(JobSlot Slot) override;
  unsigned get_अङ्कः_Jobs() ध्रुव override { पटिनिवत्त अङ्कः_Jobs; }

  तार्किक isValid() ध्रुव { पटिनिवत्त IsInitialized; }

गुप्त:
#सचे defined(LLVM_ON_UNIX)
  पूर्णाङ्क ReadFD = -1;
  पूर्णाङ्क WriteFD = -1;
  std::string FifoPath;
#नोचेद्यदि defined(_WIN32)
  शून्य *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#सचे defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Jobserver.inc"
#नोचेद्यदि defined(_WIN32)
#अन्तर्भाव "Windows/Jobserver.inc"
#अञ्ञथा
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ध्रुव JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = स्वभाव;
JobSlot JobserverClientImpl::tryAcquire() { पटिनिवत्त JobSlot(); }
शून्य JobserverClientImpl::release(JobSlot Slot) {}
#endif

नामाकाश llvm {
JobserverClient::~JobserverClient() = स्वभाव;

uint8_t JobSlot::get_Explicit_मानम्() ध्रुव {
  प्रतिज्ञा(isExplicit() && "Cannot get value of implicit or invalid slot");
  पटिनिवत्त static_cast<uint8_t>(मानम्);
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
    ध्रुव वर्ण *MakeFlagsEnv = getenv("MAKEFLAGS");
    सचे (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      पटिनिवत्त;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    सचे (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      पटिनिवत्त;
    }

    JobserverConfig Config = *ConfigOrErr;
    सचे (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      पटिनिवत्त;
    }

    सचे (Config.TheMode == JobserverConfig::PosixPipe) {
#सचे defined(LLVM_ON_UNIX)
      सचे (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        पटिनिवत्त;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    सचे (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } अञ्ञथा
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  पटिनिवत्त GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
शून्य JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  नूतन (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
