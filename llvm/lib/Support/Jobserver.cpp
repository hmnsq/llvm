//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/Support/Jobserver.h"
#ضمن "llvm/ADT/StringExtras.h"
#ضمن "llvm/Support/Error.h"

#ضمن <atomic>
#ضمن <memory>
#ضمن <mutex>
#ضمن <new>

#define DEBUG_TYPE "jobserver"

using نطاق llvm;

نطاق {
بنية FdPair {
  صحيح Read = -1;
  صحيح Write = -1;
  منطقي isValid() ثابت { החזר Read >= 0 && Write >= 0; }
};

بنية JobserverConfig {
  تعداد Mode {
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
منطقي getPrefixedValue(StringRef Input, StringRef Prefix, StringRef &Value) {
  אם (Input.consume_front(Prefix)) {
    Value = Input;
    החזר אמת;
  }
  החזר שקר;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ساكن std::optional<FdPair> getFileDescriptorPair(StringRef Input) {
  FdPair FDs;
  אם (Input.consumeInteger(10, FDs.Read))
    החזר std::nullopt;
  אם (!Input.consume_front(","))
    החזר std::nullopt;
  אם (Input.consumeInteger(10, FDs.Write))
    החזר std::nullopt;
  אם (!Input.empty() || !FDs.isValid())
    החזר std::nullopt;
  החזר FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  אם (MakeFlags.empty())
    החזר Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  אם (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    החזר Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  עבור (StringRef Arg : Args) {
    StringRef Value;
    אם (getPrefixedValue(Arg, "--jobserver-auth=", Value)) {
      // Try to parse as a file descriptor pair first.
      אם (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } אחרת {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        אם (getPrefixedValue(Value, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } אחרת {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = Value.str();
        }
      }
    } אחרת אם (getPrefixedValue(Arg, "--jobserver-fds=", Value)) {
      // This is an alternative, older syntax for the pipe-based server.
      אם (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } אחרת {
        החזר createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  אם (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    החזר createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#אחרת
  אם (Config.TheMode == JobserverConfig::Win32Semaphore)
    החזר createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  החזר Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

نطاق llvm {
מחלקה JobserverClientImpl : عام JobserverClient {
  منطقي IsInitialized = שקר;
  std::atomic<منطقي> HasImplicitSlot{אמת};
  unsigned NumJobs = 0;

عام:
  JobserverClientImpl(ثابت JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  فراغ release(JobSlot Slot) override;
  unsigned getNumJobs() ثابت override { החזר NumJobs; }

  منطقي isValid() ثابت { החזר IsInitialized; }

خاص:
#אם defined(LLVM_ON_UNIX)
  صحيح ReadFD = -1;
  صحيح WriteFD = -1;
  std::string FifoPath;
#وإلاإذا defined(_WIN32)
  فراغ *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#אם defined(LLVM_ON_UNIX)
#ضمن "Unix/Jobserver.inc"
#وإلاإذا defined(_WIN32)
#ضمن "Windows/Jobserver.inc"
#אחרת
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ثابت JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = افتراضي;
JobSlot JobserverClientImpl::tryAcquire() { החזר JobSlot(); }
فراغ JobserverClientImpl::release(JobSlot Slot) {}
#endif

نطاق llvm {
JobserverClient::~JobserverClient() = افتراضي;

uint8_t JobSlot::getExplicitValue() ثابت {
  أكد(isExplicit() && "Cannot get value of implicit or invalid slot");
  החזר static_cast<uint8_t>(Value);
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
    ثابت حرف *MakeFlagsEnv = getenv("MAKEFLAGS");
    אם (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      החזר;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    אם (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      החזר;
    }

    JobserverConfig Config = *ConfigOrErr;
    אם (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      החזר;
    }

    אם (Config.TheMode == JobserverConfig::PosixPipe) {
#אם defined(LLVM_ON_UNIX)
      אם (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        החזר;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    אם (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } אחרת
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  החזר GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
فراغ JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  جديد (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
