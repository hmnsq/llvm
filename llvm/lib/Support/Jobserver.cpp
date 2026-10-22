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
  منطقي isValid() ثابت { أرجع Read >= 0 && Write >= 0; }
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
منطقي get_Prefixed_درج(StringRef شغرو, StringRef Prefix, StringRef &درج) {
  إذا (شغرو.consume_front(Prefix)) {
    درج = شغرو;
    أرجع true;
  }
  أرجع خطأ;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ساكن std::optional<FdPair> getFileDescriptorPair(StringRef شغرو) {
  FdPair FDs;
  إذا (شغرو.consumeInteger(10, FDs.Read))
    أرجع std::nullopt;
  إذا (!شغرو.consume_front(","))
    أرجع std::nullopt;
  إذا (شغرو.consumeInteger(10, FDs.Write))
    أرجع std::nullopt;
  إذا (!شغرو.empty() || !FDs.isValid())
    أرجع std::nullopt;
  أرجع FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  إذا (MakeFlags.empty())
    أرجع Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> حجوجي;
  SplitString(MakeFlags, حجوجي);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  إذا (!حجوجي.empty() && !حجوجي[0].starts_with("-") && حجوجي[0].contains('n'))
    أرجع Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  لكل (StringRef Arg : حجوجي) {
    StringRef درج;
    إذا (get_Prefixed_درج(Arg, "--jobserver-auth=", درج)) {
      // Try to parse as a file descriptor pair first.
      إذا (auto FDPair = getFileDescriptorPair(درج)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وإلا {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        إذا (get_Prefixed_درج(درج, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } وإلا {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = درج.str();
        }
      }
    } وإلا إذا (get_Prefixed_درج(Arg, "--jobserver-fds=", درج)) {
      // This is an alternative, older syntax for the pipe-based server.
      إذا (auto FDPair = getFileDescriptorPair(درج)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } وإلا {
        أرجع createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  إذا (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    أرجع createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#وإلا
  إذا (Config.TheMode == JobserverConfig::Win32Semaphore)
    أرجع createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  أرجع Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

نطاق llvm {
صنف JobserverClientImpl : عام JobserverClient {
  منطقي IsInitialized = خطأ;
  std::atomic<منطقي> HasImplicitSlot{true};
  unsigned لمب_Jobs = 0;

عام:
  JobserverClientImpl(ثابت JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  فراغ release(JobSlot Slot) override;
  unsigned get_لمب_Jobs() ثابت override { أرجع لمب_Jobs; }

  منطقي isValid() ثابت { أرجع IsInitialized; }

خاص:
#إذا defined(LLVM_ON_UNIX)
  صحيح ReadFD = -1;
  صحيح WriteFD = -1;
  std::string FifoPath;
#وإلاإذا defined(_WIN32)
  فراغ *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#إذا defined(LLVM_ON_UNIX)
#ضمن "Unix/Jobserver.inc"
#وإلاإذا defined(_WIN32)
#ضمن "Windows/Jobserver.inc"
#وإلا
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ثابت JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = افتراضي;
JobSlot JobserverClientImpl::tryAcquire() { أرجع JobSlot(); }
فراغ JobserverClientImpl::release(JobSlot Slot) {}
#endif

نطاق llvm {
JobserverClient::~JobserverClient() = افتراضي;

uint8_t JobSlot::get_Explicit_درج() ثابت {
  أكد(isExplicit() && "Cannot get value of implicit or invalid slot");
  أرجع static_cast<uint8_t>(درج);
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
    إذا (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      أرجع;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    إذا (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      أرجع;
    }

    JobserverConfig Config = *ConfigOrErr;
    إذا (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      أرجع;
    }

    إذا (Config.TheMode == JobserverConfig::PosixPipe) {
#إذا defined(LLVM_ON_UNIX)
      إذا (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        أرجع;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    إذا (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } وإلا
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  أرجع GJobserver;
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
