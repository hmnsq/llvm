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
  तार्किक isValid() ध्रुव { प्रत्यागम Read >= 0 && Write >= 0; }
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
  यदि (आदान.consume_front(Prefix)) {
    मूल्य = आदान;
    प्रत्यागम सत्य;
  }
  प्रत्यागम असत्य;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
स्थिर std::optional<FdPair> getFileDescriptorPair(StringRef आदान) {
  FdPair FDs;
  यदि (आदान.consumeInteger(10, FDs.Read))
    प्रत्यागम std::nullopt;
  यदि (!आदान.consume_front(","))
    प्रत्यागम std::nullopt;
  यदि (आदान.consumeInteger(10, FDs.Write))
    प्रत्यागम std::nullopt;
  यदि (!आदान.empty() || !FDs.isValid())
    प्रत्यागम std::nullopt;
  प्रत्यागम FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  यदि (MakeFlags.empty())
    प्रत्यागम Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  यदि (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    प्रत्यागम Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  प्रति (StringRef Arg : Args) {
    StringRef मूल्य;
    यदि (get_Prefixed_मूल्य(Arg, "--jobserver-auth=", मूल्य)) {
      // Try to parse as a file descriptor pair first.
      यदि (auto FDPair = getFileDescriptorPair(मूल्य)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        यदि (get_Prefixed_मूल्य(मूल्य, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } अन्यथा {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = मूल्य.str();
        }
      }
    } अन्यथा यदि (get_Prefixed_मूल्य(Arg, "--jobserver-fds=", मूल्य)) {
      // This is an alternative, older syntax for the pipe-based server.
      यदि (auto FDPair = getFileDescriptorPair(मूल्य)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } अन्यथा {
        प्रत्यागम createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  यदि (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    प्रत्यागम createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#अन्यथा
  यदि (Config.TheMode == JobserverConfig::Win32Semaphore)
    प्रत्यागम createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  प्रत्यागम Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

नामाकाश llvm {
वर्ग JobserverClientImpl : सार्वजनिक JobserverClient {
  तार्किक IsInitialized = असत्य;
  std::atomic<तार्किक> HasImplicitSlot{सत्य};
  unsigned अंक_Jobs = 0;

सार्वजनिक:
  JobserverClientImpl(ध्रुव JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  शून्य release(JobSlot Slot) override;
  unsigned get_अंक_Jobs() ध्रुव override { प्रत्यागम अंक_Jobs; }

  तार्किक isValid() ध्रुव { प्रत्यागम IsInitialized; }

गुप्त:
#यदि defined(LLVM_ON_UNIX)
  पूर्णाङ्क ReadFD = -1;
  पूर्णाङ्क WriteFD = -1;
  std::string FifoPath;
#नोचेद्यदि defined(_WIN32)
  शून्य *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#यदि defined(LLVM_ON_UNIX)
#अन्तर्भाव "Unix/Jobserver.inc"
#नोचेद्यदि defined(_WIN32)
#अन्तर्भाव "Windows/Jobserver.inc"
#अन्यथा
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ध्रुव JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = स्वभाव;
JobSlot JobserverClientImpl::tryAcquire() { प्रत्यागम JobSlot(); }
शून्य JobserverClientImpl::release(JobSlot Slot) {}
#endif

नामाकाश llvm {
JobserverClient::~JobserverClient() = स्वभाव;

uint8_t JobSlot::get_Explicit_मूल्य() ध्रुव {
  प्रतिज्ञा(isExplicit() && "Cannot get value of implicit or invalid slot");
  प्रत्यागम static_cast<uint8_t>(मूल्य);
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
    यदि (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      प्रत्यागम;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    यदि (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      प्रत्यागम;
    }

    JobserverConfig Config = *ConfigOrErr;
    यदि (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      प्रत्यागम;
    }

    यदि (Config.TheMode == JobserverConfig::PosixPipe) {
#यदि defined(LLVM_ON_UNIX)
      यदि (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        प्रत्यागम;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    यदि (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } अन्यथा
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  प्रत्यागम GJobserver;
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
