//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/Support/Jobserver.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"
#અન્તર્ભાવ "llvm/Support/Error.h"

#અન્તર્ભાવ <atomic>
#અન્તર્ભાવ <memory>
#અન્તર્ભાવ <mutex>
#અન્તર્ભાવ <new>

#define DEBUG_TYPE "jobserver"

using નામાકાશ llvm;

નામાકાશ {
સંરચના FdPair {
  પૂર્ણાંક Read = -1;
  પૂર્ણાંક Write = -1;
  તાર્કિક isValid() અચલ { પાછા Read >= 0 && Write >= 0; }
};

સંરચના JobserverConfig {
  ગણના Mode {
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
તાર્કિક get_Prefixed_કિંમત(StringRef આગત, StringRef Prefix, StringRef &કિંમત) {
  જો (આગત.consume_front(Prefix)) {
    કિંમત = આગત;
    પાછા સાચું;
  }
  પાછા ખોટું;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
સ્થિર std::optional<FdPair> getFileDescriptorPair(StringRef આગત) {
  FdPair FDs;
  જો (આગત.consumeInteger(10, FDs.Read))
    પાછા std::nullopt;
  જો (!આગત.consume_front(","))
    પાછા std::nullopt;
  જો (આગત.consumeInteger(10, FDs.Write))
    પાછા std::nullopt;
  જો (!આગત.empty() || !FDs.isValid())
    પાછા std::nullopt;
  પાછા FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  જો (MakeFlags.empty())
    પાછા Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  જો (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    પાછા Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  માટે (StringRef Arg : Args) {
    StringRef કિંમત;
    જો (get_Prefixed_કિંમત(Arg, "--jobserver-auth=", કિંમત)) {
      // Try to parse as a file descriptor pair first.
      જો (auto FDPair = getFileDescriptorPair(કિંમત)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } નહીંતર {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        જો (get_Prefixed_કિંમત(કિંમત, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } નહીંતર {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = કિંમત.str();
        }
      }
    } નહીંતર જો (get_Prefixed_કિંમત(Arg, "--jobserver-fds=", કિંમત)) {
      // This is an alternative, older syntax for the pipe-based server.
      જો (auto FDPair = getFileDescriptorPair(કિંમત)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } નહીંતર {
        પાછા createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  જો (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    પાછા createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#નહીંતર
  જો (Config.TheMode == JobserverConfig::Win32Semaphore)
    પાછા createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  પાછા Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

નામાકાશ llvm {
વર્ગ JobserverClientImpl : જાહેર JobserverClient {
  તાર્કિક IsInitialized = ખોટું;
  std::atomic<તાર્કિક> HasImplicitSlot{સાચું};
  unsigned અંક_Jobs = 0;

જાહેર:
  JobserverClientImpl(અચલ JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  શૂન્ય release(JobSlot Slot) override;
  unsigned get_અંક_Jobs() અચલ override { પાછા અંક_Jobs; }

  તાર્કિક isValid() અચલ { પાછા IsInitialized; }

ખાનગી:
#જો defined(LLVM_ON_UNIX)
  પૂર્ણાંક ReadFD = -1;
  પૂર્ણાંક WriteFD = -1;
  std::string FifoPath;
#અથવાજો defined(_WIN32)
  શૂન્ય *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#જો defined(LLVM_ON_UNIX)
#અન્તર્ભાવ "Unix/Jobserver.inc"
#અથવાજો defined(_WIN32)
#અન્તર્ભાવ "Windows/Jobserver.inc"
#નહીંતર
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(અચલ JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = મૂળભૂત;
JobSlot JobserverClientImpl::tryAcquire() { પાછા JobSlot(); }
શૂન્ય JobserverClientImpl::release(JobSlot Slot) {}
#endif

નામાકાશ llvm {
JobserverClient::~JobserverClient() = મૂળભૂત;

uint8_t JobSlot::get_Explicit_કિંમત() અચલ {
  પ્રતિજ્ઞા(isExplicit() && "Cannot get value of implicit or invalid slot");
  પાછા static_cast<uint8_t>(કિંમત);
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
    અચલ અક્ષર *MakeFlagsEnv = getenv("MAKEFLAGS");
    જો (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      પાછા;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    જો (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      પાછા;
    }

    JobserverConfig Config = *ConfigOrErr;
    જો (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      પાછા;
    }

    જો (Config.TheMode == JobserverConfig::PosixPipe) {
#જો defined(LLVM_ON_UNIX)
      જો (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        પાછા;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    જો (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } નહીંતર
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  પાછા GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
શૂન્ય JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  નવું (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
