//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/Support/Jobserver.h"
#شامل "llvm/ADT/StringExtras.h"
#شامل "llvm/Support/Error.h"

#شامل <atomic>
#شامل <memory>
#شامل <mutex>
#شامل <new>

#define DEBUG_TYPE "jobserver"

using فضاینام llvm;

فضاینام {
ساختار FdPair {
  صحیح Read = -1;
  صحیح Write = -1;
  منطقی isValid() ثابت { بازگشت Read >= 0 && Write >= 0; }
};

ساختار JobserverConfig {
  شمارش Mode {
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
منطقی get_Prefixed_مقدار(StringRef درون_داد, StringRef Prefix, StringRef &مقدار) {
  اگر (درون_داد.consume_front(Prefix)) {
    مقدار = درون_داد;
    بازگشت درست;
  }
  بازگشت نادرست;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ایستا std::optional<FdPair> getFileDescriptorPair(StringRef درون_داد) {
  FdPair FDs;
  اگر (درون_داد.consumeInteger(10, FDs.Read))
    بازگشت std::nullopt;
  اگر (!درون_داد.consume_front(","))
    بازگشت std::nullopt;
  اگر (درون_داد.consumeInteger(10, FDs.Write))
    بازگشت std::nullopt;
  اگر (!درون_داد.empty() || !FDs.isValid())
    بازگشت std::nullopt;
  بازگشت FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  اگر (MakeFlags.empty())
    بازگشت Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  اگر (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    بازگشت Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  برای (StringRef Arg : Args) {
    StringRef مقدار;
    اگر (get_Prefixed_مقدار(Arg, "--jobserver-auth=", مقدار)) {
      // Try to parse as a file descriptor pair first.
      اگر (auto FDPair = getFileDescriptorPair(مقدار)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وگرنه {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        اگر (get_Prefixed_مقدار(مقدار, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } وگرنه {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = مقدار.str();
        }
      }
    } وگرنه اگر (get_Prefixed_مقدار(Arg, "--jobserver-fds=", مقدار)) {
      // This is an alternative, older syntax for the pipe-based server.
      اگر (auto FDPair = getFileDescriptorPair(مقدار)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وگرنه {
        بازگشت createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  اگر (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    بازگشت createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#وگرنه
  اگر (Config.TheMode == JobserverConfig::Win32Semaphore)
    بازگشت createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  بازگشت Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

فضاینام llvm {
رده JobserverClientImpl : عمومی JobserverClient {
  منطقی IsInitialized = نادرست;
  std::atomic<منطقی> HasImplicitSlot{درست};
  unsigned رقم_Jobs = 0;

عمومی:
  JobserverClientImpl(ثابت JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  تهی release(JobSlot Slot) override;
  unsigned get_رقم_Jobs() ثابت override { بازگشت رقم_Jobs; }

  منطقی isValid() ثابت { بازگشت IsInitialized; }

خصوصی:
#اگر defined(LLVM_ON_UNIX)
  صحیح ReadFD = -1;
  صحیح WriteFD = -1;
  std::string FifoPath;
#وگرنهاگر defined(_WIN32)
  تهی *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#اگر defined(LLVM_ON_UNIX)
#شامل "Unix/Jobserver.inc"
#وگرنهاگر defined(_WIN32)
#شامل "Windows/Jobserver.inc"
#وگرنه
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ثابت JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = پیشفرض;
JobSlot JobserverClientImpl::tryAcquire() { بازگشت JobSlot(); }
تهی JobserverClientImpl::release(JobSlot Slot) {}
#endif

فضاینام llvm {
JobserverClient::~JobserverClient() = پیشفرض;

uint8_t JobSlot::get_Explicit_مقدار() ثابت {
  أكد(isExplicit() && "Cannot get value of implicit or invalid slot");
  بازگشت static_cast<uint8_t>(مقدار);
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
    ثابت نویسه *MakeFlagsEnv = getenv("MAKEFLAGS");
    اگر (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      بازگشت;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    اگر (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      بازگشت;
    }

    JobserverConfig Config = *ConfigOrErr;
    اگر (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      بازگشت;
    }

    اگر (Config.TheMode == JobserverConfig::PosixPipe) {
#اگر defined(LLVM_ON_UNIX)
      اگر (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        بازگشت;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    اگر (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } وگرنه
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  بازگشت GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
تهی JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  جدید (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
