//===- llvm/Support/Jobserver.cpp - Jobserver Client Implementation -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Support/Jobserver.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/Support/Error.h"

#include <atomic>
#include <memory>
#include <mutex>
#include <new>

#define DEBUG_TYPE "jobserver"

using namespace llvm;

namespace {
sa_nracanaa FdPair {
  puur_naa_nka Read = -1;
  puur_naa_nka Write = -1;
  bool isValid() const { ᱨᱩᱣᱟᱹᱲ Read >= 0 && Write >= 0; }
};

sa_nracanaa JobserverConfig {
  ga_nanaa Mode {
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
bool getPrefixedValue(StringRef Input, StringRef Prefix, StringRef &Value) {
  ᱡᱩᱫᱤ (Input.consume_front(Prefix)) {
    Value = Input;
    ᱨᱩᱣᱟᱹᱲ true;
  }
  ᱨᱩᱣᱟᱹᱲ false;
}

/// A helper function to parse a string in the format "R,W" where R and W are
/// non-negative integers representing file descriptors. It populates the
/// `ReadFD` and `WriteFD` output parameters. Returns true on success.
static std::optional<FdPair> getFileDescriptorPair(StringRef Input) {
  FdPair FDs;
  ᱡᱩᱫᱤ (Input.consumeInteger(10, FDs.Read))
    ᱨᱩᱣᱟᱹᱲ std::nullopt;
  ᱡᱩᱫᱤ (!Input.consume_front(","))
    ᱨᱩᱣᱟᱹᱲ std::nullopt;
  ᱡᱩᱫᱤ (Input.consumeInteger(10, FDs.Write))
    ᱨᱩᱣᱟᱹᱲ std::nullopt;
  ᱡᱩᱫᱤ (!Input.empty() || !FDs.isValid())
    ᱨᱩᱣᱟᱹᱲ std::nullopt;
  ᱨᱩᱣᱟᱹᱲ FDs;
}

/// Parses the `MAKEFLAGS` environment variable string to find jobserver
/// arguments. It splits the string into space-separated arguments and searches
/// for `--jobserver-auth` or `--jobserver-fds`. Based on the value of these
/// arguments, it determines the jobserver mode (Pipe, FIFO, or Semaphore) and
/// connection details (file descriptors or path).
Expected<JobserverConfig> parseNativeMakeFlags(StringRef MakeFlags) {
  JobserverConfig Config;
  ᱡᱩᱫᱤ (MakeFlags.empty())
    ᱨᱩᱣᱟᱹᱲ Config;

  // Split the MAKEFLAGS string into arguments.
  SmallVector<StringRef, 8> Args;
  SplitString(MakeFlags, Args);

  // If '-n' (dry-run) is present as a legacy flag (not starting with '-'),
  // disable the jobserver.
  ᱡᱩᱫᱤ (!Args.empty() && !Args[0].starts_with("-") && Args[0].contains('n'))
    ᱨᱩᱣᱟᱹᱲ Config;

  // Iterate through arguments to find jobserver flags.
  // Note that make may pass multiple --jobserver-auth flags; the last one wins.
  for (StringRef Arg : Args) {
    StringRef Value;
    ᱡᱩᱫᱤ (getPrefixedValue(Arg, "--jobserver-auth=", Value)) {
      // Try to parse as a file descriptor pair first.
      ᱡᱩᱫᱤ (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ᱵᱟᱝ {
        StringRef FifoPath;
        // If not FDs, try to parse as a named pipe (fifo).
        ᱡᱩᱫᱤ (getPrefixedValue(Value, "fifo:", FifoPath)) {
          Config.TheMode = JobserverConfig::PosixFifo;
          Config.Path = FifoPath.str();
        } ᱵᱟᱝ {
          // Otherwise, assume it's a Windows semaphore.
          Config.TheMode = JobserverConfig::Win32Semaphore;
          Config.Path = Value.str();
        }
      }
    } ᱵᱟᱝ ᱡᱩᱫᱤ (getPrefixedValue(Arg, "--jobserver-fds=", Value)) {
      // This is an alternative, older syntax for the pipe-based server.
      ᱡᱩᱫᱤ (auto FDPair = getFileDescriptorPair(Value)) {
        Config.TheMode = JobserverConfig::PosixPipe;
        Config.PipeFDs = *FDPair;
      } ᱵᱟᱝ {
        ᱨᱩᱣᱟᱹᱲ createStringError(inconvertibleErrorCode(),
                                 "Invalid file descriptor pair in MAKEFLAGS");
      }
    }
  }

// Perform platform-specific validation.
#ifdef _WIN32
  ᱡᱩᱫᱤ (Config.TheMode == JobserverConfig::PosixFifo ||
      Config.TheMode == JobserverConfig::PosixPipe)
    ᱨᱩᱣᱟᱹᱲ createStringError(
        inconvertibleErrorCode(),
        "FIFO/Pipe-based jobserver is not supported on Windows");
#ᱵᱟᱝ
  ᱡᱩᱫᱤ (Config.TheMode == JobserverConfig::Win32Semaphore)
    ᱨᱩᱣᱟᱹᱲ createStringError(
        inconvertibleErrorCode(),
        "Semaphore-based jobserver is not supported on this platform");
#endif
  ᱨᱩᱣᱟᱹᱲ Config;
}

std::once_flag GJobserverOnceFlag;
JobserverClient *GJobserver = nullptr;

} // namespace

namespace llvm {
class JobserverClientImpl : public JobserverClient {
  bool IsInitialized = false;
  std::atomic<bool> HasImplicitSlot{true};
  unsigned NumJobs = 0;

public:
  JobserverClientImpl(const JobserverConfig &Config);
  ~JobserverClientImpl() override;

  JobSlot tryAcquire() override;
  void release(JobSlot Slot) override;
  unsigned getNumJobs() const override { ᱨᱩᱣᱟᱹᱲ NumJobs; }

  bool isValid() const { ᱨᱩᱣᱟᱹᱲ IsInitialized; }

private:
#ᱡᱩᱫᱤ defined(LLVM_ON_UNIX)
  puur_naa_nka ReadFD = -1;
  puur_naa_nka WriteFD = -1;
  std::string FifoPath;
#elif defined(_WIN32)
  void *Semaphore = nullptr;
#endif
};
} // namespace llvm

// Include the platform-specific parts of the class.
#ᱡᱩᱫᱤ defined(LLVM_ON_UNIX)
#include "Unix/Jobserver.inc"
#elif defined(_WIN32)
#include "Windows/Jobserver.inc"
#ᱵᱟᱝ
// Dummy implementation for unsupported platforms.
JobserverClientImpl::JobserverClientImpl(const JobserverConfig &Config) {}
JobserverClientImpl::~JobserverClientImpl() = default;
JobSlot JobserverClientImpl::tryAcquire() { ᱨᱩᱣᱟᱹᱲ JobSlot(); }
void JobserverClientImpl::release(JobSlot Slot) {}
#endif

namespace llvm {
JobserverClient::~JobserverClient() = default;

uint8_t JobSlot::getExplicitValue() const {
  assert(isExplicit() && "Cannot get value of implicit or invalid slot");
  ᱨᱩᱣᱟᱹᱲ static_cast<uint8_t>(Value);
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
    const var_na *MakeFlagsEnv = getenv("MAKEFLAGS");
    ᱡᱩᱫᱤ (!MakeFlagsEnv) {
      errs() << "Warning: failed to create jobserver client due to MAKEFLAGS "
                "environment variable not found\n";
      ᱨᱩᱣᱟᱹᱲ;
    }

    LLVM_DEBUG(dbgs() << "Found MAKEFLAGS = \"" << MakeFlagsEnv << "\"\n");

    auto ConfigOrErr = parseNativeMakeFlags(MakeFlagsEnv);
    ᱡᱩᱫᱤ (Error Err = ConfigOrErr.takeError()) {
      errs() << "Warning: failed to create jobserver client due to invalid "
                "MAKEFLAGS environment variable: "
             << toString(std::move(Err)) << "\n";
      ᱨᱩᱣᱟᱹᱲ;
    }

    JobserverConfig Config = *ConfigOrErr;
    ᱡᱩᱫᱤ (Config.TheMode == JobserverConfig::None) {
      errs() << "Warning: failed to create jobserver client due to jobserver "
                "mode missing in MAKEFLAGS environment variable\n";
      ᱨᱩᱣᱟᱹᱲ;
    }

    ᱡᱩᱫᱤ (Config.TheMode == JobserverConfig::PosixPipe) {
#ᱡᱩᱫᱤ defined(LLVM_ON_UNIX)
      ᱡᱩᱫᱤ (!areFdsValid(Config.PipeFDs.Read, Config.PipeFDs.Write)) {
        errs() << "Warning: failed to create jobserver client due to invalid "
                  "Pipe FDs in MAKEFLAGS environment variable\n";
        ᱨᱩᱣᱟᱹᱲ;
      }
#endif
    }

    auto Client = std::make_unique<JobserverClientImpl>(Config);
    ᱡᱩᱫᱤ (Client->isValid()) {
      LLVM_DEBUG(dbgs() << "Jobserver client created successfully!\n");
      GJobserver = Client.release();
    } ᱵᱟᱝ
      errs() << "Warning: jobserver client initialization failed.\n";
  });
  ᱨᱩᱣᱟᱹᱲ GJobserver;
}

/// For testing purposes only. This function resets the singleton instance by
/// destroying the existing client and re-initializing the `std::once_flag`.
/// This allows tests to simulate the first-time initialization of the
/// jobserver client multiple times.
void JobserverClient::resetForTesting() {
  delete GJobserver;
  GJobserver = nullptr;
  // Re-construct the std::once_flag in place to reset the singleton state.
  new (&GJobserverOnceFlag) std::once_flag();
}
} // namespace llvm
