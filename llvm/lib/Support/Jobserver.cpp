//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/Support/Jobserver.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringExtras.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Error.h"

#ಅನ್ತರ್ಭಾವ <atomic>
#ಅನ್ತರ್ಭಾವ <memory>
#ಅನ್ತರ್ಭಾವ <mutex>
#ಅನ್ತರ್ಭಾವ <new>

#define DEBUG_TYPE "jobserver"

using ನಾಮಾಕಾಶ llvm;

ನಾಮಾಕಾಶ {
ಸಂರಚನಾ FdPair {
  ಪೂರ್ಣಾಙ್ಕ Read = -1;
  ಪೂರ್ಣಾಙ್ಕ Write = -1;
  ತಾರ್ಕಿಕ isValid() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Read >= 0 && Write >= 0; }
};

ಸಂರಚನಾ JobserverConfig {
  ಗಣನಾ Mode {
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
ತಾರ್ಕಿಕ get_Prefixed_ಮೌಲ್ಯ(StringRef ಒಳಹರಿವು, StringRef Prefix, StringRef &ಮೌಲ್ಯ) {
  ಯದಿ (ಒಳಹರಿವು.consume_front(Prefix)) {
    ಮೌಲ್ಯ = ಒಳಹರಿವು;
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }
  ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ಸ್ಥಿರ std::optional<FdPair> getFileDescriptorPair(StringRef ಒಳಹರಿವು) {
  FdPair FDs;
  ಯದಿ (ಒಳಹರಿವು.consumeInteger(10, FDs.Read))
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಯದಿ (!ಒಳಹರಿವು.consume_front(","))
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಯದಿ (ಒಳಹರಿವು.consumeInteger(10, FDs.Write))
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಯದಿ (!ಒಳಹರಿವು.empty() || !FDs.isValid())
    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  ಪ್ರತ್ಯಾಗಮ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ಯದಿ (MakeFlags.empty())
    ಪ್ರತ್ಯಾಗಮ Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ಯದಿ (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    ಪ್ರತ್ಯಾಗಮ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  ಪ್ರತಿ (StringRef Arg : Args) {
    StringRef ಮೌಲ್ಯ;
    ಯದಿ (get_Prefixed_ಮೌಲ್ಯ(Arg, "--jobserver-auth=", ಮೌಲ್ಯ)) {
      // Try to parse as a file descriptor pair first.
      ಯದಿ (auto FDPair = getFileDescriptorPair(ಮೌಲ್ಯ)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ಅನ್ಯಥಾ {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        ಯದಿ (get_Prefixed_ಮೌಲ್ಯ(ಮೌಲ್ಯ, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } ಅನ್ಯಥಾ {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = ಮೌಲ್ಯ.str();
        }
      }
    } ಅನ್ಯಥಾ ಯದಿ (get_Prefixed_ಮೌಲ್ಯ(Arg, "--jobserver-fds=", ಮೌಲ್ಯ)) {
      // This is an alternative, older syntax for the pipe-based server.
      ಯದಿ (auto FDPair = getFileDescriptorPair(ಮೌಲ್ಯ)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ಅನ್ಯಥಾ {
        ಪ್ರತ್ಯಾಗಮ createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  ಯದಿ (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    ಪ್ರತ್ಯಾಗಮ createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#ಅನ್ಯಥಾ
  ಯದಿ (Config.TheMode == JobserverConfig::Win32Semaphore)
    ಪ್ರತ್ಯಾಗಮ createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  ಪ್ರತ್ಯಾಗಮ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

ನಾಮಾಕಾಶ llvm {
ವರ್ಗ JobserverClientImpl : ಸಾರ್ವಜನಿಕ JobserverClient {
  ತಾರ್ಕಿಕ IsInitialized = ಅಸತ್ಯ;
  std::atomic<ತಾರ್ಕಿಕ> HasImplicitSlot{ಸತ್ಯ};
  unsigned ಅಂಕೆ_Jobs = 0;

ಸಾರ್ವಜನಿಕ:
  JobserverClientImpl(ಧ್ರುವ JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  ಶೂನ್ಯ release(JobSlot Slot) override;
  unsigned get_ಅಂಕೆ_Jobs() ಧ್ರುವ override { ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ_Jobs; }

  ತಾರ್ಕಿಕ isValid() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ IsInitialized; }

ಗುಪ್ತ:
#ಯದಿ defined(LLVM_ON_UNIX)
  ಪೂರ್ಣಾಙ್ಕ ReadFD = -1;
  ಪೂರ್ಣಾಙ್ಕ WriteFD = -1;
  std::string FifoPath;
#ನೋಚೇದ್ಯದಿ defined(_WIN32)
  ಶೂನ್ಯ *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ಯದಿ defined(LLVM_ON_UNIX)
#ಅನ್ತರ್ಭಾವ "Unix/Jobserver.inc"
#ನೋಚೇದ್ಯದಿ defined(_WIN32)
#ಅನ್ತರ್ಭಾವ "Windows/Jobserver.inc"
#ಅನ್ಯಥಾ
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ಧ್ರುವ JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = ಸ್ವಭಾವ;
JobSlot JobserverClientImpl::tryAcquire() { ಪ್ರತ್ಯಾಗಮ JobSlot(); }
ಶೂನ್ಯ JobserverClientImpl::release(JobSlot Slot) {}
#endif

ನಾಮಾಕಾಶ llvm {
JobserverClient::~JobserverClient() = ಸ್ವಭಾವ;

uint8_t JobSlot::get_Explicit_ಮೌಲ್ಯ() ಧ್ರುವ {
  ಪ್ರತಿಜ್ಞಾ(isExplicit() && "Cannot get value of implicit or invalid slot");
  ಪ್ರತ್ಯಾಗಮ static_cast<uint8_t>(ಮೌಲ್ಯ);
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
    ಧ್ರುವ ವರ್ಣ *MakeFlagsEnv = getenv("MAKEFLAGS");
    ಯದಿ (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      ಪ್ರತ್ಯಾಗಮ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ಯದಿ (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      ಪ್ರತ್ಯಾಗಮ;
    }

    JobserverConfig Config = *ConfigOrErr;
    ಯದಿ (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      ಪ್ರತ್ಯಾಗಮ;
    }

    ಯದಿ (Config.TheMode == JobserverConfig::PosixPipe) {
#ಯದಿ defined(LLVM_ON_UNIX)
      ಯದಿ (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        ಪ್ರತ್ಯಾಗಮ;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    ಯದಿ (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } ಅನ್ಯಥಾ
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  ಪ್ರತ್ಯಾಗಮ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
ಶೂನ್ಯ JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  ನೂತನ (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
