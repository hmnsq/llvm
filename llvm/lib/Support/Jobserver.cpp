//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/Support/Jobserver.h"
#அந்தர்பாவ "llvm/ADT/StringExtras.h"
#அந்தர்பாவ "llvm/Support/Error.h"

#அந்தர்பாவ <atomic>
#அந்தர்பாவ <memory>
#அந்தர்பாவ <mutex>
#அந்தர்பாவ <new>

#define DEBUG_TYPE "jobserver"

using நாமாகாஶ llvm;

நாமாகாஶ {
கட்டமைப்பு FdPair {
  முழு Read = -1;
  முழு Write = -1;
  தருக்க isValid() மாறிலி { திரும்பு Read >= 0 && Write >= 0; }
};

கட்டமைப்பு JobserverConfig {
  எண்ணிக்கை Mode {
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
தருக்க get_Prefixed_மதிப்பு(StringRef உள்ளீடு, StringRef Prefix, StringRef &மதிப்பு) {
  எனில் (உள்ளீடு.consume_front(Prefix)) {
    மதிப்பு = உள்ளீடு;
    திரும்பு உண்மை;
  }
  திரும்பு பொய்;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
நிலையான std::optional<FdPair> getFileDescriptorPair(StringRef உள்ளீடு) {
  FdPair FDs;
  எனில் (உள்ளீடு.consumeInteger(10, FDs.Read))
    திரும்பு std::nullopt;
  எனில் (!உள்ளீடு.consume_front(","))
    திரும்பு std::nullopt;
  எனில் (உள்ளீடு.consumeInteger(10, FDs.Write))
    திரும்பு std::nullopt;
  எனில் (!உள்ளீடு.empty() || !FDs.isValid())
    திரும்பு std::nullopt;
  திரும்பு FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  எனில் (MakeFlags.empty())
    திரும்பு Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  எனில் (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    திரும்பு Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  க்கு (StringRef Arg : Args) {
    StringRef மதிப்பு;
    எனில் (get_Prefixed_மதிப்பு(Arg, "--jobserver-auth=", மதிப்பு)) {
      // Try to parse as a file descriptor pair first.
      எனில் (auto FDPair = getFileDescriptorPair(மதிப்பு)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } இல்லையெனில் {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        எனில் (get_Prefixed_மதிப்பு(மதிப்பு, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } இல்லையெனில் {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = மதிப்பு.str();
        }
      }
    } இல்லையெனில் எனில் (get_Prefixed_மதிப்பு(Arg, "--jobserver-fds=", மதிப்பு)) {
      // This is an alternative, older syntax for the pipe-based server.
      எனில் (auto FDPair = getFileDescriptorPair(மதிப்பு)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } இல்லையெனில் {
        திரும்பு createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  எனில் (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    திரும்பு createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#இல்லையெனில்
  எனில் (Config.TheMode == JobserverConfig::Win32Semaphore)
    திரும்பு createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  திரும்பு Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

நாமாகாஶ llvm {
வகுப்பு JobserverClientImpl : பொது JobserverClient {
  தருக்க IsInitialized = பொய்;
  std::atomic<தருக்க> HasImplicitSlot{உண்மை};
  unsigned இலக்கம்_Jobs = 0;

பொது:
  JobserverClientImpl(மாறிலி JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  வெறுமை release(JobSlot Slot) override;
  unsigned get_இலக்கம்_Jobs() மாறிலி override { திரும்பு இலக்கம்_Jobs; }

  தருக்க isValid() மாறிலி { திரும்பு IsInitialized; }

தனிப்பட்ட:
#எனில் defined(LLVM_ON_UNIX)
  முழு ReadFD = -1;
  முழு WriteFD = -1;
  std::string FifoPath;
#அல்லதெனில் defined(_WIN32)
  வெறுமை *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#எனில் defined(LLVM_ON_UNIX)
#அந்தர்பாவ "Unix/Jobserver.inc"
#அல்லதெனில் defined(_WIN32)
#அந்தர்பாவ "Windows/Jobserver.inc"
#இல்லையெனில்
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(மாறிலி JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = இயல்பு;
JobSlot JobserverClientImpl::tryAcquire() { திரும்பு JobSlot(); }
வெறுமை JobserverClientImpl::release(JobSlot Slot) {}
#endif

நாமாகாஶ llvm {
JobserverClient::~JobserverClient() = இயல்பு;

uint8_t JobSlot::get_Explicit_மதிப்பு() மாறிலி {
  ப்ரதிஜ்ஞா(isExplicit() && "Cannot get value of implicit or invalid slot");
  திரும்பு static_cast<uint8_t>(மதிப்பு);
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
    மாறிலி எழுத்து *MakeFlagsEnv = getenv("MAKEFLAGS");
    எனில் (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      திரும்பு;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    எனில் (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      திரும்பு;
    }

    JobserverConfig Config = *ConfigOrErr;
    எனில் (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      திரும்பு;
    }

    எனில் (Config.TheMode == JobserverConfig::PosixPipe) {
#எனில் defined(LLVM_ON_UNIX)
      எனில் (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        திரும்பு;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    எனில் (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } இல்லையெனில்
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  திரும்பு GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
வெறுமை JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  புதிய (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
