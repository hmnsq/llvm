//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/Support/Jobserver.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Error.h"

#ଅନ୍ତର୍ଭାଵ <atomic>
#ଅନ୍ତର୍ଭାଵ <memory>
#ଅନ୍ତର୍ଭାଵ <mutex>
#ଅନ୍ତର୍ଭାଵ <new>

#define DEBUG_TYPE "jobserver"

using ନାମାକାଶ llvm;

ନାମାକାଶ {
ସଂରଚନା FdPair {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Read = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Write = -1;
  ତାର୍କିକ isValid() ଅଚଳ { ଫେରନ୍ତୁ Read >= 0 && Write >= 0; }
};

ସଂରଚନା JobserverConfig {
  ଗଣନା Mode {
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
ତାର୍କିକ get_Prefixed_ମୂଲ୍ୟ(StringRef ନିବେଶ, StringRef Prefix, StringRef &ମୂଲ୍ୟ) {
  ଯଦି (ନିବେଶ.consume_front(Prefix)) {
    ମୂଲ୍ୟ = ନିବେଶ;
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }
  ଫେରନ୍ତୁ ମିଥ୍ୟା;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
ସ୍ଥିର std::optional<FdPair> getFileDescriptorPair(StringRef ନିବେଶ) {
  FdPair FDs;
  ଯଦି (ନିବେଶ.consumeInteger(10, FDs.Read))
    ଫେରନ୍ତୁ std::nullopt;
  ଯଦି (!ନିବେଶ.consume_front(","))
    ଫେରନ୍ତୁ std::nullopt;
  ଯଦି (ନିବେଶ.consumeInteger(10, FDs.Write))
    ଫେରନ୍ତୁ std::nullopt;
  ଯଦି (!ନିବେଶ.empty() || !FDs.isValid())
    ଫେରନ୍ତୁ std::nullopt;
  ଫେରନ୍ତୁ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ଯଦି (MakeFlags.empty())
    ଫେରନ୍ତୁ Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ଯଦି (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    ଫେରନ୍ତୁ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  ପାଇଁ (StringRef Arg : Args) {
    StringRef ମୂଲ୍ୟ;
    ଯଦି (get_Prefixed_ମୂଲ୍ୟ(Arg, "--jobserver-auth=", ମୂଲ୍ୟ)) {
      // Try to parse as a file descriptor pair first.
      ଯଦି (auto FDPair = getFileDescriptorPair(ମୂଲ୍ୟ)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ଅନ୍ୟଥା {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        ଯଦି (get_Prefixed_ମୂଲ୍ୟ(ମୂଲ୍ୟ, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } ଅନ୍ୟଥା {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = ମୂଲ୍ୟ.str();
        }
      }
    } ଅନ୍ୟଥା ଯଦି (get_Prefixed_ମୂଲ୍ୟ(Arg, "--jobserver-fds=", ମୂଲ୍ୟ)) {
      // This is an alternative, older syntax for the pipe-based server.
      ଯଦି (auto FDPair = getFileDescriptorPair(ମୂଲ୍ୟ)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ଅନ୍ୟଥା {
        ଫେରନ୍ତୁ createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  ଯଦି (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    ଫେରନ୍ତୁ createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#ଅନ୍ୟଥା
  ଯଦି (Config.TheMode == JobserverConfig::Win32Semaphore)
    ଫେରନ୍ତୁ createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  ଫେରନ୍ତୁ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

ନାମାକାଶ llvm {
ଶ୍ରେଣୀ JobserverClientImpl : ସର୍ବସାଧାରଣ JobserverClient {
  ତାର୍କିକ IsInitialized = ମିଥ୍ୟା;
  std::atomic<ତାର୍କିକ> HasImplicitSlot{ସତ୍ୟ};
  unsigned ଅଙ୍କ_Jobs = 0;

ସର୍ବସାଧାରଣ:
  JobserverClientImpl(ଅଚଳ JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  ଶୂନ୍ୟ release(JobSlot Slot) override;
  unsigned get_ଅଙ୍କ_Jobs() ଅଚଳ override { ଫେରନ୍ତୁ ଅଙ୍କ_Jobs; }

  ତାର୍କିକ isValid() ଅଚଳ { ଫେରନ୍ତୁ IsInitialized; }

ବ୍ୟକ୍ତିଗତ:
#ଯଦି defined(LLVM_ON_UNIX)
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ReadFD = -1;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା WriteFD = -1;
  std::string FifoPath;
#ନଚେତଯଦି defined(_WIN32)
  ଶୂନ୍ୟ *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ଯଦି defined(LLVM_ON_UNIX)
#ଅନ୍ତର୍ଭାଵ "Unix/Jobserver.inc"
#ନଚେତଯଦି defined(_WIN32)
#ଅନ୍ତର୍ଭାଵ "Windows/Jobserver.inc"
#ଅନ୍ୟଥା
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(ଅଚଳ JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
JobSlot JobserverClientImpl::tryAcquire() { ଫେରନ୍ତୁ JobSlot(); }
ଶୂନ୍ୟ JobserverClientImpl::release(JobSlot Slot) {}
#endif

ନାମାକାଶ llvm {
JobserverClient::~JobserverClient() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

uint8_t JobSlot::get_Explicit_ମୂଲ୍ୟ() ଅଚଳ {
  ପ୍ରତିଜ୍ଞା(isExplicit() && "Cannot get value of implicit or invalid slot");
  ଫେରନ୍ତୁ static_cast<uint8_t>(ମୂଲ୍ୟ);
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
    ଅଚଳ ଅକ୍ଷର *MakeFlagsEnv = getenv("MAKEFLAGS");
    ଯଦି (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      ଫେରନ୍ତୁ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ଯଦି (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      ଫେରନ୍ତୁ;
    }

    JobserverConfig Config = *ConfigOrErr;
    ଯଦି (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      ଫେରନ୍ତୁ;
    }

    ଯଦି (Config.TheMode == JobserverConfig::PosixPipe) {
#ଯଦି defined(LLVM_ON_UNIX)
      ଯଦି (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        ଫେରନ୍ତୁ;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    ଯଦି (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } ଅନ୍ୟଥା
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  ଫେରନ୍ତୁ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
ଶୂନ୍ୟ JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  ନୂତନ (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
